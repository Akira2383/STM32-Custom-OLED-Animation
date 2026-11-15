#include "ui.h"
#include "oled.h"
#include "images.h"
#include "stdio.h" 

	void UI_DrawBMP_Fast(u8 x,u8 y,u8 width,u8 height, const u8 *pic, u8 mode)
	{
		u8 i,j;
		u16 byte_index = 0;
		
		for(i = 0;i < height / 8; i++)
		{
			OLED_WR_Byte(0xB0 + i +(y / 8), OLED_CMD);
			
			OLED_WR_Byte((x & 0x0F),OLED_CMD);
			OLED_WR_Byte(((x & 0xF0) >> 4) | 0x10, OLED_CMD);
			
			for (j = 0; j < width; j++)
			{
				u8 temp = pic[byte_index++];
				if (mode == 0)
				{
					temp = ~temp;
				}
				OLED_WR_Byte(temp, OLED_DATA);
			}
		}
	}



void UI_Draw_Menu(u8 cursor_pos)
{
    OLED_ShowString(0, 0,  (u8 *)"Terminal", 12, 1);
    OLED_ShowString(0, 12, (u8 *)"Squads", 12, 1);
    OLED_ShowString(0, 24, (u8 *)"Operator", 12, 1);
    OLED_ShowString(0, 36, (u8 *)"Store", 12, 1);
    OLED_ShowString(0, 48, (u8 *)"Base", 12, 1);

    
    OLED_ShowString(110, cursor_pos * 12, (u8 *)"<", 12, 1); 
    OLED_Refresh(); 
}




void UI_Draw_Sensor_Page(u16 light_val)
{
    u8 str_buf[20];
    u8 bar_width = 0;
    u8 i;

    
    OLED_ShowString(0, 0, (u8 *)"Light Sensor", 16, 1);
    
    
    sprintf((char *)str_buf, "Val: %d", light_val);
    OLED_ShowString(0, 20, str_buf, 16, 1);
    
   
    bar_width = (u8)((light_val * 100) / 4095);
    
   
    for (i = 0; i < 100; i++) {
        OLED_DrawPoint(10 + i, 42, 1);
    }
   
    for (i = 0; i < 100; i++) {
        OLED_DrawPoint(10 + i, 55, 1);
    }
   
    for (i = 0; i < 14; i++) {
        OLED_DrawPoint(10, 42 + i, 1);
    }
   
    for (i = 0; i < 14; i++) {
        OLED_DrawPoint(109, 42 + i, 1);
    }
    
   
    for (i = 0; i < bar_width; i++) 
    {
        u8 j;
        
        for (j = 0; j < 12; j++) {
            OLED_DrawPoint(11 + i, 43 + j, 1);
        }
    }
    
    OLED_Refresh(); 
}
