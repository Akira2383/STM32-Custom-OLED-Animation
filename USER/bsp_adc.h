#ifndef __BSP_ADC_H
#define __BSP_ADC_H
#include "stm32f10x.h"

void ADC_Light_Init(void);
u16 ADC_Get_Light_Value(void);

#endif
