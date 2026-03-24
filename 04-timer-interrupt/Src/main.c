#include "driver_gpio.h"
#include "driver_uart.h"
#include "driver_timer.h"
#include "driver_systick.h"
#include "driver_interrupt.h"
#include "driver_clock.h"
#include <stdio.h>
#include <string.h>

/* -------------------------------------------------------------------------- */
/*  IRQ number for TIM2 (STM32F411 vector table position 28)                  */
/* -------------------------------------------------------------------------- */

#define IRQ_NO_TIM2     28

/* -------------------------------------------------------------------------- */
/*  TIM DIER bit positions                                                     */
/* -------------------------------------------------------------------------- */

#define TIM_DIER_UIE    0u      /* Update Interrupt Enable */

/* -------------------------------------------------------------------------- */
/*  Globals: ISR sets flag, main loop prints                                   */
/* -------------------------------------------------------------------------- */

static volatile uint32_t g_tim2_ticks  = 0;
static volatile uint8_t  g_tim2_flag   = 0;

/* -------------------------------------------------------------------------- */
/*  TIM2 ISR — fires every 500ms                                              */
/* -------------------------------------------------------------------------- */

void TIM2_IRQHandler(void)
{
    /* Check update interrupt flag */
    if (TIM2->SR & (1u << TIM_SR_UIF))
    {
        /* Clear the flag */
        TIM2->SR &= ~(1u << TIM_SR_UIF);

        g_tim2_ticks++;
        g_tim2_flag = 1;

        /* Toggle LED directly in ISR for precise timing */
        GPIO_ToggleOutputPin(GPIOA, GPIO_PIN_NO_5);
    }
}

/* -------------------------------------------------------------------------- */
/*  UART2 setup                                                                */
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

int __io_putchar(int ch)
{
    UART_WriteByte(UART2, (uint8_t)ch);
    return ch;
}


/* -------------------------------------------------------------------------- */
/*  Main                                                                       */
/* -------------------------------------------------------------------------- */

int main(void)
{

    while(1)
    {
        
    }
}