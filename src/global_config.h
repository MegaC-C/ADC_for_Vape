#ifndef GLOBAL_CONFIG_H
#define GLOBAL_CONFIG_H

#include <hal/nrf_gpio.h>

#define RED_LED      NRF_GPIO_PIN_MAP(0, 13)
#define POWER_MOSFET NRF_GPIO_PIN_MAP(0, 29)

#define POWER_MOSFET_TURN_ON_TIME_US     10
#define TURN_ON_INTERVAL_MS              100
#define SAADC_SAMPLE_INTERVAL_US         200
#define SAADC_BUF_SIZE                   2
#define FACTOR_RAW_BATTERY_TO_VOLTAGE_nV 1464000
#define FACTOR_RAW_COIL_TO_AMPERE_uA     7320 - 6500000
#define TIME_TO_SYSTEM_OFF_S             30

#endif