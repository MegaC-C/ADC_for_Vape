#ifndef GLOBAL_CONFIG_H
#define GLOBAL_CONFIG_H

#include <hal/nrf_gpio.h>

#define RED_LED      NRF_GPIO_PIN_MAP(0, 13)
#define POWER_MOSFET NRF_GPIO_PIN_MAP(0, 16)

#define SAADC_SAMPLE_INTERVAL_US 1000000 // sample every 5 µs to get the max possible 200 kHz SAADC
#define SAADC_BUF_SIZE           2       // at 200kHz it takes 20ms to fill the 4000 buffer

#define TIME_TO_SYSTEM_OFF_S 30

#endif