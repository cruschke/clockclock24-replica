# Contract: Silent Hour API

## GET /config (updated)

Returns current configuration as JSON. Gains one new field:

```json
{
  "clock_mode": 5,
  "silent_hour": 18,
  ...
}
```

| Field | Type | Values |
|-------|------|--------|
| `silent_hour` | integer | -1 (disabled) or 0–23 |

All existing fields are unchanged.

## POST /silent (new endpoint)

Sets the silent mode start hour.

**Form field**: `silent_hour` (integer as string)

| Value | Meaning |
|-------|---------|
| -1 | Disabled — no silent mode |
| 0–23 | Hour from which clock plays Lazy |

**Response**: HTTP 200 OK (no body)

**Side effect**: Forces immediate mode re-evaluation by resetting the minute tracker, so the change takes effect within the current loop iteration without waiting for the next minute.

## Web UI "Silent from" dropdown

New `<select>` element in the web page, rendered near the Mode section, labelled "Silent from":

| Option value | Label |
|-------------|-------|
| -1 | Disabled |
| 0 | 00:00 |
| 1 | 01:00 |
| … | … |
| 23 | 23:00 |

On change: POSTs to `/silent` with the selected value. On config load: reads `silent_hour` from `/config` and pre-selects the matching option.
