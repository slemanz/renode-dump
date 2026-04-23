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
/*  LED setup (PA5)                                                            */
/* -------------------------------------------------------------------------- */

static void led_init(void)
{
    GPIO_PinConfig_t cfg = {
        .pGPIOx = GPIOA, .GPIO_PinNumber = GPIO_PIN_NO_5,
        .GPIO_PinMode = GPIO_MODE_OUT, .GPIO_PinSpeed = GPIO_SPEED_LOW,
        .GPIO_PinOPType = GPIO_OP_TYPE_PP, .GPIO_PinPuPdControl = GPIO_NO_PUPD,
        .GPIO_PinAltFunMode = GPIO_PIN_NO_ALTFN
    };
    GPIO_Init(&cfg);
}

/* -------------------------------------------------------------------------- */
/*  TIM2 setup — 500ms interrupt period                                        */
/*                                                                             */
/*  Clock = 16 MHz (HSI)                                                       */
/*  Prescaler = 15999 → timer clock = 16MHz / 16000 = 1 kHz                   */
/*  Period (ARR) = 499 → overflow every 500 counts = 500ms                     */
/* -------------------------------------------------------------------------- */

static TIM_Config_t g_tim2_cfg;

static void tim2_init(void)
{
    g_tim2_cfg.pTIMx     = TIM2;
    g_tim2_cfg.prescaler = 15999u;  /* 16MHz / 16000 = 1kHz tick */
    g_tim2_cfg.period    = 499u;    /* 500 ticks = 500ms         */

    TIM_PWM_Init(&g_tim2_cfg);

    /* Enable update interrupt (UIE bit in DIER register) */
    TIM2->DIER |= (1u << TIM_DIER_UIE);

    /* Enable TIM2 IRQ in NVIC */
    interrupt_Config(IRQ_NO_TIM2, ENABLE);

    /* Start the timer */
    TIM_Start(TIM2);
}


/* -------------------------------------------------------------------------- */
/*  Main                                                                       */
/* -------------------------------------------------------------------------- */

int main(void)
{
    systick_init(TICK_HZ);
    uart2_init();
    led_init();
    tim2_init();

    printf("TIM2 started\r\n");

    while (1)
    {
        if (g_tim2_flag)
        {
            g_tim2_flag = 0;
            printf("[TIM2] tick %lu\r\n", (unsigned long)g_tim2_ticks);
        }
    }
}