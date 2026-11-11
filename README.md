# HidDriver 360 — HORI HORIPAD S (Switch USB) Static Mapping

Fork of [EinTim23/hiddriver360](https://github.com/EinTim23/hiddriver360) adding a **built-in static mapping** for the **HORI HORIPAD S for Nintendo Switch** (`VID:PID 0f0d:00c1`) so it works out of the box on Xbox 360 RGH/JTAG without relying on `HDD:\hiddriver.json` or running the mapping wizard on every boot.

## Why this fork exists

On several RGH setups (see [upstream issue #63](https://github.com/EinTim23/hiddriver360/issues/63)), `hiddriver.xex` cannot read or write `HDD:\hiddriver.json` from the plugin context. As a result:
- Dynamic mappings created via the on-screen mapping assistant only live in RAM and are lost after rebooting or unplugging the controller.
- Attempting to map the Xbox Guide button (`Home`) during the assistant in Aurora opens the Xbox Guide overlay and can corrupt the remaining button assignments.

Embedding the HORIPAD S (`0x0F0D:0x00C1`) mapping directly into `kStaticDeviceMappings` in `hiddriver/mapping.cpp` (and patching the prebuilt `v0.6-beta` binary via `tools/patch_horipad_xex.py`) makes the controller work immediately whenever connected.

## Button Mapping (Positional Xbox 360 Layout)

| HORIPAD S (Switch) | HID Button Index (`Usage - 1`) | Xbox 360 Output |
|---|---|---|
| B (Bottom) | `1` | **A** |
| A (Right) | `2` | **B** |
| Y (Left) | `0` | **X** |
| X (Top) | `3` | **Y** |
| L / R | `4` / `5` | **LB** / **RB** |
| ZL / ZR | `6` / `7` | **LT** / **RT** (`255`) |
| Minus (`-`) / Plus (`+`) | `8` / `9` | **Back** / **Start** |
| L3 / R3 (Stick clicks) | `10` / `11` | **LS** / **RS** |
| Home | `12` | **Xbox Guide** |
| D-Pad | HID Hat Switch | **D-Pad** |
| Left / Right Sticks | `X`, `Y`, `Z`, `RZ` (Y/RZ inverted) | **LX/LY**, **RX/RY** |

## Installation (Xbox 360 RGH / DashLaunch)

1. Download `hiddriver-horipad.xex` from the [Releases](../../releases) page.
2. Copy `hiddriver-horipad.xex` to `Hdd:\Plugins\HidDriver\hiddriver-horipad.xex` on your Xbox 360.
3. Add it to `[Plugins]` in `Hdd:\launch.ini`:
   ```ini
   [Plugins]
   plugin2 = Hdd:\Plugins\HidDriver\hiddriver-horipad.xex
   ```
4. Boot the Xbox 360 **with the HORIPAD S unplugged**, then plug in the USB controller once Aurora / the dashboard has loaded (both RGH glitching and `HidAddDevice` hooking require connecting the USB controller after boot).

## Reproducing the binary patch on Linux/Windows

If you do not have the Xbox 360 XDK installed, you can deterministically patch the `v0.6-beta` `hiddriver.xex` binary using `xextool` and `tools/patch_horipad_xex.py`:

```sh
wine xextool.exe -e u -c u -o hiddriver-dec.xex hiddriver.xex
python3 tools/patch_horipad_xex.py hiddriver-dec.xex hiddriver-horipad-dec.xex
wine xextool.exe -e e -c c -o hiddriver-horipad.xex hiddriver-horipad-dec.xex
```

---

## Upstream Attributions
- [EinTim23](https://github.com/EinTim23/) — Reverse engineering the Xbox 360 HID and controller implementation and implementing `hiddriver360`
- [localcc](https://github.com/localcc/) — Low-level USB assistance
- [Rapidjson](https://github.com/Tencent/rapidjson/) — JSON library
- [Lufa](https://github.com/abcminiuser/lufa) — HID report descriptor parser
- [iMoD1998](https://github.com/iMoD1998) — Detours library
