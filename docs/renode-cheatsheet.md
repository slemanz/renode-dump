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