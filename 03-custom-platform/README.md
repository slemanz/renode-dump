# 03 - Custom Platform

Build a `.repl` platform description from scratch with **two LEDs**, a **button**, and **UART debug** — then inject button presses from Robot Framework tests.

> **Renode version:** 1.16  
> **Target MCU:** STM32F411 (Cortex-M4)

---

## Project Structure

```
03-custom-platform/
├── Src/
│   └── main.c                        # Firmware: blinky + button → error LED
├── Build/
│   └── flash.elf
├── Makefile
├── custom-platform.resc              # Renode script
├── stm32f411_custom.repl             # Custom platform overlay (2 LEDs + button)
├── tests/
│   └── test_custom_platform.robot    # Tests with button injection
└── README.md
```

## How It Works

The board has three GPIO peripherals:

| Peripheral | Port/Pin | Direction | Purpose |
|-----------|----------|-----------|---------|
| Status LED | PA5 | Output | Blinks every 500ms (heartbeat) |
| Error LED | PB3 | Output | ON when button is pressed |
| User Button | PA0 | Input (pull-up) | Triggers error LED |

The firmware boots, prints `"Platform ready"`, then runs a super loop that
blinks the status LED and monitors the button. When the button is pressed, it
turns on the error LED and prints `"Error ON"`. On release: `"Error OFF"`.

## The Platform File (`.repl`)

This is the key learning in this example. Instead of using a pre-made board
file, we write a minimal overlay:

```
// Two LEDs on different GPIO ports
StatusLED: Miscellaneous.LED @ gpioPortA
gpioPortA:
    5 -> StatusLED@0

ErrorLED: Miscellaneous.LED @ gpioPortB
gpioPortB:
    3 -> ErrorLED@0

// Button that drives a GPIO input
UserButton: Miscellaneous.Button @ gpioPortA
    -> gpioPortA@0
```

### Anatomy of a `.repl` entry


```
PeripheralName: Namespace.ClassName @ parentPeripheral
```

- `PeripheralName` — the name you use in monitor commands (e.g., `gpioPortA.StatusLED`)
- `Namespace.ClassName` — Renode's C# class (e.g., `Miscellaneous.LED`, `Miscellaneous.Button`)
- `@ parentPeripheral` — where it's connected

The `5 -> StatusLED@0` line means: "connect pin 5 of gpioPortA to input 0 of
StatusLED". The `->` is an IRQ connection in Renode's platform language.

## Build & Run

```bash
make
renode custom-platform.resc
# In the monitor:
start

# Press the button:
gpioPortA.UserButton Press

# Release it:
gpioPortA.UserButton Release

pause
q
```

## Automated Tests