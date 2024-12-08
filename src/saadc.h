#ifndef SAADC_H
#define SAADC_H

#include "error_handling.h"
#include "global_config.h"
#include <nrfx_saadc.h>
#include <zephyr/logging/log.h>

void saadc_init(nrfx_saadc_event_handler_t saadc_event_handler);
void saadc_update_buffer(uint8_t buffer_num);

#endif