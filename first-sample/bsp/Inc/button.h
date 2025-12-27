#ifndef INC_BUTTON_H_
#define INC_BUTTON_H_

#include <stdint.h>
#include <stdbool.h>

void button_init(void);
bool button_get_state(void);

#endif /* INC_BUTTON_H_ */