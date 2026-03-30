# Quickstart: Validate General DST Support

## Prerequisites
- Hardware flashed with feature branch firmware on master board.
- Web UI reachable from browser.
- Ability to switch between online (NTP available) and offline network states.
- Familiarity with browser developer tools (network inspection, console)

## Timezone Configuration

Before running validation scenarios, the clock must be configured with a timezone identifier:

### Initial Setup (After Clean Flash)
1. **Connect to web UI**: Open clock's IP address in browser
2. **Check initial state**: Inspect `/config` endpoint or UI
   - `timezone_configured` should be `false` initially
   - `timezone_id` should be empty
   - `time_authority` should be `browser_manual_fallback` (no NTP yet)
3. **Select timezone profile**: Use web UI timezone selector (see web/index.html UI)
   - Choose a DST-observing region: `Europe/Berlin`, `America/New_York`, `Europe/London`, etc.
   - Confirm in browser console: `POST /time` with `timezone_id=Europe/Berlin`
   - Verify response shows `timezone_configured=true`
4. **Verify configuration persisted**: Refresh page, confirm timezone_id is retained

### Configuration Validation Steps
1. Use `/config` endpoint to verify:
   ```json
   {
     "timezone_id": "Europe/Berlin",
     "timezone_configured": true,
     "time_authority": "network_ntp" (or "browser_manual_fallback")
   }
   ```
2. Check serial monitor for debug output confirming timezone profile loaded
3. Verify web UI displays selected timezone in settings panel

---

## Validation Scenarios

## 1. Baseline Configuration
1. Open web UI and fetch current config.
2. Confirm `timezone_id` is set to a DST-observing profile (example: `Europe/Berlin`).
3. Confirm `timezone_configured=true`.

Expected:
- Clock shows correct local civil time for configured timezone.
- No manual seasonal correction needed.

## 2. Spring Forward Boundary Check (DST Observing)
1. Simulate time shortly before spring DST transition for chosen profile.
   - Berlin 2026 spring forward: 2026-03-29 02:00:00 CET → 03:00:00 CEST
2. Advance system time across boundary (via `/time` endpoint or serial command).

Expected:
- Missing-hour jump (02:00-02:59 skipped) follows local civil rules.
- Display updates within 1 minute of boundary crossing.
- Debug output confirms offset change: +01:00 → +02:00

## 3. Fall Back Boundary Check (DST Observing)
1. Simulate time shortly before autumn DST transition.
   - Berlin 2026 fall back: 2026-10-25 03:00:00 CEST → 02:00:00 CET
2. Advance system time across boundary.

Expected:
- Repeated-hour behavior (02:00-02:59 occurs twice) follows local civil rules.
- No persistent one-hour offset error after transition.
- Debug output confirms offset change: +02:00 → +01:00

## 4. Authority Precedence Check
1. Ensure network time is available (NTP synced, `time_authority=network_ntp`).
2. Send `/time` POST with intentionally incorrect timestamp (e.g., set time 10 hours in past).

Expected:
- Network authority remains active (`time_authority` stays `network_ntp`).
- Display does NOT drift to browser/manual timestamp.
- Firmware rejects overriding clock with stale browser value.

## 5. Offline Fallback and Recovery
1. Disable network time availability (disconnect NTP or simulate network outage).
2. Send browser/manual `/time` POST with correct current time.
3. Verify time updates using fallback source.
4. Re-enable network and trigger NTP sync.

Expected:
- Browser/manual fallback works only while offline (`time_authority=browser_manual_fallback`).
- On reconnect, authoritative NTP time is restored immediately.
- Time correction occurs within 1-minute accuracy target (latency ≤ 60s).

## 6. Clean-Flash Initialization Check
1. Erase EEPROM (using `pio device erase` or EEPROM_RESET.md procedure).
2. Flash firmware to device.
3. Boot and inspect `/config` endpoint immediately.

Expected:
- `timezone_configured=false` and `timezone_id` is empty (unconfigured state).
- `time_authority=browser_manual_fallback` (no NTP sync yet).
- Web UI displays timezone selection prompt.
- After selecting timezone_id, all fields update correctly.

## 7. Non-DST Timezone Check
1. Configure a non-DST timezone profile (e.g., `UTC`, `Africa/Lagos`, `Asia/Dubai`).
2. Simulate calendar periods where DST transitions occur in other regions (e.g., March 29 and October 25 for Europe).
3. Verify no offset jumps occur.

Expected:
- No DST offset jump is applied (offset remains constant year-round).
- Local time remains stable by configured non-DST rules.
- Debug output confirms `has_dst=false` for selected profile.
