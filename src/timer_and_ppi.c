#include "timer_and_ppi.h"

LOG_MODULE_REGISTER(timer_and_ppi);

static const nrfx_timer_t timer_to_sample_saadc_via_ppi_instance = NRFX_TIMER_INSTANCE(2); // never use TIMER0 as it is used by BLE SoftDevice

static const nrfx_timer_config_t timer_config = {
    .frequency          = 1000000,
    .mode               = NRF_TIMER_MODE_TIMER,
    .bit_width          = NRF_TIMER_BIT_WIDTH_32,
    .interrupt_priority = NRFX_TIMER_DEFAULT_CONFIG_IRQ_PRIORITY,
    .p_context          = NULL};

// will be connected to timer and used to determine the frequency of the SAADC measurements
static nrf_ppi_channel_t ppi_channel_to_sample_saadc_via_timer;
// will be connected to the end event of SAADC and enables scan measurment through all activ SAADC channels,
// without this, .start_on_end in nrfx_saadc_adv_config_t must be set to true. But via PPI its more efficient.
static nrf_ppi_channel_t ppi_channel_to_start_next_saadc_channel_on_end;

void timer_init(void)
{
    nrfx_err = nrfx_timer_init(&timer_to_sample_saadc_via_ppi_instance,
                               &timer_config,
                               NULL);
    NRFX_ERR_CHECK(nrfx_err, "timer initialization failed");

    nrfx_timer_extended_compare(&timer_to_sample_saadc_via_ppi_instance,
                                NRF_TIMER_CC_CHANNEL0,
                                nrfx_timer_us_to_ticks(&timer_to_sample_saadc_via_ppi_instance, SAADC_SAMPLE_INTERVAL_US),
                                NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK,
                                false);
}

void ppi_init(void)
{
    nrfx_err = nrfx_ppi_channel_alloc(&ppi_channel_to_sample_saadc_via_timer);
    NRFX_ERR_CHECK(nrfx_err, "ppi timer alloc failed");
    nrfx_err = nrfx_ppi_channel_alloc(&ppi_channel_to_start_next_saadc_channel_on_end);
    NRFX_ERR_CHECK(nrfx_err, "ppi start on end alloc failed");

    // Trigger task sample from timer
    nrfx_err = nrfx_ppi_channel_assign(ppi_channel_to_sample_saadc_via_timer,
                                       nrfx_timer_compare_event_address_get(&timer_to_sample_saadc_via_ppi_instance, NRF_TIMER_CC_CHANNEL0),
                                       nrf_saadc_task_address_get(NRF_SAADC, NRF_SAADC_TASK_SAMPLE));
    NRFX_ERR_CHECK(nrfx_err, "ppi timer assign failed");

    // Trigger task start from end event
    nrfx_err = nrfx_ppi_channel_assign(ppi_channel_to_start_next_saadc_channel_on_end,
                                       nrf_saadc_event_address_get(NRF_SAADC, NRF_SAADC_EVENT_END),
                                       nrf_saadc_task_address_get(NRF_SAADC, NRF_SAADC_TASK_START));
    NRFX_ERR_CHECK(nrfx_err, "ppi start on end assign failed");

    nrfx_err = nrfx_ppi_channel_enable(ppi_channel_to_sample_saadc_via_timer);
    NRFX_ERR_CHECK(nrfx_err, "ppi timer enable failed");

    nrfx_err = nrfx_ppi_channel_enable(ppi_channel_to_start_next_saadc_channel_on_end);
    NRFX_ERR_CHECK(nrfx_err, "ppi start on end enable failed");
}

void timer_for_saadc_sampling_start(void)
{
    nrfx_timer_enable(&timer_to_sample_saadc_via_ppi_instance);
}
