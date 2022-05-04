/*
 * dac.c - dac setup for F303k8_nucleo
 * 05-02-2022 E. Brombaugh
 */

#ifndef __dac__
#define __dac__

#include "main.h"

#define DAC_BUFSZ 128
#define DAC_RATE 48012

extern uint32_t dac1_buffer[DAC_BUFSZ], dac2_buffer[DAC_BUFSZ];

void dac_init(void);
void dac_start(void);

#endif
