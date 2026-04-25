# 05 - ADC Sensor

Simulate an **analog sensor** connected to ADC1 channel 0 — inject values from
Renode, verify threshold logic with hysteresis, and control a warning LED.

This is directly applicable to ECU development: pressure sensors, temperature
probes, and battery monitors all feed through ADC channels.

> **Renode version:** 1.16  
> **Target MCU:** STM32F411 (Cortex-M4)

---