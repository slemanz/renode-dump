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

Most real sensors use a register-pointer pattern: write 1 byte (register
address), then read N bytes (register data).

```python
# register_sensor.py — I2C sensor with register map

# Register contents (address → list of bytes)
registers = {
    0x00: [0x19, 0x00],   # Temperature: 25.0°C (TMP102 format)
    0x01: [0x60, 0xA0],   # Config register
    0xFE: [0x54, 0x49],   # Manufacturer ID
    0xFF: [0x00, 0x68],   # Device ID
}

pointer = 0x00        # Current register pointer
read_index = 0        # Byte index within current register
write_buf = []        # Accumulates bytes during a write transaction

def write(data):
    """First byte = register pointer. Subsequent bytes = register write."""
    global pointer, read_index, write_buf
    write_buf.append(data)

    if len(write_buf) == 1:
        pointer = data
        read_index = 0
    else:
        # Writing to a register
        reg = registers.get(pointer, None)
        if reg is not None:
            idx = len(write_buf) - 2
            if idx < len(reg):
                reg[idx] = data

def read():
    """Return bytes from the currently pointed register."""
    global read_index
    reg = registers.get(pointer, [0x00, 0x00])
    if read_index < len(reg):
        byte = reg[read_index]
        read_index += 1
        return byte
    return 0x00

def finish():
    """End of transaction — reset write buffer."""
    global write_buf
    write_buf = []

def reset():
    """Called on machine reset."""
    global pointer, read_index, write_buf
    pointer = 0x00
    read_index = 0
    write_buf = []
    registers[0x00] = [0x19, 0x00]  # Reset temperature to 25°C

# --- Custom commands ---

def SetTemperature(temp_c):
    """Set temperature in Celsius. Updates register 0x00."""
    raw = int(float(temp_c) / 0.0625)
    if raw > 2047: raw = 2047
    if raw < -2048: raw = -2048
    if raw < 0: raw = raw & 0x0FFF
    registers[0x00] = [(raw >> 4) & 0xFF, (raw << 4) & 0xF0]
    log("Temperature set to %.2f°C (raw=%d)" % (float(temp_c), raw))
```

### SPI Peripheral

SPI peripherals use the same `write`/`read` callbacks but are attached to an SPI
bus instead:

```bash
# In .resc:
machine PyDevFromFile @my_spi_device.py 0 sysbus.spi1 "device"
```

For SPI, the second parameter (address) is the chip-select index (usually 0).

```python
# my_spi_device.py — SPI peripheral template

command = 0x00
response_queue = []

def write(data):
    """Called for each byte the master clocks out."""
    global command, response_queue
    if len(response_queue) == 0:
        # First byte is usually a command/address
        command = data
        # Queue up response bytes for subsequent reads
        if command == 0x01:
            response_queue = [0xAB, 0xCD]  # status register
        elif command == 0x02:
            response_queue = [0x12, 0x34]  # data register
        else:
            response_queue = [0x00]

def read():
    """Called for each byte the master clocks in."""
    if len(response_queue) > 0:
        return response_queue.pop(0)
    return 0x00

def finish():
    """CS deasserted — reset state."""
    global response_queue
    response_queue = []
```