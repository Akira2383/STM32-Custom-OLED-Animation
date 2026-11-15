#include "stm32f10x.h"
#include "delay.h"
#include "oled.h"
#include "ui.h"
#include "images.h"

int main(void)
{
    u8 current_state = STATE_BOOT;
    u8 boot_timer = 0;
    u8 menu_cursor = MENU_TERMINAL;
    u8 menu_timer = 0;
    u8 menu_round = 0;

    delay_init();
    OLED_Init();
    OLED_Clear();

    while(1)
    {
        if (current_state == STATE_BOOT)
        {
					  OLED_Clear();
					  OLED_Refresh();
            delay_ms(500); 
					
            UI_DrawBMP_Fast(0, 0, 128, 64, logo_bmp, 1);    
            delay_ms(1500); 
            
            UI_DrawBMP_Fast(0, 0, 128, 64, logo_bmp, 0);        
            delay_ms(500); 

            boot_timer++;
            if (boot_timer >= 1) 
            {
                current_state = STATE_MENU;
                boot_timer = 0;
                OLED_Clear(); 
            }
        }
        else if (current_state == STATE_MENU)
        {
            delay_ms(200); 
            menu_timer++;
            
            if (menu_timer >= 5) 
            {
                menu_timer = 0;
                menu_cursor++; 
                if (menu_cursor > MENU_BASE) 
                {
                    menu_cursor = MENU_TERMINAL; 
                    menu_round++;                
                }

                if (menu_round >= 2)
                {
                    menu_round = 0;           
                    current_state = STATE_BOOT; 
                    delay_ms(1500); 
                    OLED_Clear();             
                }
                else
                {
                    OLED_Clear(); 
                }
            }

            if (current_state == STATE_MENU)
            {
               
                UI_Draw_Menu(menu_cursor); 
            }
        }
    }
}
