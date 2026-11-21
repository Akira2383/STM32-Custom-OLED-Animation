#ifndef __BSP_ADC_H
#define __BSP_ADC_H
#include "stm32f10x.h"

#define ADC_SAMPLE_COUNT 128

void ADC_Sampling_Init(void);
void ADC_SampleTick(void);
u8 ADC_IsBufferReady(void);
void ADC_CopySamples(u8 samples[ADC_SAMPLE_COUNT]);

#endif
