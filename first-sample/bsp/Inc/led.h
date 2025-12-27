#ifndef INC_LED_H_
#define INC_LED_H_

#include <stdint.h>

void led_init(void);

void led_on(void);
void led_off(void);
void led_toggle(void);

#endif /* INC_LED_H_ */