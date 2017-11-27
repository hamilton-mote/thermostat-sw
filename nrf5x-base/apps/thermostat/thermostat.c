#include "thermostat.h"

// marker values for the timer
static rtcc_time_t last_updated_time;
static uint32_t last_updated;
static const uint32_t ON_OFF_THRESHOLD = 300; // 5 minutes
static const uint32_t FREE_COOLING_TIME = 300;

int temperature_led_mapping[16][2] = {
    {58, TLC59116_PWM0},
    {61, TLC59116_PWM1},
    {63, TLC59116_PWM2},
    {65, TLC59116_PWM3},
    {67, TLC59116_PWM4},
    {69, TLC59116_PWM5},
    {71, TLC59116_PWM6},
    {73, TLC59116_PWM7},
    {75, TLC59116_PWM8},
    {77, TLC59116_PWM9},
    {79, TLC59116_PWM10},
    {81, TLC59116_PWM11},
    {83, TLC59116_PWM12},
    {85, TLC59116_PWM13},
    {88, TLC59116_PWM14},
    {92, TLC59116_PWM15},
};

int timer_led_mapping[4][2] = {
    {(1*TIMER_INTERVAL), TLC59116_PWM0},
    {(2*TIMER_INTERVAL), TLC59116_PWM3},
    {(3*TIMER_INTERVAL), TLC59116_PWM2},
    {(4*TIMER_INTERVAL), TLC59116_PWM1},
};

void clean_action(thermostat_action_t *action) {
    action->csp_direct = NULL;
    action->hsp_direct = NULL;
    action->timer_direct = NULL;
    action->hysteresis = NULL;

    action->hold_timer = false;
    action->onoff = false;
    action->inc_sp = false;
    action->dec_sp = false;
}

void init_thermostat(thermostat_t *tstat, thermostat_state_t* state, thermostat_action_t* action, thermostat_output_t* output) {

    // i2c port expander for relays
    pca9557_init(&tstat->relay_cfg, tstat->twi_instance);

    // turn everything off
    heat_off(&tstat->relay_cfg);
    cool_off(&tstat->relay_cfg);
    fan_off(&tstat->relay_cfg);

    // initialize rtc
    mcp7940n_init(&tstat->rtcc_cfg, tstat->twi_instance);

    // initialize LED drivers
    tlc59116_init(&tstat->tempdisplay_cfg, tstat->twi_instance);
    tlc59116_init(&tstat->spdisplay_cfg, tstat->twi_instance);
    tlc59116_init(&tstat->leddriver_cfg, tstat->twi_instance);

    // initialize tmemp/humidity sensor
    hdc1000_init(&tstat->sensor_cfg, tstat->twi_instance);

    state->hysteresis = 1.0;
    state->on = true;
    state->is_heating = false;
    state->is_cooling = false;
    state->temp_csp = 78;
    state->temp_hsp = 72;
    state->heat_on_time = 0;
    state->heat_off_time = 0;
    state->cool_on_time = 0;
    state->cool_off_time = 0;
    state->fan_on_time = 0;
    state->fan_off_time = 0;

    mcp7940n_readdate(&tstat->rtcc_cfg, &last_updated_time);
    last_updated = date_to_binary(&last_updated_time);

    clean_action(action);
}

// TODO: need a method to get an action vector
void transition(thermostat_t *tstat, thermostat_state_t* state, thermostat_action_t* action, uint32_t interval) {
    rtcc_time_t transition_time;
    mcp7940n_readdate(&tstat->rtcc_cfg, &transition_time);
    uint32_t elapsed = date_to_binary(&transition_time) - last_updated;

    // copy the state of the onoff button
    if (action->onoff) {
        state->on = !state->on; // toggle power
    }


    // copy over new temperature value
    state->temp_in = action->temp;

    // handle hysteresis re-configuration
    if (action->hysteresis != NULL) {
        state->hysteresis = max(min(*(action->hysteresis), MAX_HYST), MIN_HYST);
    }

    // handle inc/dec setpoint through buttons
    if (action->inc_sp) {
        state->temp_hsp = min(state->temp_hsp+2, MAX_HSP);
        state->temp_csp = min(state->temp_csp+2, MAX_CSP);
    } else if (action->dec_sp) {
        state->temp_hsp = max(state->temp_hsp-2, MIN_HSP);
        state->temp_csp = max(state->temp_csp-2, MIN_CSP);
    }

    // handle setting hsp/csp directly next
    if (action->hsp_direct != NULL) {
        state->temp_hsp = max(min(*(action->hsp_direct), MAX_HSP), MIN_HSP);
    }
    if (action->csp_direct != NULL) {
        state->temp_csp = max(min(*(action->csp_direct), MAX_CSP), MIN_CSP);
    }

    if (state->hold_timer > 0) {
        state->hold_timer -= elapsed;
    }

    // increase timer if button was pressed
    if (action->hold_timer) {
        if (state->hold_timer == MAX_TIMER_HOLD) state->hold_timer = 0;
        else state->hold_timer = min(state->hold_timer+TIMER_INTERVAL, MAX_TIMER_HOLD);
    }

    // handle direct timer setting
    if (action->timer_direct != NULL) {
        state->hold_timer = min(*(action->timer_direct), MAX_TIMER_HOLD);
    }

    // increase times
    if (state->is_heating) {
        state->heat_on_time += elapsed;
    } else {
        state->heat_off_time += elapsed;
    }

    if (state->is_cooling) {
        state->cool_on_time += elapsed;
    } else {
        state->cool_off_time += elapsed;
    }

    if (state->is_fan_on) {
        state->fan_on_time += elapsed;
    } else {
        state->fan_off_time += elapsed;
    }


    // check if inactive/active times are safe for turning on heat/cool
    bool can_heat_on = (state->heat_on_time > 0) || 
                       ((state->heat_on_time == 0) && (state->heat_off_time > ON_OFF_THRESHOLD));
    bool can_heat_off = (state->heat_on_time == 0);
    bool can_cool_on = (state->cool_on_time > 0) || 
                       ((state->cool_on_time == 0) && (state->cool_off_time > ON_OFF_THRESHOLD));
    bool can_cool_off = (state->cool_on_time == 0);


    // handle heating w/ hysteresis
    if (state->temp_in <= (state->temp_hsp - state->hysteresis) && can_heat_on && can_cool_off) {
        state->is_heating = true;
        state->is_cooling = false;
    } else if (state->is_heating && (state->temp_in <= (state->temp_hsp + state->hysteresis)) && can_heat_on && can_cool_off) {
        state->is_heating = true;
        state->is_cooling = false;
    } else if (state->temp_in >= (state->temp_csp + state->hysteresis) && can_heat_off && can_cool_on) {
        state->is_heating = false;
        state->is_cooling = true;
        state->is_fan_on = true;
    } else if (state->is_cooling && (state->temp_in >= (state->temp_csp - state->hysteresis)) && can_heat_off && can_cool_on) {
        state->is_heating = false;
        state->is_cooling = true;
        state->is_fan_on = true;
    } else if (state->heat_on_time > 0 && state->heat_on_time < ON_OFF_THRESHOLD) {
        state->is_heating = true;
    } else if (state->heat_off_time > 0 && state->heat_off_time < ON_OFF_THRESHOLD) {
        state->is_heating = false;
    } else if (state->cool_on_time > 0 && state->cool_on_time < ON_OFF_THRESHOLD) {
        state->is_cooling = true;
    } else if (state->cool_off_time > 0 && state->cool_off_time < ON_OFF_THRESHOLD) {
        state->is_cooling = false;
    } else {
        state->is_heating = false;
        state->is_cooling = false;
    }

    // handle free cooling
    if (!state->is_cooling && state->cool_on_time > 0 && state->fan_on_time == 0) { // cooling has just turned off
        state->is_fan_on = true;
    } else if (!state->is_cooling && state->fan_on_time > FREE_COOLING_TIME) {
        state->is_fan_on = false;
    }

    // done!
    clean_action(action);
}


void state_to_output(thermostat_t *tstat, thermostat_state_t *state, thermostat_output_t *output) {
    output->temp_display = state->temp_in;

    // handle if thermostat is off
    if (!state->on) {
        output->heat_stage_1 = false;
        output->heat_stage_2 = false;
        output->cool_stage_1 = false;
        output->cool_stage_2 = false;
        return;
    }

    // now assuming thermostat is on
    // display the setpoint info
    output->hsp_display = state->temp_hsp;
    output->csp_display = state->temp_csp;

    // handle heat/cool
    if (state->is_heating) {
        output->heat_stage_1 = true;
        output->cool_stage_1 = false;
        output->blinking = true;
    } else if (state->is_cooling) {
        output->heat_stage_1 = false;
        output->cool_stage_1 = true;
        output->blinking = true;
    } else {
        output->heat_stage_1 = false;
        output->cool_stage_1 = false;
        output->blinking = false;
    }

    // handle timer display
    output->timer_led_num = (state->hold_timer / TIMER_INTERVAL); // int!
}

uint32_t max(uint32_t a, uint32_t b) {
    if (a > b) return a;
    return b;
}

uint32_t min(uint32_t a, uint32_t b) {
    if (a < b) return a;
    return b;
}

void nearest_temperature(uint16_t *temp, uint16_t *display_temp, int *led_register) {
    int i=1;
    if (*temp <= 58) {
        *display_temp = temperature_led_mapping[0][0];
        *led_register = temperature_led_mapping[0][1];
        *temp = 58;
    } else if (*temp >= 92) {
        *display_temp = temperature_led_mapping[15][0];
        *led_register = temperature_led_mapping[15][1];
        *temp = 92;
    }

    for (;i<16;i++) {
        int _display_temp = temperature_led_mapping[i][0];

        if (_display_temp == *temp) {
            *display_temp = temperature_led_mapping[i][0];
            *led_register = temperature_led_mapping[i][1];
            return;
        }

        if (_display_temp > *temp) {
            *display_temp = temperature_led_mapping[i-1][0];
            *led_register = temperature_led_mapping[i-1][1];
            return;
        }
    }
}

void timer_led_settings(thermostat_state_t* state, int *led0, int *led1, int *led2, int *led3, int *num) {
    *num = 0;
    if (state->hold_timer >= timer_led_mapping[0][0]) (*num)++;
    if (state->hold_timer >= timer_led_mapping[1][0]) (*num)++;
    if (state->hold_timer >= timer_led_mapping[2][0]) (*num)++;
    if (state->hold_timer >= timer_led_mapping[3][0]) (*num)++;

    *led0 = timer_led_mapping[0][1];
    *led1 = timer_led_mapping[1][1];
    *led2 = timer_led_mapping[2][1];
    *led3 = timer_led_mapping[3][1];
}

void update_timer_press(thermostat_state_t* state) {
    if (state->hold_timer == MAX_TIMER_HOLD) {
        state->hold_timer = 0;
    } else {
        uint16_t _newtimer = state->hold_timer + TIMER_INTERVAL;
        if (_newtimer > MAX_TIMER_HOLD) {
            state->hold_timer = MAX_TIMER_HOLD;
        } else {
            state->hold_timer = _newtimer;
        }
    }
}

void enact_output(thermostat_t *tstat, thermostat_output_t *output) {

    // set relays
    if (output->heat_stage_1) {
        tlc59116_set_led(&tstat->leddriver_cfg, TLC59116_PWM6, 0xff);
        tlc59116_set_led(&tstat->leddriver_cfg, TLC59116_PWM7, 0x0);
        cool_off(&tstat->relay_cfg);
        heat_on(&tstat->relay_cfg);
    } else if (output->cool_stage_1) {
        tlc59116_set_led(&tstat->leddriver_cfg, TLC59116_PWM6, 0x0);
        tlc59116_set_led(&tstat->leddriver_cfg, TLC59116_PWM7, 0xff);
        heat_off(&tstat->relay_cfg);
        cool_on(&tstat->relay_cfg);
    } else {
        tlc59116_set_led(&tstat->leddriver_cfg, TLC59116_PWM6, 0x0);
        tlc59116_set_led(&tstat->leddriver_cfg, TLC59116_PWM7, 0x0);
        heat_off(&tstat->relay_cfg);
        cool_off(&tstat->relay_cfg);
    }


    // draw timers
    int num = output->timer_led_num;
    if (num == 0) {
        tlc59116_set_led(&tstat->leddriver_cfg, timer_led_mapping[0][1], 0x0);
        tlc59116_set_led(&tstat->leddriver_cfg, timer_led_mapping[1][1], 0x0);
        tlc59116_set_led(&tstat->leddriver_cfg, timer_led_mapping[2][1], 0x0);
        tlc59116_set_led(&tstat->leddriver_cfg, timer_led_mapping[3][1], 0x0);
    } else {
        tlc59116_set_led(&tstat->leddriver_cfg, timer_led_mapping[0][1], num >= 1 ? 0x0f : 0x0);
        tlc59116_set_led(&tstat->leddriver_cfg, timer_led_mapping[1][1], num >= 2 ? 0x0f : 0x0);
        tlc59116_set_led(&tstat->leddriver_cfg, timer_led_mapping[2][1], num >= 3 ? 0x0f : 0x0);
        tlc59116_set_led(&tstat->leddriver_cfg, timer_led_mapping[3][1], num == 4 ? 0x0f : 0x0);
    }


    // draw setpoints

    int led_register_temp;
    tlc59116_set_all(&tstat->spdisplay_cfg, 0x0);

    uint16_t _t;
    nearest_temperature(&output->csp_display, &_t, &led_register_temp);
    tlc59116_set_led(&tstat->spdisplay_cfg, led_register_temp, 0xff);

    nearest_temperature(&output->hsp_display, &_t, &led_register_temp);
    tlc59116_set_led(&tstat->spdisplay_cfg, led_register_temp, 0xff);

    // draw temperature

}
