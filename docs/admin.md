# Admin interface

The bridge provides a fully local interface at `http://<ESP32-IP>/`. No external scripts or cloud services are loaded. Configuration changes take effect only after a controlled restart.

## One-time installation and first access

1. Use a compatible classic ESP32 with at least **4 MB flash**. Verified PlatformIO targets are `esp32dev` and `m5stack-atom`.
2. **Do the first migration from old firmware over USB**, not through the previous OTA form. The new layout uses two OTA slots of `0x1e0000` bytes each. The new firmware does not fit into the old default slots. A normal PlatformIO USB upload also writes the new partition table. The existing NVS region stays at the same address.
3. Open the serial monitor at 115200 baud. On first boot, the firmware prints user `admin` and a random device-specific admin password. There is no shared default password.
4. Open the bridge IP in your browser. By default, no web login is required. If Wi-Fi placeholders are still present, the protected setup Wi-Fi `SwitchBot-XXXX` starts immediately; alternatively, it starts after 60 seconds without Wi-Fi connectivity. Its password is the admin password. Interface URL: `http://192.168.4.1/`.
5. Enter Wi-Fi, MQTT and device settings, save, and wait for the connectivity test. Then find the new IP in your router and reload the page.

```powershell
python -m platformio run -d "PlatformIO Files/SwitchBot-BLE2MQTT-ESP32" -e esp32dev
# Run only after connecting the board and selecting the correct board/port:
python -m platformio run -d "PlatformIO Files/SwitchBot-BLE2MQTT-ESP32" -e esp32dev -t upload
```

The Arduino IDE file contains the full interface and configuration logic. For Arduino IDE builds, you must use the same partition table (`PlatformIO Files/SwitchBot-BLE2MQTT-ESP32/partitions-admin.csv`). This installation path is not automatically verified; PlatformIO is the verified build path. After USB migration, later compatible firmware updates can again be done through the new interface.

## Language

The header lets you switch between **Deutsch** and **English**. On first visit, browser language is used: German for `de`, otherwise English. The choice is stored locally in the browser. Unsaved form inputs are preserved when switching. Status cards, dialogs and error messages also switch language. No translation services are contacted.

## Settings

- Wi-Fi SSID/password, DHCP or static IPv4 with gateway, subnet mask and DNS.
- Bridge hostname, plus MQTT broker, port, username, password and base topic. Empty username means anonymous MQTT login. MQTT remains unencrypted.
- Up to **16 devices**: Bot, Curtain, Meter, Contact, Motion and Plug Mini. Names/MAC addresses must be unique. Bot passwords and Home Assistant entity type (switch/button/light) can be changed.
- Initial scan duration, rescan interval and retry count. Further expert options remain in source.
- Status request for already saved devices, without actuating a Bot.
- JSON export/import, admin password change, restart and OTA upload.

Device names are also used in MQTT topics. Renaming therefore changes topic addresses. The interface only accepts simple names with letters, digits, hyphens or underscores. Stored JSON configuration is limited to **3500 bytes** so active and candidate configurations can be stored together in the existing NVS partition. Very long credentials can reduce maximum device count.

## Passwords

API responses and exports contain **no stored passwords**. Leaving a password field empty keeps the existing password; the remove checkbox explicitly clears it. For imported devices, existing Bot passwords are retained only when MAC and type match, even if the name changed. On a different ESP32, credentials must be entered again.

Password protection is off by default, including after updating from firmware without this option. Under Administration, enable password protection, enter a password with 12–63 characters twice, and click save. Username is `admin`. The setting persists across restarts and protects interface, API and OTA. To disable it, clear the checkbox and save. Write operations require a random session token in both modes. The setup Wi-Fi password is still required. HTTP Basic is not HTTPS, so use only on trusted local networks without public port forwarding. NVS stores credentials in device flash; physical access is not mitigated. Legacy `otaPass`/`otaUserId` defaults no longer grant access to the admin interface.

## Save, test and rollback

Active configuration stays in place when saving. The new configuration is written separately and tested after restart. **Wi-Fi and MQTT must be reachable within 75 seconds, and MQTT must stay connected for at least five seconds.** During this test, BLE processing pauses and config changes/OTA are blocked. Only a successful test promotes the candidate to active configuration.

If testing or writing fails, or if the ESP32 is reset during test, it boots with the previous configuration. With weak Wi-Fi or intentionally disabled broker, even an otherwise valid change can be rolled back. Reload status and interface after restart.

Before changing devices, topic or broker, old Home Assistant discovery entries are deleted through the previous broker. If devices/discovery already exist, that broker must be reachable. Cleanup uses MQTT QoS 0 and is therefore not transactional under simultaneous network loss. Historical orphan topics outside the current configuration are not deleted globally. New discovery is published only after successful config test; on rollback, old discovery is republished.

**Mesh limitation:** Device, broker and topic changes are blocked on mesh nodes because discovery/topics can be shared. Wi-Fi and polling parameters remain configurable. Multi-node mesh changes still require coordinated migration.

## Recover access

Open serial monitor and reset. **Hold BOOT immediately after startup** when `Admin setup: hold BOOT ...` appears; hold for three seconds. Do not hold BOOT before power-on, or ROM flashing mode may start instead. Firmware generates a new admin password, prints it to serial, and opens protected setup Wi-Fi. Device configuration is not erased.

Without Wi-Fi, protected setup access starts automatically after 60 seconds. Once Wi-Fi reconnects, setup access closes after ten minutes. BOOT setup mode uses GPIO0; for boards without that button, GPIO0 must be toggled accordingly after boot.

## Development and checks

`admin/index.html`, `admin/i18n.js`, `admin/translations.json`, `admin/config_core.h`, `admin/config_json.h` and `admin/runtime.inc` are editable sources. Then run `python tools/sync_admin.py`: compressed UI and C++ code are embedded into both standalone firmware files. Do not edit generated sections directly.

```
python tools/sync_admin.py --check
python -m platformio run -d "PlatformIO Files/SwitchBot-BLE2MQTT-ESP32" -e esp32dev -e m5stack-atom
python -m unittest discover -s tests -v
python tools/preview_admin.py
```

The preview at `http://127.0.0.1:8765` simulates API responses and does not change hardware. Codec/rollback tests need ArduinoJson headers installed by PlatformIO; without them these tests are explicitly skipped. CI also runs them after firmware build.

Before deployment on real hardware, validate: USB migration, first login, physical access recovery, intentionally wrong Wi-Fi/MQTT credentials, power interruption during test, successful post-restart promotion, discovery after rename/delete, and successful OTA switching between both slots. These hardware tests were not executed during development without a connected ESP32.

Test bench (2026-09-19): Both PlatformIO targets build successfully (about 1.33 MB each, 67.6% of OTA slot). All five tests pass on Windows and Linux, including codec, authentication and simulated persistent rollback. Browser preview covered load/add/type-change/delete/save/JSON import with no JavaScript console errors. Preview is not a substitute for real hardware connectivity validation.
