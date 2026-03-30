# Tasks: General Daylight Saving Time Support

**Input**: Design documents from `/specs/001-dst-support/`
**Prerequisites**: plan.md (required), spec.md (required for user stories), research.md, data-model.md, contracts/

**Tests**: Include validation tasks for every user story. Automated tests are preferred when feasible; simulation/manual hardware validation tasks are required when automation is not practical.

**Organization**: Tasks are grouped by user story to enable independent implementation and testing of each story.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story this task belongs to (e.g., US1, US2, US3)
- Include exact file paths in descriptions

## Path Conventions

- Embedded firmware: `master/src/`, `master/include/`
- Web interface: `master/web/`
- Feature docs and validation: `specs/001-dst-support/`

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Prepare feature scaffolding and baseline interfaces for DST support.

- [X] T001 Add DST feature phase checkpoints in specs/001-dst-support/plan.md
- [X] T002 [P] Add timezone configuration steps to specs/001-dst-support/quickstart.md
- [X] T003 [P] Create timezone validation evidence log template in specs/001-dst-support/validation/README.md
- [X] T004 [P] Add web asset regeneration step note for timezone UI updates in master/web/minimize.js

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Establish core data/config/runtime primitives required by all user stories.

**⚠️ CRITICAL**: No user story work can begin until this phase is complete.

- [X] T005 Extend persisted time configuration schema for timezone_id and timezone_configured fields in master/include/clock_config.h
- [X] T006 Implement timezone_id persistence and clean-state initialization defaults in master/src/clock_config.cpp
- [X] T007 [P] Add timezone profile/runtime authority declarations and helpers in master/include/ntp.h
- [X] T008 Implement timezone-rule based offset resolution and next-transition handling in master/src/ntp_timezone.cpp
- [X] T009 Wire timezone-rule runtime support into NTP provider path in master/include/ntp.h
- [X] T010 [P] Add `/config` and `/time` contract fields for timezone profile and authority in specs/001-dst-support/contracts/time-config-api.md
- [X] T011 Add DST profile constants and non-DST handling table in master/include/timezone_profiles.h

## Phase 2b: Code Quality & Test Evidence Gates

**Purpose**: Ensure quality baseline per constitution before proceeding to user story implementation.

- [X] T040 [P] Compile check: Build master firmware without warnings on target platform (PlatformIO compile step)
- [X] T041 [P] Code quality check: Run linter and formatter on all modified files in master/src and master/include
- [X] T042 [P] Code review checklist: Verify boundary conditions, error handling, and state transitions in timezone runtime logic

**Checkpoint**: Code quality gates passed; ready for user story implementation.

---

## Phase 3: User Story 1 - Correct Local Time Year-Round (Priority: P1) 🎯 MVP

**Goal**: Clock shows correct local civil time year-round using timezone identifier with automatic DST transitions.

**Independent Test**: Simulate spring/fall boundaries and verify displayed time matches official local time for configured timezone.

### Validation for User Story 1

- [X] T012 [P] [US1] Add spring/fall boundary validation procedure and expected outcomes in specs/001-dst-support/quickstart.md
- [X] T013 [US1] Record US1 DST boundary evidence checklist in specs/001-dst-support/validation/us1-dst-boundary.md

### Implementation for User Story 1

- [X] T014 [P] [US1] Replace fixed offset arithmetic with timezone-rule local time conversion in master/include/ntp.h
- [X] T015 [P] [US1] Integrate timezone profile state into main time update flow in master/src/main.cpp
- [X] T016 [US1] Ensure DST boundary transitions are applied within 1 minute in master/src/main.cpp
- [X] T017 [US1] Apply non-DST timezone behavior guard (no seasonal jumps) in master/src/ntp_timezone.cpp
- [X] T018 [US1] Expose current resolved offset and transition metadata in debug serial output from master/src/main.cpp

**Checkpoint**: User Story 1 functional and independently verifiable.

---

## Phase 4: User Story 2 - Stable Behavior During Connectivity Changes (Priority: P2)

**Goal**: Preserve correct local time behavior across offline fallback and network recovery with strict authority precedence.

**Independent Test**: Run outage/recovery scenario around DST windows and verify immediate correction on re-sync.

### Validation for User Story 2

- [X] T019 [P] [US2] Add network outage/recovery DST scenario steps in specs/001-dst-support/quickstart.md
- [X] T020 [US2] Record authority precedence and reconnect evidence in specs/001-dst-support/validation/us2-connectivity-authority.md

### Implementation for User Story 2

- [X] T021 [US2] Enforce `network_ntp` authority precedence over browser/manual time in master/src/main.cpp
- [X] T022 [US2] Reject browser/manual authoritative override while NTP is available in master/src/web_server.cpp
- [X] T023 [US2] Implement fallback-to-browser/manual behavior only when network time is unavailable in master/src/main.cpp
- [X] T024 [US2] Trigger immediate time correction after successful network resynchronization in master/src/main.cpp
- [X] T025 [US2] Persist and expose current time authority state in master/src/clock_config.cpp

**Checkpoint**: User Story 2 works independently with reliable authority transitions.

---

## Phase 5: User Story 3 - Clear Configuration Expectations (Priority: P3)

**Goal**: Users can understand and control timezone profile configuration and active behavior; UI clearly shows timezone selection state and authority status.

**Independent Test**: Read/update config and verify timezone_id and timezone_configured state match UI display; test timezone selection flow.

### Validation for User Story 3

- [X] T026 [P] [US3] Add clean-flash timezone configuration validation steps in specs/001-dst-support/quickstart.md
- [X] T027 [US3] Record timezone configuration behavior and evidence in specs/001-dst-support/validation/us3-config-ux.md

### Implementation for User Story 3

- [X] T028 [P] [US3] Extend `/config` JSON payload with timezone_id and timezone_configured in master/src/web_server.cpp
- [X] T029 [P] [US3] Extend `/time` form handling to accept and validate `timezone_id` in master/src/web_server.cpp
- [X] T030 [US3] Add timezone_id getters/setters in master/include/clock_config.h
- [X] T031 [US3] Implement timezone profile selector in master/web/index.html
- [X] T032 [US3] Update `sendDate()` and config sync client logic for timezone_id-aware requests in master/web/index.html
- [X] T033 [US3] Regenerate compressed web payload header after UI changes in master/include/web_page.h

**Checkpoint**: User Story 3 behavior is clear, consistent, and independently testable.

---

## Phase 6: Polish & Cross-Cutting Concerns

**Purpose**: Finalize docs, validation evidence, quality/performance checks, and regression testing across all stories.

- [X] T034 [P] Consolidate all scenario evidence summaries in specs/001-dst-support/validation/summary.md
- [ ] T035 Verify full quickstart pass and capture final results in specs/001-dst-support/quickstart.md
- [X] T036 [P] Update README time-sync behavior notes for timezone identifier and DST support in README.md
- [X] T037 Review and align API contract wording with implemented payloads in specs/001-dst-support/contracts/time-config-api.md
- [ ] T038 Execute manual performance check for sync/reconnect and boundary latency targets in specs/001-dst-support/validation/performance.md
- [X] T039 Constitution compliance review against all five principles in specs/001-dst-support/validation/constitution-check.md
- [ ] T043 [US1] FR-004 Regression Test: Verify non-time settings (animation mode, sleep schedule, etc.) are preserved after timezone configuration in specs/001-dst-support/validation/fr-004-regression.md
- [ ] T044 [US1] Measure DST boundary transition latency: Capture evidence that timezone boundary detection and display update occur within 1 minute; document in specs/001-dst-support/validation/performance.md

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies - can start immediately.
- **Foundational (Phase 2)**: Depends on Setup completion - blocks all user stories.
- **Code Quality (Phase 2b)**: Runs within Phase 2 after foundational features are coded; gates user story start.
- **User Stories (Phases 3-5)**: Depend on Phase 2b Code Quality gate completion.
- **Polish (Phase 6)**: Depends on completion of all targeted user stories.

### User Story Dependencies

- **US1 (P1)**: Starts immediately after Phase 2; foundation for timezone-rule correctness.
- **US2 (P2)**: Depends on US1 timezone-rule runtime to validate authority transitions correctly.
- **US3 (P3)**: Depends on US1 config/runtime primitives; can overlap late with US2 once payload fields are stable.

### Within Each User Story

- Validation design tasks before core implementation.
- Runtime model/config updates before endpoint or UI wiring.
- Endpoint/backend changes before web UI consumption updates.
- Story evidence completed before declaring story done.

### Parallel Opportunities

- Phase 1 tasks T002-T004 can run in parallel.
- Phase 2 tasks T007, T010, and T011 can run in parallel after T005 starts.
- Phase 2b quality-gate tasks T040, T041, and T042 can run in parallel after Phase 2 implementation is drafted (before merge).
- US1 tasks T014 and T015 can run in parallel.
- US2 validation task T019 can run in parallel with implementation prep.
- US3 tasks T028 and T029 can run in parallel, then UI tasks proceed.
- Polish tasks T034, T036, T039, T043, and T044 can run in parallel.

---

## Parallel Example: User Story 1

```bash
# Parallel implementation start for US1:
Task: "Replace fixed offset arithmetic with timezone-rule local time conversion in master/include/ntp.h"
Task: "Integrate timezone profile state into main time update flow in master/src/main.cpp"
```

## Parallel Example: User Story 2

```bash
# Parallel validation + implementation prep for US2:
Task: "Add network outage/recovery DST scenario steps in specs/001-dst-support/quickstart.md"
Task: "Enforce network_ntp authority precedence over browser/manual time in master/src/main.cpp"
```

## Parallel Example: User Story 3

```bash
# Parallel API contract implementation for US3:
Task: "Extend /config JSON payload with timezone fields in master/src/web_server.cpp"
Task: "Extend /time form handling to accept timezone_id in master/src/web_server.cpp"
```

---

## Implementation Strategy

### MVP First (User Story 1 Only)

1. Complete Phase 1 and Phase 2.
2. Deliver Phase 3 (US1) end-to-end.
3. Validate DST boundary correctness via quickstart scenarios.
4. Demo/deploy once US1 acceptance is met.

### Incremental Delivery

1. Deliver US1 for core DST correctness.
2. Add US2 for authority and reconnection robustness.
3. Add US3 for clear configuration UX and timezone control transparency.
4. Run Phase 6 cross-cutting validation and documentation updates.

### Parallel Team Strategy

1. One developer focuses on firmware core (`main.cpp`, `ntp` timezone logic).
2. One developer handles API/config contract and persistence (`clock_config.cpp`, `web_server.cpp`).
3. One developer handles UI and validation docs (`master/web/index.html`, `specs/001-dst-support/validation/`).

---

## Notes

- [P] tasks are limited to file-independent work where dependency conditions are explicit.
- Each user story remains independently testable per the spec.
- Prefer preserving existing endpoint compatibility while extending payloads.
- Keep implementation KISS: avoid introducing heavy timezone libraries or unnecessary abstractions.
