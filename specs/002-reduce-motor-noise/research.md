# Research: Reduce Motor Noise During Synchronized Animation

**Date**: March 31, 2026  
**Feature**: `002-reduce-motor-noise`  
**Status**: No unknowns remain; specification is complete and unambiguous.

## Summary

The feature specification (#spec.md) fully clarified all technical decisions during the specify phase. No additional research was required.

## Resolved Items

✅ **Noise reduction approach**: Confirmed combined strategy of startup staggering + speed/acceleration derating (vs. alternatives like speed-only or acceleration-only).

✅ **Applicability across modes**: User observed that all boards typically move in Fun/Waves modes, confirming global derating is preferred over adaptive per-mode tuning.

✅ **Configurability**: Confirmed source-level tuning is sufficient; no UI control required.

✅ **Timing tolerance**: Clarified +200ms maximum added completion time is acceptable.

✅ **Default values**: 
- Speed reduction: 90% (preserves visual punch)
- Acceleration reduction: 80% (effective noise reduction with minimal visual slowdown)
- Stagger interval: 10-20ms default (spreads current spikes without adding noticeable lag)

## No Further Research Needed

All technical decisions have explicit user validation and are ratified in the spec.
