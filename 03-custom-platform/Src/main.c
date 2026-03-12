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

static void gpio_init(void)
{
    /* PA5 — Status LED (output, push-pull) */
    GPIO_PinConfig_t led_status = {
        .pGPIOx = GPIOA, .GPIO_PinNumber = GPIO_PIN_NO_5,
        .GPIO_PinMode = GPIO_MODE_OUT, .GPIO_PinSpeed = GPIO_SPEED_LOW,
        .GPIO_PinOPType = GPIO_OP_TYPE_PP, .GPIO_PinPuPdControl = GPIO_NO_PUPD,
        .GPIO_PinAltFunMode = GPIO_PIN_NO_ALTFN
    };
    GPIO_Init(&led_status);

    /* PB3 — Error LED (output, push-pull) */
    GPIO_PinConfig_t led_error = {
        .pGPIOx = GPIOB, .GPIO_PinNumber = GPIO_PIN_NO_3,
        .GPIO_PinMode = GPIO_MODE_OUT, .GPIO_PinSpeed = GPIO_SPEED_LOW,
        .GPIO_PinOPType = GPIO_OP_TYPE_PP, .GPIO_PinPuPdControl = GPIO_NO_PUPD,
        .GPIO_PinAltFunMode = GPIO_PIN_NO_ALTFN
    };
    GPIO_Init(&led_error);

    /* PA0 — User Button (input, pull-up — press = low) */
    GPIO_PinConfig_t button = {
        .pGPIOx = GPIOA, .GPIO_PinNumber = GPIO_PIN_NO_0,
        .GPIO_PinMode = GPIO_MODE_IN, .GPIO_PinSpeed = GPIO_SPEED_LOW,
        .GPIO_PinOPType = GPIO_OP_TYPE_PP, .GPIO_PinPuPdControl = GPIO_PIN_PU,
        .GPIO_PinAltFunMode = GPIO_PIN_NO_ALTFN
    };
    GPIO_Init(&button);
}

/* -------------------------------------------------------------------------- */
/*  Helpers                                                                   */
/* -------------------------------------------------------------------------- */

static uint8_t button_is_pressed(void)
{
    /* Button model drives pin low when pressed, pull-up keeps it high otherwise */
    return (GPIO_ReadFromInputPin(GPIOA, GPIO_PIN_NO_0) == 0u);
}

/* -------------------------------------------------------------------------- */
/*  Main                                                                      */
/* -------------------------------------------------------------------------- */

int main(void)
{
    systick_init(TICK_HZ);
    uart2_init();
    gpio_init();

    uart_puts("Platform ready\r\n");

    uint64_t last_blink = ticks_get();
    uint8_t  error_active = 0;

    while(1)
    {
        /* Blink status LED every 500ms */
        if ((ticks_get() - last_blink) >= 500u)
        {
            GPIO_ToggleOutputPin(GPIOA, GPIO_PIN_NO_5);
            last_blink = ticks_get();
        }

        /* Button pressed → turn on error LED and report */
        if (button_is_pressed() && !error_active)
        {
            GPIO_WriteToOutputPin(GPIOB, GPIO_PIN_NO_3, 1);
            uart_puts("Error ON\r\n");
            error_active = 1;
        }

        /* Button released → turn off error LED */
        if (!button_is_pressed() && error_active)
        {
            GPIO_WriteToOutputPin(GPIOB, GPIO_PIN_NO_3, 0);
            uart_puts("Error OFF\r\n");
            error_active = 0;
        }
    }
}