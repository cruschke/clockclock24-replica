# Implementation Plan: General Daylight Saving Time Support

**Branch**: `001-dst-support` | **Date**: 2026-03-30 | **Spec**: `/specs/001-dst-support/spec.md`
**Input**: Feature specification from `/specs/001-dst-support/spec.md`

**Note**: This template is filled in by the `/speckit.plan` command. See `.specify/templates/plan-template.md` for the execution workflow.

## Summary

Replace the current fixed UTC-offset time handling with timezone-identifier based
local-time computation that supports DST transitions automatically. Keep NTP as the
authoritative source when available, and extend existing web configuration endpoints
to expose timezone profile and authority state. This release assumes clean firmware
flash with EEPROM wipe, so no legacy config migration path is required.

## Technical Context

**Language/Version**: C++ (Arduino framework on ESP8266), JavaScript (browser UI)
**Primary Dependencies**: ESP8266 Arduino core (`ESP8266WiFi`, `ESP8266WebServer`, `WiFiUdp`, `time` APIs), `TimeLib`, local `Preferences` library
**Storage**: ESP8266 non-volatile preferences (`Preferences` namespace `clockclock24`)
**Testing**: Manual hardware and web-flow validation, serial log verification, boundary-time simulation via `/time` endpoint inputs
**Target Platform**: Wemos D1 mini lite / ESP8266 master board, browser clients on local network
**Project Type**: Embedded firmware + local web interface
**Performance Goals**: Apply authoritative time updates immediately on sync/reconnect; apply DST boundary updates within 1 minute; no visible regression in animation responsiveness
**Constraints**: Constrained RAM/flash, no heavyweight timezone database dependency, preserve existing endpoint behavior where possible, release process includes EEPROM wipe
**Scale/Scope**: Single clock controller device, home LAN usage, low-concurrency web configuration access

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

- **KISS-First Engineering**: PASS. Reuse existing config/web/NTP flow and add
  minimal timezone-profile logic instead of introducing large
  new architecture.
- **Code Quality Baseline**: PASS. Changes are limited to `master/src` and
  `master/include` time/config/web modules, with focused updates, plus explicit
  compile/lint/format checks (T040-T042) in Phase 2b before user story implementation.
- **Test Evidence for Every Change**: PASS. Plan includes DST boundary tests,
  network outage/recovery tests, clean-flash initialization tests, and non-DST
  timezone tests.
- **Consistent User Experience**: PASS. Existing endpoints remain, configuration
  payload extends with explicit timezone and authority indicators.
- **Performance Within Hardware Limits**: PASS. Explicit latency goals and
  validation steps are defined for sync/reconnect and DST boundary updates.

**Post-Design Re-check (Phase 1)**: PASS. Research, data model, contracts, and
quickstart preserve the same gates with no newly introduced violations.

## Project Structure

### Documentation (this feature)

```text
specs/001-dst-support/
├── plan.md              # This file (/speckit.plan command output)
├── research.md          # Phase 0 output (/speckit.plan command)
├── data-model.md        # Phase 1 output (/speckit.plan command)
├── quickstart.md        # Phase 1 output (/speckit.plan command)
├── contracts/           # Phase 1 output (/speckit.plan command)
└── tasks.md             # Phase 2 output (/speckit.tasks command - NOT created by /speckit.plan)
```

### Source Code (repository root)
```text
master/
├── include/
│   ├── clock_config.h
│   ├── ntp.h
│   ├── timezone_profiles.h
│   └── web_server.h
├── src/
│   ├── main.cpp
│   ├── clock_config.cpp
│   ├── ntp_timezone.cpp
│   └── web_server.cpp
└── web/
  ├── index.html
  └── minimize.js

specs/001-dst-support/
├── spec.md
├── plan.md
├── research.md
├── data-model.md
├── quickstart.md
└── contracts/
  └── time-config-api.md
```

**Structure Decision**: Use existing embedded-firmware plus web-config structure
under `master/`; no new top-level projects are introduced. Add one timezone runtime
source file and one timezone profile header only.

## Implementation Phases & Checkpoints

### Phase 1: Setup (T001-T004)
**Checkpoint**: Documentation infrastructure ready; validation templates prepared
- [ ] T001 ✓ Add DST feature phase checkpoints in plan.md
- [ ] T002 ✓ Add timezone configuration steps to quickstart.md
- [ ] T003 ✓ Create timezone validation evidence log template
- [ ] T004 ✓ Add web asset regeneration step note

**Success Gate**: All documentation scaffolding in place, quickstart sections defined

### Phase 2: Foundational (T005-T011)
**Checkpoint**: Core data/config/runtime primitives ready; foundation for all user stories
- [ ] T005-T011 (firmware schema, persistence, profiles, contracts)

**Success Gate**: Compile check passed (T040); all modules compile without warnings

### Phase 2b: Code Quality & Test Gates (T040-T042)
**Checkpoint**: Quality baseline verified; code ready for user story implementation
- [ ] T040 ✓ Compile check (PlatformIO build)
- [ ] T041 ✓ Code quality/linting check
- [ ] T042 ✓ Code review checklist

**Success Gate**: All three gates passed; violations fixed or documented

### Phase 3: User Story 1 - Correct Local Time Year-Round (T012-T018)
**Checkpoint**: DST transitions functional; spec scenarios validated
- [ ] T012-T018 (boundary validation, timezone-rule conversion, latency gates)

**Success Gate**: US1 acceptance scenarios pass; DST boundary transitions work correctly

### Phase 4: User Story 2 - Stable Behavior During Connectivity (T019-T025)
**Checkpoint**: Authority precedence enforced; network recovery restores correct time
- [ ] T019-T025 (outage/recovery, precedence enforcement, authority state)

**Success Gate**: US2 acceptance scenarios pass; authority transitions are deterministic

### Phase 5: User Story 3 - Clear Configuration Expectations (T026-T033)
**Checkpoint**: Timezone selection UI complete; config behavior is transparent
- [ ] T026-T033 (UI selectors, config payload, clean-flash validation)

**Success Gate**: US3 acceptance scenarios pass; timezone configuration is user-friendly

### Phase 6: Polish & Cross-Cutting Concerns (T034-T044)
**Checkpoint**: All validation evidence gathered; documentation complete; compliance verified
- [ ] T034-T044 (evidence consolidation, API alignment, performance measurement, constitution review, FR-004 regression, latency measurement)

**Success Gate**: All requirements validated; evidence captured; constitution compliance confirmed

## Complexity Tracking

> **Fill ONLY if Constitution Check has violations that must be justified**

| Violation | Why Needed | Simpler Alternative Rejected Because |
|-----------|------------|-------------------------------------|
| None | N/A | N/A |
