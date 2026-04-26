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

---

## Common Test Patterns

### Pattern 1: Boot Verification

Simplest test — does the firmware start without crashing?

```robot
Should Boot Successfully
    Prepare Machine
    Start Emulation
    Wait For Line On Uart       Boot OK    timeout=5
```

### Pattern 2: Periodic Behavior

Verify something happens at regular intervals:

```robot
Should Print Periodic Messages
    Prepare Machine
    Start Emulation
    Wait For Line On Uart       [1] tick    timeout=3
    Wait For Line On Uart       [2] tick    timeout=3
    Wait For Line On Uart       [3] tick    timeout=3
```

### Pattern 3: Input → Output

Inject an input and verify the output:

```robot
Button Should Activate Error LED
    Prepare Machine
    Start Emulation
    Wait For Line On Uart       Ready    timeout=3

    Execute Command             sysbus.gpioPortA.Button Press
    Wait For Line On Uart       Error ON    timeout=3
    ${state}=                   Execute Command    sysbus.gpioPortB.ErrorLED State
    Should Contain              ${state}    True
```

### Pattern 4: State Transitions

Verify state changes over time:

```robot
LED Should Toggle
    Prepare Machine
    ${initial}=                 Execute Command    sysbus.gpioPortA.LED State

    Execute Command             emulation RunFor "0.6"
    ${after}=                   Execute Command    sysbus.gpioPortA.LED State
    Should Not Be Equal         ${initial}    ${after}
```

### Pattern 5: Multi-Machine

Test communication between two nodes:

```robot
*** Keywords ***
Prepare Two Machines
    Execute Command             mach create "sender"
    Execute Command             machine LoadPlatformDescription @platforms/cpus/stm32f4.repl
    Execute Command             sysbus LoadELF @${CURDIR}/../sender/Build/flash.elf

    Execute Command             mach create "receiver"
    Execute Command             machine LoadPlatformDescription @platforms/cpus/stm32f4.repl
    Execute Command             sysbus LoadELF @${CURDIR}/../receiver/Build/flash.elf

    # Connect UARTs
    Execute Command             emulation CreateUARTHub "uartBus"
    Execute Command             mach set "sender"
    Execute Command             connector Connect sysbus.usart2 uartBus
    Execute Command             mach set "receiver"
    Execute Command             connector Connect sysbus.usart2 uartBus

    # Create tester on receiver's debug UART
    Execute Command             mach set "receiver"
    Create Terminal Tester      sysbus.usart1

*** Test Cases ***
Receiver Should Get Sender Message
    Prepare Two Machines
    Start Emulation
    Wait For Line On Uart       received: hello    timeout=5
```

### Pattern 6: ADC Injection

Force an ADC reading and verify firmware reaction:

```robot
Should Detect Overvoltage
    Prepare Machine
    Start Emulation
    Wait For Line On Uart       Ready    timeout=3

    # Inject high ADC value (12-bit: 4095 = max)
    Execute Command             sysbus.adc1 FeedSample 0x0 3800 0
    Execute Command             emulation RunFor "0.5"
    Wait For Line On Uart       OVERVOLTAGE    timeout=3
```

---

## Variables and Reusability

### Project-Level Variables

```robot
*** Variables ***
${LED}                          sysbus.gpioPortA.UserLED
${BUTTON}                       sysbus.gpioPortA.UserButton
${UART}                         sysbus.usart2
${BOOT_TIMEOUT}                 5
```

### Shared Keywords Across Projects

Create a `common.resource` file:

```robot
# tests/common.resource
*** Keywords ***
Prepare STM32F411
    [Arguments]    ${repl_path}    ${elf_path}
    Execute Command             mach create "STM32F411"
    Execute Command             machine LoadPlatformDescription @platforms/cpus/stm32f4.repl
    Execute Command             machine LoadPlatformDescription @${repl_path}
    Execute Command             sysbus LoadELF @${elf_path}
```

Use it in test files:

```robot
*** Settings ***
Resource                        ${CURDIR}/common.resource

*** Test Cases ***
My Test
    Prepare STM32F411    ${CURDIR}/../my_board.repl    ${CURDIR}/../Build/flash.elf
    Create Terminal Tester      sysbus.usart2
    Start Emulation
    Wait For Line On Uart       Hello    timeout=5
```

---

## Debugging Failed Tests

### Read the Output

When a test fails, `renode-test` generates:

- `log.html` — Detailed execution log (every keyword, every result)
- `report.html` — Summary with pass/fail counts
- `output.xml` — Machine-readable results (for CI)

### Common Failures

| Symptom | Likely Cause | Fix |
|---------|-------------|-----|
| `Wait For Line On Uart` timeout | UART not configured, or firmware not printing | Check UART init, increase timeout |
| LED state never changes | Timer/SysTick not working in Renode | Verify clock setup, check `emulation RunFor` duration |
| `Keyword not found: Create Terminal Tester` | Missing `Resource ${RENODEKEYWORDS}` | Add the Resource line in Settings |
| Test passes alone but fails in suite | State leaking between tests | Add `Test Setup: Reset Emulation` |
| `Cannot create terminal tester` | `showAnalyzer` on same UART | Remove `showAnalyzer` from .resc when running tests |

### Snapshots on Failure

Renode can save machine state when a test fails. Failed snapshots go to the `snapshots/` directory alongside logs. You can load them later:

```bash
renode -e "Load @snapshots/test_name.fail0.save"
```

---

## Quick Reference Card

```robot
# Setup
Suite Setup                     Setup
Suite Teardown                  Teardown
Test Setup                      Reset Emulation
Resource                        ${RENODEKEYWORDS}

# Machine
Execute Command                 mach create "name"
Execute Command                 machine LoadPlatformDescription @file.repl
Execute Command                 sysbus LoadELF @file.elf

# Execution
Start Emulation
Execute Command                 emulation RunFor "1.0"

# UART
Create Terminal Tester          sysbus.usart2
Wait For Line On Uart           text    timeout=5
Write Line To Uart              text

# GPIO
${state}=    Execute Command    peripheral.LED State
Execute Command                 peripheral.Button Press
Execute Command                 peripheral.Button Release

# Assertions
Should Contain                  ${var}    expected
Should Not Be Equal             ${a}    ${b}
```
