
#ifndef __LED_H__
#define __LED_H__

#define GPIO_LED0    16
#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<GPIO_LED0))

void ledInit(void);
#endif

