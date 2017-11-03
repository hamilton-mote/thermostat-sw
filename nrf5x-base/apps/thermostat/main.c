/*
 * Send an advertisement periodically
 */

#include <stdbool.h>
#include <stdint.h>
//#include "sdk_config.h"
//#include "led.h"

#include "board.h"
#include "nrf.h"
#include "nrf_sdm.h"
#include "nrf_gpio.h"
#include "nrf_soc.h"
#include "nrf_drv_config.h"
#include "nrf_drv_gpiote.h"
#include "nrf_error.h"
#include "nrf_assert.h"
#include "nrf_drv_twi.h"

#include "app_timer.h"
#include "softdevice_handler.h"
#include "thermostat.h"
#include "tlc59116.h"
#include "hdc1000.h"

#define LED 19
// Some constants about timers
#define BLINK_TIMER_PRESCALER              0  // Value of the RTC1 PRESCALER register.
#define BLINK_TIMER_OP_QUEUE_SIZE          4  // Size of timer operation queues.

// How long before the timer fires.
#define BLINK_RATE     APP_TIMER_TICKS(1000, BLINK_TIMER_PRESCALER) // Blink every 1 seconds

// configure TWI
nrf_drv_twi_t twi_instance = NRF_DRV_TWI_INSTANCE(1);
hdc1000_cfg_t sensor_cfg = {
    .address = TEMP_HUMID_SENSOR
};
tlc59116_cfg_t tempdriver_cfg = {
    .address = LED_DRIVER_TEMP,
    .initial_value = 0x0,
};
tlc59116_cfg_t spdriver_cfg = {
    .address = LED_DRIVER_SP,
    .initial_value = 0x0,
};
tlc59116_cfg_t leddriver3_cfg = {
    .address = LED_DRIVER_3,
    .initial_value = 0x0,
};

int a = 0xaa;

// Timer data structure
APP_TIMER_DEF(blink_timer);
// Timer callback
static void timer_handler (void* p_context) {
    //led_toggle(LED);
    int16_t hum;
    int16_t temp;
    hdc1000_read(&sensor_cfg, &temp, &hum);

    int display_temp, led_register_temp;
    int t = (int)temp;
    if (t == 0) 
        tlc59116_set_led(&tempdriver_cfg, TLC59116_PWM15, 0xff);    
    else if (t < 0) 
        tlc59116_set_led(&tempdriver_cfg, TLC59116_PWM14, 0xff);    
    else if (t < 10)
        tlc59116_set_led(&tempdriver_cfg, TLC59116_PWM1, 0xff);    
    else if (t < 100)
        tlc59116_set_led(&tempdriver_cfg, TLC59116_PWM2, 0xff);    
    else if (t < 1000)
        tlc59116_set_led(&tempdriver_cfg, TLC59116_PWM3, 0xff);    
    else
        tlc59116_set_led(&tempdriver_cfg, TLC59116_PWM4, 0xff);    
    nearest_temperature(&t, &display_temp, &led_register_temp);
    tlc59116_set_led(&tempdriver_cfg, led_register_temp, 0xff);    
}

// Setup timer
static void timer_init(void)
{
    uint32_t err_code;

    // Initialize timer module.
    APP_TIMER_INIT(BLINK_TIMER_PRESCALER,
                   BLINK_TIMER_OP_QUEUE_SIZE,
                   false);

    // Create a timer
    err_code = app_timer_create(&blink_timer,
                                APP_TIMER_MODE_REPEATED,
                                timer_handler);
    APP_ERROR_CHECK(err_code);
}

// Start the blink timer
static void timer_start(void) {
    uint32_t err_code;

    // Start application timers.
    err_code = app_timer_start(blink_timer, BLINK_RATE, NULL);
    APP_ERROR_CHECK(err_code);
}


static void i2c_init(void) {
    nrf_drv_twi_config_t twi_config;

    // Initialize the I2C module
    twi_config.sda                = I2C_SDA_PIN;
    twi_config.scl                = I2C_SCL_PIN;
    twi_config.frequency          = NRF_TWI_FREQ_400K;
    twi_config.interrupt_priority = APP_IRQ_PRIORITY_HIGH;
    nrf_drv_twi_init(&twi_instance, &twi_config, NULL, NULL);
    nrf_drv_twi_enable(&twi_instance);
}

int main(void) {

    // Need to set the clock to something
    nrf_clock_lf_cfg_t clock_lf_cfg = {
        .source        = NRF_CLOCK_LF_SRC_RC,
        .rc_ctiv       = 16,
        .rc_temp_ctiv  = 2,
        .xtal_accuracy = NRF_CLOCK_LF_XTAL_ACCURACY_250_PPM};

    // Initialize the SoftDevice handler module.
    SOFTDEVICE_HANDLER_INIT(&clock_lf_cfg, NULL);

    timer_init();
    timer_start();

    i2c_init();
    tlc59116_init(&tempdriver_cfg, &twi_instance);
    tlc59116_init(&spdriver_cfg, &twi_instance);
    tlc59116_init(&leddriver3_cfg, &twi_instance);
    tlc59116_set_led(&tempdriver_cfg, TLC59116_PWM0, 0xff);    

    //hdc1000_init(&sensor_cfg, &twi_instance);

    // Enter main loop.
    while (1) {
        sd_app_evt_wait();
    }
    return 0;
}
