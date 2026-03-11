#include "driver_gpio.h"
#include "driver_uart.h"
#include "driver_systick.h"
#include "driver_clock.h"
#include <string.h>

/* -------------------------------------------------------------------------- */
/*  UART2 (PA2=TX, PA3=RX) - debug output                                     */
/* -------------------------------------------------------------------------- */

static void uart2_init(void)
{
    GPIO_PinConfig_t tx = {
        .pGPIOx = GPIOA, .GPIO_PinNumber = GPIO_PIN_NO_2,
        .GPIO_PinMode = GPIO_MODE_ALTFN, .GPIO_PinSpeed = GPIO_SPEED_FAST,
        .GPIO_PinOPType = GPIO_OP_TYPE_PP, .GPIO_PinPuPdControl = GPIO_PIN_PU,
        .GPIO_PinAltFunMode = PA2_ALTFN_UART2_TX
    };
    GPIO_Init(&tx);

    GPIO_PinConfig_t rx = {
        .pGPIOx = GPIOA, .GPIO_PinNumber = GPIO_PIN_NO_3,
        .GPIO_PinMode = GPIO_MODE_ALTFN, .GPIO_PinSpeed = GPIO_SPEED_FAST,
        .GPIO_PinOPType = GPIO_OP_TYPE_PP, .GPIO_PinPuPdControl = GPIO_PIN_PU,
        .GPIO_PinAltFunMode = PA3_ALTFN_UART2_RX
    };
    GPIO_Init(&rx);

    UART_Config_t cfg = {
        .pUARTx = UART2, .UART_Baud = UART_STD_BAUD_115200,
        .UART_Mode = UART_MODE_TXRX, .UART_WordLength = UART_WORDLEN_8BITS,
        .UART_NoOfStopBits = UART_STOPBITS_1, .UART_ParityControl = UART_PARITY_DISABLE,
        .UART_HWFlowControl = UART_HW_FLOW_CTRL_NONE
    };
    UART_Init(&cfg);
    UART_PeripheralControl(UART2, ENABLE);
}

static void uart_puts(const char *str)
{
    UART_Write(UART2, (const uint8_t *)str, strlen(str));
}

/* -------------------------------------------------------------------------- */
/*  GPIO: Status LED (PA5), Error LED (PB3), Button (PA0)                     */
/* -------------------------------------------------------------------------- */

int main(void)
{
    systick_init(TICK_HZ);
    uart2_init();

    while(1)
    {

    }
}