---
description: "Task list for new animations port from fork"
---

# Tasks: New Animations from Fork

**Input**: Design documents from `specs/003-new-animations/`
**Prerequisites**: plan.md ✅, spec.md ✅, research.md ✅, data-model.md ✅, contracts/ ✅, quickstart.md ✅

**Validation**: Manual hardware validation is required (no automated test framework). Each user story phase includes a hardware validation task.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel with other [P] tasks in the same phase (different files)
- **[Story]**: User story this task belongs to
- File paths are relative to repo root

---

## Phase 1: Setup

**Purpose**: Verify build baseline before any changes land.

- [x] T001 Confirm current master builds with zero warnings: `cd master && pio run` — record baseline warning count
- [x] T002 Note current EEPROM `clock_mode` stored on device (via web UI `/config`) so regression can be detected after reflash

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Low-level changes that every animation depends on. Must be complete before any user story phase begins.

**⚠️ CRITICAL**: All user story phases depend on this phase.

- [x] T003 Add `COUNTERCLOCKWISE5` after `ADJUST_HAND` in `master/include/clock_state.h` enum (value 13; keeps ADJUST_HAND at 12 — no slave reflash needed)
- [x] T004 Extend `clock_modes` enum in `master/include/clock_config.h` with PROPELLER(3)…CYCLE(11); remove `OFF` from enum and add `#define OFF 255` below it
- [x] T005 [P] Add new digit pose constants to `master/include/digit.h`: `digit_wave_a/b` + mirrors → `d_WAVE`; `digit_JOINT` → `d_joint`; `digit_cent_a/b` + mirrors → `d_CENT`; `digit_bubble` → `d_bubble`; `digit_diag` → `d_diagonal` (values verbatim from fork)
- [x] T006 Implement `set_half_digit_full(int index, t_half_digit half)` in `master/src/clock_manger.cpp`: send pre-built half-digit directly (no `get_full_half_digit()` conversion), update `_last_state[index]`, increment `_counter`
- [x] T007 Implement `set_single_clock_full(int hd, int p, t_half_digitl lite, int mode_h, int mode_m)` in `master/src/clock_manger.cpp`: read `_last_state[hd]`, set angle/speed/accel/mode for position `p` only, increment `change_counter[p]`, send half-digit
- [x] T008 Verify `master` builds cleanly after T003–T007: `cd master && pio run` — zero new warnings

**Checkpoint**: Foundation ready — user story phases can now begin.

---

## Phase 3: User Story 1 — Select a new animation via the web UI (Priority: P1) 🎯 MVP

**Goal**: All 9 new animation modes are available in firmware and selectable from the web UI. On the next minute change, the selected animation plays.

**Independent Test**: Open web UI, select PROPELLER, wait for minute change, confirm propeller-spin plays. Then repeat for SCATTER and confirm minute hand behaviour is consistent with the documented limitation.

### Implementation

- [x] T009 [US1] Add `dispatch_animation(int mode)` to `master/src/main.cpp` with a switch-case covering LAZY(0)…CYCLE(11); add all required forward declarations
- [x] T010 [US1] Replace the existing switch-case in `set_time()` in `master/src/main.cpp` with a single call to `dispatch_animation(get_clock_mode())`; update the `mode_name` timing-instrumentation switch to cover all 12 modes
- [x] T011 [P] [US1] Add `set_propeller()` to `master/src/main.cpp`: simultaneous CW/CCW per-hand spin via `set_half_digit_full()` for all 8 half-digits
- [x] T012 [P] [US1] Add `set_arrow()` to `master/src/main.cpp`: phase 1 collapse to `d_joint` via MIN_DISTANCE + hold; phase 2 diagonal wavefront of propeller-spins via `set_single_clock_full()` with `proj = 2*col - 7*row` stagger
- [x] T013 [P] [US1] Add `set_ripple()` to `master/src/main.cpp`: phase 1 collapse to `d_WAVE` + hold; phase 2 Manhattan-distance ripple from centre via `set_single_clock_full()`, left/right halves mirrored direction
- [x] T014 [P] [US1] Add `set_bubble()` to `master/src/main.cpp`: phase 1 collapse to `d_bubble` + hold; phase 2 checkerboard CW/CCW propeller-spins via `set_half_digit_full()`
- [x] T015 [P] [US1] Add `set_gear()` to `master/src/main.cpp`: phase 1 collapse to `d_CENT` + hold; phase 2 Chebyshev-radius ring expansion (all CW) via `set_single_clock_full()`
- [x] T016 [P] [US1] Add `set_scatter()` to `master/src/main.cpp`: hour hand COUNTERCLOCKWISE3 at speed 400, minute hand COUNTERCLOCKWISE5 at speed 800, left-to-right column stagger 400 ms via `set_half_digit_full()`
- [x] T017 [P] [US1] Add `set_diagonal()` to `master/src/main.cpp`: phase 1 collapse to `d_diagonal` + hold; phase 2 left-to-right wave with CLOCKWISE2 via `set_half_digit()`
- [x] T018 [P] [US1] Add `set_cascade()` to `master/src/main.cpp`: phase 1 collapse to `d_stop` + hold; phase 2 column-by-column CCW reveal, hour hand COUNTERCLOCKWISE2, minute hand COUNTERCLOCKWISE3 at double speed, via `set_half_digit_full()`
- [x] T019 [US1] Update web UI mode dropdown in `master/include/web_page.h`: extend `genModes()` JS array to `["LAZY","FUN","WAVES","PROPELLER","ARROW","RIPPLE","BUBBLE","GEAR","SCATTER","DIAGONAL","CASCADE","CYCLE","OFF"]`; update `selectMode()` OFF branch to check `255===e` instead of `3===e`
- [ ] T020 [US1] Build and flash master firmware: `cd master && pio run -e esp32 -t upload`
- [ ] T021 [US1] Hardware validation — for each of the 9 new modes: set via web UI, wait for minute change, confirm expected animation plays (see `specs/003-new-animations/quickstart.md` for per-mode criteria); record pass/fail

**Checkpoint**: All 9 new animations selectable and playing on hardware. Web UI shows 13 mode buttons.

---

## Phase 4: User Story 2 — CYCLE mode auto-rotates (Priority: P2)

**Goal**: CYCLE mode rotates through all non-LAZY animations automatically, driven by minute-of-day, agreeing with the web preview calculation.

**Independent Test**: Set CYCLE, compute expected `[FUN,WAVES,ARROW,SCATTER,RIPPLE,BUBBLE,PROPELLER,DIAGONAL,GEAR,CASCADE][(H*60+M) % 10]` for the current minute, confirm firmware plays that animation.

### Implementation

- [x] T022 [US2] Add `set_cycle()` to `master/src/main.cpp`: define `cycle_order[]` = `{FUN, WAVES, ARROW, SCATTER, RIPPLE, BUBBLE, PROPELLER, DIAGONAL, GEAR, CASCADE}`, compute `minutes_today = last_hour * 60 + last_minute`, call `dispatch_animation(cycle_order[minutes_today % cycle_count])`
- [x] T023 [US2] Register CYCLE case in `dispatch_animation()` switch in `master/src/main.cpp` (calls `set_cycle()`)
- [ ] T024 [US2] Build and flash: `cd master && pio run -e esp32 -t upload`
- [ ] T025 [US2] Hardware validation — set CYCLE mode; note current H:M; compute expected animation; confirm firmware plays the correct one; observe 3 consecutive minute changes to verify rotation

**Checkpoint**: CYCLE mode working and consistent with minute-of-day formula.

---

## Phase 5: User Story 3 — Existing animations unaffected (Priority: P3)

**Goal**: LAZY, FUN, and WAVES continue to behave identically to pre-feature behaviour.

**Independent Test**: Switch to LAZY, FUN, WAVES in turn; observe two consecutive minute changes each; confirm staggered send, speed/acceleration, and motion are unchanged.

### Implementation

- [ ] T026 [US3] Code review: confirm `set_lazy()`, `set_fun()`, `set_waves()` in `master/src/main.cpp` are unchanged from pre-feature version (use `git diff master origin/master -- master/src/main.cpp`)
- [ ] T027 [US3] Hardware validation — set LAZY; observe two minute changes; confirm MIN_DISTANCE staggered transition, no change in timing vs. pre-feature behaviour
- [ ] T028 [US3] Hardware validation — set WAVES; observe one minute change; confirm two-phase sequence (collapse to d_IIII then wave) unchanged
- [ ] T029 [US3] Hardware validation — set OFF; confirm all hands move to pointing-down and clock stops updating

**Checkpoint**: All pre-existing modes verified unchanged.

---

## Phase 6: Polish & Cross-Cutting Concerns

- [x] T030 [P] Add inline comment above `set_scatter()` in `master/src/main.cpp` noting the known slave limitation: COUNTERCLOCKWISE5 (value 13) not recognised by unflashed slave firmware — minute hand rotation will be incorrect but final time angle is correct
- [x] T031 [P] Update `master/include/clock_manager.h` to confirm `set_half_digit_full()` and `set_single_clock_full()` declarations match their new implementations (no stale stubs)
- [x] T032 Final build check: `cd master && pio run` — confirm zero new compiler warnings vs. T001 baseline
- [ ] T033 Run full quickstart.md validation checklist and record results in a PR comment
- [ ] T034 Constitution compliance review: confirm KISS (no new abstraction beyond `dispatch_animation()`), code quality (no warnings), test evidence (hardware validation done per story), UX consistency (mode names match enum names), performance (no new blocking delays > 9 s)

---

## Dependencies & Execution Order

### Phase Dependencies

- **Phase 1 (Setup)**: No dependencies — start immediately
- **Phase 2 (Foundational)**: Depends on Phase 1 — **blocks all user story phases**
- **Phase 3 (US1)**: Depends on Phase 2 — T009 must precede T010; T011–T018 can run in parallel after T009; T019 independent of T011–T018; T020 depends on T019 + all T011–T018; T021 depends on T020
- **Phase 4 (US2)**: Depends on Phase 3 complete (needs `dispatch_animation()` from T009 and flashed firmware from T020)
- **Phase 5 (US3)**: Can run alongside Phase 4 (hardware validation only, no code changes)
- **Phase 6 (Polish)**: Depends on Phases 3–5 complete

### Parallel Opportunities Within Phase 3

```
T009 (dispatch_animation skeleton) → T010 (wire into set_time)
                                   → T011 set_propeller    ┐
                                   → T012 set_arrow        │
                                   → T013 set_ripple       │ all parallel
                                   → T014 set_bubble       │
                                   → T015 set_gear         │
                                   → T016 set_scatter      │
                                   → T017 set_diagonal     │
                                   → T018 set_cascade      ┘
T019 (web UI) — independent of T011–T018
T020 (flash) — depends on all of T010–T019
T021 (hardware validation) — depends on T020
```

---

## Implementation Strategy

### MVP (Phase 1 + 2 + 3 only)

1. Complete Phase 1: baseline build check
2. Complete Phase 2: foundational enum + digit + function changes
3. Complete Phase 3: all 9 animation functions + web UI + flash + validate
4. **Stop and validate** — all 9 new modes working on hardware
5. All remaining phases are additive with no risk of regression

### Incremental

1. Phase 1 + 2 → clean build, foundation in place
2. Phase 3 → new animations live on device (MVP deliverable)
3. Phase 4 → CYCLE mode working
4. Phase 5 → regression confirmed
5. Phase 6 → polish and sign-off

---

## Notes

- Slave firmware (`slave/`) is **not modified** in any task — no slave reflash required
- SCATTER known limitation (minute hand incorrect on all slave boards) is accepted and documented in T030
- All speed/acceleration values are bare numeric constants (no `get_speed_multiplier()` ported)
- `set_lazy()`, `set_fun()`, `set_waves()` retain their existing `set_clock_time_staggered()` / `set_half_digit_staggered()` calls — do not replace with fork's non-staggered variants
- Commit after each phase checkpoint at minimum
