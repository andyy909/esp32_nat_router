# ESP32 NAT Router - Klischtronik Mod

Diese Variante basiert auf dem aktuellen Stand von `martin-ger/esp32_nat_router` und ergaenzt zunaechst die Funktionen, die im taeglichen Einsatz den groessten Nutzen bringen, ohne den Netzwerkpfad oder NAT-Code unnoetig zu destabilisieren.

## Zielhardware

Das verwendete Board besitzt ein klassisches `ESP32-WROOM-32`-Modul auf einem 38-Pin-DevKit. Fuer den Build ist deshalb das Ziel `esp32` zu verwenden, nicht `esp32c3`.

Passende Firmware-Ausgabe:

- `firmware_esp32/esp32_nat_router.bin`

C3-Unterstuetzung bleibt im Projekt enthalten, ist fuer dieses konkrete Geraet aber nicht relevant.

Zusaetzlich wird das dual-USB `ESP32-2432S028` (CYD2USB) unterstuetzt. Diese
Boardrevision verwendet einen `ST7789` im SPI-Modus 3 und einen
`XPT2046`-Touchcontroller. Sie benoetigt wegen ihrer abweichenden
Partitionierung die Dateien aus `firmware_klischtronik/cyd2usb_st7789/`.

## Neu in Version 1

- mobile Live-Statusanzeige auf der Startseite
- automatische Aktualisierung alle 5 Sekunden
- Uplink-Status mit IP-Adresse
- Signalstaerke in dBm, Prozent und Balkenanzeige
- Warnung bei getrenntem Uplink oder sehr schwachem Signal
- Warnung, wenn die Weboberflaeche kein Passwort besitzt
- aktuelle Download- und Upload-Datenrate aus exakten Byte-Zaehlern
- gesamter Download- und Upload-Verbrauch
- Laufzeit seit dem letzten Start
- Uebersicht verbundener Geraete mit Name, IP, MAC und Datenverbrauch
- direkte Verknuepfung zur Client-Verwaltung
- technische Detailansicht bleibt einklappbar erhalten
- Aktualisierung wird pausiert, waehrend der Browser-Tab nicht sichtbar ist
- neuer Menuepunkt `Zugriffsmodus` mit drei unabhaengigen Schaltern fuer Internet, Kommunikation zwischen ESP32-Clients und Zugriff auf private Netze hinter dem Uplink
- vier Schnellwahlen: nur Internet, nur Geraetenetz, Geraete und Internet sowie alles erlauben
- Zugriffsregeln werden sofort angewendet und dauerhaft in NVS gespeichert
- dunkle 320x240-Touchoberflaeche fuer das CYD2USB mit Live-Status
- WLAN-Scan und Auswahl des Uplink-Netzes direkt am CYD-Display
- Bildschirmtastatur zur Passworteingabe; offene Netze werden ebenfalls unterstuetzt
- echter dunkler Displaymodus ohne invertierte Panel-Farben
- scrollbare Liste der verbundenen Geraete mit Name, IP- und MAC-Adresse
- eigener Zurueck-Button auf der WLAN-Passwortseite

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

Fuer das CYD2USB-Profil:

```bash
idf.py -D SDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.defaults.cyd" set-target esp32
idf.py -D SDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.defaults.cyd" build
```

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
- Browser-Test mit simulierten Router-Antworten fuer Desktop- und Mobilansicht
- dynamische Geraetenamen und Fehlermeldungen auf der Client-Seite werden HTML-sicher ausgegeben
- CYD2USB startet mit ST7789 und XPT2046 ohne Panic oder Watchdog-Reset
- Touchausrichtung, WLAN-Scan mit sechs Ergebnissen, Netzwerkauswahl und Bildschirmtastatur wurden auf echter Hardware geprueft
- regulaerer ESP32-Build: `0x15d270` von `0x180000`, frei `0x22d90` (9%)
- ESP32-S3-N16R8-Build: `0x165090` von `0x300000`, frei `0x19af70` (54%)

Hardwaretests wurden auf klassischen ESP32-WROOM-32-Boards durchgefuehrt. Der
zusaetzliche ESP32-S3-N16R8-Build wurde auf einem HW-678 Rev. 0.2 geflasht und
mit aktivem AP, DHCP, NAT, Webserver und nativer USB-Konsole gestartet. Fuer
diesen S3-Build bleibt PSRAM bewusst deaktiviert, da der Router es nicht
benoetigt und das konkrete Board damit zuverlaessig startet.
