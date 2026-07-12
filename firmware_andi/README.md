# Andi Dashboard firmware

Verified builds from branch `andi-dashboard-v1`, built with ESP-IDF 5.4.1.

## ESP32-S3 N16R8 HW-678

This profile uses 16 MB Quad flash at 40 MHz, native USB Serial/JTAG, a
16 MB partition table with two 3 MB OTA slots, and no external PSRAM. PSRAM is
not required by the router and is disabled for reliable startup on this board.

For an application-only update that preserves the bootloader, partition table,
OTA state, and NVS settings:

```powershell
esptool --chip esp32s3 --port COM6 --baud 460800 write-flash `
  0x20000 firmware_andi/esp32s3_n16r8/esp32_nat_router.bin
```

For a complete installation without erasing NVS at `0x9000`:

```powershell
esptool --chip esp32s3 --port COM6 --baud 460800 write-flash `
  0x0     firmware_andi/esp32s3_n16r8/bootloader.bin `
  0x8000  firmware_andi/esp32s3_n16r8/partition-table.bin `
  0xf000  firmware_andi/esp32s3_n16r8/ota_data_initial.bin `
  0x20000 firmware_andi/esp32s3_n16r8/esp32_nat_router.bin
```

## Classic ESP32-WROOM-32

For an application-only update of a device already using this branch's
partition layout:

```powershell
esptool --chip esp32 --port COM5 --baud 460800 write-flash `
  0x20000 firmware_andi/esp32_wroom32/esp32_nat_router.bin
```

For a complete installation without erasing NVS at `0x9000`:

```powershell
esptool --chip esp32 --port COM5 --baud 460800 write-flash `
  0x1000  firmware_andi/esp32_wroom32/bootloader.bin `
  0x8000  firmware_andi/esp32_wroom32/partition-table.bin `
  0xf000  firmware_andi/esp32_wroom32/ota_data_initial.bin `
  0x20000 firmware_andi/esp32_wroom32/esp32_nat_router.bin
```

Do not interchange ESP32 and ESP32-S3 files. Port names are examples and may
change after reconnecting a board. A complete installation resets OTA slot
selection but does not touch NVS. Never add `erase-flash` when settings must be
preserved.

The S3 build was flashed to and boot-tested on an ESP32-S3 N16R8 HW-678 rev
0.2. The AP, DHCP server, NAT, web server, native USB console, access-mode
configuration, and password redaction were observed running. The classic ESP32
build completed successfully and passed image/partition size validation.
