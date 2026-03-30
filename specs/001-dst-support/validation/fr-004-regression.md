# FR-004 Regression: Non-time Settings Preservation

Goal:
Verify non-time settings (clock mode, sleep schedule, wifi credentials) are preserved after timezone configuration.

Code-level checks completed:
- [x] `set_timezone_id` and `set_timezone_configured` only touch timezone keys
- [x] Existing `clock_mode`, `sleep_time`, `ssid`, `password` keys unchanged

Hardware validation pending:
- [ ] Pre/post config comparison on device
