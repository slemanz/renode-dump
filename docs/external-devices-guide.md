# Creating External Device Models in Renode

How to simulate sensors, actuators, and external chips that Renode doesn't
include out of the box. Covers Python peripherals (quick), C# peripherals
(powerful), and how to decide which to use.

## When Do You Need a Custom Device?

enode ships with models for many common MCU peripherals (GPIO, UART, SPI, I2C,
Timer, ADC, CAN, etc.), but it does **not** include models for most external
chips. If your firmware talks to any of these, you need to create a model:

- Temperature/pressure sensors (TMP102, BMP280, LM75)
- Accelerometers/gyroscopes (ADXL345, MPU6050)
- CAN transceivers (MCP2515 via SPI)
- EEPROMs (AT24C02 via I2C)
- DACs (MCP4725 via I2C)
- Display controllers (SSD1306 via I2C/SPI)
- External ADCs (ADS1115 via I2C)
- Any proprietary ASIC on your board