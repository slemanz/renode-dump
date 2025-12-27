#ifndef INC_DRIVER_I2C_H_
#define INC_DRIVER_I2C_H_

#include "stm32f411xx.h"

/*
 * PB8 - I2C1_SCL
 * PB9 - I2C1_SDA
 */

/*
 * Defines Regs
 */

#define I2C_100KHZ					80    // 0b 0101 0000
#define SD_MOD_MAX_RISE_TIME		17

#define SR2_BUSY                    (1U<<1)
#define CR1_START                   (1U<<8)
#define	SR1_SB                      (1U<<0)
#define	SR1_ADDR                    (1U<<1)
#define	SR1_TXE                     (1U<<7)
#define	CR1_ACK                     (1U<<10)
#define CR1_STOP                    (1U<<9)
#define	SR1_RXNE                    (1U<<6)
#define	SR1_BTF                     (1U<<2)

/*
 *  Functions
 */


void i2c1_init(void);
void i2c1_byte_read(char saddr,char maddr, char *data);
void i2c1_burst_read(char saddr, char maddr, char *data, int n);
void i2c1_write_byte(char saddr, char maddr, char data);
void i2c1_burst_write(char saddr,char maddr, char *data, int n);

#endif /* INC_DRIVER_I2C_H_ */