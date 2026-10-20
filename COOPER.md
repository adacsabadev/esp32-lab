# ESP32-S3 board handoff

This is the working reference for the ESP32 projects under this folder. Read it
before changing or flashing anything.

> **Friendly note for Cooper:** “Fuck you, Coop — Cody says that.” Just kidding.
> Please leave the hardware, project structure, and evidence trail intact.

## Board identity

- Board: ESP32-S3-DevKitC-1 clone
- SoC: ESP32-S3-WROOM-class module
- Module marking reported by the user: ESP32-S3-N16R8
- Flash/PSRAM suffix: N16R8 — 16 MB flash, 8 MB octal PSRAM
- The exact clone PCB revision and silkscreen were not independently verified.

## USB connections

Both board USB connectors were attached and identified from the Windows PnP
tree:

| Function | Windows port | USB identity | Evidence |
|---|---:|---|---|
| Native ESP32-S3 USB OTG | COM7 | Espressif `VID_303A:PID_1001` | Composite USB device; `MI_00` CDC serial and `MI_02` USB JTAG/serial debug unit |
| USB-to-UART bridge | COM6 | FTDI `VID_0403:PID_6001` | FTDI USB Serial Converter; FTDI `FTSER2K` driver |

The native USB connection is the preferred path for USB/JTAG-aware work and
can expose both serial and JTAG functionality. Use COM6 when specifically
testing the FTDI UART path. COM1 is the unrelated motherboard ACPI serial port.

## Relevant pins

- Onboard addressable RGB LED: GPIO48, confirmed working on this board
  (YD-ESP32-23 2022-V1.3). It is driven with the Arduino core's
  `neopixelWrite()`.
- The board has open solder jumpers (`RGB`, `IN-OUT`, `USB-OTG`); the LED worked
  without bridging any.

## Current project

The active project is:

`E:\VSCODE_COPILOT Root\esp32\blinky-idf`

Files:

- `CMakeLists.txt` — ESP-IDF project definition
- `main\main.c` — green/red RGB blinky, WS2812 on GPIO48 driven via RMT
- `main\CMakeLists.txt` — component registration
- `build\`, `sdkconfig` — generated build state

Second project: `blinky-raw` — same blinky, but the pin is driven with raw
registers (IO_MUX, GPIO matrix, `GPIO.out1_w1ts/w1tc`) and cycle-counter
bit-banged WS2812 timing at 240 MHz. Verified working on the board. It is the
firmware currently flashed.

Both projects use `sdkconfig.defaults` (16 MB flash, QIO). Defaults only apply
when `sdkconfig` is absent: delete it and run `idf.py set-target esp32s3`.
IntelliSense uses `blinky-idf\build\compile_commands.json` via
`.vscode\c_cpp_properties.json` (build once first).

The earlier PlatformIO/Arduino project (`blinky`) was deleted, and PlatformIO
itself (`~\.platformio`, `C:\devtools\platformio-env`) was removed.

Target configuration:

- Framework: ESP-IDF 6.1, target `esp32s3`
- Monitor speed: 115200 baud
- Current behavior: green for about 700 ms, off briefly, red for about 700 ms,
  then repeat (brightness 2/255)

## Tooling

ESP-IDF 6.1 is installed via Espressif's EIM:

- Source: `C:\esp\v6.1\esp-idf`
- Tools: `C:\Espressif\tools`
- Activate in a terminal: `C:\Espressif\tools\Microsoft.v6.1.PowerShell_profile.ps1`

Build and flash from `blinky-idf` after activating:

```powershell
idf.py build
idf.py -p COM7 flash
```

JTAG debugging works over the native USB (COM7). The Espressif USB-JTAG driver
was installed with `eim install-drivers` (elevated). VS Code tasks and the
"ESP32-S3 JTAG (blinky-idf)" launch configuration are in `.vscode\`.

## Setup checklist (new machine or new board)

1. Install ESP-IDF 6.1 with EIM; open a terminal via the activation script above.
2. JTAG driver: in an **elevated** terminal run `eim install-drivers`, then
   **unplug and replug** the board. Zadig is not needed. Verify MI_02 of
   VID_303A PID_1001 is bound to the Espressif libwdi driver.
3. `idf.py set-target esp32s3`, then `idf.py build` (first build ~10 min).
4. Flash: `idf.py -p COM7 flash`.
5. Debug: F5 on "ESP32-S3 JTAG (blinky-idf)". Gotchas already handled:
   - OpenOCD needs `-c "set ESP_RTOS none"` **before** `-f board/esp32s3-builtin.cfg`,
     otherwise GDB fails with "Failed to find thread 1".
   - The OpenOCD task is a `process` task with an args array (PowerShell
     mangles inline quoted `-c` arguments).
   - `monitor` commands go in `postRemoteConnectCommands`, not `setupCommands`.
   - Only 2 hardware breakpoints exist on the S3.
   - Only one OpenOCD may run at a time (`LIBUSB_ERROR_ACCESS` = a stale one).
6. Opening COM6 toggles DTR/RTS and can reset the board; that is not a crash.

## Upload notes

1. Prefer the native USB connection on COM7 for flashing and JTAG.
2. Use COM6 for the FTDI UART console path.
3. If bootloader entry does not happen automatically, hold BOOT, tap RESET,
   release BOOT, and retry the upload.
4. After flashing, use the serial monitor at 115200 baud and check for:
   `ESP-IDF RGB blinky starting; LED GPIO=48`

## Evidence boundary

The COM-port and VID/PID mapping above is live Windows PnP evidence. GPIO48 is
confirmed working on this YD-ESP32-23 board from a real run of the Arduino
firmware; the ESP-IDF build and flash were also verified from actual runs.

