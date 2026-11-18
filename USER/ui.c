#include "ui.h"
#include "oled.h"
#include "images.h"

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

static void UI_ClearRegion(u8 y_start, u8 y_end)
{
    u16 x;
    u16 y;

    for (x = 0; x < 128; x++)
    {
        for (y = y_start; y <= y_end; y++)
        {
            OLED_DrawPoint((u8)x, (u8)y, 0);
        }
    }
}

void UI_ClearAudioRegions(void)
{
    UI_ClearRegion(0, 31);
    UI_ClearRegion(32, 63);
}

void UI_DrawWaveform(const u8 samples[ADC_SAMPLE_COUNT])
{
    u8 i;
    u8 previous_y = (u8)(31 - (samples[0] >> 3));

    UI_ClearRegion(0, 31);
    OLED_DrawPoint(0, previous_y, 1);
    for (i = 1; i < ADC_SAMPLE_COUNT; i++)
    {
        u8 y = (u8)(31 - (samples[i] >> 3));
        OLED_DrawLine((u8)(i - 1), previous_y, i, y, 1);
        previous_y = y;
    }
}

void UI_DrawSpectrum(const u8 bars[SPECTRUM_BAR_COUNT])
{
    u8 j;

    UI_ClearRegion(32, 63);
    for (j = 0; j < SPECTRUM_BAR_COUNT; j++)
    {
        u8 height = bars[j];
        u8 x;
        u8 y;

        if (height > 32) height = 32;
        for (x = (u8)(j * 8); x < (u8)(j * 8 + 8); x++)
        {
            for (y = (u8)(64 - height); y < 64; y++)
            {
                OLED_DrawPoint(x, y, 1);
            }
        }
    }
}
