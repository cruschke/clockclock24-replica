# Contract: Time and Configuration API (DST Support)

## Scope
This contract defines backward-compatible changes to existing master firmware web endpoints used by the local web UI.

## Endpoint: `GET /config`

### Response (application/json)
Existing keys remain.

Required keys after change:
- `clock_mode` (number)
- `wireless_mode` (number)
- `ssid` (string)
- `password` (string)
- `sleep_time` (number[7][24])
- `timezone_id` (string): IANA timezone identifier (e.g., `Europe/Berlin`), or empty string if unconfigured
- `timezone_configured` (boolean): True if timezone_id has been explicitly set
- `time_authority` (string enum): `network_ntp` | `browser_manual_fallback`

### Behavioral rules
- If `timezone_configured=false`, UI should prompt user to select a timezone profile.
- Web UI SHOULD prefer displaying `timezone_id` over any numeric offset representation.

## Endpoint: `POST /time`

### Request (form-data)
Existing keys remain accepted:
- `h`, `m`, `s`, `D`, `M`, `Y`

New keys:
- `timezone_id` (optional string, canonical profile identifier)

### Behavioral rules
- If network authority is available, timestamp values from this endpoint must not override authoritative network time.
- If `timezone_id` is present and valid, firmware sets `timezone_configured=true` and activates the timezone profile immediately.
- Legacy `timezone` field (numeric offset) is no longer used when `timezone_id` is configured.

## Endpoint: `POST /connection`
No payload shape changes required by DST feature.

## Compatibility
- Existing clients posting only legacy fields remain accepted.
- New UI should prefer `timezone_id` and treat numeric offset as deprecated backward-compatibility field only.
