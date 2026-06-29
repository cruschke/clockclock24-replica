# Implementation Plan: Day Mode Schedule

**Branch**: `004-day-mode-schedule` | **Date**: 2026-06-29 | **Spec**: [spec.md](spec.md)  
**Input**: Feature specification from `specs/004-day-mode-schedule/spec.md`

## Summary

Add a single configurable "silent from" hour that causes the clock to play Lazy instead of the selected animation from that hour until sleep. The implementation touches five files: `clock_config.h/.cpp` (new getter/setter), `main.cpp` (silent check in `set_time()`), `web_server.cpp` (new `/silent` endpoint + updated `/config` response), and `web_page.h` (new dropdown in JS). No new animations, no changes to sleep schedule, no new EEPROM schema beyond one new key.

## Technical Context

**Language/Version**: C++11 / Arduino framework (ESP8266, PlatformIO)  
**Primary Dependencies**: Arduino, Preferences (EEPROM) — no new dependencies  
**Storage**: Preferences NVS — one new int key `silent_hour` (default -1)  
**Testing**: Manual hardware validation + build check  
**Target Platform**: ESP8266 master  
**Project Type**: Embedded firmware + single-page web UI  
**Performance Goals**: Mode re-evaluation within one main-loop iteration of saving (~100ms)  
**Constraints**: No new libraries; minified JS in `web_page.h`; payload buffer in `web_server.cpp` is 1024 bytes — adding `"silent_hour":-1,` adds 17 chars, within budget

## Constitution Check

- **KISS-First Engineering**: ✅ One new int config value, one new endpoint, one new dropdown, one new conditional in `set_time()`. No new abstraction layers, no new files.
- **Code Quality Baseline**: ✅ PlatformIO build with zero new warnings is the gate.
- **Test Evidence for Every Change**: ✅ Manual hardware validation defined in `quickstart.md` covering persistence, immediate apply, sleep precedence, and regression.
- **Consistent User Experience**: ✅ "Silent from" label clearly distinguishes from sleep (which turns off). Existing mode selector, sleep schedule, and all animations unchanged.
- **Performance Within Hardware Limits**: ✅ One integer comparison added to `set_time()` — negligible. Immediate apply uses existing `last_minute = -1` pattern, no blocking call.

**Constitution Check result: PASS**

## Project Structure

### Documentation (this feature)

```text
specs/004-day-mode-schedule/
├── plan.md              # This file
├── spec.md              # Feature specification
├── research.md          # Resolved unknowns
├── data-model.md        # silent_hour entity
├── quickstart.md        # Validation guide
├── contracts/
│   └── web-api.md       # /silent endpoint + /config update
└── tasks.md             # Phase 2 output (/speckit-tasks)
```

### Source Code (files changed)

```text
master/
├── include/
│   ├── clock_config.h    # Declare get_silent_hour(), set_silent_hour()
│   └── web_page.h        # Add "Silent from" dropdown + JS handler
└── src/
    ├── clock_config.cpp  # Implement get/set_silent_hour(); load/save in begin/clear_config()
    ├── web_server.cpp    # Add /silent endpoint; add silent_hour to /config JSON
    └── main.cpp          # Replace dispatch_animation call with silent-aware branch
```

## Implementation Steps

### Step 1 — `master/src/clock_config.cpp` + `master/include/clock_config.h`

- Add `int _silent_hour` to static state; initialise to -1
- In `begin_config()`: load from NVS with default -1
- In `clear_config()`: reset to -1
- `get_silent_hour()`: returns `_silent_hour`
- `set_silent_hour(int value)`: clamp to valid range (-1 or 0–23), store, persist
- Declare both functions in `clock_config.h`

### Step 2 — `master/src/main.cpp`: Silent check in `set_time()`

Replace the bare `dispatch_animation(get_clock_mode())` call with:

```
int sh = get_silent_hour();
if (sh >= 0 && hour() >= sh) {
    set_lazy();
} else {
    dispatch_animation(get_clock_mode());
}
```

The `last_minute = -1` reset for immediate apply is triggered by the web server handler (Step 3), not here.

### Step 3 — `master/src/web_server.cpp`

1. Add `extern int last_minute;` (already a global in main.cpp — just need the extern declaration)
2. Register `_server.on("/silent", HTTP_POST, handle_post_silent);`
3. `handle_post_silent()`: parse `silent_hour` arg → `set_silent_hour(value)` → `last_minute = -1` → respond 200
4. Update `snprintf` in `handle_get_config()`: add `"silent_hour":%d,` field

### Step 4 — `master/include/web_page.h`

In the minified JS:
1. Add `<div class=title>Silent Mode</div>` + `<select id="silent-hour">` with options -1…23
2. `savesilentHour()` function: POST to `/silent` with selected value  
3. In `updateConfig()`: read `e.silent_hour`, pre-select dropdown
4. Wire `onchange` on the select to call `savesilentHour()`

## Complexity Tracking

No violations. No new abstractions or dependencies introduced.
