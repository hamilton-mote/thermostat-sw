#include "mcp7940n.h"


// from MPA
const unsigned long days_to_months[13] =
{
    0,31,59,90,120,151,181,212,243,273,304,334,365
};

static nrf_drv_twi_t* m_instance;

// from https://rheingoldheavy.com/mcp7940-tutorial-02-setting-and-getting-time/
uint8_t toBCD(uint8_t decimal) {
    return (decimal / 10) << 4 | (decimal % 10);
}

void mcp7940n_init(mcp7940n_cfg_t* cfg, nrf_drv_twi_t* p_instance) {
    m_instance = p_instance;

    uint8_t configure[8];

    // disable oscillator
    configure[0] = MCP7940N_RTCSEC;
    configure[1] = 0x00;
    nrf_drv_twi_tx(m_instance, cfg->address, configure, 2, false);

    // configure with current time (may need to do 'make clean' first)
    configure[0] = MCP7940N_RTCSEC;
    configure[1] = toBCD(RTC_SEC);
    configure[2] = toBCD(RTC_MIN);
    // configure 24-hour clock (0x40 bit)
    configure[3] = (toBCD(RTC_HOUR)) | (0x40);
    configure[4] = 0; // day of week TODO
    configure[5] = toBCD(RTC_DAY);
    configure[6] = toBCD(RTC_MONTH);
    configure[7] = toBCD(RTC_YEAR);

    nrf_drv_twi_tx(m_instance, cfg->address, configure, sizeof(configure), false);

    // turn on the clock
    configure[0] = MCP7940N_RTCSEC;
    configure[1] = toBCD(RTC_SEC) | 0x80;
    nrf_drv_twi_tx(m_instance, cfg->address, configure, 2, false);
}


void mcp7940n_readdate(mcp7940n_cfg_t* cfg, rtcc_time_t *output_time) {
    uint8_t time[7];
    time[0] = MCP7940N_RTCSEC;
    nrf_drv_twi_tx(m_instance, cfg->address, time, 1, true);
    nrf_drv_twi_rx(m_instance, cfg->address, time, 7);

    /*
     * RTCSEC register ones-digit is 0b00001111. Tens-digit is 0b01110000
     */
    output_time->tm_sec = 10*((time[0] & 0x70) >> 4) + (time[0] & 0x0f);
    output_time->tm_min = 10*((time[1] & 0x70) >> 4) + (time[1] & 0x0f);
    output_time->tm_hour = 10*((time[2] & 0x30) >> 4) + (time[2] & 0x0f);

    output_time->tm_mday = 10*((time[4] & 0x30) >> 4) + (time[4] & 0x0f);
    output_time->tm_mon = 10*((time[5] & 0x10) >> 4) + (time[5] & 0x0f);
    output_time->tm_year = 10*((time[6] & 0xf0) >> 4) + (time[6] & 0x0f);

    //time->tm_sec = RTC_SEC;//(command[0] & 0x80);
}

// from MPA
unsigned long date_to_binary(rtcc_time_t *datetime)
{
   unsigned long iday;
   unsigned long val;
   iday = 365 * (datetime->tm_year - 12) + days_to_months[datetime->tm_mon-1] + (datetime->tm_mday - 1);
   iday = iday + (datetime->tm_year - 9) / 4;
   if ((datetime->tm_mon > 2) && ((datetime->tm_year % 4) == 0))
   {
      iday++;
   }
   val = datetime->tm_sec + 60 * datetime->tm_min + 3600 * (datetime->tm_hour + 24 * iday);
   return val;
}
