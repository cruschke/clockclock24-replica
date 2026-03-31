# Quickstart: Testing Reduce Motor Noise

**Date**: March 31, 2026  
**Feature**: `002-reduce-motor-noise`  
**Branch**: `002-reduce-motor-noise`

## Build & Upload

```bash
cd master/
make all              # Build firmware with noise-reduction code
make upload           # Flash to Wemos D1 mini lite
make monitor          # Monitor serial output
```

## Tuning Parameters

Edit these source-level constants in `master/src/clock_manger.cpp`:

```cpp
// Line ~10 (after includes)
const int STAGGER_INTERVAL_MS = 15;      // Adjust: 0-50ms
const float SPEED_FACTOR = 0.90f;        // Adjust: 0.80-1.00
const float ACCELERATION_FACTOR = 0.80f; // Adjust: 0.70-1.00
```

After changing, rebuild and reflash:
```bash
make all && make upload
```

## Testing Procedure

### VE-001: Sound Level Measurement

**Before implementation**: Note baseline dB during minute transition (all 24 hands move).

**After implementation**: Measure dB at same condition. Target: ≥40% reduction in peak dB.

**Tools**: Phone sound meter app, decibel meter, or subjective observation.

### VE-002: High-Speed Video Capture

**Tools**: Smartphone at 120+ fps or GoPro.

**Steps**:
1. Record minute transition in each animation mode (Lazy, Fun, Waves)  
2. Compare video before/after staggering + derating
3. Look for visual smoothness, hand startup synchronization, completion timing
4. Expected: No observable difference in animation quality

**Pass**: Animation appears identical before and after

### VE-003: 24-Hour Stress Test

**Steps**:
1. Run clock normally for 24 hours (time display updates every minute, animation cycles)  
2. Monitor serial output for errors or I2C latency issues
3. Visually inspect hands for missed targets or jitter

**Pass**: No errors, all animations complete, no hand position errors

### VE-004: I2C Latency Test

**Steps**:
1. Add debug logging to measure I2C response times per board
2. Run clock during high animation activity (Waves mode)
3. Record board response times; compute variance
4. Check if staggering correctly adapts to variable latencies

**Debug output** (add to `main.cpp`):
```cpp
// During animation, log per-board response time
Serial.printf("Board %d latency: %ld ms\n", board_index, response_time_ms);
```

**Pass**: Staggering timing adapts smoothly; all boards complete despite latency variance

## Iterative Tuning

1. Start with defaults (15ms stagger, 90% speed, 80% accel)
2. If still noisy: Increase stagger to 20-25ms or decrease accel to 75%
3. If animation feels slow: Increase speed to 92-95%
4. If I2C conflicts detected: Reduce stagger to 8-12ms
5. Re-validate after each change using VE-001 through VE-004

## Commit & Merge

Once all validation steps PASS:

```bash
git add master/src/clock_manger.cpp master/src/main.cpp
git commit -m "feat(master): add startup staggering and motor derating for noise reduction"
git checkout master
git merge --no-ff 002-reduce-motor-noise -m "Merge feature: reduce motor noise"
```
