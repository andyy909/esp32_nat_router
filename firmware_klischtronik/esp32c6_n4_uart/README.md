# Klischtronik Mod fuer ESP32-DevKit-C6 V1.3 N4

Dieses Profil ist fuer das ESP32-C6-N4-Board mit 4 MB Flash und CH343-UART-
Bridge bestimmt. Die Router-Konsole liegt auf der mit `COM` beziehungsweise
`UART` beschrifteten USB-C-Buchse.

## Vollstaendiger Flash

```bash
python -m esptool --chip esp32c6 --port COM8 --baud 460800 \
  --before default_reset --after hard_reset write_flash \
  --flash_mode dio --flash_freq 80m --flash_size 4MB \
  0x0 bootloader.bin \
  0x8000 partition-table.bin \
  0xf000 ota_data_initial.bin \
  0x20000 esp32_nat_router.bin
```

`COM8` bei Bedarf durch den tatsaechlichen Port ersetzen. Ein App-Update ohne
Veraenderung der Partitionen kann nur `esp32_nat_router.bin` bei `0x20000`
schreiben.

## Gepruefter Stand

- ESP-IDF 5.4.1, Target `esp32c6`
- App-Groesse `0x17cae0` von `0x180000`, frei `0x3520` (1 Prozent)
- Flash-Hash durch esptool verifiziert
- CH343-Konsole, Router-App und Access Point auf echter Hardware geprueft
- getestetes Board: ESP32-DevKit-C6 V1.3, ESPC6-32 N4, Revision 0.2
