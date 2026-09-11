# Independent review — 2026-09-11

Scope: local firmware hardening changes after base commit `45965cb`, both firmware variants, dependency configuration, CI and regression tests. Review performed by a separate read-only agent, followed by implementation corrections and a second review.

## Findings and disposition

| Finding | Resolution |
| --- | --- |
| Temperature discovery degree sign corrupted during editing | Restored UTF-8 `°C`; unrelated contact/motion display names restored. |
| Nested connection retries could block the main loop for minutes | Shared request retry budget added to device lookup, outer send/connect loops, inner send loop and both client-connect paths. |
| Follow-up settings requests renewed the budget inside a blocking loop | Replaced with a normal queued `REQUESTSETTINGS` command so the main loop runs between requests. |
| Host `unsigned long` has different width on Windows and Linux | Explicit 32-bit elapsed-time differences; rollover regressions pass on both platforms. |

The final independent review reported no remaining concrete regression findings. It checked MQTT-only constructor semantics against EspMQTTClient 1.13.3, authorization availability before upload callbacks against the ESP32 core, and scan-stop callbacks against NimBLE 1.4.0. The reviewer independently reran the regression tests successfully.

## Verification and limits

- Native regression tests passed on Windows and Linux (`node:20-bookworm` container with read-only project mount and networking disabled).
- Final PlatformIO builds passed for `esp32dev` and `m5stack-atom` with the pinned toolchain. Existing unused-variable warnings remain.
- Firmware sources are byte-identical; `git diff --check` passed.
- The retry budget is checked between operations and is not a hard real-time deadline for an ongoing BLE call.
- Full queue/radio behavior, real broker and mesh reconnects, physical OTA and a 72-hour soak test remain hardware acceptance checks; see [validation](validation.md).

No firmware was flashed, and no remote repository was changed by this work.
