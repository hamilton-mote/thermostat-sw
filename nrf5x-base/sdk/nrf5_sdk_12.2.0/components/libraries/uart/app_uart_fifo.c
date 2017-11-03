/* Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */
#include "sdk_common.h"
#if NRF_MODULE_ENABLED(APP_UART)
#include "app_uart.h"
#include "app_fifo.h"
#include "nrf_drv_uart.h"
#include "nrf_assert.h"

static nrf_drv_uart_t app_uart_inst = NRF_DRV_UART_INSTANCE(APP_UART_DRIVER_INSTANCE);

static __INLINE uint32_t fifo_length(app_fifo_t * const fifo)
{
  uint32_t tmp = fifo->read_pos;
  return fifo->write_pos - tmp;
}

#define FIFO_LENGTH(F) fifo_length(&F)              /**< Macro to calculate length of a FIFO. */


static app_uart_event_handler_t   m_event_handler;            /**< Event handler function. */
static uint8_t tx_buffer[1];
static uint8_t rx_buffer[1];
static bool m_rx_ovf;

static app_fifo_t                  m_rx_fifo;                               /**< RX FIFO buffer for storing data received on the UART until the application fetches them using app_uart_get(). */
static app_fifo_t                  m_tx_fifo;                               /**< TX FIFO buffer for storing data to be transmitted on the UART when TXD is ready. Data is put to the buffer on using app_uart_put(). */

static void uart_event_handler(nrf_drv_uart_event_t * p_event, void* p_context)
{
    app_uart_evt_t app_uart_event;
    uint32_t err_code;

    switch (p_event->type)
    {
        case NRF_DRV_UART_EVT_RX_DONE:
            // Write received byte to FIFO.
            err_code = app_fifo_put(&m_rx_fifo, p_event->data.rxtx.p_data[0]);
            if (err_code != NRF_SUCCESS)
            {
                app_uart_event.evt_type          = APP_UART_FIFO_ERROR;
                app_uart_event.data.error_code   = err_code;
                m_event_handler(&app_uart_event);
            }
            // Notify that there are data available.
            else if (FIFO_LENGTH(m_rx_fifo) != 0)
            {
                app_uart_event.evt_type = APP_UART_DATA_READY;
                m_event_handler(&app_uart_event);
            }

            // Start new RX if size in buffer.
            if (FIFO_LENGTH(m_rx_fifo) <= m_rx_fifo.buf_size_mask)
            {
                (void)nrf_drv_uart_rx(&app_uart_inst, rx_buffer, 1);
            }
            else
            {
                // Overflow in RX FIFO.
                m_rx_ovf = true;
            }

            break;

        case NRF_DRV_UART_EVT_ERROR:
            app_uart_event.evt_type                 = APP_UART_COMMUNICATION_ERROR;
            app_uart_event.data.error_communication = p_event->data.error.error_mask;
            (void)nrf_drv_uart_rx(&app_uart_inst, rx_buffer, 1);
            m_event_handler(&app_uart_event);
            break;

        case NRF_DRV_UART_EVT_TX_DONE:
            // Get next byte from FIFO.
            if (app_fifo_get(&m_tx_fifo, tx_buffer) == NRF_SUCCESS)
            {
                (void)nrf_drv_uart_tx(&app_uart_inst, tx_buffer, 1);
            }
            else
            {
                // Last byte from FIFO transmitted, notify the application.
                app_uart_event.evt_type = APP_UART_TX_EMPTY;
                m_event_handler(&app_uart_event);
            }
            break;

        default:
            break;
    }
}


uint32_t app_uart_init(const app_uart_comm_params_t * p_comm_params,
                             app_uart_buffers_t *     p_buffers,
                             app_uart_event_handler_t event_handler,
                             app_irq_priority_t       irq_priority)
{
    uint32_t err_code;

    m_event_handler = event_handler;

    if (p_buffers == NULL)
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    // Configure buffer RX buffer.
    err_code = app_fifo_init(&m_rx_fifo, p_buffers->rx_buf, p_buffers->rx_buf_size);
    VERIFY_SUCCESS(err_code);

    // Configure buffer TX buffer.
    err_code = app_fifo_init(&m_tx_fifo, p_buffers->tx_buf, p_buffers->tx_buf_size);
    VERIFY_SUCCESS(err_code);

    nrf_drv_uart_config_t config = NRF_DRV_UART_DEFAULT_CONFIG;
    config.baudrate = (nrf_uart_baudrate_t)p_comm_params->baud_rate;
    config.hwfc = (p_comm_params->flow_control == APP_UART_FLOW_CONTROL_DISABLED) ?
            NRF_UART_HWFC_DISABLED : NRF_UART_HWFC_ENABLED;
    config.interrupt_priority = irq_priority;
    config.parity = p_comm_params->use_parity ? NRF_UART_PARITY_INCLUDED : NRF_UART_PARITY_EXCLUDED;
    config.pselcts = p_comm_params->cts_pin_no;
    config.pselrts = p_comm_params->rts_pin_no;
    config.pselrxd = p_comm_params->rx_pin_no;
    config.pseltxd = p_comm_params->tx_pin_no;

    err_code = nrf_drv_uart_init(&app_uart_inst, &config, uart_event_handler);
    VERIFY_SUCCESS(err_code);
    m_rx_ovf = false;

    // Turn on receiver if RX pin is connected
    if (p_comm_params->rx_pin_no != UART_PIN_DISCONNECTED)
    {
#ifdef UARTE_PRESENT
        if (!config.use_easy_dma)
#endif
        {
            nrf_drv_uart_rx_enable(&app_uart_inst);
        }

        return nrf_drv_uart_rx(&app_uart_inst, rx_buffer,1);
    }
    else
    {
        return NRF_SUCCESS;
    }
}


uint32_t app_uart_flush(void)
{
    uint32_t err_code;

    err_code = app_fifo_flush(&m_rx_fifo);
    VERIFY_SUCCESS(err_code);

    err_code = app_fifo_flush(&m_tx_fifo);
    VERIFY_SUCCESS(err_code);

    return NRF_SUCCESS;
}


uint32_t app_uart_get(uint8_t * p_byte)
{
    ASSERT(p_byte);
    bool rx_ovf = m_rx_ovf;

    ret_code_t err_code =  app_fifo_get(&m_rx_fifo, p_byte);

    // If FIFO was full new request to receive one byte was not scheduled. Must be done here.
    if(rx_ovf)
    {
        m_rx_ovf = false;
        uint32_t uart_err_code = nrf_drv_uart_rx(&app_uart_inst, rx_buffer, 1);

        // RX resume should never fail.
        APP_ERROR_CHECK(uart_err_code);
    }

    return err_code;
}


uint32_t app_uart_put(uint8_t byte)
{
    uint32_t err_code;
    err_code = app_fifo_put(&m_tx_fifo, byte);
    if (err_code == NRF_SUCCESS)
    {
        // The new byte has been added to FIFO. It will be picked up from there
        // (in 'uart_event_handler') when all preceding bytes are transmitted.
        // But if UART is not transmitting anything at the moment, we must start
        // a new transmission here.
        if (!nrf_drv_uart_tx_in_progress(&app_uart_inst))
        {
            // This operation should be almost always successful, since we've
            // just added a byte to FIFO, but if some bigger delay occurred
            // (some heavy interrupt handler routine has been executed) since
            // that time, FIFO might be empty already.
            if (app_fifo_get(&m_tx_fifo, tx_buffer) == NRF_SUCCESS)
            {
                err_code = nrf_drv_uart_tx(&app_uart_inst, tx_buffer, 1);
            }
        }
    }
    return err_code;
}


uint32_t app_uart_close(void)
{
    nrf_drv_uart_uninit(&app_uart_inst);
    return NRF_SUCCESS;
}
#endif //NRF_MODULE_ENABLED(APP_UART)
