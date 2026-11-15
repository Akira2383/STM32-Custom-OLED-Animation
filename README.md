# STM32 Custom OLED UI & State Machine

This project is an interactive display system based on the **STM32F103C8T6** microcontroller, driving a 0.96-inch I2C OLED (SSD1306). 

Unlike basic text-only demos, this project implements a complete **Finite State Machine (FSM)** architecture to handle custom bitmap rendering, boot animations, and an interactive terminal menu UI.

##  UI Showcase

The system boots up with a custom 128x64 Rhodes Island logo, featuring a normal/inverted blinking animation. After the boot sequence, it transitions smoothly into an "Arknights" terminal-style menu with a moving cursor.

![UI Showcase](https://cdn.jsdelivr.net/gh/Akira2383/STM32-OLED-Interactive-UI@main/images/ui_showcase.gif)

##  Key Features

*   **Custom Bitmap Rendering:** Implemented a low-level `ShowPicture` function to parse 1024-byte hex arrays into OLED GRAM pixels, displaying a full-screen 128x64 image.
*   **Finite State Machine (FSM):** Decoupled the UI logic into distinct states (`STATE_BOOT` and `STATE_MENU`), ensuring smooth transitions and preventing rendering conflicts.
*   **Hardware-to-Software I2C Fallback:** Attempted to use the STM32 hardware I2C peripheral (`I2C1`). Due to the infamous "F103 I2C Bus Deadlock" hardware bug causing the system to hang, I gracefully rolled back to a stable software-simulated I2C driver. The UI logic remains completely independent of the underlying driver.
*   **Professional UI/UX Design:** Implemented menu cursor loops with intentional delays for a breathing effect, simulating a real embedded product interface.

##  Hardware Setup

The STM32 is connected to the OLED using flying wires (no breadboard for better stability).
*   **VCC**: 3.3V
*   **GND**: GND
*   **SCL**: PB6 (Software I2C)
*   **SDA**: PB7 (Software I2C)

![Hardware Setup](https://cdn.jsdelivr.net/gh/Akira2383/STM32-OLED-Interactive-UI@main/images/setup.jpg)

##  Tech Stack & Architecture

*   **MCU**: STM32F103C8T6 (ARM Cortex-M3)
*   **Toolchain**: Keil MDK (ARMCC v5)
*   **Version Control**: Git / GitHub Desktop
*   **Image Processing**: PCtoLCD2002 (Column-by-column C51 format)

**Architecture Note:** 
The `main.c` handles only the application logic (FSM and UI rendering). The `oled.c` acts as the hardware abstraction layer (HAL), handling the I2C communication. This decoupling allows for easy future upgrades (e.g., switching to SPI or a different screen).

## How to Build

1. Clone this repository.
2. Open `USER/OLED.uvprojx` in Keil MDK.
3. Build the project (`F7`).
4. Flash the code to the board using DAP-Link or ST-Link (`F8`).
