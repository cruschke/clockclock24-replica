# Feature Specification: General Daylight Saving Time Support

**Feature Branch**: `001-dst-support`  
**Created**: 2026-03-30  
**Status**: Draft  
**Input**: User description: "The clock implementation lacks support for daylight saving time. I am living in Berlin with DST changes twice a year and as it seems, the implementation does not take DST into consideration, we need to change it" (interpreted as a general DST-support feature, with Berlin as an example locale)

## Clarifications

### Session 2026-03-30

- Q: What should be the source of truth for DST behavior? → A: Use timezone identifier with built-in DST rules.
- Q: Which time source is authoritative when both network and browser/manual time are available? → A: Network time is authoritative; browser/manual time is fallback only when network time is unavailable.
- Q: How should existing fixed UTC-offset configurations be handled on release? → A: No migration path is required; firmware rollout uses EEPROM wipe and clean initialization.
- Q: How quickly should displayed time update after sync/reconnect and DST boundary changes? → A: Apply immediately on sync/reconnect and within 1 minute of DST boundary.
- Q: Should DST be applied for all timezones or only those that observe DST? → A: Apply DST only when configured timezone defines DST transitions.

## User Scenarios & Testing *(mandatory)*

<!--
  IMPORTANT: User stories should be PRIORITIZED as user journeys ordered by importance.
  Each user story/journey must be INDEPENDENTLY TESTABLE - meaning if you implement just ONE of them,
  you should still have a viable MVP (Minimum Viable Product) that delivers value.
  
  Assign priorities (P1, P2, P3, etc.) to each story, where P1 is the most critical.
  Think of each story as a standalone slice of functionality that can be:
  - Developed independently
  - Tested independently
  - Deployed independently
  - Demonstrated to users independently
-->

### User Story 1 - Correct Local Time Year-Round (Priority: P1)

As the owner, I want the clock to always show current local civil time,
including daylight saving transitions, so I do not need to manually correct time
twice a year.

**Why this priority**: This is the core functional value of the clock and fixes the
primary reliability issue for daily use.

**Independent Test**: Set representative timestamps before and after both yearly
time transitions and verify displayed clock time matches official local time for
the configured timezone.

**Acceptance Scenarios**:

1. **Given** the clock has network time and timezone is configured to a region
  with DST rules,
  **When** current time is in standard winter time, **Then** displayed time
  matches configured local time.
2. **Given** the clock has network time and timezone is configured to a region
  with DST rules,
  **When** current time is in summer daylight time, **Then** displayed time
  matches configured local time.
3. **Given** the annual spring-forward transition window, **When** the transition
  occurs, **Then** the displayed local time advances according to configured civil
  time rules without requiring manual intervention.
4. **Given** the annual fall-back transition window, **When** the transition
  occurs, **Then** the displayed local time follows configured civil time rules
  without requiring manual intervention.

---

### User Story 2 - Stable Behavior During Connectivity Changes (Priority: P2)

As the owner, I want daylight saving behavior to remain correct when internet
connectivity changes, so the clock remains trustworthy after temporary outages.

**Why this priority**: Clocks are expected to be robust; network interruptions are
 common in home setups and should not create avoidable time drift at DST boundaries.

**Independent Test**: Disconnect and reconnect network around representative
transition dates; verify the time shown after recovery matches configured local time.

**Acceptance Scenarios**:

1. **Given** the clock has a previously synchronized time, **When** network is
  temporarily unavailable, **Then** displayed time continues in configured local time
  without an immediate one-hour offset error.
2. **Given** the clock reconnects after outage, **When** the next synchronization
  happens, **Then** displayed time corrects to current local time for the configured
  timezone if needed.

---

### User Story 3 - Clear Configuration Expectations (Priority: P3)

As the owner, I want configuration behavior to be predictable for DST use,
so I understand when manual timezone settings apply and when automatic local rules
are in effect.

**Why this priority**: Clear behavior prevents user confusion and avoids accidental
misconfiguration after updates.

**Independent Test**: Retrieve and update configuration through the existing
management flow and verify resulting displayed time behavior matches documented
expectations.

**Acceptance Scenarios**:

1. **Given** DST support is enabled, **When** configuration is read,
   **Then** user-facing configuration information clearly reflects that local
   rules are applied.
2. **Given** a user changes time-related settings, **When** settings are saved,
   **Then** resulting runtime behavior remains consistent with documented rules.

---

### Edge Cases

- Device boots during the missing hour in spring transition.
- Device boots during the repeated hour in fall transition.
- Network time source is unavailable during a DST boundary and resumes later.
- User-provided browser time and network-synced time disagree around transition.
- Device receives browser/manual time while network time is available and synchronized.
- Device starts after firmware flash with EEPROM wiped and no timezone profile configured yet.
- Configured timezone does not observe DST year-round and must never receive seasonal offset jumps.

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: System MUST keep displayed time aligned with official local
  civil time throughout the year for the configured timezone identifier.
- **FR-002**: System MUST automatically apply yearly daylight saving
  transitions without requiring manual clock adjustment.
- **FR-003**: System MUST recover to correct local time for the configured timezone after network
  reconnection if synchronization was unavailable during a transition.
- **FR-009**: System MUST store and use a timezone identifier (for example,
  `Area/City`) as the canonical source for local time and DST rules instead of a
  fixed UTC offset.
- **FR-004**: System MUST preserve existing non-time-related user settings when
  introducing DST-capable behavior.
- **FR-005**: System MUST expose deterministic configuration behavior for time
  handling so users can understand the active time rule set.
- **FR-010**: System MUST treat synchronized network time as authoritative when
  available; browser/manual time input MUST only be used as fallback when network
  time is unavailable.
- **FR-011**: System MUST initialize time configuration from a clean state after
  firmware flash when EEPROM is wiped.
- **FR-012**: System MUST require explicit timezone identifier configuration after
  clean initialization before claiming full DST-correct local-time behavior.
- **FR-013**: System MUST update displayed time immediately after authoritative
  network synchronization or reconnection events.
- **FR-014**: System MUST apply DST boundary time changes within 1 minute of the
  boundary crossing.
- **FR-015**: System MUST apply DST transitions only for configured timezones that
  define DST rules and MUST NOT apply DST adjustments for non-DST timezones.
- **FR-006**: Feature scope MUST follow KISS: no new abstraction or dependency
  without explicit problem statement and simpler alternative considered.
- **FR-007**: User-visible changes MUST preserve UX consistency or document the
  intentional change in labels, interaction flow, and expected behavior.
- **FR-008**: Feature MUST define measurable performance expectations for impacted
  paths and include validation method(s).

### Key Entities *(include if feature involves data)*

- **Local Time Profile**: Represents the active local civil-time rule set used by
  the clock (timezone identifier with seasonal standard/daylight behavior when
  applicable).
- **Time Source State**: Represents current trust state of synchronized external
  time versus fallback/runtime time progression.
- **Time Configuration**: Represents persisted user configuration affecting time
  behavior and how it is interpreted at runtime, including timezone identifier and
  initialization state.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: For all tested timestamps spanning both annual transitions, displayed
  time matches configured local time with zero one-hour offset errors.
- **SC-002**: Clock requires no manual seasonal time correction over a full
  year-long operational cycle.
- **SC-003**: After network recovery, correct local time is restored within
  1 minute of successful synchronization.
- **SC-004**: At least 95% of owner validation checks around DST windows pass on
  first attempt without additional configuration changes.

### Validation Evidence *(mandatory)*

- **VE-001**: Validate representative pre/post-transition timestamps for spring
  and fall windows and compare against official local time references for the
  configured timezone.
- **VE-002**: Execute outage-and-recovery checks around transition windows and
  capture before/after displayed time evidence.
- **VE-003**: Validate configuration read/write behavior and confirm resulting
  displayed time behavior matches documented expectations.
- **VE-004**: Validate precedence behavior by submitting browser/manual time while
  network synchronization is available, and confirm no authoritative network time
  override occurs in the wrong direction.
- **VE-005**: Validate clean-flash startup behavior (EEPROM wiped), confirming
  initialization defaults and explicit timezone-selection path.
- **VE-006**: Validate update latency by measuring time-to-correct-display after
  sync/reconnect and at DST boundary transitions.
- **VE-007**: Validate a non-DST timezone profile and confirm no seasonal
  daylight-saving adjustment is applied.

## Assumptions

- Deployment may be in locales with or without DST; behavior must follow the
  configured timezone profile rules.
- Existing network time synchronization remains the authoritative external source
  when available.
- Existing web control and configuration surfaces continue to be used for user
  interaction.
- Firmware deployment for this release includes EEPROM wipe, so no in-place
  migration of prior persisted timezone settings is required.
- Scope is limited to correct local time behavior and related configuration
  clarity; unrelated animation behavior is unchanged.
