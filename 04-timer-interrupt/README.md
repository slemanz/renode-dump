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

## Renode Concepts Covered

### Hardware Timers in Renode

Renode simulates the STM32 general-purpose timers (TIM2–TIM5). The timer
registers (PSC, ARR, DIER, SR, CNT) work as expected, you write to them, and
Renode schedules interrupts based on the virtual clock.

Key point: Renode's timer accuracy depends on the **virtual time** model. When
you call `emulation RunFor "1.0"`, the simulator advances 1 virtual second, and
all timers tick accordingly.

### NVIC Interrupts

The interrupt path works the same as on real hardware:

1. Peripheral sets an interrupt flag (`TIM2->SR |= UIF`)
2. If the interrupt is enabled in the peripheral (`TIM2->DIER |= UIE`)
3. And enabled in the NVIC (`NVIC_ISER0 |= (1 << 28)`)
4. The CPU vectors to the ISR (`TIM2_IRQHandler`)

Renode handles the full NVIC priority/preemption model, so nested interrupts
work too.

### Why This Matters for ECU Simulation

Real ECU firmware is interrupt-driven. Sensor sampling, CAN message processing,
and actuator control all happen in ISRs or in tasks triggered by ISR flags. If
you can't validate interrupts in simulation, you can't simulate an ECU. This
example proves that Renode handles the full path: timer → NVIC → ISR →
GPIO/UART.

## Troubleshooting

**No tick messages appear**  
Check that `TIM2_PCLK_EN()` is called (timer clock must be enabled). Also verify
the NVIC enable: `interrupt_Config(28, ENABLE)`.

**Ticks appear but timing seems wrong**  
Renode uses virtual time, not wall-clock time. `emulation RunFor "1.0"` is
exactly 1 virtual second regardless of host speed. If your PSC/ARR math is
correct for 16MHz, the timing will be accurate.

**LED doesn't toggle**  
Make sure the UIF flag is cleared in the ISR (`TIM2->SR &= ~(1 << 0)`). If you
don't clear it, the ISR fires continuously and starves the main loop.