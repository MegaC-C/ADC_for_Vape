#include "saadc.h"

LOG_MODULE_REGISTER(saadc);

static int16_t saadc_samples[2][SAADC_BUF_SIZE];

static const nrfx_saadc_adv_config_t saadc_peripheral_config = {
    .oversampling      = NRF_SAADC_OVERSAMPLE_DISABLED,
    .burst             = NRF_SAADC_BURST_DISABLED,
    .internal_timer_cc = 0,
    .start_on_end      = false};

static const nrfx_saadc_channel_t saadc_left_sensor_channel_config = {
    .channel_config = {
        .resistor_p = NRF_SAADC_RESISTOR_DISABLED,
        .resistor_n = NRF_SAADC_RESISTOR_DISABLED,
        .gain       = NRF_SAADC_GAIN1_2,
        .reference  = NRF_SAADC_REFERENCE_VDD4,
        .acq_time   = NRF_SAADC_ACQTIME_5US,
        .mode       = NRF_SAADC_MODE_SINGLE_ENDED,
        .burst      = NRF_SAADC_BURST_DISABLED},
    .pin_p         = (nrf_saadc_input_t)NRF_SAADC_INPUT_AIN1,
    .pin_n         = NRF_SAADC_INPUT_DISABLED,
    .channel_index = 0};

static const nrfx_saadc_channel_t saadc_right_sensor_channel_config = {
    .channel_config = {
        .resistor_p = NRF_SAADC_RESISTOR_DISABLED,
        .resistor_n = NRF_SAADC_RESISTOR_DISABLED,
        .gain       = NRF_SAADC_GAIN1_2,
        .reference  = NRF_SAADC_REFERENCE_VDD4,
        .acq_time   = NRF_SAADC_ACQTIME_10US,
        .mode       = NRF_SAADC_MODE_SINGLE_ENDED,
        .burst      = NRF_SAADC_BURST_DISABLED},
    .pin_p         = (nrf_saadc_input_t)NRF_SAADC_INPUT_AIN4,
    .pin_n         = NRF_SAADC_INPUT_DISABLED,
    .channel_index = 1};

static const nrfx_saadc_channel_t saadc_battery_voltage_channel_config = {
    .channel_config = {
        .resistor_p = NRF_SAADC_RESISTOR_DISABLED,
        .resistor_n = NRF_SAADC_RESISTOR_DISABLED,
        .gain       = NRF_SAADC_GAIN1_2,
        .reference  = NRF_SAADC_REFERENCE_INTERNAL,
        .acq_time   = NRF_SAADC_ACQTIME_40US,
        .mode       = NRF_SAADC_MODE_SINGLE_ENDED,
        .burst      = NRF_SAADC_BURST_DISABLED},
    .pin_p         = (nrf_saadc_input_t)NRF_SAADC_INPUT_VDDHDIV5,
    .pin_n         = NRF_SAADC_INPUT_DISABLED,
    .channel_index = 2};

void saadc_init(nrfx_saadc_event_handler_t saadc_event_handler)
{
    // Connect ADC interrupt to nrfx interrupt handler
    IRQ_CONNECT(DT_IRQN(DT_NODELABEL(adc)),
                DT_IRQ(DT_NODELABEL(adc), priority),
                nrfx_isr, nrfx_saadc_irq_handler, 0);

    nrfx_err = nrfx_saadc_init(DT_IRQ(DT_NODELABEL(adc), priority));
    NRFX_ERR_CHECK(nrfx_err, "initializing SAADC peripheral failed");

    // calibration should be performed when SoC temp changes 10°C
    nrfx_err = nrfx_saadc_offset_calibrate(NULL);
    NRFX_ERR_CHECK(nrfx_err, "Cannot calibrate SAADC")

    nrfx_err = nrfx_saadc_channel_config(&saadc_left_sensor_channel_config);
    nrfx_err = nrfx_saadc_channel_config(&saadc_right_sensor_channel_config);
    NRFX_ERR_CHECK(nrfx_err, "Configuring SAADC channels failed");

    nrfx_err = nrfx_saadc_advanced_mode_set(nrfx_saadc_channels_configured_get(),
                                            NRF_SAADC_RESOLUTION_12BIT,
                                            &saadc_peripheral_config,
                                            saadc_event_handler);
    NRFX_ERR_CHECK(nrfx_err, "setting SAADC mode failed");

    // Configure two buffers to make use of double-buffering feature of SAADC
    nrfx_err = nrfx_saadc_buffer_set(&saadc_samples[0], SAADC_BUF_SIZE);
    nrfx_err = nrfx_saadc_buffer_set(&saadc_samples[1], SAADC_BUF_SIZE);
    NRFX_ERR_CHECK(nrfx_err, "setting SAADC buffer failed failed");

    // Trigger the SAADC. This will not start sampling, but will prepare buffer for sampling triggered through PPI
    nrfx_err = nrfx_saadc_mode_trigger();
    NRFX_ERR_CHECK(nrfx_err, "triggering SAADC mode failed");
}

void saadc_update_buffer(uint8_t buffer_num)
{
    nrfx_err = nrfx_saadc_buffer_set(&saadc_samples[buffer_num], SAADC_BUF_SIZE);
    NRFX_ERR_CHECK(nrfx_err, " failed");
}