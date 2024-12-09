#include "error_handling.h"
#include "saadc.h"
#include "timer_and_ppi.h"
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(LOG);

//  Declare variable used to keep track of which buffer was last assigned to the SAADC driver
uint8_t saadc_buffer_num = 0;
volatile int16_t *saadc_results;
volatile bool saadc_done = false;
int32_t vddh_div5_raw    = 0;
int32_t coil_voltage_raw = 0;
int32_t coil_power_raw   = 0;

void saadc_handler(nrfx_saadc_evt_t const *p_event)
{
    LOG_INF("SAADC evt %d", p_event->type);
    switch (p_event->type)
    {

    case NRFX_SAADC_EVT_DONE:
        saadc_done    = true;
        saadc_results = p_event->data.done.p_buffer;
        break;

    case NRFX_SAADC_EVT_BUF_REQ:
        // set the already handled buffer of doubled buffer as new
        saadc_update_buffer((saadc_buffer_num++) % 2);
        break;

    case NRFX_SAADC_EVT_READY:
        // Buffer is ready, timer (and sampling) can be started
        timer_for_saadc_sampling_start();
        nrf_saadc_task_trigger(NRF_SAADC, NRF_SAADC_TASK_SAMPLE); // needed to start first SAADC sampling instantly, then PPI handles it via Timer CC
        break;

    default:
        LOG_ERR("unhandled SAADC evt %d", p_event->type);
        break;
    }
}

int main(void)
{
    timer_init();
    saadc_init(saadc_handler);
    ppi_init();
    for (;;)
    {

        if (saadc_done)
        {
            saadc_done = false;

            vddh_div5_raw    = (int32_t)saadc_results[0];
            coil_voltage_raw = (int32_t)saadc_results[1];

            coil_power_raw = vddh_div5_raw * coil_voltage_raw;

            LOG_ERR("current saadc bufffer address: 0x%x result %d ", (uint32_t)saadc_results, coil_power_raw);
        }

        k_msleep(1);
    }
}
