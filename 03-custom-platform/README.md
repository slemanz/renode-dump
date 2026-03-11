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