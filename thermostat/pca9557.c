#include "pca9557.h"

static nrf_drv_twi_t* m_instance;

void pca9557_init(pca9557_cfg_t* cfg, nrf_drv_twi_t* p_instance) {
    m_instance = p_instance;

    // configure OUTPUT setting to be 0
    uint8_t command[] = {PCA9557_OUT_REG, 0x0};
    nrf_drv_twi_tx(m_instance, cfg->address, command, sizeof(command), false);

    nrf_delay_us(15000);
    // configure all pins as output
    command[0] = PCA9557_CFG_REG;
    //command[1] = 0x0;
    nrf_drv_twi_tx(m_instance, cfg->address, command, sizeof(command), false);
}

void heat_on(pca9557_cfg_t* cfg) {
    uint8_t command[] = {PCA9557_OUT_REG, W_ON};
    nrf_drv_twi_tx(m_instance, cfg->address, command, sizeof(command), false);
    nrf_delay_us(15000);
    command[1] = 0x0;
    nrf_drv_twi_tx(m_instance, cfg->address, command, sizeof(command), false);
}
void heat_off(pca9557_cfg_t* cfg) {
    uint8_t command[] = {PCA9557_OUT_REG, W_OFF};
    nrf_drv_twi_tx(m_instance, cfg->address, command, sizeof(command), false);
    nrf_delay_us(15000);
    command[1] = 0x0;
    nrf_drv_twi_tx(m_instance, cfg->address, command, sizeof(command), false);
}

void cool_on(pca9557_cfg_t* cfg) {
    uint8_t command[] = {PCA9557_OUT_REG, Y_ON};
    nrf_drv_twi_tx(m_instance, cfg->address, command, sizeof(command), false);
    nrf_delay_us(15000);
    command[1] = 0x0;
    nrf_drv_twi_tx(m_instance, cfg->address, command, sizeof(command), false);
}
void cool_off(pca9557_cfg_t* cfg) {
    uint8_t command[] = {PCA9557_OUT_REG, Y_OFF};
    nrf_drv_twi_tx(m_instance, cfg->address, command, sizeof(command), false);
    nrf_delay_us(15000);
    command[1] = 0x0;
    nrf_drv_twi_tx(m_instance, cfg->address, command, sizeof(command), false);
}


void fan_on(pca9557_cfg_t* cfg) {
    uint8_t command[] = {PCA9557_OUT_REG, G_ON};
    nrf_drv_twi_tx(m_instance, cfg->address, command, sizeof(command), false);
    nrf_delay_us(15000);
    command[1] = 0x0;
    nrf_drv_twi_tx(m_instance, cfg->address, command, sizeof(command), false);
}
void fan_off(pca9557_cfg_t* cfg) {
    uint8_t command[] = {PCA9557_OUT_REG, G_OFF};
    nrf_drv_twi_tx(m_instance, cfg->address, command, sizeof(command), false);
    nrf_delay_us(15000);
    command[1] = 0x0;
    nrf_drv_twi_tx(m_instance, cfg->address, command, sizeof(command), false);
}

