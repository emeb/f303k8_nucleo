/*
 * filter_mg4_v1.c - 4-stage Moog ladder filter, version 1
 * from http://musicdsp.org/archive.php?classid=3#25 
 */

#ifndef __filter_mg4__
#define __filter_mg4__

#include "main.h"
#include "arm_math.h"

enum filter_modes
{
	FMG4_BYPASS,
	FMG4_LPF,
	FMG4_HPF,
	FMG4_BPF,
	FMG4_DISABLE,
};

typedef struct {
	float32_t f, p, q;				// filter coefficients
	float32_t b0, b1, b2, b3, b4;	// filter buffers (beware denormals!)
	float32_t t1, t2;				// temporary buffers
	float32_t gain;					// DC gain correction
	float32_t env;					// feedback limiter envelope
	float32_t lgain;				// feedback limiter gain
	float32_t tc_r, tc_f, thresh;	// feedback limiter params
	uint8_t mode, prev_mode;		// filter operating mode
	uint16_t xfade;					// crossfade counter
} fmg4_state;

void init_filter_mg4(fmg4_state *f);
void set_filter_mg4(fmg4_state *f, float32_t fc, float32_t res, uint8_t mode);
int16_t filter_mg4(fmg4_state *f, int16_t input);
void filter_mg4_lim(fmg4_state *f, float32_t thresh, float32_t tc_r);

#endif
