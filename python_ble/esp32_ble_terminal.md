# Python script - BT counterpat Terminal

## Windows


**esp32_ble_terminal_win.py**

Install the one dependency:

```cmd
python -m pip install bleak
```

Then run:

```cmd
python esp32_ble_terminal_win.py
```

It will scan for `ESPSERIALBLE`, connect, find `FFE1`, enable notifications, and give you an interactive terminal.

By default, pressing Enter sends:

```text
\r
```

which is close to PuTTY-style serial terminal behavior.

Useful local commands:

```text
:help
:quit
:mode utf8
:mode hex
:eol none
:eol cr
:eol lf
:eol crlf
:hex 41 42 43 0D
```

One thing implemented better than BLE Serial Pro: the receive UTF-8 decoder is **stateful across BLE notifications**, so if a 2- or 3-byte UTF-8 character is split between BLE packets, it should still render correctly.

Current Bleak 3.0.x supports Windows via the native WinRT Bluetooth stack, including notifications and GATT writes, so no extra BLE driver layer is needed. [bleak.readthedocs.io](https://bleak.readthedocs.io/en/latest/backends/windows.html?utm_source=chatgpt.com)




## Fedora

Bleak automatically switches to its **BlueZ D-Bus backend on Linux**, so the application logic stays the same while the OS Bluetooth path is completely different underneath. [Bleak](https://bleak.readthedocs.io/en/latest/backends/linux.html?utm_source=chatgpt.com)

**esp32_ble_terminal_fedora.py**

On Fedora, I recommend using a small virtual environment:

```bash
python3 -m venv .venv
```

```bash
source .venv/bin/activate
```

```bash
python -m pip install --upgrade pip bleak
```

Then run:

```bash
python esp32_ble_terminal_fedora.py
```

You should get the same behavior as Windows:

- scan for `ESPSERIALBLE`
- connect through BlueZ
- find `FFE1`
- enable notifications
- interactive UTF-8 terminal
- HEX receive mode
- CR/LF/CRLF selection
- raw HEX transmit
- correct multibyte UTF-8 loopback

Bleak officially supports Linux with **BlueZ 5.55 or newer** and uses BlueZ over D-Bus, so Fedora 41/42 is a very natural fit. [Bleak](https://bleak.readthedocs.io/en/latest/backends/linux.html?utm_source=chatgpt.com)

If Bluetooth is already working normally in GNOME/Fedora, you should not need root privileges or special drivers.