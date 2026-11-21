#include "bsp_iwdg.h"
#include "stm32f10x_iwdg.h"

/* LSI = 40kHz，预分频 64 → 625Hz，重装载 1875 → 约 3 秒超时 */
void IWDG_Init(void)
{
    RCC_LSICmd(ENABLE);
    while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);

    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    IWDG_SetPrescaler(IWDG_Prescaler_64);
    IWDG_SetReload(1875);
    IWDG_ReloadCounter();
    IWDG_Enable();
}

void IWDG_Feed(void)
{
    IWDG_ReloadCounter();
}
