# Research: Day Mode Schedule

## Q1: How is the silent start hour stored in EEPROM?

**Decision**: Store as a single int via `prefs.getInt("silent_hour", -1)` where -1 means "disabled".

**Rationale**: Matches the exact pattern used for `clock_mode` and `clock_timezone`. Fits in one EEPROM key, no migration needed for existing devices (clean flash returns -1 = disabled = existing behaviour preserved).

**Alternatives considered**: Boolean `silent_enabled` + int `silent_hour` (two keys) — unnecessary complexity for a single value that encodes both state and time.

---

## Q2: Where does the silent-hour logic live in firmware?

**Decision**: Inside `set_time()` in `main.cpp`, evaluated alongside the existing sleep check. When `get_silent_hour()` returns a valid hour (0–23) and `hour() >= silent_hour` and the current hour is not in the sleep mask, the firmware calls `set_lazy()` directly instead of `dispatch_animation()`.

**Rationale**: `set_time()` is the single point where mode selection happens each minute. Adding the silent check there keeps the logic co-located and avoids touching any other function.

**Alternatives considered**: A wrapper around `get_clock_mode()` that returns LAZY when in silent window — cleaner interface but means `get_clock_mode()` would no longer return what's actually stored in EEPROM, breaking the web UI config readback.

---

## Q3: How does "immediate apply on save" work?

**Decision**: The `/mode` POST handler (and a new `/silent` POST handler) calls `last_minute = -1` to force `set_time()` to re-evaluate on the next loop iteration. This is the same trick already used implicitly when `clock_mode` changes.

**Rationale**: The loop runs `set_time()` every iteration; setting `last_minute = -1` causes the condition `hour() != last_hour || minute() != last_minute` to be true, triggering immediate re-evaluation. No new mechanism needed.

**Alternatives considered**: Calling `set_time()` directly in the POST handler — risky because it runs inside an HTTP callback, not the main loop. The `last_minute = -1` approach defers execution to the safe main-loop context within milliseconds.

---

## Q4: How does the web UI send and receive the silent hour?

**Decision**:
- GET `/config` response gains a new `"silent_hour": -1` (or 0–23) field
- New POST `/silent` endpoint accepts `silent_hour` form field (integer, -1 = disabled)
- Web UI gains a `<select>` dropdown labelled "Silent from" with options "Disabled" + hours 0–23, rendered near the existing Mode section

**Rationale**: Follows the exact pattern of `/sleep` and `/mode` endpoints. Keeps the existing payload shape and adds minimally.

---

## Q5: Does `last_hour` need tracking alongside `last_minute`?

**Decision**: Yes — `set_time()` already tracks `last_hour`. The silent-hour comparison uses `hour()` directly (same as the existing sleep check), so no new tracking variable is needed.
