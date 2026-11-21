#ifndef __TIMER_H
#define __TIMER_H
#include "stm32f10x.h"

void Timer_Init(void);      /* TIM3 1ms 定时中断 */
u32  Timer_GetMs(void);     /* 返回开机以来的毫秒数 */

#endif
