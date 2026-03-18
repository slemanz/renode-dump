# .repl Platform Description Reference

Complete guide to writing Renode platform files — the `.repl` format that
describes your hardware.

## What Is a `.repl` File?

A `.repl` (Renode Platform Layout) file describes the hardware: CPU, memory
regions, peripherals, and how they connect. It's a declarative text file that
Renode parses to build the simulated machine.

You typically work with two kinds:

- **Base platform** — Pre-built by Renode for known MCUs (e.g.,
`platforms/cpus/stm32f4.repl`). Defines the CPU core, memory map, and standard
peripherals.
- **Overlay** — Your custom file that adds board-specific components (LEDs,
buttons, external sensors) on top of the base.

## Basic Syntax

### Peripheral Declaration

```
name: Namespace.ClassName @ parent offset
    property: value
    property: value
```

**Example — memory region:**

```
sram: Memory.MappedMemory @ sysbus 0x20000000
    size: 0x20000
```

**Example — UART:**

```
usart2: UART.STM32_UART @ sysbus 0x40004400
    IRQ -> nvic@38
```

### Key Rules

- Indentation is significant (properties must be indented under the peripheral)
- Comments start with `//`
- `@` means "attached to" (parent bus or peripheral)
- `->` means "IRQ connection" (source → destination@irq_number)
- Names are case-sensitive

## Importing Other Platforms

```
using "platforms/cpus/stm32f4.repl"
```

This imports the entire base platform. Your overlay then adds/overrides on top
of it. When loaded via `machine LoadPlatformDescription`, overlays stack:

```bash
machine LoadPlatformDescription @platforms/cpus/stm32f4.repl     # base
machine LoadPlatformDescription @my_board.repl                    # overlay
```

## Common Peripheral Types

### Memory

```
flash: Memory.MappedMemory @ sysbus 0x08000000
    size: 0x80000

sram: Memory.MappedMemory @ sysbus 0x20000000
    size: 0x20000
```

### CPU (Cortex-M4)

```
cpu: CPU.CortexM @ sysbus
    cpuType: "cortex-m4f"
    nvic: nvic

nvic: IRQControllers.NVIC @ sysbus 0xE000E000
    priorityMask: 0xF0
    systickFrequency: 16000000
    -> cpu@0
```

### GPIO Port

```
gpioPortA: GPIOPort.STM32_GPIOPort @ sysbus 0x40020000
    numberOfAFs: 16
    [0-15] -> exti@[0-15]
```

### UART

```
usart2: UART.STM32_UART @ sysbus 0x40004400
    IRQ -> nvic@38
```

### Timer

```
timer2: Timers.STM32_Timer @ sysbus 0x40000000
    frequency: 16000000
    initialLimit: 0xFFFFFFFF
    -> nvic@28
```

### SPI

```
spi1: SPI.STM32SPI @ sysbus 0x40013000
    IRQ -> nvic@35
```

### I2C

```
i2c1: I2C.STM32F4_I2C @ sysbus 0x40005400
    EventInterrupt -> nvic@31
    ErrorInterrupt -> nvic@32
```

### ADC

```
adc1: Analog.STM32_ADC @ sysbus 0x40012000
    IRQ -> nvic@18
    referenceVoltage: 3.3
    externalEventFrequency: 1000000
```

## Board Components (Overlays)

### LED

```
MyLED: Miscellaneous.LED @ gpioPortA

gpioPortA:
    5 -> MyLED@0
```

The `5 -> MyLED@0` means: pin 5 of gpioPortA drives input 0 of MyLED. When firmware sets pin 5 high, the LED turns on.

**Multiple LEDs on different ports:**

```
GreenLED: Miscellaneous.LED @ gpioPortA
gpioPortA:
    5 -> GreenLED@0

RedLED: Miscellaneous.LED @ gpioPortB
gpioPortB:
    3 -> RedLED@0

BlueLED: Miscellaneous.LED @ gpioPortC
gpioPortC:
    13 -> BlueLED@0
```

### Button

```
UserButton: Miscellaneous.Button @ gpioPortA
    -> gpioPortA@0
```

The `-> gpioPortA@0` means: the button drives pin 0 of gpioPortA. Use `Press`
and `Release` in the monitor or Robot tests.

**Button with inversion (active-low):**

```
UserButton: Miscellaneous.Button @ gpioPortA
    invert: true
    -> gpioPortA@0
```

### External I2C Device

```
tempSensor: I2C.BMP180 @ i2c1 0x77
```

This attaches a simulated BMP180 sensor at I2C address 0x77 to the i2c1 bus.
Renode has built-in models for some sensors. For custom ones, you write C# or
Python peripherals.

## IRQ Connections

IRQ connections use the `->` syntax. The format is:

```
sourcePeripheral -> destinationPeripheral@irqNumber
```

**Examples:**

```
// Timer 2 interrupt → NVIC IRQ 28
timer2:
    -> nvic@28

// UART2 interrupt → NVIC IRQ 38
usart2:
    IRQ -> nvic@38

// GPIO pin → EXTI line
gpioPortA:
    [0-15] -> exti@[0-15]

// EXTI lines → NVIC
exti:
    [0-4] -> nvic@[6-10]
```

### Multi-output Connections

```
// GPIO pin drives both an LED and an EXTI line
gpioPortA:
    5 -> MyLED@0
    [0-15] -> exti@[0-15]
```

## Named IRQ Connections

Some peripherals have named IRQ outputs instead of numbered ones:

```
i2c1: I2C.STM32F4_I2C @ sysbus 0x40005400
    EventInterrupt -> nvic@31
    ErrorInterrupt -> nvic@32
```

The names (`EventInterrupt`, `ErrorInterrupt`) are defined in the C# peripheral
model.

## Properties Reference

Properties are indented under the peripheral declaration:

```
peripheral: Type @ parent offset
    property1: value1
    property2: value2
```

Common properties:

| Property | Type | Used In | Example |
|----------|------|---------|---------|
| `size` | hex/int | Memory | `size: 0x20000` |
| `frequency` | int | Timer, CPU | `frequency: 16000000` |
| `cpuType` | string | CPU | `cpuType: "cortex-m4f"` |
| `IRQ` | connection | UART, SPI | `IRQ -> nvic@38` |
| `invert` | bool | Button | `invert: true` |
| `numberOfAFs` | int | GPIO | `numberOfAFs: 16` |
| `referenceVoltage` | float | ADC | `referenceVoltage: 3.3` |

## Complete Board Example

A full board overlay for an STM32F411 Nucleo-like setup:

```
// file: my_nucleo_board.repl
//
// Base: platforms/cpus/stm32f4.repl (loaded separately)
// Board: 1 user LED (PA5), 1 user button (PC13), UART2 terminal

// User LED (LD2)
UserLED: Miscellaneous.LED @ gpioPortA
gpioPortA:
    5 -> UserLED@0

// User button (directly readable — active-low on real board)
UserButton: Miscellaneous.Button @ gpioPortC
    -> gpioPortC@13
```

Load it:

```
machine LoadPlatformDescription @platforms/cpus/stm32f4.repl
machine LoadPlatformDescription @my_nucleo_board.repl
```

---

## Finding Available Peripheral Types

Renode's built-in peripheral models are organized by namespace:

| Namespace | Contents |
|-----------|----------|
| `CPU.CortexM` | ARM Cortex-M cores |
| `Memory.MappedMemory` | RAM, Flash regions |
| `GPIOPort.STM32_GPIOPort` | STM32 GPIO ports |
| `UART.STM32_UART` | STM32 UART/USART |
| `Timers.STM32_Timer` | STM32 general-purpose timers |
| `SPI.STM32SPI` | STM32 SPI |
| `I2C.STM32F4_I2C` | STM32F4 I2C |
| `Analog.STM32_ADC` | STM32 ADC |
| `Miscellaneous.LED` | Generic LED model |
| `Miscellaneous.Button` | Generic button model |
| `CAN.STMCAN` | STM32 bxCAN controller |

To see all available types, check [Renode's source code](https://github.com/renode/renode-infrastructure/tree/master/src/Emulator/Peripherals/Peripherals) or use tab-completion in the monitor.

## Troubleshooting `.repl` Files

**"Peripheral X not found"**  
Check the namespace and class name. Use exact spelling from Renode's source.

**"Property Y not recognized"**  
Properties are specific to each C# model. Check the model's source code for
available properties.

**Overlay doesn't apply**  
Make sure you load the base platform first, then the overlay. Order matters.

**IRQ doesn't fire**  
Verify the IRQ number matches the STM32 reference manual. Also check that the
NVIC is properly defined in the base platform.
