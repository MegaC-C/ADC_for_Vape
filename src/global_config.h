#ifndef GLOBAL_CONFIG_H
#define GLOBAL_CONFIG_H

#include <hal/nrf_gpio.h>

#define BLUE_LED NRF_GPIO_PIN_MAP(0, 17)
#define RED_LED  NRF_GPIO_PIN_MAP(0, 20)
#define TRAFO_L1 NRF_GPIO_PIN_MAP(0, 4)
#define TRAFO_L2 NRF_GPIO_PIN_MAP(0, 5)

#define SAADC_SAMPLE_INTERVAL_US 5000000 // sample every 5 µs to get the max possible 200 kHz SAADC
#define SAADC_BUF_SIZE        2       // at 200kHz it takes 20ms to fill the 4000 buffer

#define TIME_TO_SYSTEM_OFF_S 30

#endif