#ifndef INC_DRIVER_SPI_H_
#define INC_DRIVER_SPI_H_

#include "stm32f411xx.h"

/*
    PA5 - SPI1_SCK
    PA6 - SPI1_MISO
    PA7 - SPI1_MOSI
*/

/*
 * Defines Regs
 */

#define SPI_CR1_BIDIMODE            (1U << 15)
#define SPI_CR1_BIDIOE              (1U << 14)
#define SPI_CR1_CRCEN               (1U << 13)
#define SPI_CR1_DFF                 (1U << 11)
#define SPI_CR1_RXONLY              (1U << 10)
#define SPI_CR1_SSM                 (1U << 9)
#define SPI_CR1_SSI                 (1U << 8)
#define SPI_CR1_LSBFIRST            (1U << 7)
#define SPI_CR1_SPE                 (1U << 6)
#define SPI_CR1_MSTR                (1U << 2)
#define SPI_CR1_CPOL                (1U << 1)
#define SPI_CR1_CPHA                (1U << 0)

#define SPI_CR1_BR_Msk              (0x7UL << 3U)

#define SPI_CR1_BR_2                (0U << 3)
#define SPI_CR1_BR_4                (1U << 3)

#define SPI_CR2_SSOE                (1U << 2)
#define SPI_CR2_FRF                 (1U << 4)


#define SPI_SR_RXNE                 (1U << 0)
#define SPI_SR_TXE                  (1U << 1)
#define SPI_SR_OVR                  (1U << 6)
#define SPI_SR_BSY                  (1U << 7)

/*
 * Structs and Enums
 */

typedef struct
{
   uint32_t Mode;
   uint32_t Direction;
   uint32_t DataSize;
   uint32_t CLKPolarity;
   uint32_t CLKPhase;
   uint32_t NSS;
   uint32_t BaudRatePrescaler;
   uint32_t FirstBit;
   uint32_t TIMode;
   uint32_t CRCCalculation;
}SPI_Init_t;

typedef enum
{
    SPI_STATE_RESET     = 0x00,
    SPI_STATE_READY     = 0x01,
    SPI_STATE_BUSY      = 0x02,
    SPI_STATE_BUSY_TX   = 0x03,
    SPI_STATE_BUSY_RX   = 0x04
}SPI_State_e;

typedef enum
{
    DEV_OK      = 0x00,
    DEV_ERROR   = 0x01,
    DEV_BUSY    = 0x02,
    DEV_TIMEOUT = 0x03
}SPI_Status_t;

typedef struct
{
    SPI_RegDef_t    *Instance;
    SPI_Init_t      Init;
    uint8_t         *pTxBuffPtr;
    uint16_t        TxXferSize;
    __vo uint16_t   TxXferCount;
    uint8_t         *pRxBuffPtr;
    uint16_t        RxXferSize;
    __vo uint16_t   RxXferCount;
    SPI_State_e     State;
    __vo uint32_t   ErrorCode;
}SPI_Handle_t;

/*
 * Defines
 */

#define SPI_ERROR_NONE              (0x00000000U)

#define SPI_MODE_SLAVE              (0x00000000U)
#define SPI_MODE_MASTER             (SPI_CR1_MSTR | SPI_CR1_SSI)

#define SPI_DIRECTION_2LINES        (0x00000000U)
#define SPI_DIRECTION_1LINES        SPI_CR1_BIDIMODE

#define SPI_DATASIZE_8BIT           (0x00000000U)
#define SPI_DATASIZE_16BIT          SPI_CR1_DFF

#define SPI_POLARITY_LOW            (0x00000000U)
#define SPI_POLARITY_HIGH           SPI_CR1_CPOL

#define SPI_PHASE_1EDGE             (0x00000000U)
#define SPI_NSS_SOFT                SPI_CR1_SSM

#define SPI_BAUDRATEPRESCALER_4     (SPI_CR1_BR_4)
#define SPI_FIRSTBIT_MSB            (0x00000000U)
#define SPI_TIMODE_DISABLE          (0x00000000U)

#define SPI_CRCCALCULATION_DISABLE  (0x00000000U)

#define SPI_FLAG_RXNE               SPI_SR_RXNE
#define SPI_FLAG_TXE                SPI_SR_TXE
#define SPI_FLAG_BSY                SPI_SR_BSY
#define SPI_FLAG_OVR                SPI_SR_OVR

/*
 * Functions
 */

void spi_gpio_init(void);
SPI_Status_t spi_init(SPI_Handle_t *hspi);
SPI_Status_t spi_transmit(SPI_Handle_t *hspi, uint8_t *p_data, uint16_t size, uint64_t timeout);
SPI_Status_t spi_receive(SPI_Handle_t *hspi,uint8_t *p_data, uint16_t size,uint32_t timeout);

#endif /* INC_DRIVER_SPI_H_ */