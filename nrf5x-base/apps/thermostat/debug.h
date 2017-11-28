#ifndef _DEBUG_H
#define _DEBUG_H


#ifdef USERTT
#include "SEGGER_RTT.h"
#include "nrf_log.h"
#define PRINT(fmt, args...) log_rtt_printf(0, fmt, ## args)
#else
#define PRINT(fmt, args...) do{ } while ( false )
#endif

#endif // _DEBUG_H
