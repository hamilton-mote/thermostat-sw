PROJECT_NAME = $(shell basename "$(realpath ./)")

APPLICATION_SRCS = $(notdir $(wildcard ./*.c))
APPLICATION_SRCS += softdevice_handler.c
APPLICATION_SRCS += app_util_platform.c
APPLICATION_SRCS += app_timer.c
APPLICATION_SRCS += app_error.c
APPLICATION_SRCS += app_error_weak.c

APPLICATION_SRCS += nrf_delay.c
APPLICATION_SRCS += nrf_drv_common.c
APPLICATION_SRCS += nrf_drv_twi.c
APPLICATION_SRCS += nrf_drv_gpiote.c

APPLICATION_SRCS += led.c

#APPLICATION_SRCS += nrf_drv_uart.c
ifdef USERTT
	APPLICATION_SRCS += nrf_log.c
	APPLICATION_SRCS += SEGGER_RTT.c
	APPLICATION_SRCS += SEGGER_RTT_printf.c
	CFLAGS += -DNRF_LOG_USES_RTT=1 -DUSERTT
endif

RTC_YEAR=`date +'%y' | sed 's/^0//'`
RTC_MONTH=`date +'%m' | sed 's/^0//'`
RTC_DAY=`date +'%d' | sed 's/^0//'`
RTC_HOUR=`date +'%H' | sed 's/^0//'`
RTC_MIN=`date +'%M' | sed 's/^0//'`
RTC_SEC=`date +'%S' | sed 's/^0//'`
$(shell date +'%y %m %d %H:%M:%S' > FLASHED)

#APPLICATION_SRCS += simple_ble.c
#APPLICATION_SRCS += simple_adv.c

NRF_MODEL = nrf51
SDK_VERSION = 11
SOFTDEVICE_MODEL = s130

NRF_BASE_PATH ?= ../..
LIBRARY_PATHS += . ../../platform ../../include
SOURCE_PATHS += ../../platform ../../src

CFLAGS += -DRTC_YEAR=$(RTC_YEAR) -DRTC_MONTH=$(RTC_MONTH) -DRTC_DAY=$(RTC_DAY) 
CFLAGS += -DRTC_HOUR=$(RTC_HOUR) -DRTC_MIN=$(RTC_MIN) -DRTC_SEC=$(RTC_SEC)

include $(NRF_BASE_PATH)/make/Makefile
