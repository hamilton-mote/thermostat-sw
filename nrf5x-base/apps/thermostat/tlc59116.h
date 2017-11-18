#ifndef _TLC59116_H
#define _TLC59116_H

#include "nrf_drv_twi.h"
#include <stdint.h>
#include "nrf_drv_twi.h"
#include <stdbool.h>
#include <math.h>

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

typedef struct tlc59116_cfg_t {
    uint8_t address; // address
    uint8_t initial_value;
} tlc59116_cfg_t;


void tlc59116_init(tlc59116_cfg_t* cfg, nrf_drv_twi_t* p_instance);
void tlc59116_set_led(tlc59116_cfg_t* cfg, int reg, int value);
void tlc59116_set_all(tlc59116_cfg_t* cfg, int value);

#endif
