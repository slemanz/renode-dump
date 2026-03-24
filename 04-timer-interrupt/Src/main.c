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
/*  Main                                                                       */
/* -------------------------------------------------------------------------- */

int main(void)
{

    while(1)
    {
        
    }
}