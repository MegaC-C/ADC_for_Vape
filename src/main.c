#include "error_handling.h"
#include "saadc.h"
#include "timer_and_ppi.h"
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(VAPE);
K_SEM_DEFINE(saadc_sem, 0, 1);

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
        saadc_results = p_event->data.done.p_buffer;
        k_sem_give(&saadc_sem);
        break;

    case NRFX_SAADC_EVT_BUF_REQ:
        // set the already handled buffer of doubled buffer as new
        saadc_update_buffer((saadc_buffer_num++) % 2);
        break;

    case NRFX_SAADC_EVT_READY:
        // Buffer is ready, timer (and sampling) can be started
        k_sem_give(&saadc_sem);
        // timer_for_saadc_sampling_start();
        // nrf_saadc_task_trigger(NRF_SAADC, NRF_SAADC_TASK_SAMPLE); // needed to start first SAADC sampling instantly, then PPI handles it via Timer CC
        break;

    default:
        LOG_ERR("unhandled SAADC evt %d", p_event->type);
        break;
    }
}

void start_heating(void)
{
    nrf_gpio_pin_set(POWER_MOSFET); // => ON
    nrf_gpio_pin_clear(RED_LED);    // => ON

    k_usleep(10);

    timer_for_saadc_sampling_start();
    nrf_saadc_task_trigger(NRF_SAADC, NRF_SAADC_TASK_SAMPLE); // needed to start first SAADC sampling instantly, then PPI handles it via Timer CC
}

void stop_heating(void)
{
    nrf_gpio_pin_clear(POWER_MOSFET); // => OFF
    nrf_gpio_pin_set(RED_LED);        // => OFF
    timer_for_saadc_sampling_stop();

    k_msleep(5000);

    start_heating();
}

int main(void)
{
    nrf_gpio_cfg_output(POWER_MOSFET);
    nrf_gpio_pin_clear(POWER_MOSFET); // => OFF

    nrf_gpio_cfg_output(RED_LED);
    nrf_gpio_pin_set(RED_LED); // => OFF

    timer_init();
    saadc_init(saadc_handler);
    ppi_init();

    k_sem_take(&saadc_sem, K_FOREVER);
    start_heating();

    for (;;)
    {
        k_sem_take(&saadc_sem, K_FOREVER);

        vddh_div5_raw    = (int32_t)saadc_results[0];
        coil_voltage_raw = (int32_t)saadc_results[1];

        coil_power_raw = vddh_div5_raw * coil_voltage_raw;

        LOG_INF("current saadc bufffer address: 0x%x result %d ", (uint32_t)saadc_results, coil_power_raw);

        if (coil_power_raw > 200000)
        {
            stop_heating();
        }
    }
}
