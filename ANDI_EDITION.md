# ESP32 NAT Router - Andi Edition

Diese Variante basiert auf dem aktuellen Stand von `martin-ger/esp32_nat_router` und ergaenzt zunaechst die Funktionen, die im taeglichen Einsatz den groessten Nutzen bringen, ohne den Netzwerkpfad oder NAT-Code unnoetig zu destabilisieren.

## Zielhardware

Das verwendete Board besitzt ein klassisches `ESP32-WROOM-32`-Modul auf einem 38-Pin-DevKit. Fuer den Build ist deshalb das Ziel `esp32` zu verwenden, nicht `esp32c3`.

Passende Firmware-Ausgabe:

- `firmware_esp32/esp32_nat_router.bin`

C3-Unterstuetzung bleibt im Projekt enthalten, ist fuer dieses konkrete Geraet aber nicht relevant.

## Neu in Version 1

- mobile Live-Statusanzeige auf der Startseite
- automatische Aktualisierung alle 5 Sekunden
- Uplink-Status mit IP-Adresse
- Signalstaerke in dBm, Prozent und Balkenanzeige
- Warnung bei getrenntem Uplink oder sehr schwachem Signal
- Warnung, wenn die Weboberflaeche kein Passwort besitzt
- aktuelle Download- und Upload-Datenrate als Schaetzung aus den vorhandenen Zaehlern
- gesamter Download- und Upload-Verbrauch
- Laufzeit seit dem letzten Start
- Uebersicht verbundener Geraete mit Name, IP, MAC und Datenverbrauch
- direkte Verknuepfung zur Client-Verwaltung
- technische Detailansicht bleibt einklappbar erhalten
- Aktualisierung wird pausiert, waehrend der Browser-Tab nicht sichtbar ist

## Bewusst nicht in Version 1

Multi-WLAN-Failover und neue Eingriffe in DHCP, NAT oder Firewall wurden noch nicht eingebaut. Solche Aenderungen sollten auf der konkreten ESP32-Variante gebaut und auf echter Hardware getestet werden, bevor sie als stabile Firmware verteilt werden.

## Build

Das Projekt wird wie das Original mit ESP-IDF gebaut. Das vorhandene Skript `build_all_targets.sh` unterstuetzt ESP32, ESP32-S3, ESP32-C3, ESP32-C5, ESP32-C6 und WT32-ETH01.

Fuer dieses Board gilt:

```bash
idf.py set-target esp32
idf.py build
```

Ein Binary fuer eine andere Zielplattform darf nicht geflasht werden.

## Flashen auf ESP32-WROOM-32

Die geprueften Dateien fuer das 38-Pin-DevKit liegen in `firmware_esp32/`.

Mehrdatei-Flash:

```bash
esptool.py --chip esp32 \
  --before default_reset --after hard_reset write_flash \
  -z --flash_mode dio --flash_freq 40m --flash_size 4MB \
  0x1000 firmware_esp32/bootloader.bin \
  0x8000 firmware_esp32/partition-table.bin \
  0xf000 firmware_esp32/ota_data_initial.bin \
  0x20000 firmware_esp32/esp32_nat_router.bin
```

Alternativ kann das zusammengefuehrte Image geflasht werden:

```bash
esptool.py --chip esp32 \
  --before default_reset --after hard_reset write_flash \
  --flash_mode dio --flash_freq 40m --flash_size 4MB \
  0x0 firmware_esp32/esp32_nat_router-merged.bin
```

## Teststatus

Vollstaendiger ESP-IDF-Build fuer `esp32` wurde mit ESP-IDF v5.4.1 ausgefuehrt. Die erzeugte App passt in die OTA-App-Partition: `0x15b790` von `0x180000`, frei `0x24870` (10%).

Zusaetzlich geprueft:

- `esptool image_info` fuer `firmware_esp32/esp32_nat_router.bin`: Checksum und Validation Hash gueltig
- `esptool image_info` fuer `firmware_esp32/bootloader.bin`: Checksum und Validation Hash gueltig
- statischer Live-Dashboard-Test aus `page_index.h`: JavaScript-Syntax ok, alle Live-IDs vorhanden, Status- und Client-Tabelle vorhanden

Ein Hardwaretest auf dem echten ESP32-WROOM-32 steht weiterhin aus.
