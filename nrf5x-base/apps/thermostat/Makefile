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
#APPLICATION_SRCS += tlc59116.c

#APPLICATION_SRCS += simple_ble.c
#APPLICATION_SRCS += simple_adv.c

NRF_MODEL = nrf51
SDK_VERSION = 11
SOFTDEVICE_MODEL = s130

NRF_BASE_PATH ?= ../..
LIBRARY_PATHS += . ../../platform ../../include
SOURCE_PATHS += ../../platform ../../src

include $(NRF_BASE_PATH)/make/Makefile
