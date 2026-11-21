# Repository instructions

## Project and build

- This is a bare-metal STM32F103C8T6 (Cortex-M3) project built in Keil MDK-ARM with ARMCC V5.06.
- Open `USER/OLED.uvprojx` in uVision and build target `Target 1` with `F7`; flash it with `F8` using ST-Link or DAP-Link.
- The project output is `OBJ/OLED.axf` and `OBJ/OLED.hex`. `USER/_build.log` is generated build evidence, not a test suite.
- There is no package manifest, CMake/Make build, automated test framework, or lint configuration. Verification is a successful uVision build followed by hardware execution.
- The STM32F1 device, startup, firmware library sources, include paths, and source membership are defined by `USER/OLED.uvprojx`. When adding or moving a source file, update that project file as well as the include paths in the target configuration.

## Architecture

- `USER/main.c` owns the application finite state machine. It uses `STATE_BOOT`, `STATE_MENU`, and `STATE_SENSOR_PAGE` to sequence boot artwork, menu navigation, and the light-sensor page.
- `USER/ui.c` contains application-level rendering: menu layout, sensor page, and fast 128x64 bitmap drawing. It depends on the OLED and bitmap modules rather than initializing hardware.
- `HARDWARE/OLED/oled.c` is the display hardware abstraction layer. The active transport is software I2C on PB6 (SCL) and PB7 (SDA); the hardware I2C implementation is left as commented deadlock workarounds.
- `USER/images.c` stores display bitmaps. The 128x64 logo is a 1024-byte column/page-oriented byte array intended for `UI_DrawBMP_Fast`.
- `USER/key.c` is a polling key/debounce state machine. `USER/main.c`, `USER/snake.c`, and sensor/menu code must scan keys and consume events through `Key_GetEvent`.
- `SYSTEM/timer/timer.c` provides a volatile millisecond counter from TIM3. Game timing and flash timing use `Timer_GetMs()`; do not replace it with blocking delays in state-machine or game loops.
- `USER/snake.c` is a separate blocking game loop invoked from the terminal menu. It owns snake state and returns to the menu only after its exit condition.
- `USER/bsp_adc.c` samples the analog input on PA0 through ADC1 at 1 kHz from the TIM3 interrupt and maintains a 128-sample ring buffer. Keep the input within the ADC's 0-3.3 V range; microphone modules need suitable bias/amplification.
- `USER/spectrum.c` removes the sample mean, applies a Hann window, performs a fixed-point 128-point FFT, and reduces its 64 bins to 16 bars.
- The Base page is split vertically: `USER/ui.c` draws the waveform only in rows 0-31 and spectrum bars only in rows 32-63.
- STM32 peripheral support comes from `STM32F10x_FWLib`, startup/core files come from `CORE/` and `USER/RTE/`, and system delay facilities come from `SYSTEM/delay/`.

## Hardware and rendering conventions

- OLED coordinates are 128 columns by 64 rows. The in-memory display is `OLED_GRAM[144][8]`, indexed by x and page (y / 8).
- Normal display updates should draw into `OLED_GRAM` and call `OLED_Refresh()`. Use `OLED_Clear_GRAM()` for a frame replacement, then `OLED_Refresh()`, as the Snake implementation does.
- `OLED_Clear()` both zeros and refreshes the physical display, so using it in a tight loop has a display-traffic cost.
- `UI_DrawBMP_Fast()` writes bitmap bytes directly through `OLED_WR_Byte`; preserve its byte order and the `mode` convention (`0` invert, `1` normal).
- Preserve the current pin assignments: software I2C PB6/PB7, light ADC PA0, and input keys PA1/PA2/PA3/PA5/PA6. Check the OLED reset/key pin mapping before changing PA2.

## Code conventions

- The codebase uses ARMCC-era C with project typedefs such as `u8`, `u16`, `u32` from `stm32f10x.h`; follow existing declarations and avoid modern-only C syntax that the ARMCC V5 toolchain would reject.
- Keep hardware initialization, peripheral registers, and ISR behavior near the owning module. `TIM3_IRQHandler` belongs in `timer.c`, and key GPIO/read callbacks belong in `key.c`.
- Use short module prefixes for public APIs: `OLED_`, `UI_`, `Key_`, `ADC_`, `Timer_`, and `Snake_`.
- Preserve the active state-machine loop structure in `main.c`: initialization once, then state dispatch in `while (1)`. New menu actions should return to a state rather than introduce a second app-level event loop unless implementing a self-contained mode such as Snake.
- Key events are edge events. `Key_Scan()` updates state and `Key_GetEvent()` consumes and clears the event, so do not call `Key_GetEvent()` repeatedly for the same press.
- Interrupt-shared variables are `volatile`; the TIM3 millisecond counter is read via `Timer_GetMs()` rather than accessed directly.
