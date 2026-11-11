#!/usr/bin/env python3
"""Patch an uncompressed/unencrypted hiddriver360 v0.6-beta XEX with static HORI HORIPAD S (0f0d:00c1) mapping.

Usage (requires xextool):
  1. xextool -e u -c u -o hiddriver-dec.xex hiddriver.xex
  2. python3 tools/patch_horipad_xex.py hiddriver-dec.xex hiddriver-horipad-dec.xex
  3. xextool -e e -c c -o hiddriver-horipad.xex hiddriver-horipad-dec.xex
"""

import argparse
from pathlib import Path

DS3_ENTRY_OFFSET = 0x32008
DS3_BUTTON_TABLE_OFFSET = 0x3BF8

OLD_VID_PID = bytes.fromhex("054c0268")
NEW_VID_PID = bytes.fromhex("0f0d00c1")

# Original kDualShock3ButtonMapping button indices (A, B, X, Y, LB, RB, LT, RT, Back, Start, LS, RS, Xbox, Left, Right, Up, Down)
OLD_INDICES = [14, 13, 15, 12, 10, 11, 8, 9, 0, 3, 1, 2, 16, 7, 5, 4, 6]

# HORI HORIPAD S (0f0d:00c1) button indices (positional Xbox 360 layout; D-pad uses HID hat switch so button dpad entries are disabled with 200)
NEW_INDICES = [1, 2, 0, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 200, 200, 200, 200]


def patch_xex(input_path: Path, output_path: Path) -> None:
    data = bytearray(input_path.read_bytes())

    actual_vid_pid = bytes(data[DS3_ENTRY_OFFSET : DS3_ENTRY_OFFSET + 4])
    if actual_vid_pid != OLD_VID_PID:
        raise ValueError(
            f"Unexpected bytes at 0x{DS3_ENTRY_OFFSET:x}: {actual_vid_pid.hex()} (expected {OLD_VID_PID.hex()}). "
            "Make sure the input XEX is decrypted and uncompressed (`xextool -e u -c u`)."
        )

    data[DS3_ENTRY_OFFSET : DS3_ENTRY_OFFSET + 4] = NEW_VID_PID

    for i, (old_idx, new_idx) in enumerate(zip(OLD_INDICES, NEW_INDICES)):
        offset = DS3_BUTTON_TABLE_OFFSET + i * 8
        if data[offset] != old_idx:
            raise ValueError(
                f"Unexpected button index at 0x{offset:x}: {data[offset]} (expected {old_idx})"
            )
        data[offset] = new_idx

    output_path.write_bytes(data)
    print(f"Patched HORI HORIPAD S (0f0d:00c1) static mapping -> {output_path}")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("input", type=Path, help="Decrypted, uncompressed hiddriver XEX")
    parser.add_argument("output", type=Path, help="Output patched XEX")
    args = parser.parse_args()
    patch_xex(args.input, args.output)


if __name__ == "__main__":
    main()
