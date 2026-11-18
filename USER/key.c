#include "key.h"


Key_t key_next;
Key_t key_enter;
Key_t key_up;
Key_t key_left;
Key_t key_right;


u8 Read_PA1(void) { return GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1); }
u8 Read_PA2(void) { return GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_2); }
u8 Read_PA3(void) { return GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_3); }
u8 Read_PA5(void) { return GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5); }
u8 Read_PA6(void) { return GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_6); }

void Key_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 |
                                  GPIO_Pin_5 | GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    
    key_next.state = KEY_IDLE;
    key_next.event = KEY_EVENT_NONE;
    key_next.counter = 0;
    key_next.read_pin = Read_PA1;

    key_enter.state = KEY_IDLE;
    key_enter.event = KEY_EVENT_NONE;
    key_enter.counter = 0;
    key_enter.read_pin = Read_PA2;

    key_up.state = KEY_IDLE;
    key_up.event = KEY_EVENT_NONE;
    key_up.counter = 0;
    key_up.read_pin = Read_PA3;


    key_left.state = KEY_IDLE;
    key_left.event = KEY_EVENT_NONE;
    key_left.counter = 0;
    key_left.read_pin = Read_PA5;

    key_right.state = KEY_IDLE;
    key_right.event = KEY_EVENT_NONE;
    key_right.counter = 0;
    key_right.read_pin = Read_PA6;
}


void Key_Scan(Key_t *key)
{
    u8 raw = key->read_pin(); 

    switch (key->state)
    {
        case KEY_IDLE:
            if (raw == 0) {
                key->state = KEY_PRESS_DEBOUNCE;
                key->counter = 0;
            }
            break;

        case KEY_PRESS_DEBOUNCE:
            if (raw == 0) {
                key->counter++;
                if (key->counter >= 2) {
                    key->event = KEY_EVENT_DOWN;
                    key->state = KEY_PRESSED;
                }
            } else {
                key->state = KEY_IDLE;
            }
            break;

        case KEY_PRESSED:
            if (raw == 1) {
                key->state = KEY_RELEASE_DEBOUNCE;
                key->counter = 0;
            }
            break;

        case KEY_RELEASE_DEBOUNCE:
            if (raw == 1) {
                key->counter++;
                if (key->counter >= 2) {
                    key->event = KEY_EVENT_UP;
                    key->state = KEY_IDLE;
                }
            } else {
                key->state = KEY_PRESSED;
            }
            break;
    }
}


KeyEvent_t Key_GetEvent(Key_t *key)
{
    KeyEvent_t e = key->event;
    key->event = KEY_EVENT_NONE;
    return e;
}
