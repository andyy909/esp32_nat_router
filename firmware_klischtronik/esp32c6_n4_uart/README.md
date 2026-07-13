# ESP32 NAT Router Mod for ESP32-DevKit-C6 V1.3 N4

This profile is intended for the ESP32-C6-N4 board with 4 MB flash and a CH343
UART bridge. The router console is available through the USB-C connector marked
`COM` or `UART`.

## Complete Installation

```bash
python -m esptool --chip esp32c6 --port COM8 --baud 460800 \
  --before default_reset --after hard_reset write_flash \
  --flash_mode dio --flash_freq 80m --flash_size 4MB \
  0x0 bootloader.bin \
  0x8000 partition-table.bin \
  0xf000 ota_data_initial.bin \
  0x20000 esp32_nat_router.bin
```

Replace `COM8` with the actual port if necessary. An application-only update
that preserves the partition layout can write only `esp32_nat_router.bin` at
`0x20000`.

## Verified Status

- ESP-IDF 5.4.1, target `esp32c6`
- Application size: `0x17c6c0` of `0x180000`; free space: `0x3940` (1%)
- Flash hash verified with esptool
- CH343 console, router application, and access point tested on hardware
- Tested board: ESP32-DevKit-C6 V1.3, ESPC6-32 N4, revision 0.2
