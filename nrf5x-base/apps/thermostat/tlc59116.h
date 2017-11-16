#ifndef _TLC59116_H
#define _TLC59116_H

#include "nrf_drv_twi.h"
#include <stdint.h>
#include "nrf_drv_twi.h"
#include <stdbool.h>
#include <math.h>
#include "thermostat.h"

typedef struct tlc59116_cfg_t {
    uint8_t address; // address
    uint8_t initial_value;
} tlc59116_cfg_t;


void tlc59116_init(tlc59116_cfg_t* cfg, nrf_drv_twi_t* p_instance);
void tlc59116_set_led(tlc59116_cfg_t* cfg, int reg, int value);
void tlc59116_off(tlc59116_cfg_t* cfg);

#endif
