#include "timer.h"
#include "stm32f10x.h"
#include "stm32f10x_tim.h"   /* TIM3 定时器 */
#include "misc.h"             /* NVIC */
#include "bsp_adc.h"

static volatile u32 ms_count = 0;   /* 毫秒计数，中断里累加 */

void Timer_Init(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    NVIC_InitTypeDef        NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    /* 72MHz / 72 = 1MHz → 每 1us 数一格；数满 1000 格 = 1ms */
    TIM_TimeBaseStructure.TIM_Prescaler     = 72 - 1;
    TIM_TimeBaseStructure.TIM_Period        = 1000 - 1;
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

    /* 当前工程没有别的中断，先固定分组为 2 位抢占 + 2 位子优先级 */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    NVIC_InitStructure.NVIC_IRQChannel                   = TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    TIM_Cmd(TIM3, ENABLE);
}

/* 中断服务函数放在本模块内，保持 timer 模块自包含 */
void TIM3_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);   /* 必须清标志 */
        ms_count++;
        ADC_SampleTick();
    }
}

u32 Timer_GetMs(void)
{
    return ms_count;
}
