# Klischtronik Mod fuer ESP32-2432S028 CYD2USB

Diese Firmware ist ausschliesslich fuer die dual-USB-Revision mit `ST7789`
und `XPT2046` bestimmt. Sie verwendet 4 MB Flash und zwei 1920-KB-OTA-Slots.

## Update ohne Verlust der Einstellungen

Wenn bereits diese CYD-Partitionstabelle installiert ist, nur die App flashen:

```bash
python -m esptool --chip esp32 --port COM4 --baud 460800 \
  --before default_reset --after hard_reset write_flash \
  0x10000 esp32_nat_router.bin
```

Dadurch bleiben WLAN-Einstellungen und NVS erhalten. `COM4` bei Bedarf durch
den tatsaechlichen Port ersetzen.

## Vollstaendiger Erst-Flash

Der Erst-Flash ersetzt Bootloader, Partitionstabelle und OTA-Metadaten. NVS ab
`0x9000` wird nicht beschrieben, sofern der Chip vorher nicht geloescht wurde.

```bash
python -m esptool --chip esp32 --port COM4 --baud 460800 \
  --before default_reset --after hard_reset write_flash \
  --flash_mode dio --flash_freq 40m --flash_size 4MB \
  0x1000 bootloader.bin \
  0x8000 partition-table.bin \
  0xe000 ota_data_initial.bin \
  0x10000 esp32_nat_router.bin
```

Nicht die Dateien aus `firmware_esp32/` fuer dieses Displayboard verwenden:
Deren App beginnt bei `0x20000` und deren Partitionstabelle ist anders.

## Gepruefter Stand

- ESP-IDF 5.4.1, Target `esp32`
- App-Groesse: `0x19bf30`, freier Platz im OTA-Slot: `0x440d0` (14%)
- Flash-Hash durch esptool verifiziert
- Start, ST7789-Modus 3, XPT2046-Touch, Uplink, AP, DHCP, NAT und Webserver geprueft
- WLAN-Scan, Ergebnisanzeige, Netzwerkauswahl und Bildschirmtastatur auf echter Hardware geprueft
- dunkler Panelmodus, Clientliste und eigener Zurueck-Button sind enthalten
