# Xbox 360 Controller Diagnostic XEX

This companion app displays and logs the final XInput state seen by the Xbox 360 for all four player slots. It can help determine whether a controller reaches XInput and whether buttons, triggers, sticks, or connection state change there.

## RGH / Aurora diagnostic workflow

1. In Aurora, connect to the Xbox's FTP server using the IP address, port, username, and password configured on your console. Do not put your FTP password in a public issue or comment.
2. Copy `horipad_diag.xex` to a folder on the HDD, for example `/Hdd1/Apps/ControllerDiagnostic/` (create the folder first if needed). `Hdd1` is the HDD name shown by Aurora FTP; the console path is `Hdd:\Apps\ControllerDiagnostic\`.
3. Make sure hiddriver is loaded, then connect the controller you are diagnosing and launch the XEX from Aurora. The diagnostic XEX is an app, not a plugin; do not add it to `launch.ini`.
4. On the TV, check Player 1–4. Press and release one button at a time, then test the sticks and D-pad. Repeat the same sequence for each controller you want to compare. `disconnected` means XInput does not expose that slot at that moment.
5. Press Xbox Guide to exit the diagnostic and return to Aurora. If Aurora FTP stopped responding while the XEX was in the foreground, reconnect now. Download `/Hdd1/Apps/ControllerDiagnostic/horipad_diag.log`.

The log contains the XInput states observed during that run, including brief connect/disconnect events. Compare it with the same button sequence captured on a PC. The diagnostic does not capture USB reports inside the Xbox, so it cannot show the arcade stick's HID descriptor or identify its VID:PID. Get VID:PID from the PC (for example, `lsusb` on Linux) and include it with the controller model when asking for help.

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
