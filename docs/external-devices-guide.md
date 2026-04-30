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

## Python Peripherals (Quick Start)

Python peripherals are the fastest way to mock an external device. Single file,
no compilation, instant iteration.

### Minimal I2C Sensor

```python
# my_sensor.py — Minimal I2C peripheral template

value = 0

def write(data):
    """Called for each byte the master sends."""
    pass

def read():
    """Called for each byte the master reads. Return a uint8."""
    global value
    return value & 0xFF

def finish():
    """Called when the I2C transaction ends (STOP)."""
    pass

# Custom commands — callable from monitor
def SetValue(v):
    global value
    value = int(v)
    log("Value set to %d" % value)
```

### Loading It

In your `.resc` script:

```bash
machine PyDevFromFile @path/to/my_sensor.py 0x48 sysbus.i2c1 "sensor"
```

Parameters:
1. `@path/to/my_sensor.py` — Python file path
2. `0x48` — I2C 7-bit address
3. `sysbus.i2c1` — parent I2C bus
4. `"sensor"` — name in the peripheral tree

After loading: `sysbus.i2c1.sensor SetValue 42`

### I2C Peripheral with Register Map