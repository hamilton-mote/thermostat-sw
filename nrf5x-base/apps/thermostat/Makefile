PROJECT_NAME = $(shell basename "$(realpath ./)")

APPLICATION_SRCS = $(notdir $(wildcard ./*.c))
APPLICATION_SRCS += softdevice_handler.c
ifdef BLE
$(info BLE)
APPLICATION_SRCS += ble_advdata.c
APPLICATION_SRCS += ble_conn_params.c
APPLICATION_SRCS += ble_srv_common.c
CFLAGS += -DBLE=1
endif
APPLICATION_SRCS += app_timer.c
APPLICATION_SRCS += app_error.c
APPLICATION_SRCS += app_error_weak.c
APPLICATION_SRCS += app_util_platform.c
APPLICATION_SRCS += nrf_drv_common.c
APPLICATION_SRCS += nrf_drv_clock.c
APPLICATION_SRCS += nrf_drv_twi.c
APPLICATION_SRCS += nrf_drv_gpiote.c

ifdef BLE
APPLICATION_SRCS += simple_ble.c
APPLICATION_SRCS += simple_adv.c
endif

#APPLICATION_SRCS += nrf_drv_uart.c
ifdef USERTT
$(info USERTT)
#APPLICATION_SRCS += nrf_log.c
APPLICATION_SRCS += RTT_Syscalls_GCC.c
APPLICATION_SRCS += SEGGER_RTT.c
APPLICATION_SRCS += SEGGER_RTT_printf.c
APPLICATION_SRCS += nrf_log_backend_serial.c
APPLICATION_SRCS += nrf_log_frontend.c
CFLAGS += -DNRF_LOG_ENABLED=1 -DNRF_LOG_BACKEND_SERIAL_USES_RTT=1 -DNRF_LOG_BACKEND_SERIAL_USES_UART=0 -DUSERTT=1
endif

CFLAGS += -DAPP_RAM_BASE=0x20002078

RTC_YEAR=`date +'%y' | sed 's/^0//'`
RTC_MONTH=`date +'%m' | sed 's/^0//'`
RTC_DAY=`date +'%d' | sed 's/^0//'`
RTC_HOUR=`date +'%H' | sed 's/^0//'`
RTC_MIN=`date +'%M' | sed 's/^0//'`
RTC_SEC=`date +'%S' | sed 's/^0//'`
$(shell date +'%y %m %d %H:%M:%S' > FLASHED)


NRF_MODEL = nrf51
SDK_VERSION = 12
SOFTDEVICE_MODEL = s130

NRF_BASE_PATH ?= ../..
LIBRARY_PATHS += . ../../platform ../../include
SOURCE_PATHS += ../../platform ../../src

CFLAGS += -DRTC_YEAR=$(RTC_YEAR) -DRTC_MONTH=$(RTC_MONTH) -DRTC_DAY=$(RTC_DAY) 
CFLAGS += -DRTC_HOUR=$(RTC_HOUR) -DRTC_MIN=$(RTC_MIN) -DRTC_SEC=$(RTC_SEC)

include $(NRF_BASE_PATH)/make/Makefile
