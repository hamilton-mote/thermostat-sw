#ifndef _MCP7940N_H
#define _MCP7940N_H

#include <stdint.h>
#include "nrf_drv_twi.h"
#include <stdbool.h>
#include <math.h>
#include "nrf_delay.h"

// RTC + calendar

#define MCP7940N_ADDR 0x6f

// registers
#define MCP7940N_RTCSEC 0x00
#define MCP7940N_RTCMIN 0x01
#define MCP7940N_RTCHOUR 0x02
#define MCP7940N_RTCWKDAY 0x03
#define MCP7940N_RTCDATE 0x04
#define MCP7940N_RTCMTH 0x05
#define MCP7940N_RTCYEAR 0x06
#define MCP7940N_CONTROL 0x07
#define MCP7940N_OSCTRIM 0x08

// from MPA's rtcc.h
typedef struct
{
   uint8_t tm_sec;     /* seconds after the minute - [0,59]    */
   uint8_t tm_min;     /* minutes after the hour - [0,59]      */
   uint8_t tm_hour;    /* hours since midnight - [0,23]        */
   uint8_t tm_mday;    /* day of the month - [1,31]            */
   uint8_t tm_mon;     /* months since January - [0,11]        */
   uint8_t tm_year;    /* years since 2000                     */
   uint8_t tm_wday;    /* days since Sunday - [0,6]            */
} rtcc_time_t;

typedef struct mcp7940n_cfg_t {
    uint8_t address;
} mcp7940n_cfg_t;

void mcp7940n_init(mcp7940n_cfg_t* cfg, nrf_drv_twi_t* p_instance);
void mcp7940n_readdate(mcp7940n_cfg_t* cfg, rtcc_time_t* time);

// from MPA
unsigned long date_to_binary(rtcc_time_t *datetime);

#endif
