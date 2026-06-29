# Feature Specification: Day Mode Schedule

**Feature Branch**: `004-day-mode-schedule`  
**Created**: 2026-06-29  
**Status**: Draft  
**Input**: User description: "introduce a new configuration that allows to select an animation during the day and introduce a silent mode (e.g. 6pm) that runs the clock in lazy mode. off -> selected mode -> silent mode -> sleep mode. Dropdown for time, hour granularity, selected mode for regular time should pick what is selected (a dedicated animation or cycle), silent mode is always lazy."

## Context

The clock currently supports a fixed animation mode and a sleep schedule (per-hour on/off per day). This feature adds a single new configuration: a **silent mode start hour**. From that hour until sleep begins, the clock automatically plays Lazy instead of the selected animation.

The daily cycle becomes:

1. **Sleep** — existing sleep schedule keeps clock off (existing)
2. **Active mode** — plays the user's chosen animation (existing)
3. **Silent mode** — switches to Lazy at a configurable hour (new)
4. **Sleep again** — existing sleep schedule resumes (existing)

The active window is already fully controlled by the existing per-hour sleep schedule. No new "wake" time is needed.

## Clarifications

### Session 2026-06-29

- Q: Should the effective mode re-evaluate only on the next minute tick, or also immediately when the silent start setting is saved? → A: Apply on next minute tick AND immediately when saved via web UI.

## User Scenarios & Testing *(mandatory)*

### User Story 1 — Configure silent mode start time (Priority: P1)

As a clock owner, I open the web UI and set a "silent from" hour (e.g. 18:00). From that hour onwards until sleep time, the clock automatically plays Lazy instead of the selected animation.

**Why this priority**: Core value — without a configurable silent-mode time the feature does not exist.

**Independent Test**: Set silent mode start to 18:00. At 18:00 on the clock, observe the animation switches to Lazy. At 17:59 it still plays the selected animation.

**Acceptance Scenarios**:

1. **Given** active mode is CASCADE and silent start is 18:00, **When** the clock transitions from 17:59 to 18:00, **Then** the animation switches to Lazy automatically without user interaction.
2. **Given** silent start is 18:00 and sleep starts at 22:00, **When** the clock is checked at 20:00, **Then** it plays Lazy.
3. **Given** silent start is set to "disabled" (no silent mode), **When** the clock runs at any hour outside sleep, **Then** it plays the selected active animation unchanged.
4. **Given** the clock is playing CASCADE at 20:00 and the user sets silent start to 20, **When** the setting is saved, **Then** the clock switches to Lazy immediately without waiting for the next minute.

---

### User Story 2 — Full daily cycle plays correctly end-to-end (Priority: P2)

As a clock owner with silent start at 18:00 and sleep at 22:00, the clock follows: active animation → lazy → sleep — automatically across a full day without manual intervention.

**Why this priority**: Integration validation — confirms the silent transition works alongside the existing sleep schedule.

**Independent Test**: Set silent start to 18:00 with sleep from 22:00. Observe active animation before 18:00, Lazy from 18:00–22:00, and off from 22:00.

**Acceptance Scenarios**:

1. **Given** silent start is 18:00 and sleep is 22:00, **When** a full evening passes, **Then** the clock plays the selected animation until 18:00, then Lazy until 22:00, then stops.
2. **Given** sleep is not configured for some hours, **Then** the clock stays in Lazy from silent start until sleep resumes.

---

### Edge Cases

- What if silent start equals or is later than the sleep hour? Silent mode is skipped; the clock goes directly from active to sleep.
- What if no silent start is configured ("disabled")? The feature is inactive and all existing behaviour is preserved exactly.
- What happens on days where sleep is enabled for certain hours that overlap with the silent window? The existing per-hour sleep mask takes precedence — if an hour is marked as sleep, the clock is off regardless of silent mode.

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: The web UI MUST provide a "silent from" dropdown (hours 0–23 + "disabled") that sets the hour at which the clock switches to Lazy mode.
- **FR-002**: When the current hour is within the silent window (silent start ≤ current hour, and the hour is not marked as sleep), the firmware MUST play Lazy regardless of the selected animation mode.
- **FR-003**: When the current hour is before the silent start (and not marked as sleep), the firmware MUST play the animation currently set as the clock mode.
- **FR-004**: The silent start hour MUST persist across power cycles (stored in non-volatile memory).
- **FR-005**: When "disabled" is selected, no silent transition occurs and all existing behaviour is preserved.
- **FR-006**: The effective mode MUST be re-evaluated at each minute tick (catching hour boundaries within one minute) AND immediately when the silent start setting is saved via the web UI, so the change takes effect without waiting for the next minute.
- **FR-007**: Feature scope MUST follow KISS: no new animation logic, no changes to existing sleep-schedule behaviour, no new UI pages. Active window is already fully controlled by the existing sleep schedule.
- **FR-008**: The "silent from" label in the web UI MUST clearly distinguish it from the existing sleep schedule (which turns the clock off entirely).
- **FR-009**: The build MUST compile without warnings after the change.

### Key Entities

- **Silent start hour**: Integer 0–23 or "disabled"; the hour from which Lazy plays instead of the selected animation.
- **Effective mode**: The animation mode actually played at any given hour — Lazy if current hour ≥ silent start (and not in sleep), otherwise the selected mode.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: A clock owner can configure the silent start time in under 30 seconds using the web UI.
- **SC-002**: The clock transitions from active animation to Lazy within one minute of the configured silent start hour.
- **SC-003**: All existing animation modes, sleep schedule, and OFF behaviour are unchanged when silent start is disabled.
- **SC-004**: Firmware compiles with zero new warnings.

### Validation Evidence *(mandatory)*

- **VE-001**: Manual hardware test — configure silent start = current hour + 1, wait for transition, confirm Lazy plays.
- **VE-002**: Regression test — set silent start to "disabled"; verify clock behaves identically to pre-feature firmware.
- **VE-003**: Build log shows zero new compiler warnings.

## Assumptions

- "Disabled" means no silent transition is applied; existing behaviour is fully preserved.
- Silent mode always uses Lazy — fixed, not configurable.
- Hour granularity is sufficient — sub-hour precision is out of scope.
- The existing per-day/per-hour sleep schedule takes precedence: if a given hour is marked as sleep, sleep wins over silent mode.
- The active window (when the clock plays the selected animation) is already fully controlled by the existing sleep schedule — no new "active from" time is needed.
- Only one silent start hour is supported (single daily schedule, no per-day variation).
- The web UI implementation reuses the existing dropdown pattern already present in the interface.
