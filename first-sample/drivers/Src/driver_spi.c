#include "driver_spi.h"
#include "driver_gpio.h"
#include "driver_systick.h"
#include "stddef.h"

void spi_gpio_init(void)
{
    /* Config PA5,PA6,PA7*/
    GPIO_Handle_t spiPins;
    spiPins.GPIO_PinConfig.GPIO_PinAltFunMode = GPIO_MODE_ALTFN;
    spiPins.GPIO_PinConfig.GPIO_PinMode = GPIO_PIN_ALTFN_5;
    spiPins.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
    spiPins.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;

    spiPins.pGPIOx = GPIOA;
    spiPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_5;
    GPIO_Init(&spiPins);

    spiPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_6;
    GPIO_Init(&spiPins);

    spiPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_7;
    GPIO_Init(&spiPins);

    SPI1_PCLK_EN();
}

SPI_Status_t spi_init(SPI_Handle_t *hspi)
{
    /* Check SPI Handle allocation */
    if(hspi == NULL)
    {
        return DEV_ERROR;
    }

    hspi->Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE; /* Disable CRC Calculation*/

    if(hspi->State == SPI_STATE_RESET)
    {
        spi_gpio_init();
    }

    hspi->State = SPI_STATE_BUSY;
    CLEAR_BIT(hspi->Instance->CR1, SPI_CR1_SPE);

    /* Configure CR1 */
    WRITE_REG(hspi->Instance->CR1, ((hspi->Init.Mode & (SPI_CR1_MSTR | SPI_CR1_SSI)) |
                                    (hspi->Init.Direction & (SPI_CR1_RXONLY | SPI_CR1_BIDIMODE)) |
                                    (hspi->Init.DataSize & SPI_CR1_DFF) |
                                    (hspi->Init.CLKPolarity & SPI_CR1_CPOL) |
                                    (hspi->Init.CLKPhase & SPI_CR1_CPHA) |
                                    (hspi->Init.NSS & SPI_CR1_SSM) |
                                    (hspi->Init.BaudRatePrescaler & SPI_CR1_BR_Msk) |
                                    (hspi->Init.FirstBit  & SPI_CR1_LSBFIRST) |
                                    (hspi->Init.CRCCalculation & SPI_CR1_CRCEN)));
    /* Configure : CR2 */
    WRITE_REG(hspi->Instance->CR2, (((hspi->Init.NSS >> 16U) & SPI_CR2_SSOE) | (hspi->Init.TIMode & SPI_CR2_FRF)));

    hspi->ErrorCode =  SPI_ERROR_NONE;
    hspi->State =  SPI_STATE_READY;
    return DEV_OK;
}

SPI_Status_t spi_transmit(SPI_Handle_t *hspi, uint8_t *p_data, uint16_t size, uint64_t timeout)
{
    uint64_t tickstart;
    SPI_Status_t error_code = DEV_OK;
    uint16_t tx_xfer_cnt = size;

    tickstart = ticks_get();

    if(hspi->State != SPI_STATE_READY)
    {
        error_code = DEV_BUSY;
        hspi->State = SPI_STATE_READY;
        return error_code;
    }

    if((p_data == NULL) || (size == 0))
    {
        error_code = DEV_ERROR;
        hspi->State = SPI_STATE_READY;
        return error_code;
    }

    /* Set the transaction information */
    hspi->State = SPI_STATE_BUSY_TX;
    hspi->ErrorCode = SPI_ERROR_NONE;
    hspi->pTxBuffPtr = (uint8_t*)p_data;
    hspi->TxXferSize = size;
    hspi->TxXferCount = size;
    
    hspi->pRxBuffPtr = (uint8_t*)NULL;
    hspi->RxXferSize = 0;
    hspi->RxXferCount = 0;

    /* Configure communication direction */
    if(hspi->Init.Direction == SPI_DIRECTION_1LINES)
    {
        CLEAR_BIT(hspi->Instance->CR1, SPI_CR1_SPE);
        SET_BIT(hspi->Instance->CR1, SPI_CR1_BIDIOE); /* Set 1 line TX */
    }

    if((hspi->Instance->CR1) != SPI_CR1_SPE)
    {
        SET_BIT(hspi->Instance->CR1, SPI_CR1_SPE);
    }

    /* Transmit data in 16 bit mode */
    if(hspi->Init.DataSize == SPI_DATASIZE_16BIT)
    {
        if((hspi->Init.Mode == SPI_MODE_SLAVE) || (tx_xfer_cnt == 1))
        {
            hspi->Instance->DR = *((uint16_t*)hspi->pTxBuffPtr);
            hspi->pTxBuffPtr += sizeof(uint16_t);
            hspi->TxXferCount--;
        }

        while(hspi->TxXferCount > 0)
        {
            /* Check if TXE flag to bet set and then send data */
            if(hspi->Instance->SR & (SPI_FLAG_TXE))
            {
                hspi->Instance->DR = *((uint16_t*)hspi->pTxBuffPtr);
                hspi->pTxBuffPtr += sizeof(uint16_t);
                hspi->TxXferCount--;
            }else
            {
                if((((ticks_get() - tickstart) > timeout) && (timeout != MAX_DELAY)) || (timeout == 0))
                {
                    error_code = DEV_TIMEOUT;
                    hspi->State = SPI_STATE_READY;
                    return error_code;
                }
            }
        }
    }
    /* Transmit data in 8 bit mode */
    else
    {
        if((hspi->Init.Mode == SPI_MODE_SLAVE) || (tx_xfer_cnt == 1))
        {
            *((__vo uint8_t*)hspi->Instance->DR) = (*hspi->pTxBuffPtr);
            hspi->pTxBuffPtr += sizeof(uint8_t);
            hspi->TxXferCount--;
        }
        else
        {
            /* Check if TXE flag to bet set and then send data */
            if(hspi->Instance->SR & (SPI_FLAG_TXE))
            {
                *((uint8_t*)hspi->Instance->DR) = (*hspi->pTxBuffPtr);
                hspi->pTxBuffPtr += sizeof(uint8_t);
                hspi->TxXferCount--;
            }else
            {
                if((((ticks_get() - tickstart) > timeout) && (timeout != MAX_DELAY)) || (timeout == 0))
                {
                    error_code = DEV_TIMEOUT;
                    hspi->State = SPI_STATE_READY;
                    return error_code;
                }
            }
        }
    }

    if(hspi->ErrorCode != SPI_ERROR_NONE)
    {
        error_code = DEV_ERROR;
    }

    hspi->State = SPI_STATE_READY;
    return error_code;
}

SPI_Status_t spi_receive(SPI_Handle_t *hspi,uint8_t *p_data, uint16_t size,uint32_t timeout)
{
	uint32_t tickstart;
	SPI_Status_t error_code = DEV_OK;

	tickstart = ticks_get();

	if(hspi->State != SPI_STATE_READY)
	{
		error_code = DEV_BUSY;
		hspi->State = SPI_STATE_READY;
		return error_code;
	}

	if((p_data == NULL) || (size  == 0))
	{
		error_code =  DEV_ERROR;
		hspi->State = SPI_STATE_READY;
		return error_code;
	}

	/*Set the transaction information*/
	hspi->State  = SPI_STATE_BUSY_RX;
	hspi->ErrorCode = SPI_ERROR_NONE;
	hspi->pRxBuffPtr = (uint8_t *)p_data;
	hspi->RxXferSize =  size;
	hspi->RxXferCount=  size;


	hspi->pTxBuffPtr = (uint8_t *)NULL;
	hspi->TxXferSize =  0;
	hspi->TxXferCount=  0;

	if((hspi->Instance->CR1 & SPI_CR1_SPE ) !=  SPI_CR1_SPE)
	{
		SET_BIT(hspi->Instance->CR1, SPI_CR1_SPE);
	}

	/*Receive data in 8 bit mode*/
	if(hspi->Init.DataSize == SPI_DATASIZE_8BIT)
	{
		while(hspi->RxXferCount > 0)
		{
			if(hspi->Instance->SR &  (SPI_FLAG_RXNE))
			{
				*((uint8_t *)hspi->pRxBuffPtr) = *(__vo uint8_t *)&hspi->Instance->DR;
				hspi->pRxBuffPtr += sizeof(uint8_t);
				hspi->RxXferCount--;
			}
			else
			{
				if((((ticks_get() - tickstart) >= timeout )&&(timeout != MAX_DELAY)) || (timeout == 0 ))
		         {
					error_code = DEV_TIMEOUT;
					hspi->State = SPI_STATE_READY;
					return error_code;
		           }
			}
		}
	}
	else
	{
		while(hspi->RxXferCount > 0)
		{
			if(hspi->Instance->SR &  (SPI_FLAG_RXNE))
			{
				*((uint16_t *)hspi->pRxBuffPtr) = ( uint16_t)hspi->Instance->DR;
				hspi->pRxBuffPtr += sizeof(uint16_t);
				hspi->RxXferCount--;
			}
			else
			{
				if((((ticks_get() - tickstart) >= timeout )&&(timeout != MAX_DELAY)) || (timeout == 0 ))
		         {
					error_code = DEV_TIMEOUT;
					hspi->State = SPI_STATE_READY;
					return error_code;
		           }
			}
		}
	}
	if(hspi->ErrorCode  != SPI_ERROR_NONE )
	{
		error_code = DEV_ERROR;
	}

	hspi->State =  SPI_STATE_READY;
	return error_code;
}