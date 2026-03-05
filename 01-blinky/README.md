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

## Run in Renode

### Interactive (with monitor)

```bash
cd 01-blinky
renode blinky.resc
```

Then in the Renode monitor:

```
start                                    # begin simulation
gpioPortA.UserLED State                  # check LED state (True/False)
pause                                    # stop simulation
```

You'll see log messages like:
```
[INFO] gpioPortA.UserLED: LED state changed to True
[INFO] gpioPortA.UserLED: LED state changed to False
```

### Headless (run and exit)

```bash
renode --disable-xwt -e "include @blinky.resc; start; sleep 3; quit"
```

---

## Automated Tests

```bash
renode-test tests/test_blinky.robot --results-dir tests/results
```

### Test Cases

| Test | What It Verifies |
|------|-----------------|
| `Should Create Machine` | Machine boots and firmware loads without errors |
| `LED Should Be Off Initially` | LED starts in off state before any code runs |
| `LED Should Change State After Running` | LED toggles at least once after 600ms of simulation |
| `LED Should Toggle Multiple Times` | LED toggles back and forth across two periods |

---

## Renode Concepts Covered

### Platform Overlay (`.repl`)

The file `stm32f411_blinky.repl` extends the base STM32F4 platform by adding a virtual LED:

```
UserLED: Miscellaneous.LED @ gpioPortA
gpioPortA:
    5 -> UserLED@0
```

This tells Renode: "create an LED peripheral attached to GPIO port A, and
connect pin 5's output to the LED's input 0." When firmware toggles PA5, the LED
state changes.

### Simulation Script (`.resc`)

The `blinky.resc` file is the entry point. It creates a machine, loads the
platform description and firmware, and sets up logging. Key commands:

- `mach create` — creates a virtual machine instance
- `machine LoadPlatformDescription` — loads hardware definition
- `sysbus LoadELF` — loads compiled firmware into simulated flash
- `logLevel -1` — enables verbose logging (level -1 = Noisy)

### Robot Framework Tests

Tests use Renode's built-in Robot Framework keywords. The pattern is:

1. Create and configure the machine (`Prepare Machine`)
2. Run the simulation for a specific time (`emulation RunFor`)
3. Read peripheral state (`Execute Command ... State`)
4. Assert expected behavior (`Should Not Be Equal`)

---
