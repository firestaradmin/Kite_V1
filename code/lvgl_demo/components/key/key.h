
#ifndef __KEY_H__
#define __KEY_H__

#define GPIO_KEY0   17
#define GPIO_KEY_PIN_SEL  (1ULL << GPIO_KEY0) 



void key_init(void);
#endif

