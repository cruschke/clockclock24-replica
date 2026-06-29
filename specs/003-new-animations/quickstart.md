# Quickstart: Validate New Animations

## Prerequisites

- PlatformIO installed and working
- Hardware: master ESP32 + Raspberry Pi Pico slave boards connected via I2C
- **Only the master needs to be reflashed** — slave firmware is unchanged

## Build & Flash

```bash
# From repo root — master only:
cd master && pio run -e esp32 -t upload
```

Confirm zero new compiler warnings compared to the pre-feature build.

## Validation Scenarios

### 1. Firmware compiles cleanly (automated)

```bash
cd master && pio run -e esp32
```

Expected: Exit 0, zero new warnings.

### 2. New modes appear in web UI

1. Open `http://<clock-ip>/` in a browser.
2. Verify the Mode section lists: LAZY, FUN, WAVES, PROPELLER, ARROW, RIPPLE, BUBBLE, GEAR, SCATTER, DIAGONAL, CASCADE, CYCLE, OFF (13 buttons total).

### 3. Each new animation plays correctly

For each mode below, set it via the web UI and wait for the next minute change:

| Mode | What to observe |
|------|----------------|
| PROPELLER | All hands simultaneously spin CW/CCW opposite directions into new time |
| ARROW | All hands collapse to 225° joint pose, pause, then a diagonal wave of propeller-spins reveals the time |
| RIPPLE | All hands collapse to wave pose, pause, then time reveals in a Manhattan-distance ripple from centre (left/right sides mirror) |
| BUBBLE | All hands collapse to bubble pose, pause, then checkerboard CW/CCW propeller-spins reveal the time |
| GEAR | All hands collapse to centred fan pose, pause, then Chebyshev-radius rings expand outward (all CW) |
| SCATTER | Hands spin CCW column-by-column left-to-right, hour 2 rotations, minute 4 rotations, both finish together |
| DIAGONAL | All hands collapse to 225° diagonal pose, pause, then left-to-right wave with CLOCKWISE2 reveals time |
| CASCADE | All hands collapse to pointing-down pose, pause, then each column left-to-right reveals with CCW rotation |
| CYCLE | Note the current minute-of-day M. Compute `[FUN,WAVES,ARROW,SCATTER,RIPPLE,BUBBLE,PROPELLER,DIAGONAL,GEAR,CASCADE][(H*60+M) % 10]`. Confirm the observed animation matches. |

### 4. Existing animations unaffected

Set LAZY, then FUN, then WAVES. Confirm each plays identically to pre-feature behaviour:
- LAZY: smooth MIN_DISTANCE transition, staggered send
- FUN: CLOCKWISE2, staggered send
- WAVES: collapses to d_IIII, then wave left-to-right with CLOCKWISE2

### 5. OFF still stops the clock

Select OFF from the web UI. Confirm all hands move to pointing-down and the clock stops updating.

## Pass Criteria

- Build: zero new warnings
- All 9 new animations produce visually distinct output matching the description above
- LAZY, FUN, WAVES, OFF unchanged
- Note: SCATTER minute hand will show incorrect rotation on all slave boards — known accepted limitation (slave firmware not reflashed)
