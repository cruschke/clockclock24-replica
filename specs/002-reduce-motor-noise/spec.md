# Feature Specification: Reduce Motor Noise During Synchronized Animation

**Feature Branch**: `002-reduce-motor-noise`  
**Created**: March 31, 2026  
**Status**: Draft  
**Input**: User description: "I want to make the clock less noisy, especially when all clock hands are moving"

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

### User Story 1 - Reduce Peak Noise During Full-Clock Animation (Priority: P1)

Operating the clock with all 24 hands moving simultaneously (e.g., at time transition or during Waves animation) produces loud audible noise from synchronized stepper motor current spikes. Users want the clock to operate more quietly, especially in shared/home environments.

**Why this priority**: This is the primary user complaint and occurs on every minute transition or full animation. It directly impacts daily user experience and device usability.

**Independent Test**: Can be tested by observing/measuring sound level when clock transitions to a new minute (all 24 motors moving at once) before and after staggering plus mild acceleration reduction are applied.

**Acceptance Scenarios**:

1. **Given** clock is displaying time with no hands moving, **When** minute changes and all 24 hands need to reposition, **Then** motor startup is staggered temporally (not all motors start simultaneously), acceleration is mildly reduced, and perceived noise is measurably reduced
2. **Given** clock is in Waves animation mode, **When** wave animation plays (sequential hand updates), **Then** staggering preserves the intended visual sequencing and does not degrade animation quality

---

### User Story 2 - Preserve Animation Timing Accuracy (Priority: P1)

While reducing noise through staggered motor startup, the visual animation timing and hand synchronization must remain perceptually correct so users cannot detect any lag or jitter in the displayed time.

**Why this priority**: Animation quality and time accuracy are essential features. Noise reduction must not trade off user-visible correctness.

**Independent Test**: Run all three animation modes (Lazy, Fun, Waves) and compare before/after staggering using high-speed video capture. Animation should appear visually identical in smoothness and timing.

**Acceptance Scenarios**:

1. **Given** clock displays a specific time, **When** staggered motor timing and mild acceleration derating are active, **Then** all 8 board sections complete their moves within ±100ms of synchronized baseline
2. **Given** any animation mode is active, **When** motors are staggered, **Then** visual animation quality is indistinguishable from non-staggered version when observed at normal viewing distance

---

### User Story 3 - Support All Animation Modes (Priority: P1)

Noise reduction must work seamlessly across all three clock animation modes (Lazy, Fun, Waves) without requiring mode-specific configuration or special handling.

**Why this priority**: Users should not experience different noise levels or behaviors depending on animation mode—the feature must be transparent and universal.

**Independent Test**: Cycle through all three animation modes and verify noise reduction is consistent and effective in each mode.

**Acceptance Scenarios**:

1. **Given** any animation mode is selected, **When** clock animates hand movement, **Then** motors are staggered, acceleration is mildly reduced, and noise is reduced uniformly
2. **Given** user switches animation modes, **When** next animation plays, **Then** staggering adapts automatically with no user configuration needed

### Edge Cases

- What happens when user rapidly changes time via web UI or I2C? Staggering must adapt to new target time without causing motor conflicts.
- How does system handle boards that respond at different speeds to I2C commands? Staggering must not assume uniform board latency.
- What if a single motor fails to move or reaches target differently than others? Staggering must not prevent other motors from completing animation.
- How does system behave when only some hands need to move (e.g., minute changes but hour stays same)? Staggering should apply only to motors that are actually moving.

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: System MUST stagger motor startup timing across all 8 boards to reduce peak current draw and audible noise when multiple motors accelerate simultaneously
- **FR-002**: Motor staggering interval MUST be implemented as an easy-to-find source-level constant/tunable value (recommended range 0-50ms, default 10-20ms) for iterative adjustment during testing; runtime/UI configurability is optional and not required
- **FR-003**: System MUST apply a mild global speed reduction in all animation modes, with a default target around 90% of current speed settings
- **FR-004**: System MUST apply a mild global acceleration reduction in all animation modes, with a default target around 80% of current acceleration settings
- **FR-005**: Staggered startup plus speed/acceleration reduction MUST NOT be visibly disruptive; all 24 hands MUST appear to start and complete motion naturally from the user's perspective
- **FR-006**: Noise-reduction behavior MUST work across all three animation modes (Lazy, Fun, Waves) without requiring user-facing mode-specific configuration
- **FR-007**: System MUST ensure all hands complete their target positions within 0 to +200ms of the synchronized (non-staggered) baseline (allowing staggering overhead while maintaining visual synchronization)
- **FR-008**: Implementation MUST NOT add new external dependencies or significantly increase code complexity
- **FR-009**: User-visible animation timing and smoothness MUST remain visually indistinguishable before and after noise-reduction changes
- **FR-010**: Noise-reduction settings MUST be internal/transparent—no new required controls in the web interface

### Key Entities *(include if feature involves data)*

- **Board Index**: Unique identifier (0-7) for each of the 8 PCB boards, used to determine stagger offset
- **Stagger Interval**: Time delay (milliseconds) applied between sequential board startup commands to reduce peak current
- **Motor State**: Per-board tracking of which motors have received move commands and their completion status

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: Perceived noise level during full-clock animation (all 24 hands moving) is reduced by at least 40% compared to baseline prior to staggering and speed/acceleration reduction (measured subjectively and/or via sound level meter)
- **SC-002**: All 24 hands complete their animation within 0 to +200ms of the synchronized (non-staggered) completion time, maintaining visual synchronization
- **SC-003**: Animation quality (smoothness, visual timing) is visually indistinguishable from non-staggered version when observed at normal viewing distance or via standard video (≤60fps)
- **SC-004**: Zero additional user-visible configuration required; staggering plus mild speed/acceleration reduction work transparently across all animation modes
- **SC-005**: Total animation completion time increases by no more than +200ms due to combined staggering and speed/acceleration reduction (staggered completion time ≤ baseline + 200ms)

### Validation Evidence *(mandatory)*

- **VE-001**: Sound level measurement before and after implementation during full-clock minute transition (all 24 motors accelerating). Success = ≥40% reduction in peak dB.
- **VE-002**: High-speed video capture (≥120fps) of animation in all three modes, comparing before/after staggering. Success = visual animation appearance is identical to observers.
- **VE-003**: Hardware test: Run clock for 24 hours with staggering enabled, verify all motors complete moves and no user-visible jitter or missed animations.
- **VE-004**: Cross-board latency test: Measure actual board response times and verify staggering adapts correctly when boards have variable I2C latency.

## Assumptions

- Noise reduction will use a combined approach: staggered startup plus mild global speed and acceleration reduction
- Human perceptual tolerance for animation timing jitter is approximately ±100-150ms; staggering within this window will be undetectable
- The bottleneck for noise is peak current draw during simultaneous motor acceleration, not individual motor torque
- All 8 boards are present, connected, and functionally identical (no board-specific noise mitigation needed)
- Web UI and time-display accuracy do not require sub-100ms precision; motor completion timing within ±100ms is user-acceptable
- Stagger intervals of 10-20ms per board are sufficient to noticeably reduce noise without extending total animation time excessively
- The existing I2C communication protocol remains unchanged; noise reduction is achieved through startup staggering and tuned acceleration values

## Recent Discussion Summary

- User confirmed the goal is to reduce audible motor noise without making animations visibly slower.
- User proposed reducing speed/acceleration slightly (around 80%); discussion converged on acceleration reduction being the safer first lever for noise.
- User observed that in `Fun` and `Waves`, all boards usually move, while when only a subset of boards moves the clock is much quieter.
- Based on that observation, adaptive reduction by active-board count was considered less useful for `Fun`/`Waves` and the preferred baseline became a global strategy valid for all modes.
- Final agreed direction for this feature: apply short startup staggering plus mild speed and acceleration reduction across all modes.
- User clarified FR-002 configurability should be optional: source-level iterative tuning is preferred; no new UI control is required.

## Clarifications

### Session 2026-03-31

- Q: What is the acceptable maximum added animation completion time from staggering and speed/acceleration reduction? → A: +200ms tolerance (recommended option)
