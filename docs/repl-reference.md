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