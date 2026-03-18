# .repl Platform Description Reference

Complete guide to writing Renode platform files — the `.repl` format that
describes your hardware.

## What Is a `.repl` File?

A `.repl` (Renode Platform Layout) file describes the hardware: CPU, memory
regions, peripherals, and how they connect. It's a declarative text file that
Renode parses to build the simulated machine.

You typically work with two kinds:

- **Base platform** — Pre-built by Renode for known MCUs (e.g.,
`platforms/cpus/stm32f4.repl`). Defines the CPU core, memory map, and standard
peripherals.
- **Overlay** — Your custom file that adds board-specific components (LEDs,
buttons, external sensors) on top of the base.