#include "stm32f411xx.h"
#include "driver_i2c.h"
#include "driver_gpio.h"

void i2c1_init(void)
{
    GPIO_Handle_t i2cPins;
    i2cPins.pGPIOx = GPIOB;
    i2cPins.GPIO_PinConfig.GPIO_PinNumber       = GPIO_PIN_NO_8;
    i2cPins.GPIO_PinConfig.GPIO_PinMode         = GPIO_MODE_ALTFN;
    i2cPins.GPIO_PinConfig.GPIO_PinAltFunMode   = GPIO_PIN_ALTFN_4;
    i2cPins.GPIO_PinConfig.GPIO_PinPuPdControl  = GPIO_PIN_PU;
    i2cPins.GPIO_PinConfig.GPIO_PinOPType       = GPIO_OP_TYPE_OD;
    i2cPins.GPIO_PinConfig.GPIO_PinSpeed        = GPIO_SPEED_FAST;
    GPIO_Init(&i2cPins);

    i2cPins.GPIO_PinConfig.GPIO_PinNumber       = GPIO_PIN_NO_9;
    GPIO_Init(&i2cPins);

	I2C1_PCLK_EN(); /*Enable clock access to I2C1*/
	I2C1->CR1 |= (1U<<15); /*Enter reset mode*/
	I2C1->CR1 &=~(1U<<15); /*Come out of reset mode*/

	/*Set the peripheral clock*/
	I2C1->CR2 = (1U<<4); // 16Mhz
	I2C1->CCR =	I2C_100KHZ; /*Set I2C to standard mode, 100kH clock*/
	I2C1->TRISE = SD_MOD_MAX_RISE_TIME; /*Set rise time*/

	I2C1->CR1 |=(1U<<0); /*Enable I2C module*/
}

void i2c1_byte_read(char saddr,char maddr, char *data)
{
	volatile int tmp;

	while (I2C1->SR2 & (SR2_BUSY)){} /*Wait until is BUS  not busy*/

    I2C1->CR1 |= CR1_START; /*Generate start*/
    while (!(I2C1->SR1 & (SR1_SB))){} /*Wait until start flag is set*/
    I2C1->DR = saddr << 1; /*Transmit slave address + write*/
	while (!(I2C1->SR1 & (SR1_ADDR))){} /*Wait until address flag is set*/
    tmp = I2C1->SR2; /*Clear addr flag*/
    (void)tmp;

	I2C1->DR = maddr; /*Send memory address*/
	while (!(I2C1->SR1 & SR1_TXE)){} /*Wait until transmitter is empty*/
    I2C1->CR1 |= CR1_START; /*Generate restart*/
    while (!(I2C1->SR1 & SR1_SB)){} /*Wait until start flag is set*/

    I2C1->DR = saddr << 1 | 1; /*Transmit slave address + read*/

    while (!(I2C1->SR1 & (SR1_ADDR))){} /*Wait until address flag is set*/
    I2C1->CR1 &= ~CR1_ACK; /*Disable the Acknowledge*/
	tmp = I2C1->SR2; /*Clear addr flag*/
	I2C1->CR1 |= CR1_STOP; /*Generate stop after data is received*/
	while (!(I2C1->SR1 & SR1_RXNE)){} /*Wait until RXNE flag is set*/
	
    *data++  =  I2C1->DR; /*Read data from DR*/

}

void i2c1_burst_read(char saddr, char maddr, char *data, int n)
{

    volatile int tmp;

    while (I2C1->SR2 & (SR2_BUSY)){} /*Wait until is BUS  not busy*/
    I2C1->CR1 |= CR1_START; /*Generate start*/
    while (!(I2C1->SR1 & (SR1_SB))){} /*Wait until start flag is set*/

    I2C1->DR = saddr << 1; /*Transmit slave address + write*/
    while (!(I2C1->SR1 & (SR1_ADDR))){} /*Wait until address flag is set*/
    tmp = I2C1->SR2; /*Clear addr flag*/
    while (!(I2C1->SR1 & SR1_TXE)){} /*Wait until transmitter is empty*/

    I2C1->DR = maddr; /*Send memory address*/
    I2C1->CR1 |= CR1_START; /*Generate restart*/
    while (!(I2C1->SR1 & SR1_SB)){} /*Wait until start flag is set*/
    I2C1->DR = saddr << 1 | 1; /*Transmit slave address + read*/
    while (!(I2C1->SR1 & (SR1_ADDR))){} /*Wait until address flag is set*/
 	tmp = I2C1->SR2; /*Clear addr flag*/
    (void)tmp;

    I2C1->CR1 |= CR1_ACK; /*Disable the Acknowledge*/

    while(n > 0)
    {
        if(n == 1)
        {
            I2C1->CR1 &= ~CR1_ACK; /*Disable the Acknowledge*/
            I2C1->CR1 |= CR1_STOP; /*Generate stop after data is received*/
            while (!(I2C1->SR1 & SR1_RXNE)){} /*Wait until RXNE flag is set*/
            *data++  =  I2C1->DR; /*Read data from DR*/
            break;
    	}
        else
        {
            while (!(I2C1->SR1 & SR1_RXNE)){} /*Wait until RXNE flag is set*/
            *data++  =  I2C1->DR; /*Read data from DR*/
            n--;
        }
    }
}

void i2c1_write_byte(char saddr, char maddr, char data)
{
    volatile int tmp;

    while (I2C1->SR2 & (SR2_BUSY)){} /*Wait until is BUS  not busy*/
    I2C1->CR1 |= CR1_START; /*Generate start*/
    while (!(I2C1->SR1 & (SR1_SB))){} /*Wait until start flag is set*/

    I2C1->DR = saddr << 1; /*Transmit slave address + write*/
    while (!(I2C1->SR1 & (SR1_ADDR))){} /*Wait until address flag is set*/
    tmp = I2C1->SR2; /*Clear addr flag*/
    (void)tmp;
    
    while (!(I2C1->SR1 & SR1_TXE)){} /*Wait until transmitter is empty*/
    I2C1->DR = maddr; /*Send memory address*/
    I2C1->DR = data; /*Send data*/
    while (!(I2C1->SR1 & (SR1_BTF))){} /* Wait until transfer finished */

    I2C1->CR1 |= CR1_STOP; /* Generate stop */
}

void i2c1_burst_write(char saddr,char maddr, char *data, int n)
{
    volatile int tmp;

    while (I2C1->SR2 & (SR2_BUSY)){} /*Wait until is BUS  not busy*/
    I2C1->CR1 |= CR1_START; /*Generate start*/
    while (!(I2C1->SR1 & (SR1_SB))){} /*Wait until start flag is set*/

    I2C1->DR = saddr << 1; /*Transmit slave address + write*/
    while (!(I2C1->SR1 & (SR1_ADDR))){} /*Wait until address flag is set*/
    tmp = I2C1->SR2; /*Clear addr flag*/
    while (!(I2C1->SR1 & SR1_TXE)){} /*Wait until transmitter is empty*/
    (void)tmp;

    I2C1->DR = maddr; /*Send memory address*/

    for(int i =0; i < n; i++)
    {
        while (!(I2C1->SR1 & SR1_TXE)){} /*Wait until transmitter is empty*/
        I2C1->DR = *data++; /*Transmit data*/
	}
    while (!(I2C1->SR1 & (SR1_BTF))){} /* Wait until transfer finished */
    I2C1->CR1 |= CR1_STOP; /* Generate stop */
}