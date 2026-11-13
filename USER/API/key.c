#include "stm32f10x.h"
#include "key.h"
#include "sys.h" 
#include "delay.h"
/*
KEY1 PB15
KEY2 PB14
KEY3 PB13
KEY4 PB12

*/
//按键初始化函数
void KEY_Init(void) //IO初始化
{ 
 	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);//使能PORTA,PORTE时钟
//	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOC|RCC_APB2Periph_AFIO,ENABLE );
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
 	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOE2,3,4
 
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);//使能PORTA,PORTE时钟
//	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOC|RCC_APB2Periph_AFIO,ENABLE );
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
 	GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIOE2,3,4
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);//使能PORTA,PORTE时钟
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_14|GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
 	GPIO_Init(GPIOC, &GPIO_InitStructure);//初始化GPIOE2,3,4
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;	//PB13~Wind	PB14~Water_pump
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 	//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 	//IO口速度为50MHz
	GPIO_Init(GPIOC, &GPIO_InitStructure);					 			//根据设定参数初始化GPIOB.5
	GPIO_ResetBits(GPIOC,GPIO_Pin_13);	

}


