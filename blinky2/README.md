# Blinky - Learning Renode with STM32F411

This is a simple blinky project that toggles an LED on **GPIOA pin 5** of an STM32F411. We'll use it to learn how [Renode](https://renode.io) can simulate your firmware without any physical hardware.

By the end of this guide, you'll be able to:

- Run your firmware inside Renode and watch an LED blink
- Understand how Renode platform files (`.repl`) and scripts (`.resc`) work
- Inspect peripherals, read registers, and debug your code
- Write automated tests using Robot Framework

> **Renode version used:** 1.16
> **Target MCU:** STM32F411 (Cortex-M4)

---

## Table of Contents

1. [Project Structure](#project-structure)
2. [How the Firmware Works](#how-the-firmware-works)
3. [Quick Start - See the LED Blinking](#quick-start---see-the-led-blinking)
4. [Understanding Renode Files](#understanding-renode-files)
5. [Step by Step: Running the Simulation](#step-by-step-running-the-simulation)
6. [Watching the LED Toggle](#watching-the-led-toggle)
7. [Exploring Peripherals](#exploring-peripherals)
8. [Debugging with GDB](#debugging-with-gdb)
9. [Logging and Tracing](#logging-and-tracing)
10. [Automated Testing with Robot Framework](#automated-testing-with-robot-framework)
11. [Useful Monitor Commands](#useful-monitor-commands)
12. [Common Issues and Tips](#common-issues-and-tips)
13. [Next Steps](#next-steps)

---

## Project Structure

```
blinky/
├── Src/
│   └── main.c                    # Application code (toggles PA5)
├── Build/
│   ├── flash.elf                 # Compiled firmware (this is what Renode loads)
│   └── flash.bin                 # Raw binary (alternative)
├── Makefile                      # Build system
├── blinky.resc                   # Renode script - sets up and runs the simulation
├── stm32f411_blinky.repl         # Platform overlay - adds LED on GPIOA pin 5
├── tests/
│   └── test_blinky.robot         # Automated test using Robot Framework
└── README.md                     # You are here
```

---

## How the Firmware Works

The code in `Src/main.c` does three things:

1. Configures **GPIOA pin 5** as a push-pull output
2. Enters an infinite loop
3. Toggles the pin and waits (software delay)

```c
int main(void)
{
    pa5_init();           // Set PA5 as output

    while(1)
    {
        GPIO_ToggleOutputPin(GPIOA, GPIO_PIN_NO_5);
        delay();          // Simple busy-wait loop
    }
}
```

On a real Nucleo-F411RE board, PA5 is connected to the green LED (LD2). In Renode, we'll create a virtual LED connected to the same pin.

---

## Quick Start - See the LED Blinking

If you just want to see it working right away:

```bash
# 1. Build the firmware (if you haven't already)
make

# 2. Launch Renode with the blinky script
renode blinky.resc

# 3. In the Renode Monitor window, type:
start
```

You should see log messages like this appearing repeatedly:

```
[NOISY] STM32F411/gpioPortA.UserLED: LED state changed to True
[NOISY] STM32F411/gpioPortA.UserLED: LED state changed to False
[NOISY] STM32F411/gpioPortA.UserLED: LED state changed to True
...
```

Each `True` means the LED turned ON, each `False` means it turned OFF. That's your LED blinking!

To stop: type `pause` (or just `p`) to pause, or `quit` (or `q`) to exit Renode.

---

## Understanding Renode Files

Renode uses two main types of files to describe your simulation:

### `.repl` files (Platform Description)

These describe the **hardware** — what peripherals exist and how they're connected. Think of it as a virtual circuit board.

Renode already includes a generic STM32F4 platform at `platforms/cpus/stm32f4.repl` with the CPU, memory, GPIO ports, UARTs, timers, etc. We just need to add our LED.

Our `stm32f411_blinky.repl` file adds a virtual LED connected to GPIOA pin 5:

```
// Create an LED peripheral attached to GPIO port A
UserLED: Miscellaneous.LED @ gpioPortA

// Connect pin 5 of GPIOA to the LED
gpioPortA:
    5 -> UserLED@0
```

That's it! Two lines to wire up a virtual LED.

### `.resc` files (Scripts)

These are like a recipe that tells Renode what to do step by step. Our `blinky.resc` does:

```bash
$bin?=@Build/flash.elf                                        # Where the firmware is
mach create "STM32F411"                                       # Create a virtual machine
machine LoadPlatformDescription @platforms/cpus/stm32f4.repl  # Load base hardware
machine LoadPlatformDescription @stm32f411_blinky.repl        # Add our LED
logLevel -1 gpioPortA.UserLED                                 # Enable verbose LED logs

macro reset
"""
    sysbus LoadELF $bin                                       # Load firmware into flash
"""
runMacro $reset
```

The `$bin?=@Build/flash.elf` syntax sets a default path. The `?=` means "only set this if it wasn't already defined", so you can override it:

```bash
renode -e '$bin=@some/other/firmware.elf; include @blinky.resc; start'
```

---

## Step by Step: Running the Simulation

### Step 1: Build the firmware

```bash
cd blinky/
make
```

This produces `Build/flash.elf` — the ELF file that Renode will load.

### Step 2: Open Renode

```bash
renode
```

This opens the Renode Monitor (a command-line interface). Everything you type here controls the simulation.

### Step 3: Load the script

In the Monitor, type:

```
include @blinky.resc
```

This creates the machine, loads the hardware description, and loads your firmware. You won't see much output yet — the simulation is paused.

### Step 4: Start the simulation

```
start
```

Or just `s`. The CPU starts executing your firmware. You'll see the LED toggling in the logs.

### Step 5: Pause

```
pause
```

Or just `p`. The simulation freezes — useful when you want to inspect the state of things.

### Step 6: Quit

```
quit
```

Or just `q`.

### One-liner (skip the interactive steps)

You can do everything in one command from your terminal:

```bash
renode -e 'include @blinky.resc; start'
```

The `-e` flag executes Monitor commands directly.

---

## Watching the LED Toggle

The simplest way to see the LED is through log messages. We already enabled verbose logging with:

```
logLevel -1 gpioPortA.UserLED
```

The `-1` means "NOISY" level — the most verbose. You'll see every state change.

### Check the LED state manually

While the simulation is paused, type:

```
gpioPortA.UserLED State
```

This returns `True` (LED is on) or `False` (LED is off).

### Save the log to a file

If you want to keep a record:

```
logFile @/tmp/blinky.log
```

All output will be saved there.

---

## Exploring Peripherals

Renode lets you poke around the virtual hardware just like you would with a debugger on real hardware.

### List all peripherals

```
peripherals
```

This shows everything in the system: CPU, memory, GPIO ports, UARTs, timers, etc.

### What's at a specific address?

```
sysbus WhatIsAt 0x40020000
```

This tells you which peripheral sits at that memory address (in this case, GPIOA).

### Read a register directly

```
sysbus ReadDoubleWord 0x40020014
```

That reads the GPIOA ODR (Output Data Register). If bit 5 is set, the LED is on.

### Log everything that touches a peripheral

```
sysbus LogPeripheralAccess gpioPortA
```

Now every read/write to any GPIOA register will be printed:

```
[INFO] gpioPortA: WriteDoubleWord to 0x14 (GPIOX_ODR), value 0x20
```

To log **all** peripherals (can be very verbose):

```
sysbus LogAllPeripheralsAccess true
```

---

## Debugging with GDB

Renode has a built-in GDB server. This means you can connect your favorite debugger (GDB, VS Code, etc.) and step through code as if you were debugging on real hardware.

### Start the GDB server

In the Renode Monitor:

```
machine StartGdbServer 3333
```

### Connect with GDB

In another terminal:

```bash
arm-none-eabi-gdb Build/flash.elf
(gdb) target remote :3333
(gdb) break main
(gdb) continue
```

Now you can:
- Set breakpoints: `break pa5_init`
- Step through code: `step`, `next`
- Print variables: `print i`
- Read registers: `info registers`
- Continue running: `continue`

### Useful GDB commands for embedded

```
(gdb) x/10x 0x40020000       # Examine 10 words at GPIOA base address
(gdb) info registers          # Show all CPU registers
(gdb) bt                      # Backtrace (call stack)
```

---

## Logging and Tracing

### Trace function calls

Want to see which functions your firmware is calling?

```
sysbus.cpu LogFunctionNames true
```

This prints every function entry, which is incredibly useful for understanding program flow.

### Check the program counter

```
sysbus.cpu PC
```

Shows the address the CPU is currently executing.

### Step one instruction at a time

```
sysbus.cpu Step
```

Or step multiple instructions:

```
sysbus.cpu Step 10
```

### Run for a specific amount of time

```
emulation RunFor "1.0"
```

This runs the simulation for exactly 1 second of virtual time, then pauses.

### Log levels

You can control how much output each peripheral generates:

| Level | Value | What you get |
|-------|-------|-------------|
| NOISY | -1 | Everything (LED state changes, etc.) |
| DEBUG | 0 | Debug info |
| INFO | 1 | Normal (default) |
| WARNING | 2 | Only warnings |
| ERROR | 3 | Only errors |

Set for a specific peripheral:

```
logLevel -1 gpioPortA.UserLED
```

Set globally:

```
logLevel -1
```

---

## Automated Testing with Robot Framework

One of Renode's most powerful features is automated testing. You can write tests that run your firmware, check peripheral states, and verify behavior — all without touching hardware.

Tests use [Robot Framework](https://robotframework.org/) syntax and are run with the `renode-test` command.

### The test file

Check out `tests/test_blinky.robot`:

```robot
*** Settings ***
Suite Setup                     Setup
Suite Teardown                  Teardown
Test Setup                      Reset Emulation
Resource                        ${RENODEKEYWORDS}

*** Keywords ***
Prepare Machine
    Execute Command             include @${CURDIR}/../blinky.resc
    Execute Command             logLevel -1 gpioPortA.UserLED

*** Test Cases ***
Should Create Machine
    [Documentation]             Verify that the machine is created and firmware loads.
    Prepare Machine

LED Should Change State After Running
    [Documentation]             Verify that the LED toggles after running.
    Prepare Machine
    Start Emulation
    Execute Command             emulation RunFor "0.5"
```

### Running the test

```bash
renode-test tests/test_blinky.robot
```

This runs Renode in headless mode, executes the tests, and generates a report in HTML format. You'll see output like:

```
Starting suites: test_blinky
  Should Create Machine                    :: PASS
  LED Should Change State After Running    :: PASS
2 tests, 2 passed, 0 failed
```

### Why this matters

Automated testing means you can:

- Run tests in CI/CD (GitHub Actions, Jenkins, etc.)
- Catch regressions before they reach real hardware
- Test edge cases that are hard to reproduce physically
- Validate firmware across different configurations

---

## Useful Monitor Commands

Here's a cheat sheet of the most useful Renode Monitor commands:

### Simulation

| Command | Short | What it does |
|---------|-------|-------------|
| `start` | `s` | Start the simulation |
| `pause` | `p` | Pause the simulation |
| `quit` | `q` | Exit Renode |
| `machine Reset` | | Reset the machine |
| `emulation RunFor "1.0"` | | Run for 1 second then pause |

### Inspection

| Command | What it does |
|---------|-------------|
| `peripherals` | List all peripherals |
| `sysbus WhatIsAt 0x40020000` | What's at this address? |
| `sysbus.cpu PC` | Current program counter |
| `gpioPortA.UserLED State` | Is the LED on or off? |
| `sysbus ReadDoubleWord 0x40020014` | Read a 32-bit register |

### Logging

| Command | What it does |
|---------|-------------|
| `logLevel -1 gpioPortA.UserLED` | Verbose logs for LED |
| `sysbus LogPeripheralAccess gpioPortA` | Log all GPIOA accesses |
| `sysbus.cpu LogFunctionNames true` | Trace function calls |
| `logFile @/tmp/log.txt` | Save logs to file |

### Debugging

| Command | What it does |
|---------|-------------|
| `machine StartGdbServer 3333` | Start GDB server |
| `sysbus.cpu Step` | Execute one instruction |
| `sysbus.cpu Step 10` | Execute 10 instructions |

---

## Common Issues and Tips

### "Could not find file" error
Make sure you run Renode from inside the `blinky/` directory, or use absolute paths in the `.resc` file. Renode resolves paths relative to where you launched it.

### The simulation runs but nothing happens
- Check that `logLevel -1 gpioPortA.UserLED` is set
- Make sure you typed `start` after loading the script
- Verify the ELF was built correctly: `arm-none-eabi-readelf -h Build/flash.elf`

### Renode says "peripheral not found"
The peripheral name is case-sensitive. It's `gpioPortA`, not `GPIOA` or `GpioPortA`. Use the `peripherals` command to see the exact names.

### The LED toggles too fast / too slow
Renode simulates at its own pace. The delay loop in the firmware (`for(uint32_t i = 0; i < 500000; i++)`) may run faster or slower than real time. This is normal — the important thing is that the logic works correctly.

### Memory size mismatch
The base `stm32f4.repl` defines 2MB of flash and 256KB of SRAM (for the STM32F407). Your STM32F411 has 512KB flash and 128KB SRAM. This doesn't matter for a small blinky, but for larger projects you can adjust it by adding to your `.repl`:

```
sram:
    size: 0x20000

flash:
    size: 0x80000
```

---

## Next Steps

Now that you know the basics, here are some ideas to keep exploring:

- **Add UART output** — use `showAnalyzer sysbus.usart2` to see serial output in a terminal window
- **Add more LEDs** — define multiple LEDs in the `.repl` and wire them to different GPIO pins
- **Try a timer interrupt** — replace the software delay with a hardware timer and see how Renode handles interrupts
- **Connect two machines** — Renode can simulate networks of devices communicating over UART, SPI, or I2C
- **Write more Robot tests** — test specific behaviors like "LED should toggle exactly N times per second"
- **Use in CI/CD** — run `renode-test` in GitHub Actions to test every commit automatically

Happy simulating!
