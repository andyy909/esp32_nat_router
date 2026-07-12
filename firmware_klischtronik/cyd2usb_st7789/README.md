# ESP32 NAT Router Mod for ESP32-2432S028 CYD2USB

This firmware is intended only for the dual-USB board revision with an `ST7789`
display and `XPT2046` touch controller. It uses 4 MB flash and two 1920 KB OTA
slots.

## Update Without Losing Settings

If this CYD partition table is already installed, flash only the application:

```bash
python -m esptool --chip esp32 --port COM4 --baud 460800 \
  --before default_reset --after hard_reset write_flash \
  0x10000 esp32_nat_router.bin
```

This preserves Wi-Fi settings and NVS. Replace `COM4` with the actual port if
necessary.

## Complete First Installation

The first installation replaces the bootloader, partition table, and OTA
metadata. NVS starting at `0x9000` is not written unless the chip was erased
beforehand.

```bash
python -m esptool --chip esp32 --port COM4 --baud 460800 \
  --before default_reset --after hard_reset write_flash \
  --flash_mode dio --flash_freq 40m --flash_size 4MB \
  0x1000 bootloader.bin \
  0x8000 partition-table.bin \
  0xe000 ota_data_initial.bin \
  0x10000 esp32_nat_router.bin
```

Do not use files from `firmware_esp32/` for this display board. Its application
starts at `0x20000`, and its partition table is incompatible with this profile.

## Verified Status

- ESP-IDF 5.4.1, target `esp32`
- Application size: `0x19c1e0`; free OTA slot space: `0x43e20` (14%)
- Flash hash verified with esptool
- Startup, ST7789 mode 3, XPT2046 touch, uplink, AP, DHCP, NAT, and web server tested
- Wi-Fi scanning, results, network selection, and on-screen keyboard tested on hardware
- Scan navigation protected against trailing touch events; client name and MAC address use separate lines
- XPT2046 readings filtered and debounced; repeated Wi-Fi scans tested without unintended jumps to the status page
- Dark panel mode, client list, and a dedicated Back button are included
