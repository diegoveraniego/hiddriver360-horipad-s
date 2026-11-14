# Xbox 360 Controller Diagnostic XEX

This companion app displays and logs the final XInput state seen by the Xbox 360 for all four player slots. It can help determine whether a controller reaches XInput and whether buttons, triggers, sticks, or connection state change there.

## Run it

1. Copy `horipad_diag.xex` to a folder on the Xbox, for example `Hdd:\Apps\ControllerDiagnostic\`.
2. Launch it from Aurora while the controller driver plugin is loaded.
3. Check Player 1–4 on screen. Connect the controller and press one button at a time. `disconnected` means XInput does not currently expose that slot.
4. Press Xbox Guide to exit. The app writes `horipad_diag.log` beside the XEX. Return to Aurora, then download the log over FTP. With the usual Aurora FTP setup, the remote path is `/Hdd1/Apps/ControllerDiagnostic/horipad_diag.log`.

The prebuilt app was tested on an Xbox 360 RGH console. This is a diagnostic homebrew XEX, not a DashLaunch plugin; it does not change `launch.ini` or controller mappings.

## What it can and cannot show

It reads `XamInputGetState` for slots 1–4. It can show what the Xbox exposes through XInput, including brief connect/disconnect events and simultaneous button states. It does not read the raw USB report, HID descriptor, or VID:PID, and it cannot start or control hiddriver's mapping wizard. If the wizard does not appear, this app can help narrow down whether input reaches XInput, but it cannot identify why USB detection or descriptor parsing failed.

The screen title still says `HORIPAD / XBOX 360 INPUT DIAG`; the sampling code is generic and polls all four XInput slots, so it can be used with other controllers too.

## Source and build

`main.c` and `CMakeLists.txt` are included. Build with [xsdk](https://github.com/xenosworks/xsdk) and an xsdk PPC toolchain:

```sh
make -C /path/to/xsdk build/libxsdk.a
cmake -S . -B build -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=/path/to/xsdk/cmake/xsdk.cmake \
  -DXSDK_TOOLCHAIN=/path/to/xsdk-toolchain
cmake --build build
```

The repository's GPL-3.0 license applies to the included source.
