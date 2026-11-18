#include "spectrum.h"
#include "bsp_adc.h"

#define FFT_SIZE 128
#define FFT_BINS 64

static const s16 fft_cos_q15[33] = {
    32767, 32728, 32609, 32412, 32137, 31785, 31356, 30852,
    30273, 29621, 28898, 28105, 27245, 26319, 25329, 24279,
    23170, 22005, 20787, 19519, 18204, 16846, 15446, 14010,
    12539, 11039, 9512, 7962, 6393, 4808, 3212, 1608, 0
};

static s16 fft_real[FFT_SIZE];
static s16 fft_imaginary[FFT_SIZE];
static u32 fft_magnitudes[FFT_BINS];

static s16 FFT_CosQ15(u8 index)
{
    if (index > FFT_SIZE / 2)
    {
        index = (u8)(FFT_SIZE - index);
    }
    return fft_cos_q15[index];
}

static void FFT_GetTwiddle(u8 index, s16 *cosine, s16 *sine)
{
    if (index <= FFT_SIZE / 4)
    {
        *cosine = fft_cos_q15[index];
        *sine = fft_cos_q15[FFT_SIZE / 4 - index];
    }
    else
    {
        u8 reflected = (u8)(FFT_SIZE / 2 - index);
        *cosine = (s16)-fft_cos_q15[reflected];
        *sine = fft_cos_q15[FFT_SIZE / 4 - reflected];
    }
}

static u32 FFT_Sqrt(u32 value)
{
    u32 low = 0;
    u32 high = 46340;
    u32 root = 0;

    while (low <= high)
    {
        u32 middle = (low + high) / 2;
        u32 square = middle * middle;
        if (square <= value)
        {
            root = middle;
            low = middle + 1;
        }
        else
        {
            high = middle - 1;
        }
    }
    return root;
}

void Spectrum_ComputeBars(const u8 samples[ADC_SAMPLE_COUNT], u8 bars[SPECTRUM_BAR_COUNT])
{
    u32 sum = 0;
    u32 max_magnitude = 0;
    u16 i;
    u16 j;
    u16 length;
    u8 average;
    u8 group;

    for (i = 0; i < FFT_SIZE; i++)
    {
        sum += samples[i];
    }
    average = (u8)(sum / FFT_SIZE);

    for (i = 0; i < FFT_SIZE; i++)
    {
        s16 centered = (s16)samples[i] - average;
        s16 window = (s16)((32767 - FFT_CosQ15((u8)i)) / 2);
        fft_real[i] = (s16)(((s32)centered * 128 * window) / 32768);
        fft_imaginary[i] = 0;
    }

    j = 0;
    for (i = 1; i < FFT_SIZE; i++)
    {
        u16 bit = FFT_SIZE / 2;
        s16 temp;

        while (j & bit)
        {
            j ^= bit;
            bit >>= 1;
        }
        j ^= bit;
        if (i < j)
        {
            temp = fft_real[i];
            fft_real[i] = fft_real[j];
            fft_real[j] = temp;
            temp = fft_imaginary[i];
            fft_imaginary[i] = fft_imaginary[j];
            fft_imaginary[j] = temp;
        }
    }

    for (length = 2; length <= FFT_SIZE; length <<= 1)
    {
        u16 half = length / 2;
        u16 step = FFT_SIZE / length;
        u16 base;

        for (base = 0; base < FFT_SIZE; base += length)
        {
            u16 offset;
            for (offset = 0; offset < half; offset++)
            {
                u8 twiddle_index = (u8)(offset * step);
                s16 cosine;
                s16 sine;
                s16 wr;
                s16 wi;
                s16 upper_real = fft_real[base + offset];
                s16 upper_imaginary = fft_imaginary[base + offset];
                s16 lower_real = fft_real[base + offset + half];
                s16 lower_imaginary = fft_imaginary[base + offset + half];
                s32 product_real;
                s32 product_imaginary;
                s16 temp_real;
                s16 temp_imaginary;

                FFT_GetTwiddle(twiddle_index, &cosine, &sine);
                wr = cosine;
                wi = (s16)-sine;
                product_real = ((s32)lower_real * wr - (s32)lower_imaginary * wi) / 32768;
                product_imaginary = ((s32)lower_real * wi + (s32)lower_imaginary * wr) / 32768;
                temp_real = (s16)product_real;
                temp_imaginary = (s16)product_imaginary;

                fft_real[base + offset] = (s16)(((s32)upper_real + temp_real) / 2);
                fft_imaginary[base + offset] = (s16)(((s32)upper_imaginary + temp_imaginary) / 2);
                fft_real[base + offset + half] = (s16)(((s32)upper_real - temp_real) / 2);
                fft_imaginary[base + offset + half] = (s16)(((s32)upper_imaginary - temp_imaginary) / 2);
            }
        }
    }

    for (i = 0; i < FFT_BINS; i++)
    {
        s32 real_value = fft_real[i];
        s32 imaginary_value = fft_imaginary[i];
        u32 squared = (u32)(real_value * real_value + imaginary_value * imaginary_value);
        fft_magnitudes[i] = FFT_Sqrt(squared);
        if (fft_magnitudes[i] > max_magnitude)
        {
            max_magnitude = fft_magnitudes[i];
        }
    }

    for (group = 0; group < SPECTRUM_BAR_COUNT; group++)
    {
        u32 magnitude_sum = 0;
        u8 bin;

        for (bin = 0; bin < 4; bin++)
        {
            magnitude_sum += fft_magnitudes[group * 4 + bin];
        }
        bars[group] = max_magnitude == 0
            ? 0
            : (u8)((magnitude_sum * 32) / (4 * max_magnitude));
    }
}
