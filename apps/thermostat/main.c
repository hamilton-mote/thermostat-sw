/*
 * Send an advertisement periodically
 */

#include <stdbool.h>
#include <stdint.h>
#include "nrf_gpio.h"
#include "ble_advdata.h"
#include "boards.h"
#include "nordic_common.h"
#include "softdevice_handler.h"
#include "ble_debug_assert_handler.h"
#include "led.h"
#include "twi_master.h"
#include "twi_master_config.h"

#include "simple_ble.h"
#include "simple_adv.h"
#include "thermostat.h"

bool writeI2cReg(uint8_t address, uint8_t reg, uint8_t value)
{
    uint8_t data[2];
    data[0] = reg;
    data[1] = value;
    // Write: register protocol
    if (twi_master_transfer(address, data, 2, TWI_ISSUE_STOP))
    {
        return true;
    }
    // read or write failed.
    return false;
}

// Intervals for advertising and connections
static simple_ble_config_t ble_config = {
    .platform_id       = 0x80,              // used as 4th octect in device BLE address
    .device_id         = 0x8080,
    .adv_name          = DEVICE_NAME,       // used in advertisements if there is room
    .adv_interval      = MSEC_TO_UNITS(500, UNIT_0_625_MS),
    .min_conn_interval = MSEC_TO_UNITS(500, UNIT_1_25_MS),
    .max_conn_interval = MSEC_TO_UNITS(1000, UNIT_1_25_MS)
};

int main(void) {
    //uint32_t err_code;
    bool err = twi_master_init();

    led_init(19);

    // Setup BLE
    simple_ble_init(&ble_config);

    // Advertise because why not
    simple_adv_only_name();

    led_off(19);

    writeI2cReg(LED_DRIVER_TEMP, TLC59116_MODE1, 0x01);
    writeI2cReg(LED_DRIVER_TEMP, TLC59116_MODE2, 0x00);
    writeI2cReg(LED_DRIVER_TEMP, TLC59116_LEDOUT0, 0xff);
    writeI2cReg(LED_DRIVER_TEMP, TLC59116_LEDOUT1, 0xff);
    writeI2cReg(LED_DRIVER_TEMP, TLC59116_LEDOUT2, 0xff);
    writeI2cReg(LED_DRIVER_TEMP, TLC59116_LEDOUT3, 0xff);

    writeI2cReg(ALLCALLADR, TLC59116_PWM0, 0xff);
    
    thermostat_state_t state;
    thermostat_action_t action;
    thermostat_output_t output;
    init_thermostat(&state, &action, &output);


    while (1) {
        power_manage();
    }

    return err;
}
