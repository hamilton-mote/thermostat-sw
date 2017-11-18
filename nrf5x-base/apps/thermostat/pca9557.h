#ifndef _PCA9557_H
#define _PCA9557_H

#include <stdint.h>
#include "nrf_drv_twi.h"
#include <stdbool.h>
#include <math.h>
#include "nrf_delay.h"

#define PCA9557_ADDR 24
#define PCA9557_IN_REG 0x0
#define PCA9557_OUT_REG 0x1
#define PCA9557_POL_REG 0x2
#define PCA9557_CFG_REG 0x3

#define W_ON  (0x1)
#define W_OFF (0x1 << 1)
#define Y_ON  (0x1 << 2)
#define Y_OFF (0x1 << 3)
#define G_ON  (0x1 << 4)
#define G_OFF (0x1 << 5)

typedef struct pca9557_cfg_t {
    uint8_t address;
} pca9557_cfg_t;

void pca9557_init(pca9557_cfg_t* cfg, nrf_drv_twi_t* p_instance);
void heat_on(pca9557_cfg_t* cfg);
void heat_off(pca9557_cfg_t* cfg);
void cool_on(pca9557_cfg_t* cfg);
void cool_off(pca9557_cfg_t* cfg);
void fan_on(pca9557_cfg_t* cfg);
void fan_off(pca9557_cfg_t* cfg);

#endif // _PCA9557_H
