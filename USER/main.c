				#include "stm32f10x.h"
				#include "delay.h"
				#include "oled.h"
				#include "ui.h"
				#include "images.h"
				#include "bsp_adc.h"
				#include "key.h"
				#include "timer.h"
				#include "snake.h"
				#include "spectrum.h"

				static u8 adc_samples[ADC_SAMPLE_COUNT];
				static u8 spectrum_bars[SPECTRUM_BAR_COUNT];

				int main(void)
				{
						u8 current_state = STATE_BOOT;
						u8 boot_timer = 0;
						u8 menu_cursor = MENU_TERMINAL;
									u32 waveform_timer = 0;
									u32 spectrum_timer = 0;
				
						delay_init();
						OLED_Init();
									ADC_Sampling_Init();
						OLED_Clear();
						Key_Init();
	          Timer_Init();
					
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
							  Key_Scan(&key_next);
							  Key_Scan(&key_enter);

						 
							if (Key_GetEvent(&key_next) == KEY_EVENT_DOWN)
							{
									menu_cursor++;
									if (menu_cursor > MENU_BASE) menu_cursor = MENU_TERMINAL;
									OLED_Clear();
									UI_Draw_Menu(menu_cursor); 
							}

						 
							if (Key_GetEvent(&key_enter) == KEY_EVENT_DOWN)
							{
								 
									if (menu_cursor == MENU_BASE) 
									{
											current_state = STATE_SENSOR_PAGE;
											waveform_timer = Timer_GetMs();
											spectrum_timer = waveform_timer;
											UI_ClearAudioRegions();
											OLED_Refresh();
									}
									else if (menu_cursor == MENU_TERMINAL)
									{
										Snake_Run();
										current_state = STATE_MENU;
										OLED_Clear();
										UI_Draw_Menu(menu_cursor);
									}
							}
								}
						else if (current_state == STATE_SENSOR_PAGE)
						{
								u32 now;
								u8 refresh = 0;

								Key_Scan(&key_enter);
								if (Key_GetEvent(&key_enter) == KEY_EVENT_DOWN)
								{
										current_state = STATE_MENU;
										OLED_Clear();
										UI_Draw_Menu(menu_cursor);
										continue;
								}

								now = Timer_GetMs();
								if (ADC_IsBufferReady() &&
									(now - waveform_timer >= 30 || now - spectrum_timer >= 100))
								{
										ADC_CopySamples(adc_samples);
										if (now - waveform_timer >= 30)
										{
												UI_DrawWaveform(adc_samples);
												waveform_timer = now;
												refresh = 1;
										}
										if (now - spectrum_timer >= 100)
										{
												Spectrum_ComputeBars(adc_samples, spectrum_bars);
												UI_DrawSpectrum(spectrum_bars);
												spectrum_timer = now;
												refresh = 1;
										}
										if (refresh) OLED_Refresh();
								}
						}
							}
						}
				
