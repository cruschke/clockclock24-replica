# Quickstart: Validate Day Mode Schedule

## Prerequisites

- Master firmware flashed with this feature
- Clock connected to Wi-Fi, reachable at `http://<clock-ip>/`

## Build & Flash

```bash
cd master
curl -s -X POST http://<clock-ip>/mode -d "mode=255" && sleep 15
make build && make upload
```

## Validation Scenarios

### 1. Firmware compiles cleanly

```bash
cd master && make build
```

Expected: Exit 0, zero new warnings.

### 2. Silent hour appears in /config

```bash
curl -s http://<clock-ip>/config | python3 -m json.tool | grep silent_hour
```

Expected: `"silent_hour": -1` (disabled by default on clean flash).

### 3. Set silent hour and verify persistence

```bash
curl -s -X POST http://<clock-ip>/silent -d "silent_hour=18"
curl -s http://<clock-ip>/config | python3 -m json.tool | grep silent_hour
```

Expected: `"silent_hour": 18`

Power-cycle the clock. Re-run the GET. Expected: still `"silent_hour": 18`.

### 4. Silent mode activates at configured hour

1. Note the current hour H.
2. Set silent hour to H: `curl -s -X POST http://<clock-ip>/silent -d "silent_hour=<H>"`
3. Set an active animation (e.g. CASCADE): `curl -s -X POST http://<clock-ip>/mode -d "mode=10"`
4. Within one minute, observe the clock switches to Lazy.

Expected: Clock plays Lazy, not CASCADE.

### 5. Immediate apply on save

1. Set clock to CASCADE: `curl -s -X POST http://<clock-ip>/mode -d "mode=10"`
2. Wait for a minute tick (confirm CASCADE plays).
3. Set silent hour to current hour: `curl -s -X POST http://<clock-ip>/silent -d "silent_hour=<current-hour>"`
4. Observe within seconds — clock switches to Lazy without waiting for the next minute.

### 6. Disabled restores original behaviour

```bash
curl -s -X POST http://<clock-ip>/silent -d "silent_hour=-1"
```

Expected: Clock returns to playing the selected animation at all non-sleep hours.

### 7. Sleep takes precedence over silent

1. Mark the next hour as sleep for today in the web UI.
2. Set silent hour to the same hour.
3. At that hour, confirm clock is OFF (not playing Lazy).

### 8. Web UI dropdown

1. Open `http://<clock-ip>/` in a browser.
2. Locate the "Silent from" dropdown.
3. Verify options include "Disabled" and hours 00:00–23:00.
4. Select a value, verify it persists on page reload.

## Pass Criteria

- Build: zero new warnings
- `silent_hour` field present in `/config` response
- Silent mode activates correctly at configured hour
- Immediate apply works within one loop iteration
- Disabled restores pre-feature behaviour
- Sleep mask takes precedence over silent window
- Web UI dropdown present and functional
