# Data Model: Day Mode Schedule

## New config value — `silent_hour`

Stored in non-volatile memory (Preferences) under key `"silent_hour"`.

| Field | Type | Range | Default | Meaning |
|-------|------|-------|---------|---------|
| `silent_hour` | int | -1 … 23 | -1 | Hour from which Lazy plays; -1 = disabled |

**Validation rules**:
- Valid stored values: -1 (disabled) or 0–23
- Values outside this range are treated as -1 (disabled) at read time

**State transitions**:

```
[any hour, silent_hour = -1]  → selected animation plays (disabled)
[hour < silent_hour]          → selected animation plays
[hour >= silent_hour, not sleep] → Lazy plays
[sleep hour]                  → clock off (sleep mask takes precedence)
```

## Existing config values (unchanged)

| Field | Type | Notes |
|-------|------|-------|
| `clock_mode` | int | Selected animation (0–11, 255=OFF) |
| `sleep_time` | bool[7][24] | Per-day/hour sleep mask — takes precedence over silent mode |
