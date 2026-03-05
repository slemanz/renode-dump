#include "driver_gpio.h"
#include "driver_systick.h"

/* -------------------------------------------------------------------------- */
/*  GPIO setup: PA5 as push-pull output                                       */
/* -------------------------------------------------------------------------- */

static void pa5_init(void)
{
    GPIO_PinConfig_t cfg;
    cfg.pGPIOx              = GPIOA;
    cfg.GPIO_PinNumber      = GPIO_PIN_NO_5;
    cfg.GPIO_PinSpeed       = GPIO_SPEED_FAST;
    cfg.GPIO_PinOPType      = GPIO_OP_TYPE_PP;
    cfg.GPIO_PinPuPdControl = GPIO_NO_PUPD;
    cfg.GPIO_PinAltFunMode  = GPIO_PIN_NO_ALTFN;
    GPIO_Init(&cfg);
}

/* -------------------------------------------------------------------------- */
/*  Main: toggle LED every 500 ms using SysTick                               */
/* -------------------------------------------------------------------------- */

int main(void)
{
    systick_init(TICK_HZ);
    pa5_init();

    uint64_t last_toggle = ticks_get();

    while(1)
    {
        if((ticks_get() - last_toggle) >= 500u)
        {
            GPIO_ToggleOutputPin(GPIOA, GPIO_PIN_NO_5);
            last_toggle = ticks_get();
        }
    }
}