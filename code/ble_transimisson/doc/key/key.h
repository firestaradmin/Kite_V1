
#ifndef __KEY_H__
#define __KEY_H__



typedef enum 
{
	KEY_RELEASED = 0,
	KEY_PRESSED = ~ KEY_RELEASED,
}keyState_t;

typedef union {
    struct{
        unsigned char click:1 ;
        unsigned char longPressedClick:1 ;
        unsigned char doubleClick:1 ;
        unsigned char isLongPressing:1 ;
    };
    uint8_t value;
} key_event_t;


typedef struct key_struct{
    int gpio_num;
    keyState_t state;
    uint32_t lastPressedTime;
    key_event_t event;
} key_struct_t;

void key_init(void);



#endif

