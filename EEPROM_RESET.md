# EEPROM Reset Guide

This document describes how to reset the EEPROM (non-volatile storage) on the **master** board (ESP8266) in the ClockClock24 replica project.

## Overview

The ESP8266 master board stores configuration data (timezone, network settings, etc.) in EEPROM via the Arduino `Preferences` library. This data persists across power cycles and resets. When deploying a new firmware version, you may want to reset EEPROM to clear old configuration and ensure a clean initialization.

## When to Reset EEPROM

- **Upgrading firmware**: Fresh start with new firmware version
- **Troubleshooting**: Corrupt or stale configuration causing issues
- **Changing hardware**: Moving code to a different board
- **Wiping user data**: Preparing device for redeployment or sale

## Method 1: PlatformIO CLI

### Option A: Using `pio device erase`

```bash
# Connect the ESP8266 board via USB, then run:
cd master/
pio device erase
```

This erases the entire flash memory including EEPROM. You'll typically see output like:
```
Erasing flash (this may take a while)...
Erase completed successfully.
```

After erasing, build and upload fresh firmware:
```bash
pio run -t upload
```

### Option B: Using Monitor with a Serial Command (if implemented in firmware)

If your firmware implements a serial command handler for EEPROM reset, you can:

```bash
# Terminal 1: Start the serial monitor
cd master/
pio device monitor --port <PORT> --baud 115200

# Terminal 2: Send reset command (example - verify your firmware's command syntax)
# Common commands: "RESET_EEPROM" or "EEPROM_CLEAR"
```

Check `master/src/main.cpp` for available serial commands.

### Option C: Programmatic Erase via Upload Script

Edit `master/platformio.ini` to add an upload script:

```ini
[env:esp8266]
board = d1_mini
...
extra_scripts = pre:erase_eeprom.py
```

Create `master/erase_eeprom.py`:

```python
Import("env")

def erase_eeprom(source, target, env):
    print("Erasing EEPROM...")
    platform = env.PioPlatform("espressif8266")
    # This is called before upload

env.AddPreAction("upload", erase_eeprom)
```

## Method 2: PlatformIO IDE / VS Code Plugin

### Step 1: Open PlatformIO Home
1. Click the **PlatformIO** icon in the VS Code sidebar (alien icon)
2. Click **PlatformIO Home** or press `Alt+H` (macOS: `Cmd+Shift+P` > "PlatformIO Home")

### Step 2: Access Device Erase
1. In PlatformIO Home, go **Devices** tab
2. Select your connected **ESP8266** board from the list
3. Click the **three-dot menu** next to the device
4. Select **Erase Flash**

Alternatively:

1. Open the **PlatformIO: Quick Access** menu (`Cmd+Shift+P` > "PlatformIO" in VS Code)
2. Type: `PlatformIO: Device Manager`
3. Right-click your ESP8266 device → **Erase Flash**

### Step 3: Upload New Firmware
After erasing, build and upload:
1. Click **Build** (source tree icon in PlatformIO toolbar)
2. Click **Upload** (arrow icon in toolbar)

Or use keyboard shortcuts:
- **Build**: `Cmd+Shift+B` (or `Ctrl+Shift+B` on Linux/Windows)
- **Upload**: `Cmd+Shift+U` (or `Ctrl+Shift+U` on Linux/Windows)

---

## Verifying EEPROM Was Reset

After flashing firmware with erased EEPROM:

1. **Monitor serial output**:
   ```bash
   cd master/
   pio device monitor
   ```

2. **Check firmware logs** for messages like:
   - `Timezone not configured` (app-specific)
   - `First boot detected` (if firmware logs this)
   - Any initialization messages

3. **Connect to web interface** and verify:
   - Timezone is unconfigured
   - Network settings are defaults
   - No stale user data appears

---

## Troubleshooting

### Board Not Detected
```bash
# List all connected devices
pio device list

# If no devices appear, check:
# - USB cable is properly connected
# - USB drivers installed for your OS
# - Board jumper settings (if applicable)
# - Try different USB port
```

### "Unexpected end of data while reading" Error
This often means the USB connection was interrupted. Solutions:
- Reconnect the USB cable
- Power cycle the ESP8266 by unplugging USB for 5 seconds
- Try a different USB cable
- Restart VS Code or terminal

### Erase Appears to Hang
On some systems, erasing can take 30+ seconds. Wait at least 60 seconds before force-stopping. If it fails:
```bash
# Force-quit and retry with explicit baud rate
pio device erase --port /dev/cu.SLAB_USBtoUART --baud 115200
```

---

## Advanced: Selective EEPROM Erase in Code

If you want firmware to reset only specific Preferences namespaces, add this to `master/src/main.cpp`:

```cpp
#include <Preferences.h>

Preferences prefs;

void eraseClockClockPreferences() {
  prefs.begin("clockclock24", false);  // namespace, read-write mode
  prefs.clear();  // Erase all keys in this namespace
  prefs.end();
  Serial.println("ClockClock24 preferences cleared");
}
```

Then call `eraseClockClockPreferences()` during startup or via a serial command handler.

---

## Quick Reference

| Task | Command |
|------|---------|
| Erase entire flash | `cd master && pio device erase` |
| List connected boards | `pio device list` |
| Start serial monitor | `pio device monitor` |
| Combine erase + upload | `cd master && pio device erase && pio run -t upload` |

---

## See Also
- PlatformIO Docs: [Device Management](https://docs.platformio.org/en/latest/core/userguide/device/index.html)
- ESP8266 Arduino Preferences: [Library Docs](https://github.com/espressif/arduino-esp32/tree/master/libraries/Preferences)
- Firmware Initialization: See `master/src/clock_config.cpp`
