#ifndef __UI_H
#define __UI_H
#include "stm32f10x.h"


#define STATE_BOOT  0
#define STATE_MENU  1

#define MENU_TERMINAL 0
#define MENU_SQUADS   1
#define MENU_OPERATOR 2
#define MENU_STORE    3
#define MENU_BASE     4


void UI_DrawBMP_Fast(u8 x,u8 y,u8 width,u8 height, const u8 *pic, u8 mode);
void UI_Draw_Menu(u8 cursor_pos);

#endif
