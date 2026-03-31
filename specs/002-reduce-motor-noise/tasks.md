# Tasks: Reduce Motor Noise During Synchronized Animation

**Feature Branch**: `002-reduce-motor-noise`  
**Created**: March 31, 2026  
**Spec**: [spec.md](spec.md) | **Plan**: [plan.md](plan.md)

---

## Phase 1: Setup & Environment

Establish build environment and verify baseline noise characteristics.

### Tasks

- [ ] T001 [P] Verify feature branch `002-reduce-motor-noise` is checked out and `make build` succeeds with baseline code
- [ ] T002 [P] Record baseline sound level during full-clock minute transition (all 24 motors moving) for comparison post-implementation
- [ ] T003 [P] Verify serial monitor is capturing `Set time:` logs and confirm current log frequency (expected: multiple per second)

---

## Phase 2: Foundational - Source Constants & Helper Functions

Implement the tuneable parameters and supporting infrastructure for noise reduction.

### Tasks

- [ ] T004 [P] [US1] Add source-level constants to `master/src/clock_manger.cpp` for staggering and derating:
  - `const int STAGGER_INTERVAL_MS = 15;` (default 10-20ms range)
  - `const float SPEED_FACTOR = 0.90f;` (default 90%)
  - `const float ACCELERATION_FACTOR = 0.80f;` (default 80%)
  - Comment that these are tunable for testing (0-50ms, 0.70-1.00 range)

- [ ] T005 [P] [US1] Modify `get_full_half_digit()` in `master/src/clock_manger.cpp` to apply speed/acceleration derating:
  - Change: `tmp.clocks[i].speed_h = _speed * SPEED_FACTOR;`
  - Change: `tmp.clocks[i].speed_m = _speed * SPEED_FACTOR;`
  - Change: `tmp.clocks[i].accel_h = _acceleration * ACCELERATION_FACTOR;`
  - Change: `tmp.clocks[i].accel_m = _acceleration * ACCELERATION_FACTOR;`
  - (Apply same to all 3 clocks in the loop)

- [ ] T006 [P] [US1] Add staggering helper in `master/src/main.cpp`:
  - Implement `staggered_send_half_digit(board_index, half_digit)` function
  - Compute stagger offset: `delay_ms = board_index * STAGGER_INTERVAL_MS;`
  - Use `delay()` to stagger board startup (board 0 sends immediately, board 1 waits STAGGER_INTERVAL_MS, etc.)
  - Add to `master/include/clock_manager.h` declaration

- [ ] T007 [P] Compile and verify no warnings in modified functions; run `make build`

---

## Phase 3: User Story 1 - Reduce Peak Noise During Full-Clock Animation

Implement staggering and derating across animation modes.

### Tasks

- [ ] T008 [US1] Integrate staggering into `set_lazy()` animation in `master/src/main.cpp`:
  - Replace `set_clock(clock_state)` call with staggered board-by-board approach
  - Call `staggered_send_half_digit(i, clock_state.digit[i/2].halfs[i%2])` for boards 0-7 in sequence
  - Instead of sending all 8 boards simultaneously, spread them by STAGGER_INTERVAL_MS

- [ ] T009 [US1] Integrate staggering into `set_fun()` animation in `master/src/main.cpp`:
  - Replace `set_clock(clock_state)` with staggered board-by-board calls
  - Same staggering pattern as T008

- [ ] T010 [US1] Integrate staggering into `set_waves()` animation in `master/src/main.cpp`:
  - Within the domino wave loop, stagger `set_half_digit()` calls per board
  - For each of 8 boards, apply `STAGGER_INTERVAL_MS` delay before sending move command

- [ ] T011 [US1] Update `get_clock_state_from_time()` log to avoid serial spam:
  - Verify existing debounce logic (static `_last_logged_h`, `_last_logged_m`) is in place and working
  - Log only when time actually changes, not on every animation frame

- [ ] T012 [P] [US1] Compile and verify no errors; run `make build`

---

## Phase 4: User Story 2 - Preserve Animation Timing Accuracy

Validate timing tolerance and optimize derating parameters.

### Tasks

- [ ] T013 [US2] Measure actual animation completion time before/after implementation:
  - Instrument `set_time()` and animation functions with `millis()` timestamps
  - Log start and end times for each animation cycle
  - Verify all animations complete within 0 to +200ms of baseline (allow up to 200ms overhead)
  - Check serial output for completion times during test run

- [ ] T014 [US2] Visually verify animation quality with high-speed video:
  - Record minute transition animation at 120+ fps (phone camera or GoPro)
  - Compare video before (T002 baseline) and after staggering
  - Verify no visible jitter, slowdown, or hand mis-target
  - Document result: animation is imperceptibly smooth

- [ ] T015 [P] [US2] Iterative tuning of derating factors if animations feel slow or jittery:
  - If completion time > 200ms: Increase SPEED_FACTOR to 0.92-0.95
  - If still noisy after testing: Decrease ACCELERATION_FACTOR to 0.75
  - If I2C conflicts observed: Decrease STAGGER_INTERVAL_MS to 8-12ms
  - Recompile and test after each adjustment
  - Done when: timing is within ±200ms AND animation quality is imperceptible

---

## Phase 5: User Story 3 - Support All Animation Modes

Validate noise reduction works uniformly across Lazy, Fun, and Waves.

### Tasks

- [ ] T016 [P] [US3] Test Lazy animation mode:
  - Boot clock, wait for minute change
  - Verify hands move with staggering applied
  - Measure noise level (dB meter or app)
  - Verify completion time log shows ≤+200ms overhead

- [ ] T017 [P] [US3] Test Fun animation mode:
  - Set clock mode to Fun via web UI
  - Trigger animation (wait for minute change or press reset)
  - Verify all 24 hands move with staggering applied; full 360° turns should be smooth
  - Measure noise level
  - Verify completion time ≤+200ms

- [ ] T018 [P] [US3] Test Waves animation mode:
  - Set clock mode to Waves via web UI
  - Trigger animation (minute change or manual trigger)
  - Verify wave propagation across 8 boards respects staggering
  - Measure noise level during domino animation
  - Verify all hands reach targets within timing window

- [ ] T019 [US3] Verify mode switching does not cause crashes or missed animations:
  - Cycle between Lazy → Fun → Waves → Lazy rapidly (every 30 seconds)
  - Monitor serial output for errors
  - Verify all animations complete without jitter or misalignment

---

## Phase 6: Validation & Testing

Execute all VE (Validation Evidence) criteria from spec.

### Tasks

- [ ] T020 [P] VE-001 - Sound level measurement:
  - Compare baseline (T002) with post-implementation noise during +4 minute transitions
  - Target: ≥40% reduction in peak dB (measured with sound meter app or decibel meter)
  - Document: baseline dB, post-implementation dB, % reduction
  - PASS if: dB reduction ≥40%

- [ ] T021 [P] VE-002 - High-speed video capture:
  - Record 60-second video of each animation mode (Lazy, Fun, Waves) at ≥120fps
  - Visually compare before/after staggering (use T002 video or re-record baseline)
  - Look for synchronization issues, jitter, slowdown
  - PASS if: animation quality is visually identical

- [ ] T022 VE-003 - 24-hour stress test:
  - Run clock continuously for 24 hours with noise-reduction code active
  - Monitor serial output for any I2C errors, motor failures, or timing issues
  - Visually inspect actual clock periodically (every 6 hours) for hand position errors or jitter
  - PASS if: zero errors logged, all animations complete, no visual anomalies

- [ ] T023 [P] VE-004 - I2C latency test:
  - Add temporary debug logging in `main.cpp` to measure I2C response time per board
  - Run Waves animation for 5 minutes (stresses staggering timing)
  - Compute response time variance across boards
  - Verify staggering adapts to variable latencies (no conflicts, no missed moves)
  - Remove debug logging before final commit
  - PASS if: no I2C conflicts, staggering adapts smoothly

- [ ] T024 [P] Compile final code with `make all` and verify no warnings or errors

---

## Phase 7: Code Review & Documentation

Finalize implementation and prepare for merge.

### Tasks

- [ ] T025 [P] Review all changes:
  - Verify `master/src/clock_manger.cpp` modifications to `get_full_half_digit()` are correct
  - Verify `master/src/main.cpp` staggering logic in `set_lazy()`, `set_fun()`, `set_waves()`
  - Verify source constants are clearly documented and easy to tune
  - Confirm no dead code or debug logging remains (except intended serial output like "Set time:")
  - Ensure code style is consistent with existing patterns

- [ ] T026 Update `master/Makefile` if needed (likely no change required; already supports `make all`)

- [ ] T027 Update documentation:
  - Verify [quickstart.md](quickstart.md) testing procedure is complete
  - No changes needed to [README.md](../README.md) as feature is transparent

- [ ] T028 [P] Create commit message:
  - Title: `feat(master): add motor staggering and speed/acceleration derating for noise reduction`
  - Body: Summarize staggering approach, derating factors, validation method


---

## Phase 8: Merge to Master

Finalize and integrate.

### Tasks

- [ ] T029 Stage all changes: `git add master/src/clock_manger.cpp master/src/main.cpp`

- [ ] T030 Commit: `git commit -m "feat(master): add motor staggering and speed/acceleration derating for noise reduction"`

- [ ] T031 Checkout master: `git checkout master`

- [ ] T032 Merge branch: `git merge --no-ff 002-reduce-motor-noise -m "Merge feature: reduce motor noise during synchronized animation"`

- [ ] T033 Verify merge: `git log --oneline -3` and inspect final state

---

## Dependencies & Parallel Opportunities

### Critical Path

T001 → T002 → T003 → T004 → T005 → T006 → T007 → T008 → T009 → T010 → T011 → T012 → T013 → T014 → T015 → T016 → T017 → T018 → T019 → T020 → T021 → T022 → T023 → T024 → T025 → T026 → T027 → T028 → T029 → T030 → T031 → T032 → T033

### Parallelizable Groups (after T015 is complete)

- **Testing Group A** (T016, T017, T018): All three modes can be tested in parallel on the same device
  - T019 (mode switching) depends on T016 + T017 + T018
- **Validation Group B** (T020, T021, T022, T023): Can run in parallel
  - T020 (sound measurement) and T021 (video) can run simultaneously
  - T022 (24-hour test) can start after T024
  - T023 (I2C latency test) can run after T024

---

## Success Criteria Summary

**ALL tasks must complete before merge:**

✅ T001-T012: Implementation complete, code compiles  
✅ T013-T015: Timing validated, derating tuned  
✅ T016-T019: All animation modes tested  
✅ T020-T023: All 4 validation evidence criteria PASS  
✅ T024-T028: Code reviewed, documented, committed  
✅ T029-T033: Merged to master branch
