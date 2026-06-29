---
description: "Task list for day mode schedule feature"
---

# Tasks: Day Mode Schedule

**Input**: Design documents from `specs/004-day-mode-schedule/`
**Prerequisites**: plan.md ✅, spec.md ✅, research.md ✅, data-model.md ✅, contracts/ ✅, quickstart.md ✅

**Validation**: Manual hardware validation per quickstart.md. No automated test framework.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel with other [P] tasks in the same phase
- **[Story]**: User story this task belongs to
- File paths relative to repo root

---

## Phase 1: Setup

**Purpose**: Baseline build check before any changes.

- [x] T001 Verify clean baseline build: `cd master && make build` — confirm zero warnings before any changes

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Config layer and firmware logic — required before web server or UI can be tested.

**⚠️ CRITICAL**: Web server and UI phases depend on this phase.

- [x] T002 Add `int _silent_hour` state variable to `master/src/clock_config.cpp`; initialise to -1; load from NVS in `begin_config()` with `prefs.getInt("silent_hour", -1)`; reset to -1 in `clear_config()`; persist in setter with `prefs.putInt("silent_hour", value)`
- [x] T003 Implement `get_silent_hour()` and `set_silent_hour(int value)` in `master/src/clock_config.cpp`; clamp value to -1 or 0–23 in setter
- [x] T004 Declare `get_silent_hour()` and `set_silent_hour(int value)` in `master/include/clock_config.h`
- [x] T005 Replace bare `dispatch_animation(get_clock_mode())` in `set_time()` in `master/src/main.cpp` with silent-aware branch: if `get_silent_hour() >= 0 && hour() >= get_silent_hour()` then `set_lazy()`, else `dispatch_animation(get_clock_mode())`
- [x] T006 Verify build after T002–T005: `cd master && make build` — zero new warnings

**Checkpoint**: Config layer and firmware logic complete. Silent mode will activate on hardware after flash.

---

## Phase 3: User Story 1 — Configure silent mode start time (Priority: P1) 🎯 MVP

**Goal**: User can set a "silent from" hour via the web UI; clock switches to Lazy at that hour and the setting persists across power cycles.

**Independent Test**: Set `silent_hour=18` via curl; confirm `/config` returns it; power-cycle; confirm it persists; confirm clock plays Lazy at hour 18.

### Implementation

- [x] T007 [US1] Add `extern int last_minute;` declaration and register `/silent` endpoint in `master/src/web_server.cpp`: `_server.on("/silent", HTTP_POST, handle_post_silent)`
- [x] T008 [US1] Implement `handle_post_silent()` in `master/src/web_server.cpp`: parse `silent_hour` arg → call `set_silent_hour(value)` → set `last_minute = -1` for immediate re-evaluation → respond HTTP 200
- [x] T009 [US1] Add `"silent_hour":%d,` field to the `snprintf` payload in `handle_get_config()` in `master/src/web_server.cpp`; pass `get_silent_hour()` as the argument; confirm payload stays within 1024-byte buffer
- [x] T010 [P] [US1] Add "Silent Mode" section to web UI in `master/include/web_page.h`: add `<div class=title>Silent Mode</div>` + `<select id="silent-hour">` with options value=-1 label="Disabled" and values 0–23 labelled "00:00"–"23:00"; wire `onchange` to `saveSilentHour()` function that POSTs to `/silent`
- [x] T011 [P] [US1] Add `saveSilentHour()` JS function in `master/include/web_page.h` that reads `document.getElementById("silent-hour").value` and POSTs to `/silent`; add `updateConfig()` handling to pre-select the dropdown from `e.silent_hour`
- [x] T012 [US1] Build and flash: `cd master && curl -s -X POST http://192.168.178.123/mode -d "mode=255" && sleep 15 && make upload`
- [ ] T013 [US1] Hardware validation — set silent hour via curl and web UI; confirm `/config` returns correct value; power-cycle and confirm persistence; confirm clock plays Lazy when `hour() >= silent_hour`; confirm immediate apply when setting saved

**Checkpoint**: Silent mode configurable, persistent, and activating correctly on hardware.

---

## Phase 4: User Story 2 — Full daily cycle end-to-end (Priority: P2)

**Goal**: Active animation → Lazy → Sleep transitions fire automatically in the correct order across a full evening.

**Independent Test**: Configure silent start 18:00 + sleep 22:00; observe all three phases over an evening.

### Implementation

- [ ] T014 [US2] Hardware validation — configure silent start = current hour + 1 and sleep = current hour + 2; observe active animation, then Lazy, then clock off at correct hours
- [ ] T015 [US2] Edge case validation — configure silent start = same hour as sleep start; confirm silent mode is skipped and clock goes directly from active to sleep
- [ ] T016 [US2] Edge case validation — set `silent_hour=-1` (disabled); confirm clock plays selected animation at all non-sleep hours (regression check)

**Checkpoint**: Full daily cycle verified. All edge cases pass.

---

## Phase 5: Polish & Cross-Cutting Concerns

- [ ] T017 [P] Final build check: `cd master && make build` — confirm zero new warnings vs T001 baseline
- [ ] T018 [P] Run full quickstart.md validation (all 8 scenarios) and record results
- [ ] T019 Constitution compliance review: KISS (one new int key, one endpoint, one conditional), quality (no warnings), test evidence (hardware validation done), UX consistency (label distinguishes from sleep), performance (integer comparison only)

---

## Dependencies & Execution Order

### Phase Dependencies

- **Phase 1 (Setup)**: No dependencies
- **Phase 2 (Foundational)**: Depends on Phase 1 — **blocks Phases 3 and 4**
- **Phase 3 (US1)**: Depends on Phase 2 — T007–T009 sequential; T010–T011 parallel; T012 depends on all of T007–T011; T013 depends on T012
- **Phase 4 (US2)**: Depends on Phase 3 complete (flashed firmware)
- **Phase 5 (Polish)**: Depends on Phases 3–4 complete

### Parallel Opportunities Within Phase 3

```
T007 (register endpoint) → T008 (implement handler) → T009 (update /config)
T010 (dropdown HTML)    ┐ both independent of T007–T009
T011 (JS functions)     ┘
T012 (flash) — depends on all of T007–T011
T013 (hardware validation) — depends on T012
```

---

## Implementation Strategy

### MVP (Phases 1–3 only)

1. Phase 1: baseline build check
2. Phase 2: config + firmware logic
3. Phase 3: web server + UI + flash + validate
4. **Stop and validate** — silent mode working end-to-end on hardware

### Full Delivery

1. Phases 1–3 → MVP complete
2. Phase 4 → full daily cycle validated
3. Phase 5 → polish and sign-off

---

## Notes

- `last_minute = -1` reset in `handle_post_silent()` triggers immediate re-evaluation in the main loop — no blocking call needed
- Payload buffer in `web_server.cpp` is 1024 bytes; adding `"silent_hour":-1,` (17 chars) keeps it within budget
- All existing modes, sleep schedule, and OFF behaviour unchanged when `silent_hour = -1`
