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
#include "tlc59116.h"
#include "pca9557.h"
#include "hdc1000.h"
#include "thermostat.h"

#define LED 19
// Some constants about timers
#define BLINK_TIMER_PRESCALER              0  // Value of the RTC1 PRESCALER register.
#define BLINK_TIMER_OP_QUEUE_SIZE          4  // Size of timer operation queues.

// How long before the timer fires.
#define BLINK_RATE     APP_TIMER_TICKS(2000, BLINK_TIMER_PRESCALER) // Blink every 5 seconds

// configure TWI
nrf_drv_twi_t twi_instance = NRF_DRV_TWI_INSTANCE(1);
pca9557_cfg_t relay_cfg = {
    .address = RELAY_ADDR,
};

/*
 * Thermostat state!
 */
thermostat_t THERMOSTAT;
thermostat_state_t THERMOSTAT_STATE;
thermostat_action_t THERMOSTAT_ACTION;
thermostat_output_t THERMOSTAT_OUTPUT;


int a = 0xaa;

// Timer data structure
APP_TIMER_DEF(blink_timer);
// Timer callback
static void timer_handler (void* p_context) {
    //led_toggle(LED);
    int16_t hum;
    int16_t temp;
    hdc1000_read(&THERMOSTAT.sensor_cfg, &temp, &hum);
    temp = (1.8*temp+3200) / 100;

    uint16_t display_temp;
    int led_register_temp;

    // turn off old LED
    nearest_temperature(&(THERMOSTAT_OUTPUT.temp_display), &display_temp, &led_register_temp);
    tlc59116_set_led(&THERMOSTAT.tempdisplay_cfg, led_register_temp, 0x0);

    // turn on new LED
    THERMOSTAT_OUTPUT.temp_display = (int)temp;
    nearest_temperature(&(THERMOSTAT_OUTPUT.temp_display), &display_temp, &led_register_temp);
    tlc59116_set_led(&THERMOSTAT.tempdisplay_cfg, led_register_temp, 0xff);
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

void draw_setpoints() {
    int led_register_temp;
    tlc59116_set_all(&THERMOSTAT.spdisplay_cfg, 0x0);
    nearest_temperature(&(THERMOSTAT_STATE.temp_csp), &(THERMOSTAT_OUTPUT.csp_display), &led_register_temp);
    tlc59116_set_led(&THERMOSTAT.spdisplay_cfg, led_register_temp, 0xff);

    nearest_temperature(&(THERMOSTAT_STATE.temp_hsp), &(THERMOSTAT_OUTPUT.hsp_display), &led_register_temp);
    tlc59116_set_led(&THERMOSTAT.spdisplay_cfg, led_register_temp, 0xff);
}

void draw_timer() {
    int led0, led1, led2, led3;
    int num;
    timer_led_settings(&THERMOSTAT_STATE, &led0, &led1, &led2, &led3, &num);
    tlc59116_set_led(&THERMOSTAT.leddriver_cfg, led0, num >= 1 ? 0x0f : 0x0);
    tlc59116_set_led(&THERMOSTAT.leddriver_cfg, led1, num >= 2 ? 0x0f : 0x0);
    tlc59116_set_led(&THERMOSTAT.leddriver_cfg, led2, num >= 3 ? 0x0f : 0x0);
    tlc59116_set_led(&THERMOSTAT.leddriver_cfg, led3, num == 4 ? 0x0f : 0x0);
}

void button_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {

    // turn off old LED
    if (pin == SP_DEC) {
        THERMOSTAT_STATE.temp_csp -= 2;
        THERMOSTAT_STATE.temp_hsp -= 2;
        draw_setpoints();
    } else if (pin == SP_INC) {
        THERMOSTAT_STATE.temp_csp += 2;
        THERMOSTAT_STATE.temp_hsp += 2;
        draw_setpoints();
    } else if (pin == TIMER_BUTTON) {
        update_timer_press(&THERMOSTAT_STATE);
        draw_timer();
    }

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

    // initialize i2c bus
    i2c_init();

    // configure thermostat
    THERMOSTAT.twi_instance = &twi_instance;
    THERMOSTAT.sensor_cfg.address = TEMP_HUMID_SENSOR;

    THERMOSTAT.tempdisplay_cfg.address = LED_DRIVER_TEMP;
    THERMOSTAT.tempdisplay_cfg.initial_value = 0x0;

    THERMOSTAT.spdisplay_cfg.address = LED_DRIVER_SP;
    THERMOSTAT.spdisplay_cfg.initial_value = 0x0;

    THERMOSTAT.leddriver_cfg.address = LED_DRIVER_3;
    THERMOSTAT.leddriver_cfg.initial_value = 0x0;

    THERMOSTAT.relay_cfg.address = RELAY_ADDR;

    /*
     * Initialize thermsotat state
     */
    init_thermostat(&THERMOSTAT, &THERMOSTAT_STATE, &THERMOSTAT_ACTION, &THERMOSTAT_OUTPUT);
    draw_setpoints();

    // Initialize buttons
    ret_code_t err_code;
    err_code = nrf_drv_gpiote_init();
    if (err_code > 0) {
        tlc59116_set_all(&THERMOSTAT.tempdisplay_cfg, 0x0a);
        return 1;
    }
    nrf_drv_gpiote_in_config_t dec_in_config = GPIOTE_CONFIG_IN_SENSE_HITOLO(true);
    dec_in_config.pull = NRF_GPIO_PIN_PULLUP;
    err_code = nrf_drv_gpiote_in_init(SP_DEC, &dec_in_config, button_handler);
    if (err_code > 0) {
        tlc59116_set_all(&THERMOSTAT.tempdisplay_cfg, 0x0a);
        return 1;
    }
    nrf_drv_gpiote_in_event_enable(SP_DEC, true);

    nrf_drv_gpiote_in_config_t inc_in_config = GPIOTE_CONFIG_IN_SENSE_HITOLO(true);
    inc_in_config.pull = NRF_GPIO_PIN_PULLUP;
    err_code = nrf_drv_gpiote_in_init(SP_INC, &inc_in_config, button_handler);
    if (err_code > 0) {
        tlc59116_set_all(&THERMOSTAT.tempdisplay_cfg, 0x0a);
        return 1;
    }
    nrf_drv_gpiote_in_event_enable(SP_INC, true);

    nrf_drv_gpiote_in_config_t timer_config = GPIOTE_CONFIG_IN_SENSE_HITOLO(true);
    timer_config.pull = NRF_GPIO_PIN_PULLUP;
    err_code = nrf_drv_gpiote_in_init(TIMER_BUTTON, &timer_config, button_handler);
    if (err_code == NRF_ERROR_NO_MEM) {
        tlc59116_set_all(&THERMOSTAT.tempdisplay_cfg, 0x0a);
        return 1;
    }
    nrf_drv_gpiote_in_event_enable(TIMER_BUTTON, true);


    // Enter main loop.
    while (1) {
        transition(&THERMOSTAT, &THERMOSTAT_STATE, &THERMOSTAT_ACTION, 1000000);
        state_to_output(&THERMOSTAT, &THERMOSTAT_STATE, &THERMOSTAT_OUTPUT);
        sd_app_evt_wait();
    }
    return 0;
}
