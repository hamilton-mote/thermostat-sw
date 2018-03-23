#ifndef THERMOSTAT_H
#define THERMOSTAT_H

#include "tlc59116.h"
#include "schedule.h"
#include "mcp7940n.h"
#include "pca9557.h"
#include "hdc1000.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// hdc1000
#define TEMP_HUMID_SENSOR 0x40

// GPIO pins
#define D18    18
#define D19    19
#define D24    24
#define D25    25
#define SP_DEC    18
#define SP_INC    19
#define POWER_BUTTON    24
#define TIMER_BUTTON    25
#define PIR    6

// LED actuation groups
// display setpoint
#define LED_DRIVER_SP 0x63
// displays temperature
#define LED_DRIVER_TEMP 0x61
#define LED_DRIVER_3 0x67
#define ALLCALLADR 0x68

#define RELAY_ADDR 24

/** Thermostat Constraints **/
// max/min heating set point
#define MAX_HSP 800
#define MIN_HSP 540

// max/min cooling set point
#define MAX_CSP 880
#define MIN_CSP 620

// max/min hysteresis amounts
#define MAX_HYST 20
#define MIN_HYST 0

// maximum timer amount (1 hour)
#define MAX_TIMER_HOLD 3600
// maximum timer interval (15 min)
#define TIMER_INTERVAL 900

typedef enum TSTAT_MODE { AUTO, HEAT, COOL, OFF } MODE;

typedef struct thermostat_t {
    nrf_drv_twi_t   *twi_instance;
    hdc1000_cfg_t   sensor_cfg;
    tlc59116_cfg_t  tempdisplay_cfg;
    tlc59116_cfg_t  spdisplay_cfg;
    tlc59116_cfg_t  leddriver_cfg;
    pca9557_cfg_t   relay_cfg;
    mcp7940n_cfg_t  rtcc_cfg;
    schedule_cfg_t  schedule;
} thermostat_t;

typedef struct thermostat_state_t {
    // timer values for safety
    uint32_t    heat_on_time;
    uint32_t    heat_off_time;
    uint32_t    cool_on_time;
    uint32_t    cool_off_time;
    uint32_t    fan_on_time;
    uint32_t    fan_off_time;

    uint16_t    temp_in;    // measured inside temperature
    uint16_t    temp_csp;   // cooling setpoint
    uint16_t    temp_hsp;   // heating setpoint
    uint8_t     hysteresis; // hysteresis value
    MODE        mode;       // current mode
    uint16_t    hold_timer; // current timer value
    bool        is_heating; // true if heating
    bool        is_cooling; // true if cooling
    bool        is_fan_on;  // true if fan is on
    bool        on;         // true if thermostat is active
} thermostat_state_t;

typedef struct thermostat_action_t {
    uint32_t    *timer_direct;   // direct timer setting (in seconds)
    uint16_t    *csp_direct;     // direct setting of cooling setpoint
    uint16_t    *hsp_direct;     // direct setting of heating setpoint
    uint8_t     *hysteresis;     // changing hysteresis value
    MODE        *mode;          // changing thermostat mode
    uint16_t    temp;           // temperature sensor reading
    bool        hold_timer;     // hold timer button press
    bool        onoff;          // power button press
    bool        inc_sp;         // increment setpoint button press
    bool        dec_sp;         // decrement setpoint button press
} thermostat_action_t;

typedef struct thermostat_output_t {
    bool        heat_stage_1;   // true if enable heating stage 1
    bool        heat_stage_2;   // true if enable heating stage 2
    bool        cool_stage_1;   // true if enable cooling stage 1
    bool        cool_stage_2;   // true if enable cooling stage 2
    bool        fan_on;         // true if enable fan
    bool        tstat_on;       // true if tstat is on
    uint8_t     timer_led_num;  // number of timer LEDs to display
    uint16_t    temp_display;   // which temperature to display?
    uint16_t    hsp_display;    // display hsp
    uint16_t    csp_display;    // display csp
} thermostat_output_t;

typedef struct _thermostat_report_t {
    uint16_t    temp_in;
    uint16_t    temp_hsp;
    uint16_t    temp_csp;
    uint8_t     state;
    uint8_t     mode;
    uint16_t    hold_timer;
} _thermostat_report_t;

typedef union thermostat_report_t {
    uint8_t bytes[10];
    _thermostat_report_t report;
} thermostat_report_t;

void init_thermostat(thermostat_t*, thermostat_state_t*, thermostat_action_t*, thermostat_output_t*);
void transition(thermostat_t*, thermostat_state_t*, thermostat_action_t*);
void state_to_output(thermostat_t*, thermostat_state_t*, thermostat_output_t*);
void enact_output(thermostat_t*, thermostat_state_t*, thermostat_output_t*);

uint32_t max(uint32_t, uint32_t);
uint32_t min(uint32_t, uint32_t);

void nearest_temperature(uint16_t *temp, uint16_t *display_temp, int *led_register);
void update_timer_press(thermostat_state_t*);
void timer_led_settings(thermostat_state_t*, int *led0, int *led1, int *led2, int *led3, int *num);

#endif
