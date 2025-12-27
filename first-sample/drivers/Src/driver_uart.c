#include "driver_uart.h"
#include "driver_gpio.h"


static uint16_t compute_uart_div(uint32_t PeriphClk, uint32_t BaudRate);

static uint16_t compute_uart_div(uint32_t PeriphClk, uint32_t BaudRate)
{
    return ((PeriphClk + (BaudRate/2U))/BaudRate);
}


void uart2_init(void)
{
    uart2_init_pins();
    UART2_PCLK_EN();

    // no flow control (default reset)
    uint32_t temp = ((UART_CR1_TE) | (1 << 2)); // tx and rx enable
    UART2->CR1 = temp; 
    UART2->BRR = compute_uart_div(16000000, 115200); // baurate

    UART2->CR1 |= UART_CR1_UE;// enable uart periph
}

void uart2_write_byte(uint8_t data)
{
	while(!(UART2->SR & UART_SR_TXE));
    UART2->DR = data;
}

void uart2_init_pins(void)
{
    GPIO_Handle_t UartPin;

    UartPin.pGPIOx = GPIOA;
    UartPin.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
    UartPin.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_2;
    UartPin.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
    UartPin.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
    UartPin.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
    UartPin.GPIO_PinConfig.GPIO_PinAltFunMode = PA2_ALTFN_UART2_TX;

    GPIO_Init(&UartPin);
}