# Admin-Oberfläche

Die Bridge stellt unter `http://<ESP32-IP>/` eine vollständig lokale Oberfläche bereit. Es werden keine externen Skripte oder Cloud-Dienste geladen. Konfigurationsänderungen gelten nach einem kontrollierten Neustart.

## Einmalige Installation und Anmeldung

1. Den passenden klassischen ESP32 mit mindestens **4 MB Flash** auswählen. Die geprüften PlatformIO-Ziele sind `esp32dev` und `m5stack-atom`.
2. **Den ersten Wechsel von der alten Firmware per USB durchführen**, nicht über das bisherige OTA-Formular. Das neue Layout hat zwei OTA-Slots mit jeweils `0x1e0000` Bytes. Die neue Firmware passt nicht mehr in die bisherigen Standard-Slots. PlatformIO schreibt beim normalen USB-Upload auch die neue Partitionstabelle. Die vorhandene NVS-Region bleibt an gleicher Adresse.
3. Seriellen Monitor mit 115200 Baud öffnen. Beim ersten Start werden der Benutzer `admin` und ein zufälliges, gerätespezifisches Admin-Passwort angezeigt. Es gibt kein gemeinsames Standardpasswort.
4. Die IP der Bridge im Browser öffnen. Standardmäßig ist keine Anmeldung erforderlich. Sind noch die WLAN-Platzhalter gesetzt, startet direkt das offene WLAN `SwitchBot-XXXX`; alternativ startet es nach 60 Sekunden ohne WLAN-Verbindung. Oberfläche: `http://192.168.4.1/`.
5. WLAN, MQTT und Geräte eintragen, speichern und den Verbindungstest abwarten. Danach die neue IP im Router nachsehen und die Seite neu laden.

```powershell
python -m platformio run -d "PlatformIO Files/SwitchBot-BLE2MQTT-ESP32" -e esp32dev
# Erst nach Anschluss und Auswahl des passenden Boards/Ports ausführen:
python -m platformio run -d "PlatformIO Files/SwitchBot-BLE2MQTT-ESP32" -e esp32dev -t upload
```

Die Arduino-IDE-Datei enthält die gesamte Oberfläche und Konfigurationslogik. Für einen Arduino-IDE-Build muss dieselbe Partitionstabelle gewählt/eingebunden werden (`PlatformIO Files/SwitchBot-BLE2MQTT-ESP32/partitions-admin.csv`). Dieser Installationsweg ist nicht automatisiert geprüft; PlatformIO ist der verifizierte Buildweg. Nach der USB-Migration funktionieren weitere passende Firmwareupdates wieder über die neue Oberfläche.

## Sprache

Im Kopfbereich kann zwischen **Deutsch** und **English** gewechselt werden. Beim ersten Besuch wird die Browsersprache verwendet: Deutsch bei `de`, ansonsten Englisch. Die Auswahl wird lokal im Browser gespeichert. Ungespeicherte Formulareingaben bleiben beim Umschalten erhalten. Statusanzeigen, Dialoge und Fehlermeldungen wechseln ebenfalls die Sprache. Es werden keine Übersetzungsdienste kontaktiert.

## Einstellungen

- WLAN-SSID, Passwort, DHCP oder statische IPv4-Adresse einschließlich Gateway, Maske und DNS.
- Bridge-Hostname sowie MQTT-Broker, Port, Benutzer, Passwort und Basistopic. Kein Benutzer bedeutet anonyme MQTT-Anmeldung. Die Verbindung verwendet wie bisher unverschlüsseltes MQTT.
- Bis zu **16 Geräte**: bestehende Typen Bot, Curtain, Meter, Contact, Motion und Plug Mini. Namen/MAC-Adressen müssen eindeutig sein. Bot-Passwörter und HA-Darstellung als Schalter, Taster oder Licht können geändert werden.
- Initiale Scan-Dauer, Rescan-Intervall und Retry-Anzahl. Weitere Expertenoptionen bleiben im Quellcode.
- Statusabfrage für bereits gespeicherte Geräte, ohne Betätigen des Bots.
- JSON-Export und -Import, Admin-Passwortwechsel, Neustart und OTA-Upload.

Die Namen werden auch in MQTT-Topics verwendet. Umbenennen ist daher eine Änderung der Topic-Adresse. Die Oberfläche lässt nur einfache Namen mit Buchstaben, Zahlen, Bindestrichen oder Unterstrichen zu. Die gesamte gespeicherte JSON-Konfiguration ist auf **3500 Bytes** begrenzt, damit aktive und neue Konfiguration gemeinsam in der bisherigen NVS-Partition gespeichert werden können. Sehr lange Zugangsdaten können die maximal mögliche Gerätezahl reduzieren.

## Passwörter

API-Antworten und Exporte enthalten **keine gespeicherten Passwörter**. Ein leeres Eingabefeld behält das bisherige Passwort; die Checkbox zum Entfernen setzt es ausdrücklich leer. Bei importierten Geräten werden vorhandene Bot-Passwörter nur bei identischer MAC und identischem Typ übernommen, auch wenn sich der Name ändert. Auf einem anderen ESP32 müssen die Zugangsdaten neu eingegeben werden.

Der Passwortschutz ist standardmäßig ausgeschaltet, auch nach einem Update von einer Version ohne diese Option. Unter Administration „Passwortschutz aktivieren“ auswählen, ein Passwort mit 12–63 Zeichen zweimal eingeben und „Passwortschutz speichern“ drücken. Der Benutzername lautet `admin`. Die Einstellung bleibt nach Neustarts erhalten und schützt Oberfläche, API und OTA. Zum Abschalten das Häkchen entfernen und speichern. Schreibende Aufrufe benötigen in beiden Modi einen zufälligen Sitzungstoken. HTTP Basic ist kein HTTPS. Deshalb nur im vertrauenswürdigen lokalen Netzwerk betreiben und keine öffentliche Portweiterleitung einrichten. NVS speichert Zugangsdaten im Gerätespeicher; physischer Zugriff ist dadurch nicht abgewehrt. Die alten `otaPass`-/`otaUserId`-Startwerte sind kein Zugang mehr zur Admin-Oberfläche.

## Speichern, Test und Rollback

Die aktive Konfiguration bleibt beim Speichern erhalten. Die neue Konfiguration wird separat geschrieben und nach dem Neustart getestet. **WLAN und MQTT müssen innerhalb von 75 Sekunden erreichbar sein und die MQTT-Verbindung mindestens fünf Sekunden stehen.** Während der Prüfung ist BLE-Verarbeitung angehalten; Konfigurationsänderungen und OTA sind gesperrt. Erst nach erfolgreicher Prüfung ersetzt der Kandidat die aktive Konfiguration.

Scheitert die Prüfung, das Schreiben oder wird der ESP32 während der Prüfung zurückgesetzt, startet er wieder mit der vorherigen Konfiguration. Bei schlechtem WLAN oder absichtlich abgeschaltetem Broker kann deshalb auch eine ansonsten korrekte Änderung zurückgerollt werden. Statusanzeigen und die Oberfläche müssen nach dem Neustart neu geladen werden.

Vor Änderungen an Geräten, Topic oder Broker werden die alten HA-Discovery-Einträge über den bisherigen Broker entfernt. Wenn bereits Geräte/Discovery existieren, muss dieser erreichbar sein. Das Löschen nutzt MQTT QoS 0 und ist deshalb keine transaktionale Garantie bei einem gleichzeitigen Netzwerkausfall. Historische fremde/orphaned Topics außerhalb der aktuellen Konfiguration werden nicht pauschal gelöscht. Neue Discovery wird erst nach erfolgreicher Konfigurationsprüfung veröffentlicht; bei Rollback wird die alte wieder veröffentlicht.

**Mesh-Grenze:** Geräte-, Broker- und Topic-Wechsel sind im Mesh gesperrt, da Discovery und Topics gemeinsam verwendet werden können. WLAN und Abfrageparameter bleiben konfigurierbar. Änderungen an mehreren Mesh-Knoten benötigen weiterhin eine koordinierte Migration.

## Zugang wiederherstellen

Seriellen Monitor öffnen und neu starten. **BOOT unmittelbar nach dem Start gedrückt halten**, wenn die Meldung `Admin setup: hold BOOT ...` erscheint; drei Sekunden halten. Nicht schon beim Einschalten halten, sonst kann der ROM-Flashmodus starten. Die Firmware erzeugt ein neues Admin-Passwort, zeigt es seriell an und öffnet das offene Einrichtungs-WLAN. Die Gerätekonfiguration wird dabei nicht gelöscht.

Ohne WLAN startet der Einrichtungszugang nach 60 Sekunden automatisch. Ist WLAN wieder verbunden, wird der Zugang nach zehn Minuten geschlossen. Der BOOT-Einrichtungsmodus verwendet GPIO0; bei Boards ohne diese Taste muss GPIO0 nach dem Start entsprechend geschaltet werden.

## Entwicklung und Prüfung

`admin/index.html`, `admin/i18n.js`, `admin/translations.json`, `admin/config_core.h`, `admin/config_json.h` und `admin/runtime.inc` sind die bearbeitbaren Quellen. Anschließend `python tools/sync_admin.py` ausführen: Die komprimierte Oberfläche und der C++-Code werden in beide eigenständig nutzbaren Firmwaredateien eingebettet. Nicht direkt in den generierten Bereichen ändern.

```
python tools/sync_admin.py --check
python -m platformio run -d "PlatformIO Files/SwitchBot-BLE2MQTT-ESP32" -e esp32dev -e m5stack-atom
python -m unittest discover -s tests -v
python tools/preview_admin.py
```

Die Vorschau unter `http://127.0.0.1:8765` simuliert API-Antworten und verändert keine Hardware. Die Codec-/Rollback-Tests benötigen die durch PlatformIO installierten ArduinoJson-Header; ohne sie werden diese Tests ausdrücklich übersprungen. In CI laufen sie zusätzlich nach dem Firmwarebuild.

Vor Einsatz auf dem Gerät prüfen: USB-Migration, erster Login, physische Zugangswiederherstellung, absichtlich falsche WLAN-/MQTT-Zugangsdaten, Stromunterbrechung während des Tests, erfolgreiche Übernahme nach Neustart, Discovery nach Umbenennen/Löschen sowie erfolgreicher OTA-Wechsel zwischen beiden Slots. Diese Hardwaretests wurden bei der Entwicklung ohne angeschlossenen ESP32 nicht ausgeführt.

Prüfstand vom 19.09.2026: Beide PlatformIO-Ziele bauen erfolgreich (jeweils rund 1,33 MB, 67,6 % des OTA-Slots). Alle fünf Tests bestehen unter Windows und Linux, einschließlich Codec, Authentifizierung und simuliertem persistentem Rollback. In der Browser-Vorschau wurden Laden, Hinzufügen, Typwechsel, Löschen, Speichern und JSON-Import geprüft; dabei traten keine JavaScript-Konsolenfehler auf. Die Vorschau ersetzt keine Prüfung der Verbindungen auf echter Hardware.
