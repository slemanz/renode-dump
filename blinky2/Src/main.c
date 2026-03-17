#include "driver_gpio.h"

void pa5_init(void)
{
    GPIO_PinConfig_t gpio_config;
    gpio_config.pGPIOx = GPIOA;
    gpio_config.GPIO_PinNumber = GPIO_PIN_NO_5;
    gpio_config.GPIO_PinMode = GPIO_MODE_OUT;
    gpio_config.GPIO_PinSpeed = GPIO_SPEED_FAST;
    gpio_config.GPIO_PinOPType = GPIO_OP_TYPE_PP;
    gpio_config.GPIO_PinPuPdControl = GPIO_NO_PUPD;
    gpio_config.GPIO_PinAltFunMode = GPIO_PIN_NO_ALTFN;
    GPIO_Init(&gpio_config);
}

void delay(void)
{
    for(uint32_t i = 0; i < 5000000; i++);
}

int main(void)
{
    pa5_init();

    while(1)
    {
        GPIO_ToggleOutputPin(GPIOA, GPIO_PIN_NO_5);
        delay();
    }
}