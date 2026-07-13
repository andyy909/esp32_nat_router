# ESP32 NAT Router Mod

This repository extends the current `martin-ger/esp32_nat_router` codebase with
a modern live dashboard, access controls, and dedicated display-board profiles.
The changes are designed to add practical features without destabilizing the
core NAT and networking paths.

## Extended Features

- Mobile live dashboard with automatic updates every five seconds
- Uplink state, IP address, RSSI in dBm and percent, and signal indicator
- Warnings for a disconnected uplink, weak signal, or an unprotected Web UI
- Current upload and download rates based on exact byte counters
- Total traffic, uptime, and connected-device overview
- Client names, IP addresses, MAC addresses, and traffic totals
- Access-mode page with independent controls for Internet access, communication
  between ESP32 clients, and access to private networks behind the uplink
- Four access presets: Internet only, device network only, devices and Internet,
  or unrestricted access
- Access rules are applied immediately and stored in NVS
- Dark touch dashboard for the ESP32-2432S028 CYD2USB
- Asynchronous Wi-Fi scanner, uplink selection, and on-screen keyboard on CYD
- Scrollable connected-device list with separate name and MAC-address lines
- Button-driven status, client, and Wi-Fi scanner pages on the Waveshare
  ESP32-C6-LCD-1.47

## Supported Mod Profiles

| Profile | Package | Notes |
|---|---|---|
| Classic ESP32-WROOM-32 | `firmware_extended/esp32_wroom32/` | 4 MB flash, OTA layout |
| ESP32-S3 N16R8 HW-678 | `firmware_extended/esp32s3_n16r8/` | 16 MB flash, native USB, PSRAM disabled |
| ESP32-2432S028 CYD2USB | `firmware_klischtronik/cyd2usb_st7789/` | ST7789 touch dashboard, dedicated partition layout |
| ESP32-DevKit-C6 V1.3 N4 | `firmware_klischtronik/esp32c6_n4_uart/` | 4 MB flash, CH343 UART console |
| Waveshare ESP32-C6-LCD-1.47 | `firmware_waveshare_c6_lcd_1_47/` | 172x320 display, BOOT-button navigation |

Do not interchange firmware files between targets. The CYD application starts
at `0x10000`, while the standard ESP32 and ESP32-C6 application starts at
`0x20000`.

## Build

The project uses ESP-IDF 5.4.1. A normal ESP32 build can be created with:

```bash
idf.py set-target esp32
idf.py build
```

Build the CYD2USB profile with its dedicated configuration:

```bash
idf.py -B build_cyd -D SDKCONFIG=sdkconfig.cyd \
  -D "SDKCONFIG_DEFAULTS=sdkconfig.defaults;sdkconfig.defaults.cyd" \
  set-target esp32
idf.py -B build_cyd -D SDKCONFIG=sdkconfig.cyd \
  -D "SDKCONFIG_DEFAULTS=sdkconfig.defaults;sdkconfig.defaults.cyd" build
```

Build the Waveshare display profile with:

```bash
idf.py -B build_waveshare_c6_lcd_1_47 \
  -D SDKCONFIG=sdkconfig.waveshare_c6_lcd_1_47 \
  -D "SDKCONFIG_DEFAULTS=sdkconfig.defaults;sdkconfig.defaults.waveshare_c6_lcd_1_47" \
  build
```

## Validation Status

- Classic ESP32-WROOM-32: application `0x15ce50` of `0x180000`, 9% free
- ESP32-S3 N16R8: application `0x15f7c0` of `0x300000`, 54% free
- CYD2USB: application `0x19bd90` of `0x1e0000`, 14% free
- ESP32-C6 N4 UART: application `0x17c6c0` of `0x180000`, 1% free
- Waveshare ESP32-C6-LCD-1.47: application `0x1b7b60` of `0x300000`, 43% free
- Firmware hashes and partition-size checks passed for the packaged builds
- CYD startup, touch, repeated Wi-Fi scans, network selection, keyboard, client
  list, AP, DHCP, NAT, and web server were tested on hardware
- ESP32-S3 N16R8 AP, DHCP, NAT, web server, and native USB console were tested on
  an HW-678 revision 0.2 board
- ESP32-C6 N4 CH343 console, router application, and AP were tested on hardware

## Target Limitations

Features depend on the selected hardware and build profile. The compact
Waveshare profile uses one 3 MB factory application partition and therefore has
no Web UI OTA support. PCAP streaming, remote console, syslog, and mDNS are also
disabled for that profile. Update it over USB or serial.

Multi-Wi-Fi failover is not part of this mod. Changes to DHCP, NAT, or firewall
internals should be built for the affected targets and verified on real hardware
before distribution.
