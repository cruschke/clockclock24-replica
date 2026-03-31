# Data Model: Reduce Motor Noise During Synchronized Animation

**Date**: March 31, 2026  
**Feature**: `002-reduce-motor-noise`  
**Status**: No new data entities required.

## Summary

This feature modifies motor control parameters (speed, acceleration) and adds startup timing coordination, but introduces no new persistent data structures or entities.

## Existing Entities (Unchanged)

- **Motor/Board State**: Already tracked in `clock_manger.cpp` via `_speed`, `_acceleration`, `_direction`
- **Animation Mode**: Existing enum (LAZY, FUN, WAVES) remains unchanged
- **Configuration**: Persisted via existing Preferences (no new config keys needed)

## New Parameters (Source-Level Only)

The following are **source-code constants** (not entities):

| Parameter | Type | Scope | Default | Purpose |
|-----------|------|-------|---------|---------|
| `STAGGER_INTERVAL_MS` | int | const | 15 | Delay (ms) between board startup commands |
| `SPEED_FACTOR` | float | const | 0.90 | Speed multiplier (90% of baseline) |
| `ACCELERATION_FACTOR` | float | const | 0.80 | Acceleration multiplier (80% of baseline) |

These constants will be defined in `clock_manger.cpp` and are trivial to tune for testing.

## No Persistent Storage Changes

- No new EEPROM keys required
- No Preferences schema changes
- No web API changes
- No I2C protocol changes

## Relationships

Staggering logic will be implemented in `main.cpp` during animation calls:
- `set_lazy()`, `set_fun()`, `set_waves()` will coordinate board startup via `send_half_digit()` with timing delays
- Speed/acceleration multipliers apply globally in `get_full_half_digit()` before motor parameters are sent to boards
