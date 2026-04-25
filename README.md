# Renode Dump

A collection of STM32F411 firmware examples built to learn and practice [Renode](https://renode.io/) — an open-source simulator that lets you run and test embedded firmware without hardware.

Each example adds something new on top of the previous one: GPIO, UART, custom platforms, hardware timers, and ADC simulation. Every project includes automated Robot Framework tests that run headless in CI.

---

## Examples

| # | Project | What It Covers |
|---|---------|---------------|
| 01 | [Blinky](01-blinky/) | GPIO output, platform overlays, basic Robot tests |
| 02 | [UART Printf](02-uart-printf/) | USART2 at 115200, printf redirection, Terminal Tester |
| 03 | [Custom Platform](03-custom-platform/) | Writing `.repl` files from scratch, button injection in tests |
| 04 | [Timer Interrupt](04-timer-interrupt/) | TIM2, NVIC, ISR-driven GPIO + UART |
| 05 | [ADC Sensor](05-adc-sensor/) | ADC1 simulation, value injection, threshold/hysteresis logic |

---

## Requirements

- `arm-none-eabi-gcc` — to build the firmware
- [Renode 1.16](https://github.com/renode/renode/releases) — to simulate it
- `renode-test` (ships with Renode) — to run the Robot Framework test suites

---

## Running a project

Each directory is self-contained. Pick one and follow its README:

```bash
cd 01-blinky
make
renode blinky.resc
```

To run the automated tests:

```bash
renode-test tests/test_blinky.robot --results-dir tests/results
```

---

## Docs

- [Renode cheatsheet](docs/renode-cheatsheet.md) — common monitor commands
- [REPL reference](docs/repl-reference.md) — platform description syntax
