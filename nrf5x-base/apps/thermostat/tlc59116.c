#include "tlc59116.h"

static nrf_drv_twi_t* m_instance;

void tlc59116_init(tlc59116_cfg_t* cfg, nrf_drv_twi_t* p_instance) {
    m_instance = p_instance;

    uint8_t command[] = {TLC59116_MODE1, 0x01};
    nrf_drv_twi_tx(
        m_instance, cfg->address, command, sizeof(command), false
    );

    command[0] = TLC59116_MODE2; command[1] = 0x20;
    nrf_drv_twi_tx(
        m_instance, cfg->address, command, sizeof(command), false
    );

    command[0] = TLC59116_LEDOUT0; command[1] = 0xff;
    nrf_drv_twi_tx(
        m_instance, cfg->address, command, sizeof(command), false
    );
    command[0] = TLC59116_LEDOUT1; command[1] = 0xff;
    nrf_drv_twi_tx(
        m_instance, cfg->address, command, sizeof(command), false
    );
    command[0] = TLC59116_LEDOUT2; command[1] = 0xff;
    nrf_drv_twi_tx(
        m_instance, cfg->address, command, sizeof(command), false
    );
    command[0] = TLC59116_LEDOUT3; command[1] = 0xff;
    nrf_drv_twi_tx(
        m_instance, cfg->address, command, sizeof(command), false
    );

    uint8_t leds[] = {TLC59116_PWM0, TLC59116_PWM1, TLC59116_PWM2, TLC59116_PWM3, TLC59116_PWM4,
                    TLC59116_PWM5, TLC59116_PWM6, TLC59116_PWM7, TLC59116_PWM8, TLC59116_PWM9,
                    TLC59116_PWM10,TLC59116_PWM11,TLC59116_PWM12,TLC59116_PWM13,TLC59116_PWM14,
                    TLC59116_PWM15};

    for (int i=0;i<16;i++)
    {
        command[0] = leds[i]; command[1] = cfg->initial_value;
        nrf_drv_twi_tx(
            m_instance, cfg->address, command, sizeof(command), false
        );
    }
}

void tlc59116_set_led(tlc59116_cfg_t* cfg, int reg, int value) {
    uint8_t command[] = {reg, value};
    nrf_drv_twi_tx(
        m_instance, cfg->address, command, sizeof(command), false
    );
}

void tlc59116_set_all(tlc59116_cfg_t* cfg, int value) {
    uint8_t command[] = {0x0,value};
    uint8_t leds[] = {TLC59116_PWM0, TLC59116_PWM1, TLC59116_PWM2, TLC59116_PWM3, TLC59116_PWM4,
                    TLC59116_PWM5, TLC59116_PWM6, TLC59116_PWM7, TLC59116_PWM8, TLC59116_PWM9,
                    TLC59116_PWM10,TLC59116_PWM11,TLC59116_PWM12,TLC59116_PWM13,TLC59116_PWM14,
                    TLC59116_PWM15};
    for (int i=0;i<16;i++)
    {
        command[0] = leds[i];
        nrf_drv_twi_tx(
            m_instance, cfg->address, command, sizeof(command), false
        );
    }
}

void tlc59116_blink(tlc59116_cfg_t* cfg) {
    uint8_t command[] = {TLC59116_MODE2, 0x20};
    command[0] = TLC59116_GRPFREQ; command[1] = 0x18; // 1 second
    nrf_drv_twi_tx(
        m_instance, cfg->address, command, sizeof(command), false
    );
    command[0] = TLC59116_GRPPWM; command[1] = 0x7f; // 1 second
    nrf_drv_twi_tx(
        m_instance, cfg->address, command, sizeof(command), false
    );
    //tlc59116_set_led(cfg, TLC59116_PWM0, 0x0f);
}

void tlc59116_no_blink(tlc59116_cfg_t* cfg) {
    //nrf_drv_twi_tx(
    //    m_instance, cfg->address, command, sizeof(command), false
    //);
    //command[0]=  TLC59116_GRPPWM;
    //command[1] = 0x7f;
    
    uint8_t command[] = {TLC59116_MODE2, 0x00};
    //nrf_drv_twi_tx(
    //    m_instance, cfg->address, command, sizeof(command), false
    //);
    command[0] = TLC59116_GRPPWM; command[1] = 0xff; // 1 second
    nrf_drv_twi_tx(
        m_instance, cfg->address, command, sizeof(command), false
    );
}
