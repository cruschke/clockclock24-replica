# Feature Specification: New Animations from Fork

**Feature Branch**: `003-new-animations`  
**Created**: 2026-06-29  
**Status**: Draft  
**Input**: User description: "add new animations, there is a fork https://github.com/aslafsky/clockclock24-replica-all-animtions of the repo that has a lot of features not relevant for me, e.g. porting to a new ESP platform or mqtt, but it also has new animations that I want to cherry pick - only transfer these animations to my code base"

## Context

The fork `aslafsky/clockclock24-replica-all-animtions` adds 9 new animation modes on top of the 3 currently present in this repo (LAZY, FUN, WAVES). The fork also carries unrelated changes (MQTT, new ESP platform port, captive portal, daily restart, status LED, OTA handler) that are explicitly out of scope.

**Animations to port (new in fork, absent here)**:

| Mode name  | Enum value | Description |
|------------|------------|-------------|
| PROPELLER  | 3          | All hands spin CW/CCW opposite directions simultaneously into target time |
| ARROW      | 4          | All clocks collapse to a joint pose then a diagonal wavefront of propeller-spins reveals the time |
| RIPPLE     | 5          | All clocks collapse to a wave pose then a Manhattan-distance ripple from the centre reveals the time, left/right halves mirror each other |
| BUBBLE     | 6          | All clocks collapse to a bubble pose then a checkerboard of CW/CCW propeller-spins reveals the time |
| GEAR       | 7          | All clocks collapse to a centred fan pose then a Chebyshev-distance ring expansion (all hands CW) reveals the time |
| SCATTER    | 8          | Hour hand makes 2 CCW rotations at speed 400; minute hand makes 4 CCW rotations at speed 800; left-to-right column stagger |
| DIAGONAL   | 9          | Clocks collapse to a diagonal pose then a left-to-right wave reveals the time with CLOCKWISE2 |
| CASCADE    | 10         | Clocks collapse to pointing-down pose then column-by-column CCW reveal; hour hand 1 extra rotation, minute hand 2 extra rotations |
| CYCLE      | 11         | Round-robin through all non-LAZY modes, driven by minute-of-day, so web preview and firmware always agree |

**Prerequisite infrastructure** also in the fork and needed here:

- `get_speed_multiplier()` — the fork scales all speed/acceleration values by this multiplier; the current repo uses fixed constants. Needed to keep the new animations consistent with how the fork wrote them.
- `set_half_digit_full()` / `set_single_clock_full()` — per-hand direction control already merged into our `clock_manager.h`; confirm all helpers used by the new animations are present.
- New `digit.h` constants: `d_joint`, `d_CENT`, `d_bubble`, `d_diagonal`, `d_WAVE` (and supporting sub-digits). All are purely additive to `digit.h`.
- `dispatch_animation()` helper in `main.cpp` — keeps mode dispatch DRY; required by CYCLE.

**Out of scope** (fork additions not to port):

- MQTT (`mqtt_handler.cpp/.h`)
- Captive portal (`captive_portal.cpp/.h`)
- OTA update handler (`update_handler.cpp/.h`)
- Status LED (`status_led.cpp/.h`)
- `board_definitions.h` (new ESP target)
- Daily restart logic
- Watchdog / reconnection logic changes in `main.cpp`
- Any `clock_config` additions beyond the new mode enum values

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Select a new animation via the web UI (Priority: P1)

As a clock owner, I open the web interface, choose one of the nine new animation modes from the mode dropdown, and the clock immediately begins using that animation on the next minute change.

**Why this priority**: Core user value — the whole point of the feature is being able to pick a new animation.

**Independent Test**: Navigate to the web UI, change the mode to PROPELLER (or any other new mode), wait for the next minute tick, confirm the animation plays correctly.

**Acceptance Scenarios**:

1. **Given** the clock is running in LAZY mode, **When** the user selects PROPELLER in the web UI and saves, **Then** on the next minute change all clock hands perform the opposite-direction propeller spin into the new time.
2. **Given** the clock is in CYCLE mode, **When** the minute advances, **Then** the firmware selects the next animation from the cycle list (excluding LAZY) based on minutes-today modulo cycle count.
3. **Given** the user selects CASCADE, **When** the minute changes, **Then** the clock first collapses all hands to pointing-down, pauses, then reveals each column left-to-right with CCW rotation.

---

### User Story 2 - Cycle mode auto-rotates through all new animations (Priority: P2)

As a clock owner who prefers variety, I set CYCLE mode and the clock rotates through all non-LAZY animations automatically, using the same minute-of-day calculation that the web preview uses.

**Why this priority**: CYCLE is the most valuable animation for passive display; it keeps the clock visually interesting without manual intervention.

**Independent Test**: Set mode to CYCLE, note the current minute, verify the animation played matches the expected entry in the cycle order list.

**Acceptance Scenarios**:

1. **Given** CYCLE mode is active, **When** 10 successive minute changes occur, **Then** each one plays a different animation from the cycle order, cycling back to the first after the last.
2. **Given** the firmware just rebooted with CYCLE mode, **When** the first minute tick fires, **Then** the animation chosen matches `cycle_order[minutes_today % cycle_count]` — consistent with the web UI preview.

---

### User Story 3 - Existing animations are unaffected (Priority: P3)

As an existing user who prefers LAZY, FUN, or WAVES, these modes continue to work exactly as before after the port.

**Why this priority**: Regression guard — the feature should be purely additive.

**Independent Test**: Run LAZY, FUN, and WAVES before and after the change; verify identical behaviour and timing.

**Acceptance Scenarios**:

1. **Given** LAZY mode was working before this change, **When** the firmware is rebuilt with the new animations added, **Then** LAZY behaves identically (staggered send, same speed/acceleration).
2. **Given** WAVES mode was working, **When** the new firmware is flashed, **Then** WAVES still executes its two-phase wave sequence without regression.

---

### Edge Cases

- What happens when CYCLE mode wraps past the last animation? It silently wraps back to the first entry using modulo — no special handling needed.
- How does the system handle an invalid mode value (e.g., a stored EEPROM value from a future version)? Existing `dispatch_animation` switch-case falls through silently; the clock does nothing for the unknown mode until the next minute.
- SCATTER uses `COUNTERCLOCKWISE5` (enum value 13) for the minute hand. Slave boards running unmodified firmware interpret value 13 as an unknown mode, causing incorrect minute-hand rotation during the animation. The final time angle is still reached correctly. This is a known, accepted limitation — slave firmware will not be reflashed to maintain consistency across all boards.

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: The firmware MUST expose nine new animation modes: PROPELLER, ARROW, RIPPLE, BUBBLE, GEAR, SCATTER, DIAGONAL, CASCADE, CYCLE — selectable from the web UI in addition to the existing LAZY, FUN, WAVES. SCATTER is included with a known limitation: the minute hand will not animate correctly on slave boards that have not been reflashed (all slave boards in this deployment). The final time position is still correct.
- **FR-002**: Each new animation MUST implement the same two-phase or single-phase motion described in the fork's `main.cpp` without logic changes that would alter the visible output.
- **FR-003**: The CYCLE mode MUST rotate through all non-LAZY modes using `minutes_today % cycle_count` so the firmware and web preview always agree.
- **FR-004**: New digit pose constants (`d_joint`, `d_CENT`, `d_bubble`, `d_diagonal`, and supporting sub-digits already in fork's `digit.h`) MUST be added to `digit.h` in this repo.
- **FR-005**: `dispatch_animation()` MUST be introduced in `main.cpp` so the mode-switch logic is shared between `set_time()` and `set_cycle()`.
- **FR-006**: `get_speed_multiplier()` MUST be available (either ported from fork or implemented as a stub returning a fixed value) because all fork animation speed/acceleration values are multiplied by it.
- **FR-007**: The `clock_modes` enum in `clock_config.h` MUST be extended with the new mode identifiers in the same order as the fork (PROPELLER=3 … CYCLE=11) so stored EEPROM values remain compatible.
- **FR-008**: The web UI mode dropdown MUST list and label all new animation modes so users can select them by name.
- **FR-009**: Feature scope MUST follow KISS: MQTT, captive portal, OTA handler, status LED, board definitions, and watchdog changes from the fork MUST NOT be ported.
- **FR-010**: User-visible changes MUST preserve UX consistency: mode names shown in the web UI MUST match the firmware enum names.
- **FR-011**: The build MUST compile without warnings after the port on the existing target platform.

### Key Entities

- **Animation mode** (enum `clock_modes`): Named integer constant that identifies which animation function `dispatch_animation()` calls.
- **Digit pose constant** (`t_full_clock` / `t_digit`): Compile-time array of hand angles used as the intermediate "collapsed" state in two-phase animations.
- **Speed multiplier** (`get_speed_multiplier()`): Integer scaling factor applied to all speed/acceleration values, allowing the user to globally slow or speed up animations.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: All nine new animation modes are selectable via the web UI and each plays its distinct visual pattern on the next minute change — verified on hardware or simulator.
- **SC-002**: CYCLE mode plays a different animation each minute for at least 11 consecutive minutes without repeating the same mode twice in a row (given no wrap).
- **SC-003**: LAZY, FUN, and WAVES continue to function identically to their pre-feature behaviour (no observable change in hand motion or timing).
- **SC-004**: Firmware compiles with zero new warnings on the existing PlatformIO target.

### Validation Evidence *(mandatory)*

- **VE-001**: For each new animation, provide a manual hardware validation step: set the clock to that mode and confirm the documented two-phase sequence executes (e.g., hands collapse to intermediate pose, then reveal time with the expected stagger/rotation pattern).
- **VE-002**: For CYCLE mode: record the minute-of-day at test time, compute expected `cycle_order[minutes_today % 10]`, and confirm the firmware plays that animation.
- **VE-003**: For regression: run LAZY and WAVES before and after the port; confirm no change in hand motion by observing the clock for two consecutive minute changes.
- **VE-004**: Build log must show zero new compiler warnings compared to the pre-feature build.

## Clarifications

### Session 2026-06-29

- Q: Should SCATTER be included given its minute hand will visually misfire on all slave boards (slave firmware not reflashed)? → A: Include SCATTER with the known slave limitation documented in the UI and code.

## Assumptions

- `get_speed_multiplier()` is either already present in this repo's `clock_config` or will be added as a minimal function returning `100` (matching the fork's default) — the exact implementation is a task-level decision.
- The web UI HTML (`web_page.h`) can be updated by adding new `<option>` entries to the mode dropdown; no structural changes to the UI framework are required.
- The existing `set_half_digit_full()` and `set_single_clock_full()` functions are already present in this repo's `clock_manager` (confirmed: they appear in `master/include/clock_manager.h` at HEAD).
- The port targets the existing ESP8266/ESP32 platform and PlatformIO setup; no board or toolchain changes are needed.
- The fork's `d_stop` and `d_IIII` and `d_fun` pose constants already exist in this repo's `digit.h`; only the new pose constants need to be added.
- Mobile / responsive web UI testing is out of scope for this feature.
