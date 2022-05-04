/*
 * audio.c - audio generation for f303k8_nucleo
 * 05-02-22 E. Brombaugh
 */
#include "audio.h"
#include "arm_math.h"
#include "dac.h"
#include "filter_mg4_v1.h"

fmg4_state f0, f1, f2;

/*
 * set up audio
 */
void audio_init()
{
    /* enable CRC for noise gen */
    __HAL_RCC_CRC_CLK_ENABLE();

    /* init filters */
    init_filter_mg4(&f0);
    init_filter_mg4(&f1);
    init_filter_mg4(&f2);

    /* set up filter params */
    set_filter_mg4(&f0, 50.0F*2.0F/(float32_t)DAC_RATE, 0.5F, FMG4_LPF);
    set_filter_mg4(&f1, 500.0F*2.0F/(float32_t)DAC_RATE, 0.5F, FMG4_LPF);
    set_filter_mg4(&f2, 5000.0F*2.0F/(float32_t)DAC_RATE, 0.5F, FMG4_LPF);
}

/*
 * generate noise
 */
int16_t noise(int16_t seed)
{
    CRC->DR = seed;
    return CRC->DR;
}

/*
 * generate audio
 */
void audio(int16_t *buf, uint8_t chl)
{
    uint16_t i;

    for(i=0;i<DAC_BUFSZ/2;i++)
    {
        if(chl == 0)
        {
            /* comput two channels interleaved */
            *buf++ = 0x8000 ^ filter_mg4(&f0, noise(i));
            *buf++ = 0x8000 ^ filter_mg4(&f1, noise(i));
        }
        else
        {
            /* just one channel */
            *buf = 0x8000 ^ filter_mg4(&f2, noise(i));
            buf += 2;
        }
    }
}