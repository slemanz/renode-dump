# Robot Framework Guide for Renode

How to write, run, and organize automated tests for simulated firmware using
Robot Framework and Renode's built-in keywords.

---

## Quick Start

### Running a Test

```bash
# Basic run
renode-test tests/test_file.robot

# Multiple test files
renode-test tests/test_blinky.robot tests/test_uart.robot

# With output in a specific directory (keeps workspace clean)
renode-test tests/test_file.robot --results-dir tests/results

# Run a specific test case
renode-test tests/test_file.robot -t "Should Print Boot Message"
```

### Keeping Test Output Clean

By default, `renode-test` drops `report.html`, `log.html`, and `output.xml` in
the current directory. Use `--results-dir` to redirect them:

```bash
renode-test tests/test_file.robot --results-dir tests/results
```

This creates `tests/results/` with all HTML/XML reports inside. Add the results
directory to your `.gitignore`:

```gitignore
# Robot Framework output
**/results/
```

You can also create a helper script in your project root:

```bash
#!/bin/bash
# run-tests.sh — Run all Robot tests with clean output
set -e
for test in */tests/*.robot; do
    dir=$(dirname "$test")/results
    mkdir -p "$dir"
    echo "Running: $test"
    renode-test "$test" --results-dir "$dir"
done
```

## Test File Structure

```robot
*** Settings ***
Suite Setup                     Setup
Suite Teardown                  Teardown
Test Setup                      Reset Emulation
Resource                        ${RENODEKEYWORDS}

*** Variables ***
${LED}                          sysbus.gpioPortA.UserLED

*** Keywords ***
Prepare Machine
    Execute Command             mach create "STM32F411"
    Execute Command             machine LoadPlatformDescription @platforms/cpus/stm32f4.repl
    Execute Command             machine LoadPlatformDescription @${CURDIR}/../my_board.repl
    Execute Command             sysbus LoadELF @${CURDIR}/../Build/flash.elf
    Create Terminal Tester      sysbus.usart2

*** Test Cases ***
My First Test
    [Documentation]             Description of what this test verifies.
    Prepare Machine
    Start Emulation
    Wait For Line On Uart       Hello World    timeout=5
```

### Key Sections

| Section | Purpose |
|---------|---------|
| `*** Settings ***` | Suite-level configuration (setup, teardown, imports) |
| `*** Variables ***` | Reusable values (peripheral paths, timeouts) |
| `*** Keywords ***` | Custom keywords (your reusable test steps) |
| `*** Test Cases ***` | The actual tests |

---

## Essential Settings

Always include these three lines in every test file:

```robot
Suite Setup                     Setup
Suite Teardown                  Teardown
Test Setup                      Reset Emulation
Resource                        ${RENODEKEYWORDS}
```

- `Setup` / `Teardown` — Renode-provided keywords that start/stop the emulation environment
- `Reset Emulation` — Clears all machines between test cases (clean state)
- `${RENODEKEYWORDS}` — Path to Renode's Robot keyword library (auto-set by `renode-test`)


## Renode Keywords Reference

### Machine Setup

```robot
# Execute any Renode monitor command
Execute Command             mach create "name"
Execute Command             machine LoadPlatformDescription @file.repl
Execute Command             sysbus LoadELF @firmware.elf

# Execute and capture output
${result}=                  Execute Command    peripherals
${value}=                   Execute Command    sysbus ReadDoubleWord 0x40020014
```

### Execution Control

```robot
# Start the simulation (non-blocking)
Start Emulation

# Run for a specific virtual time (blocking)
Execute Command             emulation RunFor "1.0"

# Pause
Execute Command             pause
```

### UART Testing

```robot
# Create a listener on a UART peripheral (MUST be before Start Emulation)
Create Terminal Tester      sysbus.usart2

# Wait for a specific string on UART
Wait For Line On Uart       expected text    timeout=5

# Wait for text (partial match, doesn't need to be a full line)
Wait For Prompt On Uart     partial text     timeout=5

# Wait for a specific number of lines
Wait For Line On Uart       first line       timeout=5
Wait For Line On Uart       second line      timeout=5

# Send characters TO the UART (simulating terminal input)
Write Line To Uart          command text
Write Char On Uart          A
```

### GPIO / LED / Button

```robot
# Read LED state
${state}=                   Execute Command    sysbus.gpioPortA.MyLED State
Should Contain              ${state}    True
Should Contain              ${state}    False

# Press/release button
Execute Command             sysbus.gpioPortA.MyButton Press
Execute Command             sysbus.gpioPortA.MyButton Release

# Read GPIO register directly
${odr}=                     Execute Command    sysbus ReadDoubleWord 0x40020014
```

### Assertions

```robot
# String comparisons
Should Contain              ${state}    True
Should Not Contain          ${state}    Error
Should Be Equal             ${a}    ${b}
Should Not Be Equal         ${a}    ${b}

# With custom error messages
Should Contain              ${state}    True
...                         msg=LED should be ON after button press
```
