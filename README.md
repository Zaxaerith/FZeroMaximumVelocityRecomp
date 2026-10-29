# F-Zero: Maximum Velocity Recomp (Windows x64)

An unofficial native static recompilation port of **F-Zero: Maximum Velocity (USA, Europe)** for 64-bit Windows.

This project statically recompiles the original Game Boy Advance binary into native C++ code using **GBARecomp v0.3.0** and runs natively on Windows x64 via Direct3D 11, SDL2 audio, and the official **recomp-ui** launcher.

---

## Technical Baseline

- **Framework**: GBARecomp v0.3.0
- **Launcher GUI**: recomp-ui (`6374aae201b930c97425718249f5d39b2275ca13`)
- **Platform**: Windows 10 / Windows 11 (64-bit)
- **Graphics Backend**: Direct3D 11
- **Audio Backend**: SDL2 (wasapi / directsound)
- **Runtime**: Microsoft Visual C++ 2015–2022/2026 x64 Redistributable (VC CRT DLLs are also deployed app-locally)

---

## Requirements

You must provide your own legally obtained copies of:

1. **Game ROM**:
   - **File**: `F-Zero - Maximum Velocity (USA, Europe).gba`
   - **Internal Code**: `AFZE`
   - **Size**: 4,194,304 bytes (4 MiB)
   - **SHA-1**: `8a08e29ec987f9cbdde21c34d5f7657aa7ba0be6`
   - **CRC32**: `BD5E9798`

2. **GBA BIOS**:
   - **File**: Authentic retail GBA BIOS (`gba_bios.bin` or `bios.bin`)
   - **Size**: 16,384 bytes (16 KiB)
   - **SHA-1**: `300c20df6731a33952ded8c436f7f186d25d3492`

*Note: No copyrighted ROM or BIOS binaries are included with this distribution.*

---

## Quick Start

1. Extract the release ZIP to a folder of your choice.
2. Double-click `FZeroMaximumVelocityRecomp.exe` to launch the **recomp-ui** launcher.
3. In the launcher:
   - Click **Browse** under **ROM** and select your `F-Zero - Maximum Velocity (USA, Europe).gba` file.
   - Click **Browse** under **BIOS** and select your retail `gba_bios.bin` file.
   - Configure display scaling, window mode, audio volume, and input bindings as desired.
4. Once both files are verified, click **PLAY** to start the game!

The launcher remembers your configured ROM and BIOS paths in `rom.cfg` and `bios.cfg`.

---

## Launcher Options & Command Line

- **Skip Launcher on Boot**:
  Check the **Skip Launcher on Boot** option in the launcher settings to start the game directly on future launches.
- **Force Launcher**:
  If you have enabled skip launcher, you can force the launcher to open at any time by running:
  ```cmd
  FZeroMaximumVelocityRecomp.exe --launcher
  ```
- **Direct Launch**:
  To bypass the launcher from command line:
  ```cmd
  FZeroMaximumVelocityRecomp.exe --no-launcher
  ```
- **Command Line Help**:
  ```cmd
  FZeroMaximumVelocityRecomp.exe --help
  ```

---

## Default Controls

### Keyboard

| GBA Button | PC Keyboard Key | Function in Game |
| :--- | :--- | :--- |
| **D-Pad Up** | Up Arrow | Pitch / Menu Up |
| **D-Pad Down** | Down Arrow | Pitch / Menu Down |
| **D-Pad Left** | Left Arrow | Steer Left |
| **D-Pad Right** | Right Arrow | Steer Right |
| **A** | **X** | Accelerate / Confirm |
| **B** | **Z** | Brake / Cancel |
| **L** | **C** | Drift Left |
| **R** | **V** | Drift Right |
| **START** | **Enter** | Pause / Start Race |
| **SELECT** | **Right Shift** | Change View / Select |

### Gamepad

Direct XInput, Xbox, DualShock/DualSense, and generic SDL2 gamepads are supported automatically:
- **Steering**: Left Stick or D-Pad
- **Accelerate (A)**: Face Button Down (A on Xbox / Cross on PlayStation)
- **Brake (B)**: Face Button Left (X on Xbox / Square on PlayStation)
- **Drift (L / R)**: Left Bumper / Right Bumper (or Triggers)
- **Pause (Start)**: Menu / Start
- Custom button mapping is available in the launcher's **Controls** tab.

---

## Save Data

Save files are stored using standard GBA 32 KiB SRAM format in:
```text
saves/fzero_maximum_velocity.sav
```
The `saves` directory is created automatically on your first save. Existing save files from authentic cartridges or GBA emulators (32 KiB uncompressed SRAM) can be placed here and will be loaded seamlessly.

---

## Known Limitations

- **Preview Release (v0.1.0-preview-rc1)**: This is an early preview build. Grand Prix opening tracks (e.g. Bianca City: Stretch Circuit), vehicle physics, Mode 7 rendering, audio, and save systems have been verified.
- **Link Cable / Multiplayer**: GBA link cable serial communication and multiplayer modes are currently not supported.
- **Retail Assets Required**: An authentic ROM and retail BIOS are strictly required for execution.

---

## License & Disclaimer

- The recompilation host, modifications, and project tooling are licensed under the **PolyForm Noncommercial License 1.0.0** (see `LICENSE`).
- Third-party components and libraries are licensed under their respective terms (see `THIRD_PARTY_LICENSES.md`).
- This project is an unofficial fan project and static recompilation port created strictly for educational, research, and noncommercial purposes. It is **not** endorsed by, affiliated with, or sponsored by Nintendo or NdCube.
- All original game assets, music, character designs, and trademarks are the intellectual property of their respective copyright holders.
