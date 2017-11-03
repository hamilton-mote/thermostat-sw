#ifndef _HDC1000_H
#define _HDC1000_H

#include "nrf_drv_twi.h"
#include <stdint.h>
#include "nrf_drv_twi.h"
#include <stdbool.h>
#include <math.h>
#include "nrf_delay.h"
#include "hdc1000_regs.h"

typedef struct hdc1000_cfg_t {
    uint8_t address;
} hdc1000_cfg_t;

void hdc1000_init(hdc1000_cfg_t* cfg, nrf_drv_twi_t* p_instance);
void hdc1000_trigger_conversion(hdc1000_cfg_t* cfg);
void hdc1000_get_results(hdc1000_cfg_t* cfg, int16_t *temp, int16_t *hum);
void hdc1000_read(hdc1000_cfg_t* cfg, int16_t *temp, int16_t *hum);

#endif
