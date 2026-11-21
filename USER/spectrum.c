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
    if (index > FFT_SIZE / 4)
    {
        index = (u8)(FFT_SIZE / 2 - index);
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

/* 平方根查表：Q8 格式，table[i] = sqrt(i*256) * 256，i = 0..255 */
static const u16 FFT_SqrtTable[256] = {
        0,  4096,  5793,  7094,  8192,  9159, 10033, 10837,
    11585, 12288, 12953, 13585, 14189, 14768, 15326, 15864,
    16384, 16888, 17378, 17854, 18318, 18770, 19212, 19644,
    20066, 20480, 20886, 21283, 21674, 22058, 22435, 22806,
    23170, 23530, 23884, 24232, 24576, 24915, 25249, 25580,
    25905, 26227, 26545, 26859, 27170, 27477, 27780, 28081,
    28378, 28672, 28963, 29251, 29537, 29819, 30099, 30377,
    30652, 30924, 31194, 31462, 31727, 31991, 32252, 32511,
    32768, 33023, 33276, 33527, 33776, 34024, 34270, 34514,
    34756, 34996, 35235, 35472, 35708, 35942, 36175, 36406,
    36636, 36864, 37091, 37316, 37540, 37763, 37985, 38205,
    38424, 38642, 38858, 39073, 39287, 39500, 39712, 39923,
    40132, 40341, 40548, 40755, 40960, 41164, 41368, 41570,
    41771, 41972, 42171, 42369, 42567, 42763, 42959, 43154,
    43348, 43541, 43733, 43925, 44115, 44305, 44494, 44682,
    44869, 45056, 45242, 45427, 45611, 45795, 45977, 46160,
    46341, 46522, 46702, 46881, 47059, 47237, 47415, 47591,
    47767, 47942, 48117, 48291, 48465, 48637, 48809, 48981,
    49152, 49322, 49492, 49661, 49830, 49998, 50166, 50332,
    50499, 50665, 50830, 50995, 51159, 51323, 51486, 51649,
    51811, 51972, 52134, 52294, 52454, 52614, 52773, 52932,
    53090, 53248, 53405, 53562, 53719, 53874, 54030, 54185,
    54340, 54494, 54647, 54801, 54954, 55106, 55258, 55410,
    55561, 55712, 55862, 56012, 56162, 56311, 56459, 56608,
    56756, 56903, 57051, 57198, 57344, 57490, 57636, 57781,
    57926, 58071, 58215, 58359, 58503, 58646, 58789, 58931,
    59073, 59215, 59357, 59498, 59639, 59779, 59919, 60059,
    60199, 60338, 60477, 60615, 60753, 60891, 61029, 61166,
    61303, 61440, 61576, 61712, 61848, 61984, 62119, 62254,
    62388, 62523, 62657, 62790, 62924, 63057, 63190, 63323,
    63455, 63587, 63719, 63850, 63982, 64113, 64243, 64374,
    64504, 64634, 64763, 64893, 65022, 65151, 65279, 65408,
};

static u32 FFT_Sqrt(u32 value)
{
    s8  shift = 0;
    u32 v = value;
    u8  idx;
    u32 q8;

    if (value == 0) return 0;

    /* 把输入归一化到 16 位：每右移 2 位，平方根对应右移 1 位 */
    while (v > 0xFFFFu) { v >>= 2; shift++; }
    while (v < 256u)    { v <<= 2; shift--; }

    idx = (u8)(v >> 8);
    if (idx > 254) idx = 254;   /* 给插值留出 idx+1 */

    /* 线性插值，结果单位 Q8（真实值 × 256） */
    q8 = (u32)FFT_SqrtTable[idx] +
         (u32)(FFT_SqrtTable[idx + 1] - FFT_SqrtTable[idx]) * (v & 0xFF) / 256u;

    if (shift > 0) return (q8 >> 8) << shift;
    if (shift < 0) return (q8 >> 8) >> (u8)(-shift);
    return q8 >> 8;
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
