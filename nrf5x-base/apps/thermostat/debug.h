#ifndef _DEBUG_H
#define _DEBUG_H


#ifdef USERTT
//#include "SEGGER_RTT.h"
#include "nrf_log_ctrl.h"
#define NRF_LOG_MODULE_NAME "TSTAT"
#include "nrf_log.h"
#define PRINT(fmt, args...) NRF_LOG_INFO(fmt, ## args)
#else
#define PRINT(fmt, args...) do{ } while ( false )
#endif

#endif // _DEBUG_H
