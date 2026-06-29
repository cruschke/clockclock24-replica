# Data Model: New Animations

This feature is purely additive firmware logic. There are no new persistent data entities.
Changes to existing data structures:

## `directions` enum (`master/include/clock_state.h` and `slave/include/clock_state.h`)

New value inserted before `ADJUST_HAND`:

| Name | Value | Meaning |
|------|-------|---------|
| `COUNTERCLOCKWISE5` | 12 | Counter-clockwise with 4 extra full rotations |
| `ADJUST_HAND` | 13 | (previously 12) Hand trim adjustment |

> **Wire protocol impact**: The enum value of `ADJUST_HAND` shifts from 12 to 13. Both master and slave must be reflashed together. No EEPROM data is affected.

## `clock_modes` enum (`master/include/clock_config.h`)

New values appended; `OFF` moved out of enum to `#define`:

| Name | Value | Notes |
|------|-------|-------|
| `LAZY` | 0 | Existing |
| `FUN` | 1 | Existing |
| `WAVES` | 2 | Existing |
| `PROPELLER` | 3 | New |
| `ARROW` | 4 | New |
| `RIPPLE` | 5 | New |
| `BUBBLE` | 6 | New |
| `GEAR` | 7 | New |
| `SCATTER` | 8 | New |
| `DIAGONAL` | 9 | New |
| `CASCADE` | 10 | New |
| `CYCLE` | 11 | New |
| `OFF` | 255 | Moved to `#define OFF 255` |

> **EEPROM impact**: Any stored `clock_mode` value of 3 (previously `OFF`) will now be interpreted as `PROPELLER`. On first flash after this change, users who had `OFF` stored may see unexpected behaviour. Acceptable for a hobby project; document in commit notes.

## New `digit.h` pose constants (additive, `master/include/digit.h`)

| Constant | Type | Used by |
|----------|------|---------|
| `digit_JOINT` | `t_digit` | `d_joint` |
| `d_joint` | `t_full_clock` | ARROW phase 1 |
| `digit_cent_a/b` + mirrors | `t_digit` | `d_CENT` |
| `d_CENT` | `t_full_clock` | GEAR phase 1 |
| `digit_bubble` | `t_digit` | `d_bubble` |
| `d_bubble` | `t_full_clock` | BUBBLE phase 1 |
| `digit_diag` | `t_digit` | `d_diagonal` |
| `d_diagonal` | `t_full_clock` | DIAGONAL phase 1 |
| `digit_wave_a/b` + mirrors | `t_digit` | `d_WAVE` |
| `d_WAVE` | `t_full_clock` | RIPPLE phase 1 |

> All values are compile-time constants copied verbatim from the fork.
