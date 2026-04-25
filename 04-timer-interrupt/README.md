# 04 - Timer Interrupt

Use **TIM2** to generate a periodic hardware interrupt that toggles an LED and
reports tick count over UART — all verified in Renode.

This is the first example that uses the **NVIC** and a peripheral **ISR** inside
the simulator, which is essential for validating real-time firmware behavior.

> **Renode version:** 1.16  
> **Target MCU:** STM32F411 (Cortex-M4)

---

## How It Works

### Timer Configuration

TIM2 is configured as a simple upcounting timer with an update interrupt:

```
Clock source:  16 MHz (HSI)
Prescaler:     15999   → timer tick = 16MHz / 16000 = 1 kHz
Period (ARR):  499     → overflow every 500 ticks  = 500ms
```

When the counter reaches 499 and rolls over, the hardware sets the UIF (Update Interrupt Flag) in `TIM2->SR` and triggers `TIM2_IRQHandler`.


### ISR Flow

```
TIM2 overflow (every 500ms)
  └─ TIM2_IRQHandler
       ├─ Clear UIF flag in TIM2->SR
       ├─ Increment global tick counter
       ├─ Set flag for main loop
       └─ Toggle PA5 (LED)

Main loop (background)
  └─ if flag is set
       ├─ Clear flag
       └─ printf("[TIM2] tick N")
```

The LED toggle happens inside the ISR for precise timing. The printf happens in
the main loop because UART writes are too slow for an ISR.

## Build & Run


```bash
make
renode timer-interrupt.resc
# Then: start
```

Expected UART output:
```
TIM2 started
[TIM2] tick 1
[TIM2] tick 2
[TIM2] tick 3
...
```

---

## Automated Tests

```bash
renode-test tests/test_timer.robot --results-dir tests/results
```

### Test Cases

| Test | What It Verifies |
|------|-----------------|
| `Should Print TIM2 Started` | Timer initializes without crashing |
| `Should Print First Tick After 500ms` | ISR fires and sets flag for main loop |
| `Should Print Sequential Ticks` | ISR fires repeatedly (tick 1, 2, 3) |
| `LED Should Toggle On Each Tick` | PA5 changes state in the ISR |
| `Boot Message Should Precede Timer Ticks` | Init sequence is correct |

---