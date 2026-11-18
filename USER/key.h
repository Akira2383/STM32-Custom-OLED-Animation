#ifndef __KEY_H
#define __KEY_H
#include "stm32f10x.h"


typedef enum {
    KEY_IDLE = 0,       
    KEY_PRESS_DEBOUNCE, 
    KEY_PRESSED,        
    KEY_RELEASE_DEBOUNCE
} KeyState_t;


typedef enum {
    KEY_EVENT_NONE = 0, 
    KEY_EVENT_DOWN,     
    KEY_EVENT_UP        
} KeyEvent_t;


typedef struct {
    KeyState_t state;
    KeyEvent_t event;
    u8 counter;
    u8 (*read_pin)(void); 
} Key_t;


extern Key_t key_next;  
extern Key_t key_enter; 
extern Key_t key_up;
extern Key_t key_left;
extern Key_t key_right;

void Key_Init(void);
void Key_Scan(Key_t *key);
KeyEvent_t Key_GetEvent(Key_t *key);

#endif
