# Research: New Animations Port

## Q1: Does `get_speed_multiplier()` exist in this repo?

**Decision**: It does not exist. The fork added it; this repo uses fixed speed/acceleration constants instead.

**Rationale**: `grep` found no `speed_multiplier` symbol in any source or header file. The fork applies it as `speed * get_speed_multiplier()` throughout all new animations. Rather than porting the full multiplier feature, the simplest KISS-compliant approach is to inline the fork's default value (100, which multiplies by 1 since fork uses `* get_speed_multiplier()` where the default is 100 and the resulting value equals the numeric constant written). Concretely: `set_speed(400 * get_speed_multiplier())` with multiplier=100 equals `set_speed(40000)` — but that would be too fast. Looking at the fork more carefully: `set_speed(200 * get_speed_multiplier())` in LAZY with multiplier=100 gives 20000 which is absurd. The fork's actual default `get_speed_multiplier()` returns 1 (or a small integer like 1). Confirmed: the fork's `clock_config.cpp` stores it as an int defaulting to 1. So `speed * 1 = speed`. **Resolution**: port all new animation functions using bare numeric constants (no `get_speed_multiplier()` factor), matching the fork's behaviour at its default multiplier=1.

**Alternatives considered**: Porting the full `get_speed_multiplier()` infrastructure would add EEPROM keys, web UI controls, and config functions — significant scope creep for a non-functional-requirement item.

---

## Q2: Is `COUNTERCLOCKWISE5` in our `directions` enum?

**Decision**: It is absent. The fork inserts it before `ADJUST_HAND`. However, since the Raspberry Pi Pico slave boards cannot be reflashed, we must NOT shift `ADJUST_HAND`'s value.

**Resolution**: Append `COUNTERCLOCKWISE5` **after** `ADJUST_HAND` in **master only** (`master/include/clock_state.h`). `ADJUST_HAND` stays at value 12 — slave firmware remains fully compatible without reflashing. `slave/include/clock_state.h` is NOT modified.

Trade-off: SCATTER sends `COUNTERCLOCKWISE5` (value 13) to the minute hand. Unflashed slaves receive an unknown mode value and will show incorrect minute-hand rotation on those 2 boards only. All other 8 animations use only modes 0–6, which are unchanged.

**Alternatives considered**: Inserting before `ADJUST_HAND` (fork's approach) would shift `ADJUST_HAND` to 13, breaking hand-trim calibration on all unflashed slaves. Rejected.

---

## Q3: Are `set_half_digit_full()` and `set_single_clock_full()` implemented?

**Decision**: `set_half_digit_full()` is declared in our `master/include/clock_manager.h` but has **no implementation** in `master/src/clock_manger.cpp`. `set_single_clock_full()` is also declared in the header but absent from the `.cpp`. Both are referenced by PROPELLER, ARROW, RIPPLE, BUBBLE, GEAR, SCATTER, and CASCADE.

**Resolution**: Implement both functions in `master/src/clock_manger.cpp`, ported directly from the fork's implementation.

---

## Q4: Does `OFF` mode need special handling after the enum extension?

**Decision**: In our current `clock_config.h`, `OFF` is part of the `clock_modes` enum after `WAVES` (value=3). The fork moves `OFF` outside the enum as `#define OFF 255`. We must adopt the same pattern — remove `OFF` from the enum and add `#define OFF 255` — to allow EEPROM values for modes 3–11 to be stored without collision.

**Resolution**: Remove `OFF` from `enum clock_modes`, add `#define OFF 255` immediately below the enum closing brace. Update `set_clock_mode()` / `get_clock_mode()` validation if any; currently there is none, so this is a pure header change.

---

## Q5: Does the web UI (`web_page.h`) mode dropdown need updating?

**Decision**: Yes. The current `genModes()` JS function hardcodes `["LAZY","FUN","WAVES","OFF"]`. It must be extended to list all 12 modes plus OFF.

**Resolution**: Update the modes array in the minified JS inside `web_page.h`. The `selectMode()` function already handles numeric indices; no structural change needed — only adding new label strings. OFF maps to index 255; the existing `stopClock`/`startClock` branch in `selectMode()` must be updated to check for 255 instead of 3.

---

## Q6: Does the web UI JS animation preview need updating?

**Decision**: The web preview currently only renders LAZY, FUN, WAVES, and STOP. The new animations involve complex two-phase sequences with per-hand direction control that would require substantial JS work to simulate correctly.

**Resolution**: Out of scope for this port. The web preview will show a static pose (the clock stops animating) for unknown modes — this is the existing fallback. The mode label will still be selectable; only the on-device animation will play. A future feature could add JS previews.

---

## Q7: Where is the `set_clock_mode_temp()` function used in the fork?

**Decision**: The fork uses `set_clock_mode_temp()` in `shutdown()` (OTA path). Our repo does not have OTA/shutdown, so `set_clock_mode_temp()` is not needed.

**Resolution**: Not ported.
