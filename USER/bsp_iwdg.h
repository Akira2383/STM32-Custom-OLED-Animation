#ifndef __BSP_IWDG_H
#define __BSP_IWDG_H
#include "stm32f10x.h"

void IWDG_Init(void);   /* 独立看门狗初始化，约 3 秒超时 */
void IWDG_Feed(void);   /* 喂狗 */

#endif
