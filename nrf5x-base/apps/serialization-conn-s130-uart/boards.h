#pragma once

#define LEDS_NUMBER 0

// Nucleum
#define SER_CON_RX_PIN              11    // UART RX pin number.
#define SER_CON_TX_PIN              12    // UART TX pin number.
#define SER_CON_CTS_PIN             9    // UART Clear To Send pin number.
#define SER_CON_RTS_PIN             10    // UART Request To Send pin number.

#define LED_0          18
#define LED_1          19
#define LED_2          20

#define NRF_CLOCK_LFCLKSRC {.source        = NRF_CLOCK_LF_SRC_XTAL,            \
                            .rc_ctiv       = 0,                                \
                            .rc_temp_ctiv  = 0,                                \
                            .xtal_accuracy = NRF_CLOCK_LF_XTAL_ACCURACY_20_PPM}
