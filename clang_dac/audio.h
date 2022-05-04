/*
 * audio.h - generate audio for f303k8_nucleo
 */

#ifndef __audio__
#define __audio__

#include "main.h"

void audio_init();
void audio(int16_t *buf, uint8_t chl);

#endif