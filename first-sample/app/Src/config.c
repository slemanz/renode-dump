#include "config.h"

#include "driver_fpu.h"
#include "driver_gpio.h"
#include "driver_adc.h"
#include "driver_uart.h"
#include "driver_systick.h"

#include "led.h"
#include "button.h"

void gpio_setup(void)
{
    // Set GPIOA pin 13 as output
    GPIO_Handle_t GpioLed;
	GpioLed.pGPIOx = LED_PORT;
	GpioLed.GPIO_PinConfig.GPIO_PinNumber = LED_PIN;
	GpioLed.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
	GpioLed.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_LOW;
	GpioLed.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
	GpioLed.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;

    GPIO_Init(&GpioLed);
    GPIO_WriteToOutputPin(LED_PORT, LED_PIN, GPIO_PIN_SET);
}


void config_drivers(void)
{
    fpu_enable();
    //gpio_setup();
    uart2_init();
    systick_init(1000);
    //adc_pa1_init();
}

void config_bsp(void)
{
    led_init();
    button_init();
}

// printf retarget
extern int __io_putchar(int ch)
{
    uart2_write_byte((uint8_t)ch);
    return ch;

}