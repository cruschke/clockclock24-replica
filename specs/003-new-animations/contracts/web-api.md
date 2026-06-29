# Contract: Web UI Mode API

The clock exposes a `/mode` HTTP POST endpoint. This contract documents the updated valid values after the port.

## POST /mode

Sets the current animation mode.

**Form field**: `mode` (integer as string)

| Value | Mode |
|-------|------|
| 0 | LAZY |
| 1 | FUN |
| 2 | WAVES |
| 3 | PROPELLER |
| 4 | ARROW |
| 5 | RIPPLE |
| 6 | BUBBLE |
| 7 | GEAR |
| 8 | SCATTER |
| 9 | DIAGONAL |
| 10 | CASCADE |
| 11 | CYCLE |
| 255 | OFF (clock stopped) |

**Response**: HTTP 200 OK (no body)

## GET /config

Returns current configuration as JSON. The `clock_mode` field uses the same integer values as above.

```json
{
  "clock_mode": 3,
  ...
}
```

## Web UI `genModes()` JS function

The modes array in the minified JS inside `web_page.h` must list all 12 modes plus OFF. The `selectMode()` branch that calls `stopClock()` must match value 255 (not 3).

Expected rendered order in the UI:
`LAZY | FUN | WAVES | PROPELLER | ARROW | RIPPLE | BUBBLE | GEAR | SCATTER | DIAGONAL | CASCADE | CYCLE | OFF`
