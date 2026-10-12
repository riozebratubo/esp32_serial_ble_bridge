#!/usr/bin/env python3
"""
ESP32 BLE Serial Terminal for Windows

Target:
  Device name    : ESPSERIALBLE
  Service UUID   : 0000FFE0-0000-1000-8000-00805F9B34FB
  Characteristic : 0000FFE1-0000-1000-8000-00805F9B34FB

Install:
  python -m pip install bleak

Run:
  python esp32_ble_terminal_win.py

Local commands:
  :help
  :quit
  :mode utf8
  :mode hex
  :eol none
  :eol cr
  :eol lf
  :eol crlf
  :hex 41 42 43 0D
"""

import argparse
import asyncio
import codecs

from bleak import BleakClient, BleakScanner

DEFAULT_NAME = "ESPSERIALBLE"
CHAR_UUID = "0000FFE1-0000-1000-8000-00805F9B34FB"

EOL_BYTES = {
    "none": b"",
    "cr": b"\r",
    "lf": b"\n",
    "crlf": b"\r\n",
}


def parse_args():
    p = argparse.ArgumentParser(description="BLE UART terminal for ESP32 serial bridge")
    p.add_argument("--name", default=DEFAULT_NAME,
                   help=f"BLE device name (default: {DEFAULT_NAME})")
    p.add_argument("--eol", choices=EOL_BYTES, default="cr",
                   help="Bytes appended when Enter is pressed (default: cr)")
    p.add_argument("--mode", choices=("utf8", "hex"), default="utf8",
                   help="Receive display mode (default: utf8)")
    p.add_argument("--scan-timeout", type=float, default=8.0,
                   help="BLE scan timeout in seconds (default: 8)")
    return p.parse_args()


async def find_device(name: str, timeout: float):
    print(f"Scanning for {name!r} ...")
    devices = await BleakScanner.discover(timeout=timeout, return_adv=True)

    for _, (device, adv) in devices.items():
        adv_name = adv.local_name or device.name or ""
        if adv_name.casefold() == name.casefold():
            return device

    print("\nDevice not found. BLE devices seen:")
    shown = set()
    for _, (device, adv) in devices.items():
        adv_name = adv.local_name or device.name or "(unnamed)"
        key = (adv_name, device.address)
        if key not in shown:
            shown.add(key)
            print(f"  {adv_name:30s}  {device.address}")
    return None


async def terminal(client: BleakClient, initial_mode: str, initial_eol: str):
    rx_mode = initial_mode
    tx_eol = initial_eol

    # Stateful UTF-8 decoder: preserves incomplete multibyte characters
    # across BLE notification boundaries.
    utf8_decoder = codecs.getincrementaldecoder("utf-8")(errors="replace")

    def on_notify(_sender, data: bytearray):
        nonlocal utf8_decoder
        raw = bytes(data)

        if rx_mode == "hex":
            print("RX HEX:", raw.hex(" ").upper(), flush=True)
        else:
            text = utf8_decoder.decode(raw, final=False)
            if text:
                print(text, end="", flush=True)

    await client.start_notify(CHAR_UUID, on_notify)

    print("\nConnected.")
    print(f"RX display : {rx_mode}")
    print(f"Enter EOL  : {tx_eol.upper()}")
    print("Type :help for local commands.\n")

    while client.is_connected:
        try:
            line = await asyncio.to_thread(input)
        except (EOFError, KeyboardInterrupt):
            print()
            break

        if line.startswith(":"):
            parts = line.strip().split()
            cmd = parts[0].lower()

            if cmd in (":quit", ":q", ":exit"):
                break

            if cmd == ":help":
                print(
                    "Local commands:\n"
                    "  :quit                 disconnect and exit\n"
                    "  :mode utf8|hex        receive display mode\n"
                    "  :eol none|cr|lf|crlf  bytes appended to normal typed lines\n"
                    "  :hex XX XX ...        send raw hexadecimal bytes\n"
                )
                continue

            if cmd == ":mode" and len(parts) == 2 and parts[1].lower() in ("utf8", "hex"):
                rx_mode = parts[1].lower()
                if rx_mode == "utf8":
                    utf8_decoder = codecs.getincrementaldecoder("utf-8")(errors="replace")
                print(f"RX display: {rx_mode}")
                continue

            if cmd == ":eol" and len(parts) == 2 and parts[1].lower() in EOL_BYTES:
                tx_eol = parts[1].lower()
                print(f"Enter EOL: {tx_eol.upper()}")
                continue

            if cmd == ":hex" and len(parts) >= 2:
                try:
                    payload = bytes.fromhex(" ".join(parts[1:]))
                except ValueError:
                    print("Invalid hex. Example: :hex 41 42 43 0D")
                    continue
                await client.write_gatt_char(CHAR_UUID, payload, response=False)
                continue

            print("Unknown local command. Type :help")
            continue

        payload = line.encode("utf-8") + EOL_BYTES[tx_eol]

        try:
            await client.write_gatt_char(CHAR_UUID, payload, response=False)
        except Exception as exc:
            print(f"\nWrite failed: {exc}")
            break

    try:
        await client.stop_notify(CHAR_UUID)
    except Exception:
        pass


async def main():
    args = parse_args()

    device = await find_device(args.name, args.scan_timeout)
    if device is None:
        return 2

    print(f"Found: {device.name or args.name}  [{device.address}]")
    print("Connecting ...")

    def disconnected(_client):
        print("\n*** BLE disconnected ***")

    try:
        async with BleakClient(device, disconnected_callback=disconnected) as client:
            if not client.is_connected:
                print("Connection failed.")
                return 3

            char = client.services.get_characteristic(CHAR_UUID)
            if char is None:
                print(f"Characteristic not found: {CHAR_UUID}")
                print("Available characteristics:")
                for service in client.services:
                    for c in service.characteristics:
                        print(f"  {c.uuid}  {', '.join(c.properties)}")
                return 4

            print(f"Characteristic: {char.uuid}")
            print(f"Properties    : {', '.join(char.properties)}")

            await terminal(client, args.mode, args.eol)

    except Exception as exc:
        print(f"\nBLE error: {exc}")
        return 1

    print("Disconnected.")
    return 0


if __name__ == "__main__":
    raise SystemExit(asyncio.run(main()))
