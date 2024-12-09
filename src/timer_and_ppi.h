#ifndef TIMER_AND_PPI_H
#define TIMER_AND_PPI_H

#include "error_handling.h"
#include "global_config.h"
#include <nrfx_ppi.h>
#include <nrfx_saadc.h>
#include <nrfx_timer.h>
#include <zephyr/logging/log.h>

void timer_init(void);
void ppi_init(void);
void timer_for_saadc_sampling_start(void);
void timer_for_saadc_sampling_stop(void);

#endif