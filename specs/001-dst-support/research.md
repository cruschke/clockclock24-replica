# Phase 0 Research: General Daylight Saving Time Support

## Decision 1: Use timezone identifier as canonical config key
- Decision: Persist a timezone identifier string as the canonical time-profile setting, while retaining legacy numeric UTC offset as temporary fallback.
- Rationale: Matches clarified requirements, supports DST/no-DST behavior switching by profile, and avoids reintroducing manual seasonal edits.
- Alternatives considered:
  - Fixed UTC offset only: rejected because DST transitions cannot be represented robustly.
  - Manual DST toggle: rejected because it is error-prone and does not scale across locales.

## Decision 2: Keep network time authoritative, browser/manual as fallback
- Decision: Continue treating synchronized NTP time as authoritative whenever external connection is available; only use browser/manual time when network time is unavailable.
- Rationale: Prevents conflicting authorities and preserves deterministic behavior during normal operation.
- Alternatives considered:
  - Browser/manual override always wins: rejected because it can silently desynchronize clock from authoritative source.
  - Prompt user each time authority changes: rejected as unnecessary UX complexity for a hobby clock.

## Decision 3: Compute local civil time from timezone rules, not by adding a fixed hour offset
- Decision: Move from fixed-offset adjustment to timezone-rule based local-time evaluation for display logic.
- Rationale: Required to support spring-forward and fall-back transitions and to avoid one-hour errors.
- Alternatives considered:
  - Continue `offset * SECS_PER_HOUR` arithmetic: rejected due to no DST support.
  - Hardcode one locale's DST rules globally: rejected because feature scope is general DST support.

## Decision 4: Clean-flash initialization strategy
- Decision: Firmware release will include EEPROM wipe as part of the flash process, ensuring all devices start with empty persistent config. Initialization will default to unconfigured timezone state and require explicit user selection before claiming full DST-correct behavior.
- Rationale: Eliminates migration-mode state machine complexity and dual-code-path testing burden. All devices begin from a known clean state with no legacy offset to preserve. Simpler initialization logic, fewer edge cases, and deterministic behavior from day one.
- Alternatives considered:
  - In-place migration mode with fallback: rejected because it doubles implementation complexity and extends technical debt (requires parallel legacy-offset and timezone-identifier code paths, migration-required state tracking, mode-switching transitions).
  - Immediate reset to UTC/default timezone on flash: rejected because it can cause abrupt incorrect local display after upgrade without user intervention.
  - Preserve legacy offset in separate unused config namespace: rejected as unnecessary complexity when EEPROM wipe is performed by build/flash process.

## Decision 5: Extend existing HTTP config/time contract incrementally
- Decision: Extend current `/config` and `/time` payloads with timezone-profile fields and migration status while keeping current endpoints and existing fields available.
- Rationale: Preserves current web app flow and avoids introducing new endpoint surface area.
- Alternatives considered:
  - New dedicated timezone endpoint only: rejected as extra complexity for little gain.
  - Breaking payload replacement: rejected due to compatibility risk with current UI logic.

## Decision 6: Validation strategy is scenario-based manual + simulation checks
- Decision: Use documented boundary-time simulation, reconnect tests, and non-DST profile checks as primary evidence for this embedded feature.
- Rationale: Repository currently has no robust automated test harness for hardware timing behavior; manual protocol is required by constitution and practical for this scope.
- Alternatives considered:
  - Full automated integration harness: rejected for current scope/effort disproportion.
  - Ad-hoc smoke checks only: rejected because they miss DST edge cases and migration paths.
