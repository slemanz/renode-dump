# 02 - UART Printf

Transmit formatted messages over **USART2** and verify them inside Renode using
the Terminal Tester.

This example is the foundation for all future projects — virtually every
firmware test needs a serial debug channel.

> **Renode version:** 1.16  
> **Target MCU:** STM32F411 (Cortex-M4)

---

## Project Structure

```
02-uart-printf/
├── Src/
│   └── main.c                    # Firmware: UART init + printf every 1s
├── Build/
│   ├── flash.elf                 # Compiled firmware
│   └── flash.bin
├── Makefile
├── uart-printf.resc              # Renode script (opens UART terminal)
├── stm32f411_uart.repl           # Platform overlay (LED on PA5)
├── tests/
│   └── test_uart.robot           # Robot Framework tests (UART verification)
└── README.md
```

---

## How It Works

The firmware does three things:

1. Configures **USART2** at 115200 baud (PA2=TX, PA3=RX) — the same UART connected to the ST-Link virtual COM port on a real Nucleo board
2. Redirects `printf` to UART via the `__io_putchar` hook (newlib)
3. Prints `"Boot OK"` once, then `"[N] LED toggled"` every second

```c
int main(void)
{
    systick_init(TICK_HZ);
    uart2_init();
    led_init();

    uart_puts("Boot OK\r\n");

    uint32_t count = 0;
    uint64_t last_tick = ticks_get();

    while (1)
    {
        if ((ticks_get() - last_tick) >= 1000u)
        {
            count++;
            GPIO_ToggleOutputPin(GPIOA, GPIO_PIN_NO_5);
            printf("[%lu] LED toggled\r\n", (unsigned long)count);
            last_tick = ticks_get();
        }
    }
}
```

### Printf Redirection

The newlib `_write` syscall (in `syscalls.c`) calls `__io_putchar` for each character. We override it to send bytes to UART2:

```c
int __io_putchar(int ch)
{
    UART_WriteByte(UART2, (uint8_t)ch);
    return ch;
}
```

---

## Build

```bash
cd 02-uart-printf
make
```

---

## Run in Renode

### Interactive (with terminal window)

```bash
cd 02-uart-printf
renode uart-printf.resc
```

A UART terminal window will open automatically (`showAnalyzer`). Type `start` in the Renode monitor to begin simulation. You should see:

```
Boot OK
[1] LED toggled
[2] LED toggled
[3] LED toggled
...
```

### Headless

```bash
renode --disable-xwt -e "include @uart-printf.resc; start; sleep 5; quit"
```

---

## Automated Tests

```bash
renode-test tests/test_uart.robot --results-dir tests/results
```

### Test Cases

| Test | What It Verifies |
|------|-----------------|
| `Should Print Boot Message` | UART is configured correctly and "Boot OK" appears |
| `Should Print Toggle Messages` | Periodic messages arrive at ~1s intervals |
| `Should Print Sequential Counter` | Counter increments: [1], [2], [3]... |
| `LED Should Toggle With UART Messages` | LED state changes correlate with printed messages |
| `Boot Message Should Appear Before Toggle` | Init sequence is correct (boot → toggle) |

---

## Renode Concepts Covered

### Terminal Tester (Robot Framework)

The `Create Terminal Tester` keyword sets up an internal listener on a UART
peripheral. Combined with `Wait For Line On Uart`, it lets you assert that
specific strings appear within a timeout:

```robot
Prepare Machine
    ...
    Create Terminal Tester      sysbus.usart2

Should Print Boot Message
    Prepare Machine
    Start Emulation
    Wait For Line On Uart       Boot OK    timeout=5
```

This is the primary way to test firmware behavior in Renode — your firmware
talks via UART, and the test framework listens.

### showAnalyzer vs Terminal Tester

- **`showAnalyzer`** (in `.resc`) — Opens a GUI window showing UART output in
real time. Great for interactive debugging. Does NOT work in headless/test mode.
- **`Create Terminal Tester`** (in `.robot`) — Programmatic listener used in
automated tests. Works headless. Cannot be used at the same time as
`showAnalyzer` on the same UART.

In tests, we only use `Create Terminal Tester`. In interactive sessions, we use
`showAnalyzer`.

### UART in Renode's STM32F4 Platform

Renode's built-in `stm32f4.repl` already defines USART1, USART2, and USART6 at
the correct memory-mapped addresses. The UART driver writes to the data
register, and Renode intercepts it — no physical TX/RX lines needed.

Key addresses (from the STM32F411 reference manual):
- USART2: `0x40004400`
- USART1: `0x40011000`
- USART6: `0x40011400`