# Renode Cheatsheet

Quick reference for daily use. Covers monitor commands, CLI shortcuts, and
common patterns.

## Starting Renode

```bash
# Interactive (with GUI)
renode script.resc

# Headless (no GUI, for CI)
renode --disable-xwt -e "include @script.resc; start; sleep 5; quit"

# Run Robot Framework tests
renode-test tests/test_file.robot

# Run tests with output in a specific directory
renode-test tests/test_file.robot --results-dir tests/results
```

## Machine Management

```bash
# Create / select / list machines
mach create "name"                      # create a new machine
mach create                             # create with default name
mach set "name"                         # switch to a machine
mach set 0                              # switch by index
mach list                               # show all machines

# Load platform and firmware
machine LoadPlatformDescription @path/to/file.repl
sysbus LoadELF @path/to/firmware.elf
sysbus LoadBinary @path/to/firmware.bin 0x08000000

# Reset
machine Reset                           # reload firmware, restart CPU
runMacro $reset                         # execute the reset macro
```

## Execution Control

```bash
# Basic
start                                   # start simulation (alias: s)
pause                                   # pause simulation (alias: p)
quit                                    # exit Renode

# Timed execution
emulation RunFor "1.0"                  # run for 1 virtual second
emulation RunFor "0.001"                # run for 1ms

# Single-step
cpu Step                                # execute one instruction
cpu Step 100                            # execute 100 instructions
```

## Inspecting Peripherals

```bash
# List all peripherals
peripherals                             # show peripheral tree
sysbus WhatIsAt 0x40020000              # what's at this address?
sysbus WhatPeripheralIsAt 0x40020000    # same, more detail

# Read/write memory-mapped registers
sysbus ReadDoubleWord 0x40020014        # read 32-bit (e.g., GPIOA->ODR)
sysbus WriteDoubleWord 0x40020014 0x20  # write 32-bit
sysbus ReadByte 0x20000000              # read 8-bit from SRAM
sysbus ReadWord 0x40020014              # read 16-bit

# CPU registers
cpu PC                                  # program counter
cpu SP                                  # stack pointer
cpu GetRegisterUnsafe 0                 # R0
cpu GetRegisterUnsafe 1                 # R1
```

## GPIO / LED / Button

```bash
# LED state
gpioPortA.UserLED State                 # True (on) / False (off)

# Button control
gpioPortA.UserButton Press              # press (drive pin)
gpioPortA.UserButton Release            # release

# Read GPIO output register directly
sysbus ReadDoubleWord 0x40020014       # GPIOA ODR
```


## UART

```bash
# Open interactive terminal (GUI only)
showAnalyzer sysbus.usart2

# In Robot Framework tests (programmatic)
# Create Terminal Tester    sysbus.usart2
# Wait For Line On Uart     expected text    timeout=5
```

## Logging

```bash
# Log levels: -1 (Noisy) → 0 (Debug) → 1 (Info) → 2 (Warning) → 3 (Error)
logLevel -1 gpioPortA.UserLED           # see every state change
logLevel 1 sysbus.usart2               # info only for UART
logLevel 3                              # errors only globally

# Log to file
logFile @/tmp/renode.log
logFile @/tmp/renode.log true           # append mode

# Per-peripheral log
logLevel -1 sysbus.timer2              # see timer events
```

## Debugging with GDB

```bash
# In Renode monitor
machine StartGdbServer 3333

# In another terminal
arm-none-eabi-gdb Build/flash.elf
(gdb) target remote :3333
(gdb) break main
(gdb) continue
(gdb) info registers
(gdb) x/10x 0x20000000                 # examine SRAM
```

## Multi-Machine / Networking

```bash
# Create multiple machines
mach create "node_A"
machine LoadPlatformDescription @platforms/cpus/stm32f4.repl
sysbus LoadELF @node_a.elf

mach create "node_B"
machine LoadPlatformDescription @platforms/cpus/stm32f4.repl
sysbus LoadELF @node_b.elf

# Connect UARTs
emulation CreateUARTHub "uartBus"
mach set "node_A"
connector Connect sysbus.usart2 uartBus
mach set "node_B"
connector Connect sysbus.usart2 uartBus

# Connect CAN (for MCUs with CAN peripheral)
emulation CreateCANHub "canBus"
mach set "node_A"
connector Connect sysbus.can1 canBus
mach set "node_B"
connector Connect sysbus.can1 canBus
```

## Snapshots (Save / Restore State)

```bash
# Save current state
Save @snapshot_name.save

# Load saved state
Load @snapshot_name.save
```


## Scripting (.resc) Patterns

### Minimal Script

```bash
$bin?=@Build/flash.elf
mach create "STM32F411"
machine LoadPlatformDescription @platforms/cpus/stm32f4.repl
macro reset
"""
    sysbus LoadELF $bin
"""
runMacro $reset
```

### With Overlay and Logging

```bash
$bin?=@Build/flash.elf
mach create "STM32F411"
machine LoadPlatformDescription @platforms/cpus/stm32f4.repl
machine LoadPlatformDescription @my_board.repl
showAnalyzer sysbus.usart2
logLevel -1 gpioPortA.MyLED
macro reset
"""
    sysbus LoadELF $bin
"""
runMacro $reset
```

### Variables in Scripts

```bash
$bin?=@default_path.elf                 # default value, overridable
$timeout?="5"                           # string variable

# Override from command line:
# renode -e '$bin=@other.elf; include @script.resc'
```


## Useful Monitor Tricks

```bash
# Execute Python in Renode
python "print('Hello from Python')"

# Get elapsed virtual time
machine ElapsedVirtualTime

# List available platforms
list @platforms/cpus/                    # see all CPU platforms
list @platforms/boards/                  # see all board platforms

# Tab completion
# Type partial command + Tab in the monitor

# Command history
# Up/Down arrows in the monitor
```
