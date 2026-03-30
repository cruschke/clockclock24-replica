# US2 Connectivity and Authority Evidence

Status: Runtime precedence logic implemented; hardware validation pending.

Checks:
- [x] Web handler rejects browser/manual time override when external network mode is active
- [x] Browser/manual fallback used when NTP is not set (`timeStatus()!=timeSet`)
- [x] Authority persisted in config (`time_authority`)
- [ ] Offline/reconnect hardware scenario executed and recorded

Notes:
- `/config` now exposes `time_authority`.
