#include "thermostat.h"


void init_thermostat(thermostat_state_t* state, thermostat_action_t* action, thermostat_output_t* output) {
    state->hysteresis = 1.0;
    state->on = true;
    state->is_heating = false;
    state->is_cooling = false;
}

// TODO: need a method to get an action vector
void transition(thermostat_state_t* state, thermostat_action_t* action, uint32_t interval) {
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
        state->temp_hsp = min(state->temp_hsp+1, MAX_HSP);
        state->temp_csp = min(state->temp_csp+1, MAX_CSP);
    }
    if (action->dec_sp) {
        state->temp_hsp = max(state->temp_hsp-1, MIN_HSP);
        state->temp_csp = max(state->temp_csp-1, MIN_CSP);
    }

    // handle setting hsp/csp directly next
    if (action->hsp_direct != NULL) {
        state->temp_hsp = max(min(*(action->hsp_direct), MAX_HSP), MIN_HSP);
    }
    if (action->csp_direct != NULL) {
        state->temp_csp = max(min(*(action->csp_direct), MAX_CSP), MIN_CSP);
    }

    // decrement hold timer
    if (state->hold_timer > 0) {
        state->hold_timer -= TIMER_INTERVAL;
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

    // handle heating w/ hysteresis
    if (state->temp_in <= (state->temp_hsp - state->hysteresis)) {
        state->is_heating = true;
        state->is_cooling = false;
    } else if (state->is_heating && (state->temp_in <= (state->temp_hsp + state->hysteresis))) {
        state->is_heating = true;
        state->is_cooling = false;
    } else if (state->temp_in >= (state->temp_csp + state->hysteresis)) {
        state->is_heating = false;
        state->is_cooling = true;
    } else if (state->is_cooling && (state->temp_in >= (state->temp_csp - state->hysteresis))) {
        state->is_heating = false;
        state->is_cooling = true;
    } else {
        state->is_heating = false;
        state->is_cooling = false;
    }

    // done!
}

void state_to_output(thermostat_state_t *state, thermostat_output_t *output) {
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
    if (state->hold_timer) {
        output->timer_led_num = (state->hold_timer / TIMER_INTERVAL); // int!
    }
}

uint32_t max(uint32_t a, uint32_t b) {
    if (a > b) return a;
    return b;
}

uint32_t min(uint32_t a, uint32_t b) {
    if (a < b) return a;
    return b;
}
