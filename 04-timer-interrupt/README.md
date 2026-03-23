# 04 - Timer Interrupt

Use **TIM2** to generate a periodic hardware interrupt that toggles an LED and
reports tick count over UART — all verified in Renode.

This is the first example that uses the **NVIC** and a peripheral **ISR** inside
the simulator, which is essential for validating real-time firmware behavior.

> **Renode version:** 1.16  
> **Target MCU:** STM32F411 (Cortex-M4)

---