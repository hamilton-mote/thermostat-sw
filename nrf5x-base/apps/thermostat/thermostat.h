#ifndef THERMOSTAT_H
#define THERMOSTAT_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// hdc1000
#define TEMP_HUMID_SENSOR 0x40

// LED actuation groups
// display setpoint
#define LED_DRIVER_SP 0x61
// displays temperature
#define LED_DRIVER_TEMP 0x63
#define LED_DRIVER_3 0x67
#define ALLCALLADR 0x68

// TLC59116 registers
#define TLC59116_MODE1 0x00
#define TLC59116_MODE2 0x01
#define TLC59116_PWM0 0x02
#define TLC59116_PWM1 0x03
#define TLC59116_PWM2 0x04
#define TLC59116_PWM3 0x05
#define TLC59116_PWM4 0x06
#define TLC59116_PWM5 0x07
#define TLC59116_PWM6 0x08
#define TLC59116_PWM7 0x09
#define TLC59116_PWM8 0x0a
#define TLC59116_PWM9 0x0b
#define TLC59116_PWM10 0x0c
#define TLC59116_PWM11 0x0d
#define TLC59116_PWM12 0x0e
#define TLC59116_PWM13 0x0f
#define TLC59116_PWM14 0x10
#define TLC59116_PWM15 0x11
#define TLC59116_GRPPWM 0x12
#define TLC59116_GRPFREQ 0x13
#define TLC59116_LEDOUT0 0x14
#define TLC59116_LEDOUT1 0x15
#define TLC59116_LEDOUT2 0x16
#define TLC59116_LEDOUT3 0x17
#define TLC59116_SUBADR1 0x18
#define TLC59116_SUBADR2 0x19
#define TLC59116_SUBADR3 0x1a
#define TLC59116_ALLCALLADR 0x1b
#define TLC59116_IREF 0x1c
#define TLC59116_EFLAG1 0x1d
#define TLC59116_EFLAG2 0x1e

/** Thermostat Constraints **/
// max/min heating set point
#define MAX_HSP 70
#define MIN_HSP 50

// max/min cooling set point
#define MAX_CSP 90
#define MIN_CSP 70

// max/min hysteresis amounts
#define MAX_HYST 2
#define MIN_HYST 0

// maximum timer interval (1 hour)
#define MAX_TIMER_HOLD 3600
#define TIMER_INTERVAL 900

typedef struct thermostat_state_t {
    uint32_t    temp_in;    // measured inside temperature
    uint32_t    temp_csp;   // cooling setpoint
    uint32_t    temp_hsp;   // heating setpoint
    uint32_t    hold_timer; // current timer value
    uint8_t     hysteresis; // hysteresis value
    bool        is_heating; // true if heating
    bool        is_cooling; // true if cooling
    bool        on;         // true if thermostat is active
} thermostat_state_t;

typedef struct thermostat_action_t {
    uint32_t    *csp_direct;     // direct setting of cooling setpoint
    uint32_t    *hsp_direct;     // direct setting of heating setpoint
    uint32_t    *timer_direct;   // direct timer setting (in seconds)
    uint32_t    *hysteresis;     // changing hysteresis value
    uint32_t    temp;           // temperature sensor reading
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
    bool        blinking;       // blink to indicate action?
    uint8_t     timer_led_num;  // number of timer LEDs to display
    uint32_t    temp_display;   // which temperature to display?
    uint32_t    hsp_display;   // display hsp
    uint32_t    csp_display;   // display csp
} thermostat_output_t;

void init_thermostat(thermostat_state_t*, thermostat_action_t*, thermostat_output_t*);

uint32_t max(uint32_t, uint32_t);
uint32_t min(uint32_t, uint32_t);

void nearest_temperature(int *temp, int *display_temp, int *led_register);
#endif
