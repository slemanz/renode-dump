#include "led.h"
#include "driver_gpio.h"

void led_init(void)
{
    GPIO_Handle_t GpioLed;
	GpioLed.pGPIOx = GPIOC;
	GpioLed.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_13;
	GpioLed.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
	GpioLed.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_LOW;
	GpioLed.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
	GpioLed.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;

    GPIO_Init(&GpioLed);
}

void led_on(void)
{
    GPIO_WriteToOutputPin(GPIOC, GPIO_PIN_NO_13, GPIO_PIN_SET);
}

void led_off(void)
{
    GPIO_WriteToOutputPin(GPIOC, GPIO_PIN_NO_13, GPIO_PIN_RESET);
}

void led_toggle(void)
{
    GPIO_ToggleOutputPin(GPIOC, GPIO_PIN_NO_13);
}