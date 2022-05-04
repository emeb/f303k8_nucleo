/*
 * filter_mg4_v1.c - 4-stage Moog ladder filter, version 1
 * from http://musicdsp.org/archive.php?classid=3#25
 */
 
// Moog 24 dB/oct resonant lowpass VCF
// References: CSound source code, Stilson/Smith CCRMA paper.
// Modified by paul.kellett@maxim.abel.co.uk July 2000

#include "filter_mg4_v1.h"

#define SAT_LIM (10.0F)
#define XFADE_BITS 10
#define XFADE_MAX (1<<XFADE_BITS)

/*
 * init_mg4 - initialize the filter state
 */
void init_filter_mg4(fmg4_state *f)
{
	set_filter_mg4(f, 0.5F, 0.0F, 1);
	f->b0 = 0.0F;
	f->b1 = 0.0F;
	f->b2 = 0.0F;
	f->b3 = 0.0F;
	f->b4 = 0.0F;
	f->t1 = 0.0F;
	f->t2 = 0.0F;
	f->gain = 1.0F;
	f->mode = FMG4_LPF;
	f->prev_mode = FMG4_LPF;
	f->xfade = 0;
	f->env = 1.0F;
	f->lgain = 1.0F;
	f->thresh = 0.0F;	// limiter disabled by default
	f->tc_r = 0.0F;
	f->tc_f = 0.0F;
}

/*
 * set_filter - set filter parameters & precalculate some values.
 * 
 * fc = cutoff, nearly linear [0,1] -> [0, fs/2]
 * res = resonance [0, 1] -> [no resonance, self-oscillation]
 * bypass = mode (0 = lowpass, 1 = bypass, 2 = highpass, 3 = bandpass)
 */
void set_filter_mg4(fmg4_state *f, float32_t fc, float32_t res, uint8_t mode)
{
	// Set coefficients given frequency & resonance [0.0...1.0]
	f->q = 1.0F - fc;
	f->p = fc + 0.8F * fc * f->q;
	f->f = f->p + f->p - 1.0F;
	f->q = res * (1.0F + 0.5F * f->q * (1.0F - f->q + 5.6F * f->q * f->q));
	f->gain = 1.0F + res + res*2.0F*(1.0F - fc);

	// Bypass - explicitly, or if cutoff is > 90%
	f->mode = mode;
	if(fc > 0.9F)
	{
		if(mode < FMG4_HPF)
			f->mode = FMG4_BYPASS;	// bypassed
		else
			f->mode = FMG4_DISABLE;	// disabled
	}
	
	// start crossfade?
	if((f->xfade == 0) && (f->mode != f->prev_mode))
	{
		f->xfade = XFADE_MAX;
	}
}

/*
 * set filter limiter params
 */
void filter_mg4_lim(fmg4_state *f, float32_t thresh, float32_t tc_r)
{
	f->thresh = thresh;
	f->tc_r = 1.0F - tc_r;
	f->tc_f = 1.0F - tc_r/2.0F;
}

/*
 * filter_mg4 - run the filter
 */
int16_t filter_mg4(fmg4_state *f, int16_t input)
{
	float32_t raw, in, out, xfout, fb, rect;
	
	/* convert to float in [-1.0...+1.0) */
	raw = (float32_t)input/32768.0F;
	in = raw;
	
	/* feedback limiter */
	fb = f->q * f->b4;
	if(f->thresh)
	{
		rect = fabsf(fb);
		if(rect > f->env)
			f->env = f->tc_r * (f->env - rect) + rect;
		else
			f->env = f->tc_f * (f->env - rect) + rect;
		if(f->env > f->thresh)
		{
			f->lgain = f->thresh / f->env;
			fb *= f->lgain;
		}
	}
	
	/* Filter */
	in -= fb;	// feedback
	f->t1 = f->b1;
	f->b1 = (in + f->b0) * f->p - f->b1 * f->f;
	f->t2 = f->b2;
	f->b2 = (f->b1 + f->t1) * f->p - f->b2 * f->f;
	f->t1 = f->b3;
	f->b3 = (f->b2 + f->t2) * f->p - f->b3 * f->f;
	f->b4 = (f->b3 + f->t1) * f->p - f->b4 * f->f;
	f->b4 = f->b4 - f->b4 * f->b4 * f->b4 * 0.166667F;	// clipping
	f->b0 = in;
	
	/* saturate feedback to prevent overflow & NaN */
	f->b4 = f->b4 >  SAT_LIM ?  SAT_LIM : f->b4;
	f->b4 = f->b4 < -SAT_LIM ? -SAT_LIM : f->b4;
	
	/* select current output */
	switch(f->prev_mode)
	{
		default:
		case FMG4_BYPASS: // bypass
			out = raw;
			break;
		
		case FMG4_LPF: // Lowpass  output:  b4
			out = f->gain * f->b4;
			break;
		
		case FMG4_HPF: // Highpass output:  in - b4;
			out = raw - f->gain * f->b4;
			break;
		
		case FMG4_BPF: // Bandpass output:  3.0f * (b3 - b4);
			out = f->gain * (3.0F * (f->b3-f->b4));
			break;
		
		case FMG4_DISABLE: // disabled
			out = 0;
			break;
	}
	
	/* crossfading? */
	if(f->xfade)
	{
		float32_t fade = (float32_t)f->xfade / (float32_t)XFADE_MAX;
		
		/* select current output */
		switch(f->mode)
		{
			default:
			case FMG4_BYPASS: // bypass
				xfout = raw;
				break;
			
			case FMG4_LPF: // Lowpass  output:  b4
				xfout = f->gain * f->b4;
				break;
			
			case FMG4_HPF: // Highpass output:  in - b4;
				xfout = raw - f->gain * f->b4;
				break;
			
			case FMG4_BPF: // Bandpass output:  3.0f * (b3 - b4);
				xfout = f->gain * (3.0F * (f->b3-f->b4));
				break;
			
			case FMG4_DISABLE: // disabled
				xfout = 0;
				break;
		}
		
		/* mix */
		out = fade * out + (1.0F-fade) * xfout;
	
		/* update xfade */
		f->xfade--;
		if(f->xfade == 0)
		{
			/* done so swap in new path */
			f->prev_mode = f->mode;
		}
	}
	
	/* convert output back to int16 */
	int32_t sat = (out * 32768.0F) + 0.5F;
	return __SSAT(sat, 16);
}


