#include "bsp_adc.h"
#include "stm32f10x_adc.h"
#include "core_cm3.h"

static volatile u8 adc_buffer[ADC_SAMPLE_COUNT];
static volatile u8 write_idx;
static volatile u8 samples_stored;

void ADC_Sampling_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    ADC_InitTypeDef ADC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1, ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_1Cycles5);

    ADC_Cmd(ADC1, ENABLE);
    ADC_ResetCalibration(ADC1);
    while(ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1));
}

void ADC_SampleTick(void)
{
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);

    adc_buffer[write_idx] = (u8)(ADC_GetConversionValue(ADC1) >> 4);
    write_idx = (u8)((write_idx + 1) & (ADC_SAMPLE_COUNT - 1));
    if (samples_stored < ADC_SAMPLE_COUNT)
    {
        samples_stored++;
    }
}

u8 ADC_IsBufferReady(void)
{
    return samples_stored >= ADC_SAMPLE_COUNT;
}

void ADC_CopySamples(u8 samples[ADC_SAMPLE_COUNT])
{
    u32 primask = __get_PRIMASK();
    u8 start;
    u8 i;

    __disable_irq();
    start = write_idx;
    for (i = 0; i < ADC_SAMPLE_COUNT; i++)
    {
        samples[i] = adc_buffer[(start + i) & (ADC_SAMPLE_COUNT - 1)];
    }
    __set_PRIMASK(primask);
}
