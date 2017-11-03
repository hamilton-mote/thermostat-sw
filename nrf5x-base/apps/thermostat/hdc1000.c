#include "hdc1000.h"

// shameless copy of RIOTOS driver

static nrf_drv_twi_t* m_instance;

void hdc1000_init(hdc1000_cfg_t* cfg, nrf_drv_twi_t* p_instance) {
    m_instance = p_instance;

    uint16_t tmp;
    uint8_t command[3];
    uint8_t HDC1000_14BIT = (HDC1000_TRES14 | HDC1000_HRES14);
    //command = 
    tmp = (HDC1000_SEQ_MOD | HDC1000_14BIT);
    command[0] = HDC1000_CONFIG;
    command[1] = (tmp >> 8);
    command[2] = tmp;
    nrf_drv_twi_tx(
        m_instance, cfg->address, command, sizeof(command), false
    );
}

void hdc1000_trigger_conversion(hdc1000_cfg_t* cfg) {
    uint8_t write[] = {HDC1000_TEMPERATURE};
    nrf_drv_twi_tx(
        m_instance, cfg->address, write, sizeof(write), true
    );
    //TODO: sleep for 6.50ms for each value for 14bit res
}

void hdc1000_get_results(hdc1000_cfg_t* cfg, int16_t *temp, int16_t *hum) {
    //uint8_t command[] = {HDC1000_TEMPERATURE};
    uint8_t buf[5] = {0,0,0,0,0};
    uint16_t traw, hraw;
    //nrf_drv_twi_tx(
    //    m_instance, cfg->address, buf, 1, true
    //);
    nrf_drv_twi_rx(
        m_instance, cfg->address, buf, false
    );
    if (temp) {
        traw = ((uint16_t)buf[0] << 8) | buf[1];
        *temp = (int16_t)((((int32_t)traw * 16500) >> 16) - 4000);
    }
    if (hum) {
        hraw = ((uint16_t)buf[2] << 8) | buf[3];
        *hum  = (int16_t)(((int32_t)hraw * 10000) >> 16);
    }
}

void hdc1000_read(hdc1000_cfg_t* cfg, int16_t *temp, int16_t *hum) {
    hdc1000_trigger_conversion(cfg);
    //nrf_delay_us(20000);
    hdc1000_get_results(cfg, temp, hum);
}
