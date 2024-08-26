| Supported Targets | ESP32 | ESP32-C3 |
| ----------------- | ----- | -------- |

# ESP32 SERIAL BLE BRIDGE

Author: riozebratubo

Copyright: (c) riozebratubo

Project license: CC BY-NC 4.0

_If you want to make a commercial use of this code, please contact the author._

---

# What is this software?

This software, or `firmware` as it runs on a simple device, is intended to allow an ESP32 module to be a bluetooth BLE "access point". It allows other module or hardware to use an UART interface to send/receive data via bluetooth BLE. This firmware is intended to allow an ESP32 module to replace an HC-10 or similar module.

# Why another ESP32 BLE bridge?

I, the author, wanted something that could easily replace a cheap HC-05, HC-06, HC-09 or HM-10 bluetooth module with a better user experience. Nowadays, ESP32 boards are fairly inexpensive and can be used to their potential on a simple serial bluetooth application. ESP32 allows the bluetooth communication by this firmware to have close to no restrictions on top of the BLE specs.

# Who is this for?

If you want to replace an HM-10 BLE module, or even and older HC-05 or HC-06 transitioning to BLE but keeping a serial-like interface, this project is for you! You may want to replace the HM-10 for a lot of reasons, including:
- The HM-10 and similar modules may have restrictions on which characters you can send over the serial to be transmitted by bluetooth;
- Some modules have broken configuration modes or do not allow certain configuration changes;
- The configuration mode on these modules **is shared with the serial data channel** via sending AT commands, and should be entered quickly whithin some timeframe of the module's boot or via setting a configuration enable pin, which may be difficult to do;
- These modules bluetooth connections ranges are somewhat limited;
- These modules user manuals are difficult to obtain and to assert that they correspond to the physical module you have.

The replacement of an HM-10 board by an ESP32 module with this firmware is plug and play, just use the serial pins defined on the configuration settings and power the module with `vin` and `gnd` pins.

The HC-05 and 06 modules use an older bluetooth spec and protocol, usually referred by Classic Bluetooth. To replace a HC-05 or 06 module, you need to, as well as using those pins, have an application or firmware connecting to your module capable of handling BLE devices (Bluetooth 4.0+). The BLE protocol is an entire other beast compared to the serial bluetooth protocol that HC-05 or 06 uses. If you manage to have support for BLE on the connecting device, though, it is as straightforward as the serial classic bluetooth option because of the subscribe mechanism of BLE. Also, as BLE is a newer bluetooth version, newer devices may only support BLE in the future. TL;DR: Ìf you can use BLE on your connecting device, you should make the change when possible.

# How does it work?

This firmware makes the ESP32 module work as a bluetooth BLE device that **has a single service** and that **has a single characteristic**. You can **write to this characteristic to send data** to the module and **subscribe to the characteristic to receive data** from the module.

* The service default UUID is: `0000FFE0-0000-1000-8000-00805F9B34FB`*

* The characteristic default UUID is: `0000FFE1-0000-1000-8000-00805F9B34FB`*

\* These uuids can be changed on the hardcoded settings or changed via the usb serial interface as described below.

You should not read the single characteristic as part of your communication protocol. Each attempt to read data from it will receive an empty response. That is by design. **To receive data from the characteristic, subscribe to it.**

If your current device does not support subscribing, currently this firmware will not work for your case.

# How to deploy on an ESP32 board?

Very easy! Connect the board with an usb cable, load the project on Visual Studio Code with the extension ESP-IDF installed and click the Fire button on the bottom toolbar.

# Once deployed, how to configure the module?

There are two ways to configure it.

## 1. Hardcode your settings

When opening the project on Visual Studio Code, open `esp32_serial_ble_bridge.c` and look for `initial settings - change here`. Edit the desired data and deploy again to your board with the Fire button.

The individual settings are explained below on the method 2.

## 2. Configure over USB

Plug your board with an usb cable and start a serial terminal application. On windows, you can use `Putty`. On Linux, `minicom` on a terminal window. To connect to the board, use the correct COM (look on your Windows Device Manager or on Linux on `/dev`, whichever `COM` or `tty` appears when you connect the board) with an initial `115200` baud rate, if you did not change the default.

The delimiter of a command is the enter key, more precisely the value '\r', which is the ASCII character with value 13. Please configure the terminal software you're using to only send '\r', if this setting is available.

The commands you can use to configure the board are below.

| Command             | Description |
| ------------------- | ----------- |
| `set name="value"` | Alters the setting defined by `name` to the value given by `value`. Example: `set complete_service_uuid="0000FFE0-0000-1000-8000-00805F9B34FB"`. On any value type you should use quotes. **Note that values modified with this command are not applied immediately, but only when the module is restarted, if they were saved with the `save` command**. If you do not use the `save` command after a series of `set` commands, the configuration changes you made will not have any effect and will be lost after a module restart. |
| `list` | Lists the current settings. Changes made with `set` are not applied to the module immediately, only after a module restart. |
| `save` | Saves the current settings to the board's long-term memory and restarts the module. Even if the board is turned off and on again, it will retain the values you changed. |
| `restart` | Restarts the module. |
| `restore` | Restores the default hardcoded settings defined by the code when you compiled and flashed the firmware and restarts the module. |
|  |  |

The available settings are:

| Setting             | Default value (unmodified) | Description |
| ------------------- | -------------------------- | ----------- |
| `complete_service_uuid` | 0000FFE0-0000-1000-8000-00805F9B34FB | The module only service's UUID. Needs to be a full uuid spec. Example: `00001100-0000-0000-0000-000000000000`. |
| `complete_characteristic_uuid` | 0000FFE1-0000-1000-8000-00805F9B34FB | The service's only characteristic UUID. Needs to be a full uuid spec. Example: `00001111-0000-0000-0000-000000000000`. Needs to be different from the service UUID. Usually, the characteristic UUID differs from the service's one within the X marked bytes: `000000XX-0000-0000-0000-000000000000`. |
| `device_name` | ESPSERIALBLE | The bluetooth announced device name. Should not exceed 15 characters, as the BLE advertising package has a hard limit on size and is very small. |
| `desired_ble_mtu` | 500 | The desired maximum bytes transmited by each message on the BLE channel. Keep in mind that BLE inner protocols will break down long messages into smaller packages anyway. Usually a higher MTU is the result of a negotiation between the communicating parts, and usually a higher MTU allows for a quicker bluetooth communication. |
| `force_mac_address` | "" (empty) | The MAC address that the device should use on all bluetooth operations. If empty, the device will use its own factory mac address (that follows industry standards and usually do not collide). Example: `aa:bb:cc:dd:ee:ff". You can use lowercase or uppercase characters. The full address with 6 blocks need to be specified. |
| `uart_pin_tx` | 17 | The UART data transmission pin on your ESP32 board. |
| `uart_pin_rx` | 18 | The UART data receiving pin on your ESP32 board. |
| `uart_buffer_size` | 2048 | The UART buffer size to use. This is the maximum bytes that can be received by the UART without the module's microcontroller reading it. In practice this is the longer data stream that can be quickly transmitted without waiting time intervals. |
| `uart_baud_rate` | 115200 | The baud rate to use on the UART connection. Should match the connecting UART settings. Needs to be one of the ESP32 supported UART baud rates: 110, 150, 300, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400 or 921600. |
|  |  |  |

# Problems. Bugs. Issues.

Please use the Issues page on this repo to report possible bugs. 

# Contributing

If you want to contribute to this project, please discuss the contribution beforehand, and when you're done just open a pull request. This project is intended to be open to contributions, adding features, bugfixes, documentation changes! Please help making it better if you have the skills and the time.
