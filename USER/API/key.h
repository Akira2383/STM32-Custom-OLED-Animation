#ifndef __KEY_H
#define __KEY_H	 
#include "sys.h"


#define KEY1  GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8)//读取按键1
#define KEY2  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_5)//读取按键2
#define KEY3  GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_15)//读取按键3
#define KEY4  GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_14)//读取按键4

#define KEY1_PRES 	1	//KEY0按下
#define KEY2_PRES		2	//KEY1按下
#define KEY3_PRES		3	//KEY2按下
#define KEY4_PRES   4	//KEY_UP按下(即WK_UP/KEY_UP)

extern u8 row;

void KEY_Init(void);//IO初始化
void Key1_Init(void);
void Key2_Init(void);
u8 KEY_Scan(u8);  	//按键扫描函数	
//void KeyRow_Scan(u8);
//u8 KeyCol_Scan(u8);
int KeyScan(u8 mode);
#endif