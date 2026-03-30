# Phase 1 Data Model: General Daylight Saving Time Support

## Entity: TimeConfiguration
- Purpose: Persist user-configurable timezone and local-time behavior.
- Fields:
  - `timezone_id` (string, required): Canonical timezone identifier (for example `Europe/Berlin`).
  - `timezone_configured` (bool): True when a valid timezone has been explicitly set after cleaflash initialization; false until user selects a profile.
- Validation rules:
  - `timezone_id` MUST be a valid IANA timezone identifier (e.g., `Europe/Berlin`, `America/New_York`, `UTC`).
  - `timezone_configured=true` iff `timezone_id` is valid and set by user.
  - No fallback mode or legacy offset: clean-flash initialization ensures all persistent config references the timezone-identifier model.

## Entity: TimeSourceState
- Purpose: Track authority and availability of time input sources.
- Fields:
  - `authority` (enum): `network_ntp` | `browser_manual_fallback`.
  - `network_time_available` (bool): NTP sync currently available.
  - `last_sync_epoch_utc` (uint32): Last authoritative UTC epoch obtained from sync.
  - `last_fallback_set_epoch_utc` (uint32, nullable): Last fallback set-time epoch.
- Validation rules:
  - `authority=network_ntp` requires `network_time_available=true`.
  - Browser/manual update must not replace authority while network time is available.
  - Authority always defaults to `browser_manual_fallback` during clean-flash initialization until first NTP sync.

## Entity: LocalTimeProfile
- Purpose: Runtime-resolved local-time behavior from configured timezone.
- Fields:
  - `timezone_id` (string): Identifier associated with profile.
  - `has_dst` (bool): Whether configured timezone defines DST transitions.
  - `effective_utc_offset_seconds` (int): Current resolved offset for the current epoch.
  - `next_transition_epoch_utc` (uint32, nullable): Next DST transition point when applicable.
- Validation rules:
  - If `has_dst=false`, seasonal transitions must not alter offset.
  - For DST profiles, offset change direction/time must match profile rule.

## State Transitions
1. `browser_manual_fallback` -> `network_ntp`
- Trigger: network sync restored.
- Effects: authoritative epoch replaced by network sync; display corrected immediately.

3. `network_ntp` with DST profile at transition boundary
- Trigger: runtime crosses transition epoch.
- Effects: `effective_utc_offset_seconds` updates within 1 minute; display follows local civil time.
