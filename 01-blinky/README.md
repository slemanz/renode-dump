# 01 - Blinky

Toggle an LED on **GPIOA pin 5** of an STM32F411, verified through Renode
simulation and automated tests.

> **Renode version:** 1.16  
> **Target MCU:** STM32F411 (Cortex-M4)

---

## Project Structure

```
01-blinky/
├── Src/
│   └── main.c                    # Firmware: toggles PA5 every 500ms via SysTick
├── Build/
│   ├── flash.elf                 # Compiled firmware (loaded by Renode)
│   └── flash.bin                 # Raw binary
├── Makefile                      # Build system
├── blinky.resc                   # Renode script — sets up and starts the simulation
├── stm32f411_blinky.repl         # Platform overlay — connects LED to GPIOA pin 5
├── tests/
│   └── test_blinky.robot         # Robot Framework automated tests
└── README.md
```

---

## How It Works

The firmware configures PA5 as a push-pull output, initializes SysTick at 1kHz,
and toggles the pin every 500ms in a super loop. Unlike a busy-wait delay,
SysTick gives us accurate timing that the Renode simulation can reproduce
reliably.

```c
int main(void)
{
    systick_init(TICK_HZ);
    pa5_init();

    uint64_t last_toggle = ticks_get();

    while (1)
    {
        if ((ticks_get() - last_toggle) >= 500u)
        {
            GPIO_ToggleOutputPin(GPIOA, GPIO_PIN_NO_5);
            last_toggle = ticks_get();
        }
    }
}
```

---

## Build

```bash
cd 01-blinky
make
```

Requires `arm-none-eabi-gcc` in your PATH.

---
