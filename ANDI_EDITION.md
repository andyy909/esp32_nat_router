# ESP32 NAT Router – Andi Edition

Diese Variante basiert auf dem aktuellen Stand von `martin-ger/esp32_nat_router` und ergänzt zunächst die Funktionen, die im täglichen Einsatz den größten Nutzen bringen, ohne den Netzwerkpfad oder NAT-Code unnötig zu destabilisieren.

## Zielhardware

Das verwendete Board besitzt ein klassisches `ESP32-WROOM-32`-Modul auf einem 38-Pin-DevKit. Für den Build ist deshalb das Ziel `esp32` zu verwenden, nicht `esp32c3`.

Passende Firmware-Ausgabe:

- `firmware_esp32/esp32_nat_router.bin`

C3-Unterstützung bleibt im Projekt enthalten, ist für dieses konkrete Gerät aber nicht relevant.

## Neu in Version 1

- mobile Live-Statusanzeige auf der Startseite
- automatische Aktualisierung alle 5 Sekunden
- Uplink-Status mit IP-Adresse
- Signalstärke in dBm, Prozent und Balkenanzeige
- Warnung bei getrenntem Uplink oder sehr schwachem Signal
- Warnung, wenn die Weboberfläche kein Passwort besitzt
- aktuelle Download- und Upload-Datenrate als Schätzung aus den vorhandenen Zählern
- gesamter Download- und Upload-Verbrauch
- Laufzeit seit dem letzten Start
- Übersicht verbundener Geräte mit Name, IP, MAC und Datenverbrauch
- direkte Verknüpfung zur Client-Verwaltung
- technische Detailansicht bleibt einklappbar erhalten
- Aktualisierung wird pausiert, während der Browser-Tab nicht sichtbar ist

## Bewusst nicht in Version 1

Multi-WLAN-Failover und neue Eingriffe in DHCP, NAT oder Firewall wurden noch nicht eingebaut. Solche Änderungen sollten auf der konkreten ESP32-Variante gebaut und auf echter Hardware getestet werden, bevor sie als stabile Firmware verteilt werden.

## Build

Das Projekt wird wie das Original mit ESP-IDF gebaut. Das vorhandene Skript `build_all_targets.sh` unterstützt ESP32, ESP32-S3, ESP32-C3, ESP32-C5, ESP32-C6 und WT32-ETH01.

Für dieses Board gilt:

```bash
idf.py set-target esp32
idf.py build
```

Ein Binary für eine andere Zielplattform darf nicht geflasht werden.

## Teststatus

Der geänderte Header wurde auf C-Syntax und Makro-Kompatibilität geprüft. Ein vollständiger ESP-IDF-Build für `esp32` und ein Test auf der gezeigten ESP32-WROOM-32-Hardware stehen noch aus.
