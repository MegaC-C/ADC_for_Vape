#include "error_handling.h"
#include "saadc.h"
#include "timer_and_ppi.h"
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(LOG);

//  Declare variable used to keep track of which buffer was last assigned to the SAADC driver
static uint8_t saadc_buffer_num = 0;
static volatile uint16_t *saadc_results;
static volatile bool saadc_done = false;

void saadc_handler(nrfx_saadc_evt_t const *p_event)
{
    LOG_INF("SAADC evt %d", p_event->type);
    switch (p_event->type)
    {

    case NRFX_SAADC_EVT_DONE:

        saadc_done = true;

        int32_t current_value;

        saadc_results = p_event->data.done.p_buffer;

        current_value = (int32_t)(((int16_t *)(p_event->data.done.p_buffer))[0]) * (int32_t)(((int16_t *)(p_event->data.done.p_buffer))[1]);

        LOG_INF("SAADC buffer at 0x%x filled with result %d", (uint32_t)p_event->data.done.p_buffer, current_value);
        break;

    case NRFX_SAADC_EVT_BUF_REQ:
        // set the already handled buffer of doubled buffer as new
        saadc_update_buffer((saadc_buffer_num++) % 2);
        break;

    case NRFX_SAADC_EVT_READY:
        // Buffer is ready, timer (and sampling) can be started
        timer_for_saadc_sampling_start();
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

            uint32_t start_cycles = k_cycle_get_32(); // Get start cycle count

            int32_t current_value = (int32_t)saadc_results[0] * (int32_t)saadc_results[1];
            if (current_value > 200000)
            {
                for (size_t i = 0; i < 5000; i++)
                {
                    saadc_done = !saadc_done;
                }
            }

            uint32_t end_cycles = k_cycle_get_32(); // Get end cycle count

            // Calculate elapsed cycles
            uint32_t elapsed_cycles = end_cycles - start_cycles;

            // Convert to time in nanoseconds (if needed)
            uint64_t elapsed_ns = k_cyc_to_ns_floor64(elapsed_cycles);

            LOG_ERR("current saadc bufffer address: 0x%x result %d ", (uint32_t)saadc_results, current_value);

            LOG_WRN("Result: %d, Elapsed Cycles: %u, Elapsed Time: %llu ns\n",
                    end_cycles, start_cycles, elapsed_ns);
        }

        k_msleep(1000);
    }
}
