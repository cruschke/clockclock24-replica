# Implementation Plan: New Animations from Fork

**Branch**: `003-new-animations` | **Date**: 2026-06-29 | **Spec**: [spec.md](spec.md)  
**Input**: Feature specification from `specs/003-new-animations/spec.md`

## Summary

Port 9 new animation modes (PROPELLER, ARROW, RIPPLE, BUBBLE, GEAR, SCATTER, DIAGONAL, CASCADE, CYCLE) from `aslafsky/clockclock24-replica-all-animtions` into this repo. The port is purely additive to animation logic; MQTT, OTA, captive portal, status LED, and board-target changes from the fork are explicitly excluded. Key supporting changes required: move `OFF` out of `clock_modes` enum to `#define OFF 255`, implement two missing `clock_manager` functions, add new `digit.h` pose constants, extend the web UI mode dropdown, and add `dispatch_animation()` to `main.cpp`.

**Slave firmware (Raspberry Pi Pico boards): no reflash required.** `COUNTERCLOCKWISE5` is appended after `ADJUST_HAND` in the enum (not before it), so `ADJUST_HAND` stays at value 12 and all existing slave firmware remains compatible. SCATTER's minute hand will behave slightly incorrectly on all slave boards, but all other animations are unaffected.

## Technical Context

**Language/Version**: C++11 / Arduino framework (ESP32, PlatformIO)  
**Primary Dependencies**: Arduino, Wire (I2C), TimeLib — no new dependencies  
**Storage**: EEPROM via Preferences — one existing `clock_mode` key extended (see data-model.md)  
**Testing**: Manual hardware validation + PlatformIO build check (no automated test framework)  
**Target Platform**: ESP32 (master), Raspberry Pi Pico slave boards via I2C — slave firmware NOT reflashed  
**Project Type**: Embedded firmware + web UI (single-page HTML in `web_page.h`)  
**Performance Goals**: All animations must complete within a 60-second minute window; no blocking loop longer than a few seconds  
**Constraints**: No new libraries; slave firmware must not be modified  
**Scale/Scope**: ~400 lines of new firmware code across 5 files; ~50 chars of JS change in `web_page.h`

## Constitution Check

- **KISS-First Engineering**: ✅ All 9 animations are ported as direct function bodies; no new abstraction layer, no new class, no new library. The only new helper is `dispatch_animation()`, which replaces a duplicated switch-case — a net simplification. `get_speed_multiplier()` is NOT ported; bare numeric constants are used instead, matching the fork's default behaviour.
- **Code Quality Baseline**: ✅ PlatformIO build (`pio run`) is the quality gate. Zero new compiler warnings required. No linting tool is configured for this project; consistent naming follows existing conventions.
- **Test Evidence for Every Change**: ✅ Manual hardware validation defined in `quickstart.md` for each animation and for existing animations (regression). No slave compatibility check needed — slave firmware is unchanged.
- **Consistent User Experience**: ✅ Mode names in web UI match firmware enum names exactly. OFF is preserved as a selectable mode. Existing LAZY/FUN/WAVES behaviour is unchanged. Web preview for new modes degrades gracefully (static display) — documented assumption.
- **Performance Within Hardware Limits**: ✅ Each animation's timing is copied verbatim from the fork (which runs on the same hardware). Longest blocking delay is ~9 seconds (WAVES `_delay(9000)`) — unchanged from existing WAVES. No new delay patterns introduced.

**Constitution Check result: PASS — no violations, no complexity tracking entries needed.**

## Project Structure

### Documentation (this feature)

```text
specs/003-new-animations/
├── plan.md              # This file
├── spec.md              # Feature specification
├── research.md          # Phase 0: resolved unknowns
├── data-model.md        # Phase 1: enum/struct changes
├── quickstart.md        # Phase 1: validation guide
├── contracts/
│   └── web-api.md       # Phase 1: /mode API contract
└── tasks.md             # Phase 2 output (/speckit-tasks — not yet created)
```

### Source Code (files changed by this feature)

```text
master/
├── include/
│   ├── clock_state.h       # Add COUNTERCLOCKWISE5 after ADJUST_HAND (slave stays compatible)
│   ├── clock_config.h      # Extend clock_modes enum; move OFF to #define
│   ├── clock_manager.h     # Declare set_half_digit_full(), set_single_clock_full() (already declared; confirm)
│   ├── digit.h             # Add d_joint, d_CENT, d_bubble, d_diagonal, d_WAVE pose constants
│   └── web_page.h          # Update genModes() JS and selectMode() OFF check
└── src/
    ├── clock_manger.cpp    # Implement set_half_digit_full(), set_single_clock_full()
    ├── clock_config.cpp    # No change needed (set_clock_mode already stores int; no validation)
    └── main.cpp            # Add dispatch_animation(); add 9 new animation functions; update set_time() switch

slave/
└── include/
    └── clock_state.h       # NOT modified — no reflash required
```

**Structure Decision**: Single project layout with in-place file edits. No new files created outside the spec directory.

## Implementation Steps

### Step 1 — `master/include/clock_state.h`: Add `COUNTERCLOCKWISE5`

Append `COUNTERCLOCKWISE5` **after** `ADJUST_HAND` so the slave firmware (not reflashed) remains fully compatible — `ADJUST_HAND` stays at value 12:

```cpp
enum directions
{
  CLOCKWISE,          // 0
  CLOCKWISE2,         // 1
  CLOCKWISE3,         // 2
  COUNTERCLOCKWISE,   // 3
  COUNTERCLOCKWISE2,  // 4
  COUNTERCLOCKWISE3,  // 5
  MIN_DISTANCE,       // 6
  MIN_DISTANCE2,      // 7
  MIN_DISTANCE3,      // 8
  MAX_DISTANCE,       // 9
  MAX_DISTANCE2,      // 10
  MAX_DISTANCE3,      // 11
  ADJUST_HAND,        // 12  — unchanged, slaves stay compatible
  COUNTERCLOCKWISE5   // 13  — 4 extra full CCW rotations (master only)
};
```

> Slave `clock_state.h` is **not changed**. No slave reflash required.
> Trade-off: SCATTER sends value 13 to slaves; all unflashed slaves receive an unknown mode and will exhibit incorrect minute-hand rotation on those boards. All other animations are unaffected.

### Step 2 — `master/include/clock_config.h`: Extend `clock_modes`, move `OFF`

Replace the enum body and add `#define OFF 255`:

```cpp
enum clock_modes
{
  LAZY,        // 0
  FUN,         // 1
  WAVES,       // 2
  PROPELLER,   // 3
  ARROW,       // 4
  RIPPLE,      // 5
  BUBBLE,      // 6
  GEAR,        // 7
  SCATTER,     // 8
  DIAGONAL,    // 9
  CASCADE,     // 10
  CYCLE        // 11
};

#define OFF 255
```

Remove the existing `OFF` enum member. Verify no other code uses `case OFF:` in a switch on `clock_modes` (currently only `main.cpp` uses `get_clock_mode() != OFF`; this comparison still works with `#define OFF 255`).

### Step 3 — `master/include/digit.h`: Add pose constants

Append the following blocks from the fork (values copied verbatim):
- `digit_wave_a`, `digit_wave_b`, `digit_wave_a_mirror`, `digit_wave_b_mirror` → `d_WAVE`
- `digit_JOINT` → `d_joint`
- `digit_cent_a`, `digit_cent_b`, `digit_cent_a_mirror`, `digit_cent_b_mirror` → `d_CENT`
- `digit_bubble` → `d_bubble`
- `digit_diag` → `d_diagonal`

### Step 4 — `master/src/clock_manger.cpp`: Implement missing functions

Add `set_half_digit_full()` and `set_single_clock_full()` from the fork:

**`set_half_digit_full(int index, t_half_digit half)`**: Sends a pre-built `t_half_digit` directly (no `get_full_half_digit()` conversion), increments `_counter`, and updates `_last_state`. Used when per-hand direction is set by the caller.

**`set_single_clock_full(int hd, int p, t_half_digitl lite, int mode_h, int mode_m)`**: Updates a single clock face within a half-digit. Reads `_last_state[hd]`, sets angle/speed/accel/mode for position `p` only, increments `change_counter[p]`, and sends the half-digit.

### Step 5 — `master/src/main.cpp`: Add `dispatch_animation()` and 9 new animation functions

1. Add forward declarations for all 9 new `set_*()` functions.
2. Add `dispatch_animation(int mode)` that switch-cases on all 12 modes (LAZY–CYCLE).
3. Replace the existing switch-case in `set_time()` with a single call to `dispatch_animation(get_clock_mode())`.
4. Add the 9 animation function bodies verbatim from the fork, with `get_speed_multiplier()` calls removed (bare numeric constants used instead).

**Existing `set_lazy()`, `set_fun()`, `set_waves()`**: Keep as-is. They use `set_clock_time_staggered()` (our noise-reduction variant); the fork uses `set_clock_time()`. Do NOT change these — preserving existing staggered behaviour is a regression requirement.

**Timing instrumentation** in `set_time()` (T013 from prior feature): Update the `mode_name` switch to cover all 12 modes. The `anim_start_ms` / `anim_end_ms` instrumentation block is preserved.

### Step 6 — `master/include/web_page.h`: Update web UI

In the minified JS, update `genModes()` to:
```js
for(e of["LAZY","FUN","WAVES","PROPELLER","ARROW","RIPPLE","BUBBLE","GEAR","SCATTER","DIAGONAL","CASCADE","CYCLE","OFF"])
```

Update the index for OFF in `selectMode()`:
```js
// Before: (3===e?stopClock:startClock)()
// After:  (255===e?stopClock:startClock)()
```

And update the mode index when building button `id="mode-N"`: OFF button gets `id="mode-255"`.

> Note: The web preview JS (`setTime()` switch) does NOT need updating — unsupported modes simply do nothing, which is the acceptable fallback.

## Complexity Tracking

No violations. No new abstractions or dependencies introduced.
