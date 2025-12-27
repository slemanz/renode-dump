#ifndef DRIVER_UART_H_
#define DRIVER_UART_H_

#include "stm32f411xx.h"

#define UART_CR1_TE					(1U<<3)
#define UART_CR1_UE					(1U<<13)
#define UART_SR_TXE					(1U<<7)

void uart2_init(void);
void uart2_init_pins(void);
void uart2_write_byte(uint8_t data);

#endif