
#include <stdbool.h>
#include <stdint.h>
//#include "sdk_config.h"
//#include "led.h"
#include "debug.h"
#include "app_timer.h"

#include "board.h"
#include "nrf.h"
#include "nrf_sdm.h"
#include "nrf_gpio.h"
#include "nrf_soc.h"
//#include "nrf_drv_config.h"
#include "nrf_drv_gpiote.h"
#include "nrf_error.h"
#include "nrf_assert.h"
#include "nrf_drv_twi.h"

#include "tlc59116.h"
#include "pca9557.h"
#include "hdc1000.h"
#include "thermostat.h"

#include "nordic_common.h"
#include "softdevice_handler.h"
#ifdef BLE
    #include "ble_advdata.h"
    #include "ble_debug_assert_handler.h"
    //#include "app_util_platform.h"
    #include "simple_ble.h"
    #include "simple_adv.h"
#endif

// Some constants about timers
#define BLINK_TIMER_PRESCALER              0  // Value of the RTC1 PRESCALER register.
#define BLINK_TIMER_OP_QUEUE_SIZE          4  // Size of timer operation queues.

// How long before the timer fires.
#define BLINK_RATE     APP_TIMER_TICKS(5000, BLINK_TIMER_PRESCALER) // Blink every 5 seconds

// configure TWI
nrf_drv_twi_t twi_instance = NRF_DRV_TWI_INSTANCE(0);

/*
 * Thermostat state!
 */
thermostat_t THERMOSTAT;
thermostat_state_t THERMOSTAT_STATE;
thermostat_action_t THERMOSTAT_ACTION;
thermostat_output_t THERMOSTAT_OUTPUT;

/*
 * BLE adverts
 */
#ifdef BLE
static simple_ble_config_t ble_config = {
    .platform_id       = 0x80,              // used as 4th octect in device BLE address
    .device_id         = 0x8080,
    .adv_name          = "thermostat!",       // used in advertisements if there is room
    .adv_interval      = MSEC_TO_UNITS(500, UNIT_0_625_MS),
    .min_conn_interval = MSEC_TO_UNITS(500, UNIT_1_25_MS),
    .max_conn_interval = MSEC_TO_UNITS(1000, UNIT_1_25_MS)
};

static simple_ble_service_t tstat_report_service = {
    .uuid128 = {{0x87, 0xa4, 0xde, 0xa0, 0x96, 0xea, 0x4e, 0xe6,
                 0x87, 0x45, 0x83, 0x28, 0x89, 0x0f, 0xad, 0x7b}}
};
static simple_ble_char_t internal_status_char = {.uuid16 = 0x8910};
static simple_ble_char_t tstat_status_char = {.uuid16 = 0x8911};

/*
 * internal_status:
 * [0] = heating?
 * [1] = cooling?
 * [2] = can_heat_on?
 * [3] = can_heat_off?
 * [4] = can_cool_on?
 * [5] = can_cool_off?
 */
static uint8_t internal_status[8] = {1,2,3,4,0,0,0,0};

static thermostat_report_t report;

void services_init(void) {
    simple_ble_add_service(&tstat_report_service);

    simple_ble_add_characteristic(1, 0, 1, 0, // read, write, notify, vlen
            8, (uint8_t*)&internal_status,
            &tstat_report_service, &internal_status_char);

    simple_ble_add_characteristic(1, 0, 1, 0, // read, write, notify, vlen
            8, (uint8_t*)&report.bytes,
            &tstat_report_service, &tstat_status_char);
}


//void ble_evt_write(ble_evt_t* p_ble_evt) {
//
//    if (simple_ble_is_char_event(p_ble_evt, &internal_status_char)) {
//    }
//}

#endif



// Timer data structure
APP_TIMER_DEF(blink_timer);

// Timer callback
static void timer_handler (void* p_context) {
    int16_t hum;
    int16_t temp;
    hdc1000_read(&THERMOSTAT.sensor_cfg, &temp, &hum);
    if (temp < 0) {
        return;
    }
    temp = (1.8*temp+3200) / 10;
    //PRINT("\033[0;35mtemp: %d\033[0m\n", temp);

    uint16_t display_temp;
    int led_register_temp;

    // turn off old LED
    nearest_temperature(&(THERMOSTAT_ACTION.temp), &display_temp, &led_register_temp);
    tlc59116_set_led(&THERMOSTAT.tempdisplay_cfg, led_register_temp, 0x0);

    // turn on new LED
    THERMOSTAT_ACTION.temp = (uint16_t)temp;
    nearest_temperature(&(THERMOSTAT_ACTION.temp), &display_temp, &led_register_temp);
    tlc59116_set_led(&THERMOSTAT.tempdisplay_cfg, led_register_temp, 0x0f);

#ifdef BLE
    internal_status[0] = THERMOSTAT_STATE.is_heating;
    internal_status[1] = THERMOSTAT_STATE.is_cooling;
    internal_status[2] = THERMOSTAT_STATE.is_fan_on;
    internal_status[3] = THERMOSTAT_STATE.heat_on_time < 300;
    internal_status[4] = THERMOSTAT_STATE.heat_off_time < 300;
    internal_status[5] = THERMOSTAT_STATE.cool_on_time < 300;
    internal_status[6] = THERMOSTAT_STATE.cool_off_time < 300;
    simple_ble_notify_char(&internal_status_char);

    report.report.temp_in = THERMOSTAT_STATE.temp_in;
    report.report.temp_hsp = THERMOSTAT_STATE.temp_hsp;
    report.report.temp_csp = THERMOSTAT_STATE.temp_csp;
    report.report.hold_timer = THERMOSTAT_STATE.hold_timer;
    if (THERMOSTAT_STATE.is_heating) {
        report.report.state |= 0x08;
    } else {
        report.report.state &= 0xf7;
    }

    if (THERMOSTAT_STATE.is_cooling) {
        report.report.state |= 0x04;
    } else {
        report.report.state &= 0xfb;
    }

    if (THERMOSTAT_STATE.is_fan_on) {
        report.report.state |= 0x02;
    } else {
        report.report.state &= 0xfd;
    }

    if (THERMOSTAT_STATE.on) {
        report.report.state |= 0x01;
    } else {
        report.report.state &= 0xfe;
    }
    simple_ble_notify_char(&tstat_status_char);
#endif

}
//
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

void button_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {

    // turn off old LED
    if (pin == SP_DEC) {
        THERMOSTAT_ACTION.dec_sp = true;
        THERMOSTAT_ACTION.inc_sp = false;
    } else if (pin == SP_INC) {
        THERMOSTAT_ACTION.dec_sp = false;
        THERMOSTAT_ACTION.inc_sp = true;
    } else if (pin == TIMER_BUTTON) {
        THERMOSTAT_ACTION.hold_timer = true;
    } else if (pin == POWER_BUTTON) {
        THERMOSTAT_ACTION.onoff = true;
    }
    transition(&THERMOSTAT, &THERMOSTAT_STATE, &THERMOSTAT_ACTION);
    state_to_output(&THERMOSTAT, &THERMOSTAT_STATE, &THERMOSTAT_OUTPUT);
    enact_output(&THERMOSTAT, &THERMOSTAT_STATE, &THERMOSTAT_OUTPUT);
}



void ble_error(uint32_t error_code) {
    while(1);
}

int main(void) {
    uint32_t err_code;

#ifdef USERTT
    NRF_LOG_INIT(NULL);
    PRINT("Debug to RTT\n");
#endif

#ifndef BLE
    PRINT("Start softdevice from app\n");
    // Need to set the clock to something
    nrf_clock_lf_cfg_t clock_lf_cfg = {
        .source        = NRF_CLOCK_LF_SRC_RC,
        .rc_ctiv       = 16,
        .rc_temp_ctiv  = 2,
        .xtal_accuracy = NRF_CLOCK_LF_XTAL_ACCURACY_250_PPM};

    // Initialize the SoftDevice handler module.
    SOFTDEVICE_HANDLER_INIT(&clock_lf_cfg, NULL);
#else
    PRINT("Defer SD to BLE\n");
    // Setup BLE
    simple_ble_init(&ble_config);
    simple_adv_only_name();
    //services_init();
#endif

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

    THERMOSTAT.rtcc_cfg.address = MCP7940N_ADDR;

    /* Built-in schedule definition */
    modality_t day = {.label="DAYTIME", .hsp=70, .csp=74, .occupied=true};
    modality_t night = {.label="NIGHT", .hsp=50, .csp=84, .occupied=false};
    // storage of all defined modalities
    THERMOSTAT.schedule.modalities[0] = day;
    THERMOSTAT.schedule.modalities[1] = night;

    daysched_t weekday;
    for (int i=0;i<8;i++) weekday.modalities[i] = 1; // night
    for (int i=8;i<18;i++) weekday.modalities[i] = 0; // day
    for (int i=18;i<24;i++) weekday.modalities[i] = 1;
    daysched_t weekend;
    for (int i=0;i<24;i++) weekend.modalities[i] = 1;

    THERMOSTAT.schedule.days[0] = weekend;
    THERMOSTAT.schedule.days[1] = weekday;
    THERMOSTAT.schedule.days[2] = weekday;
    THERMOSTAT.schedule.days[3] = weekday;
    THERMOSTAT.schedule.days[4] = weekday;
    THERMOSTAT.schedule.days[5] = weekday;
    THERMOSTAT.schedule.days[6] = weekend;

    /*
     * Initialize thermsotat state
     */
    init_thermostat(&THERMOSTAT, &THERMOSTAT_STATE, &THERMOSTAT_ACTION, &THERMOSTAT_OUTPUT);
    //draw_setpoints();

    // Initialize buttons
    err_code = nrf_drv_gpiote_init();
    if (err_code > 0) {
        tlc59116_set_all(&THERMOSTAT.tempdisplay_cfg, 0x0a);
        return 1;
    }

    // Setpoint decrement button
    nrf_drv_gpiote_in_config_t dec_in_config = GPIOTE_CONFIG_IN_SENSE_HITOLO(true);
    dec_in_config.pull = NRF_GPIO_PIN_PULLUP;
    err_code = nrf_drv_gpiote_in_init(SP_DEC, &dec_in_config, button_handler);
    if (err_code > 0) {
        tlc59116_set_all(&THERMOSTAT.tempdisplay_cfg, 0x0a);
        return 1;
    }
    nrf_drv_gpiote_in_event_enable(SP_DEC, true);

    // Setpoint increment button
    nrf_drv_gpiote_in_config_t inc_in_config = GPIOTE_CONFIG_IN_SENSE_HITOLO(true);
    inc_in_config.pull = NRF_GPIO_PIN_PULLUP;
    err_code = nrf_drv_gpiote_in_init(SP_INC, &inc_in_config, button_handler);
    if (err_code > 0) {
        tlc59116_set_all(&THERMOSTAT.tempdisplay_cfg, 0x0a);
        return 1;
    }
    nrf_drv_gpiote_in_event_enable(SP_INC, true);

    // timer button
    nrf_drv_gpiote_in_config_t timer_config = GPIOTE_CONFIG_IN_SENSE_HITOLO(true);
    timer_config.pull = NRF_GPIO_PIN_PULLUP;
    err_code = nrf_drv_gpiote_in_init(TIMER_BUTTON, &timer_config, button_handler);
    if (err_code == NRF_ERROR_NO_MEM) {
        tlc59116_set_all(&THERMOSTAT.tempdisplay_cfg, 0x0a);
        return 1;
    }
    nrf_drv_gpiote_in_event_enable(TIMER_BUTTON, true);

    // power button
    nrf_drv_gpiote_in_config_t power_config = GPIOTE_CONFIG_IN_SENSE_HITOLO(true);
    power_config.pull = NRF_GPIO_PIN_PULLUP;
    err_code = nrf_drv_gpiote_in_init(POWER_BUTTON, &power_config, button_handler);
    if (err_code == NRF_ERROR_NO_MEM) {
        tlc59116_set_all(&THERMOSTAT.tempdisplay_cfg, 0x0a);
        return 1;
    }
    nrf_drv_gpiote_in_event_enable(POWER_BUTTON, true);

    rtcc_time_t time;


    transition(&THERMOSTAT, &THERMOSTAT_STATE, &THERMOSTAT_ACTION);
    state_to_output(&THERMOSTAT, &THERMOSTAT_STATE, &THERMOSTAT_OUTPUT);
    enact_output(&THERMOSTAT, &THERMOSTAT_STATE, &THERMOSTAT_OUTPUT);


    PRINT("Temp: %u, CSP: %d HSP: %d\n", THERMOSTAT_STATE.temp_in, THERMOSTAT_STATE.temp_csp, THERMOSTAT_STATE.temp_hsp);
    // Advertise because why not
    // Enter main loop.
    while (1) {
        sd_app_evt_wait();
        transition(&THERMOSTAT, &THERMOSTAT_STATE, &THERMOSTAT_ACTION);
        state_to_output(&THERMOSTAT, &THERMOSTAT_STATE, &THERMOSTAT_OUTPUT);
        enact_output(&THERMOSTAT, &THERMOSTAT_STATE, &THERMOSTAT_OUTPUT);

        mcp7940n_readdate(&(THERMOSTAT.rtcc_cfg), &time);
        PRINT("Date: %u-%u-%u %u:%u:%u\n", time.tm_year, time.tm_mon, time.tm_mday, time.tm_hour, time.tm_min, time.tm_sec);
        PRINT("Temp: %d, CSP: %d HSP: %d\n", THERMOSTAT_STATE.temp_in, THERMOSTAT_STATE.temp_csp, THERMOSTAT_STATE.temp_hsp);

    }
    return 0;
}
