# Firmware hardening and validation

This fork retains existing device protocols, MQTT topics and unique IDs.

## Changes

- The current admin firmware always checks authentication before OTA flash writes and requires a session token for writes. Missing, failed and aborted uploads do not reboot. Login uses `admin` and a generated per-device password; the old `useLoginScreen`, `otaUserId` and `otaPass` source settings no longer configure access. This is HTTP Basic authentication, not HTTPS. See the [admin and USB migration guide](admin.md) for setup, recovery and the required partition layout.
- Curtain light is a level, not lux. The incompatible illuminance device class is removed without changing its topic or unique ID.
- EspMQTTClient uses its MQTT-only constructor. WiFi is managed separately and retried every 30 seconds when disconnected. Broker failures no longer trigger the library's eight-failure WiFi reset. MQTT retry delay is 10,000 ms instead of 10 ms.
- Scoped queue processing releases its busy flag on early return. Timed rescans accept integers or digit strings from 1 to 300 seconds. Invalid input does not change scan state; zero is rejected because NimBLE treats it as an endless scan.
- Existing scans are stopped before replacement. Scan waits time out; failed starts release scan state. Busy responses have a retry ceiling. Notification waits yield to background tasks.
- Device requests share a 20-second retry budget, including nested connect/send retries. A BLE operation already in progress can finish after that deadline (connection timeout is 10 seconds). This is not a hard real-time deadline. Follow-up settings requests are queued separately, allowing the main loop to service MQTT and HTTP between requests.
- BOOT button handling no longer blocks for 500 ms or continuously repeats while held. Its empty bot map leaves this optional feature inactive.
- Uptime accumulates across millis wrap and is not reset on MQTT reconnect.

## Builds and automated checks

Use Python 3 and PlatformIO Core 6.1.19. The project pins Espressif32 3.5.0 (Arduino ESP32 1.0.6), NimBLE 1.4.0, EspMQTTClient 1.13.3, PubSubClient 2.8, ArduinoJson 6.19.4 and ArduinoQueue 1.2.5. Do not independently upgrade these for Arduino IDE builds.

```
python -m pip install platformio==6.1.19
python -m platformio run -d "PlatformIO Files/SwitchBot-BLE2MQTT-ESP32" -e esp32dev -e m5stack-atom
python -m unittest discover -s tests -v
```

Tests require g++ on PATH. They compile actual firmware functions and OTA callbacks with hardware doubles, checking unauthorized writes, successful/failed/aborted uploads, missing files, stale authorization, rescan bounds, scan timeout including clock wrap, scoped processing cleanup, busy-retry limits, failed BLE reconnect budgets, Curtain discovery and identical firmware variants. Physical BLE, flash and broker behavior are not simulated. Full queue, real broker reconnect and mesh integration remain hardware acceptance tests.

The Gitea workflow in `.gitea/workflows/` builds esp32dev and m5stack-atom; it does not run as GitHub Actions merely by pushing this repository to GitHub. Other historical board entries are not a verified support matrix. ESP32-S2 entries were removed because S2 has no Bluetooth. ESP32-C3/S3 need a separately validated core/toolchain; this change makes no support claim for them. Arduino IDE is not covered by the PlatformIO matrix.

## Hardware acceptance tests still required

Use test devices and a test broker. Record serial logs, MQTT status/availability, uptime and free heap.

1. After the initial USB partition migration, upload without credentials and with invalid credentials. Expect HTTP 401, no flash writes and unchanged firmware. Also check rejection of missing/invalid session tokens. A valid authenticated upload with a valid token should reboot once. Missing, interrupted or invalid uploads must not reboot.
2. Reconnect and check retained discovery: existing entity IDs remain; Curtain light has no lux class and its raw level remains unchanged.
3. Publish malformed rescan JSON, missing `sec`, and values 0, -1 and 301, then a valid one-second rescan and bot command. Expect rejection of invalid inputs and continued processing.
4. Stop the broker for at least five minutes. WiFi must remain associated without recurring DHCP requests. Restore it and check subscriptions, state publishing and uninterrupted uptime.
5. Disable/restore the access point; verify reconnect with static IP enabled and disabled.
6. Remove a BLE device during a command and restore it. Check bounded retries, continued processing and subsequent successful control, including continuous scanning.
7. Reconnect a mesh secondary while its primary receives commands. Check resumed telemetry and queue processing.
8. Run at least 72 hours with intermittent weak/missing BLE devices and broker/mesh reconnects. Check declining heap and lost telemetry even if MQTT availability still says online.

These changes fix identified control-flow defects. They do not prove that every upstream panic or multi-day stall has the same cause or is resolved.
