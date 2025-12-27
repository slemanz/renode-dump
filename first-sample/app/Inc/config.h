#ifndef INC_CONFIG_H_
#define INC_CONFIG_H_

#include "stdint.h"

#define LED_PORT        GPIOC
#define LED_PIN         GPIO_PIN_NO_13

void config_drivers(void);
void config_bsp(void);

#endif /* INC_CONFIG_H_ */