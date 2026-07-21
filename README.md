# RGB LED Controller with 16x2 Character LCD

An embedded hardware and software solution featuring an ATmega328P microcontroller designed to manage RGB LED configurations through a custom user interface. The project integrates an optimized firmware architecture with a production-grade printed circuit board layout designed in Altium Designer.

---

## Hardware Specifications

The hardware layer is engineered to provide reliable signal routing, clean power delivery, and an intuitive physical interface for the user:

* **Core Architecture:** ATmega328P (8-bit AVR Microcontroller).
* **Display Interface:** 16x2 Character LCD driven via optimized digital pin configurations with custom character support.
* **Control Output:** Multi-channel RGB LED driver circuitry supporting dynamic brightness adjustments and smooth transition fade animations.
* **PCB Design:** Meticulously routed printed circuit board created in Altium Designer, emphasizing compact trace architecture, minimal loop areas, and absolute signal integrity.
* **Manufacturing Outputs:** Complete production blueprints including schematic sheets and PCB overlay documents are structured within the project deployment.

---

## Firmware Architecture

The software is implemented using structured C++ Object-Oriented Programming (OOP) principles within the Visual Studio Code and PlatformIO ecosystems. It is strictly optimized to minimize runtime memory footprint and ensure runtime stability on resource-constrained microcontrollers:

* **Modular Encapsulation:** Transitioned from legacy procedural C functions to dedicated C++ classes to manage memory, UI flow, and error states independently.
* **MemoryManager:** Handles secure structural tracking of configuration settings within the EEPROM, ensuring static memory offsets are maintained across updates.
* **SelfTest:** Initiates real-time SRAM health checks and automated hardware diagnostics during the boot sequence.
* **ErrorManager:** Implements systematic runtime exception handling, automated system trapping, and forced factory reset safe-guards.
* **Memory Optimization:** Maximizes available resources using low-overhead pointer manipulation, localization of global scopes via extern linkages, data type downscaling (such as integer-to-byte conversions), and widespread usage of the `F()` macro to conserve SRAM.
* **Timer Stability:** Features a robust time tracking and execution interval loop with built-in immunity against long-term operational crashes caused by standard system clock overflow.

---
