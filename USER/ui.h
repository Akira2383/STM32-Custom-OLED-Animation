#ifndef __UI_H
#define __UI_H
#include "stm32f10x.h"
#include "bsp_adc.h"
#include "spectrum.h"

#define STATE_BOOT  0
#define STATE_MENU  1

#define MENU_TERMINAL 0
#define MENU_SQUADS   1
#define MENU_OPERATOR 2
#define MENU_STORE    3
#define MENU_BASE     4

#define STATE_SENSOR_PAGE 2 

void UI_DrawBMP_Fast(u8 x,u8 y,u8 width,u8 height, const u8 *pic, u8 mode);
void UI_Draw_Menu(u8 cursor_pos);
void UI_ClearAudioRegions(void);
void UI_DrawWaveform(const u8 samples[ADC_SAMPLE_COUNT]);
void UI_DrawSpectrum(const u8 bars[SPECTRUM_BAR_COUNT]);
#endif

