#include "driver_gpio.h"
#include "driver_uart.h"
#include "driver_systick.h"
#include "driver_clock.h"
#include <stdio.h>
#include <string.h>

/* -------------------------------------------------------------------------- */
/*  UART2 instance (PA2=TX, PA3=RX on STM32F411 Nucleo)                      */
/* -------------------------------------------------------------------------- */

static UART_Config_t g_uart2_cfg;

static void uart2_init(void)
{
    /* Configure PA2 as UART2_TX (AF7) */
    GPIO_PinConfig_t tx_pin;
    tx_pin.pGPIOx              = GPIOA;
    tx_pin.GPIO_PinNumber      = GPIO_PIN_NO_2;
    tx_pin.GPIO_PinMode        = GPIO_MODE_ALTFN;
    tx_pin.GPIO_PinSpeed       = GPIO_SPEED_FAST;
    tx_pin.GPIO_PinOPType      = GPIO_OP_TYPE_PP;
    tx_pin.GPIO_PinPuPdControl = GPIO_PIN_PU;
    tx_pin.GPIO_PinAltFunMode  = PA2_ALTFN_UART2_TX;
    GPIO_Init(&tx_pin);

    /* Configure PA3 as UART2_RX (AF7) */
    GPIO_PinConfig_t rx_pin;
    rx_pin.pGPIOx              = GPIOA;
    rx_pin.GPIO_PinNumber      = GPIO_PIN_NO_3;
    rx_pin.GPIO_PinMode        = GPIO_MODE_ALTFN;
    rx_pin.GPIO_PinSpeed       = GPIO_SPEED_FAST;
    rx_pin.GPIO_PinOPType      = GPIO_OP_TYPE_PP;
    rx_pin.GPIO_PinPuPdControl = GPIO_PIN_PU;
    rx_pin.GPIO_PinAltFunMode  = PA3_ALTFN_UART2_RX;
    GPIO_Init(&rx_pin);

    /* Configure UART2: 115200 8N1 */
    g_uart2_cfg.pUARTx              = UART2;
    g_uart2_cfg.UART_Baud           = UART_STD_BAUD_115200;
    g_uart2_cfg.UART_Mode           = UART_MODE_TXRX;
    g_uart2_cfg.UART_WordLength     = UART_WORDLEN_8BITS;
    g_uart2_cfg.UART_NoOfStopBits   = UART_STOPBITS_1;
    g_uart2_cfg.UART_ParityControl  = UART_PARITY_DISABLE;
    g_uart2_cfg.UART_HWFlowControl  = UART_HW_FLOW_CTRL_NONE;

    UART_Init(&g_uart2_cfg);
    UART_PeripheralControl(UART2, ENABLE);
}

/* -------------------------------------------------------------------------- */
/*  printf redirection via _write (newlib)                                    */
/* -------------------------------------------------------------------------- */

int __io_putchar(int ch)
{
    UART_WriteByte(UART2, (uint8_t)ch);
    return ch;
}

/* -------------------------------------------------------------------------- */
/*  Simple string send (does not depend on printf/newlib)                     */
/* -------------------------------------------------------------------------- */

static void uart_puts(const char *str)
{
    UART_Write(UART2, (const uint8_t *)str, strlen(str));
}

/* -------------------------------------------------------------------------- */
/*  GPIO: PA5 as output (LED LD2)                                             */
/* -------------------------------------------------------------------------- */

static void led_init(void)
{
    GPIO_PinConfig_t cfg;
    cfg.pGPIOx              = GPIOA;
    cfg.GPIO_PinNumber      = GPIO_PIN_NO_5;
    cfg.GPIO_PinMode        = GPIO_MODE_OUT;
    cfg.GPIO_PinSpeed       = GPIO_SPEED_FAST;
    cfg.GPIO_PinOPType      = GPIO_OP_TYPE_PP;
    cfg.GPIO_PinPuPdControl = GPIO_NO_PUPD;
    cfg.GPIO_PinAltFunMode  = GPIO_PIN_NO_ALTFN;
    GPIO_Init(&cfg);
}

/* -------------------------------------------------------------------------- */
/*  Main: print messages over UART and toggle LED                             */
/* -------------------------------------------------------------------------- */

int main(void)
{
    systick_init(TICK_HZ);
    uart2_init();
    led_init();

    uart_puts("Boot OK\r\n");

    uint32_t count = 0;
    uint64_t last_tick = ticks_get();

    while (1)
    {
        if ((ticks_get() - last_tick) >= 1000u)
        {
            count++;
            GPIO_ToggleOutputPin(GPIOA, GPIO_PIN_NO_5);

            /* Using printf (depends on __io_putchar -> UART) */
            printf("[%lu] LED toggled\r\n", (unsigned long)count);

            last_tick = ticks_get();
        }
    }
}