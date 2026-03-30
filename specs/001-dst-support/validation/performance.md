# Performance Validation

Targets:
- Sync/reconnect correction: <= 60 seconds
- DST boundary transition application: <= 60 seconds

Implementation status:
- [x] Transition-window sync trigger added in main loop
- [x] NTP sync provider updates on reconnect path
- [ ] Measured reconnect latency on hardware
- [ ] Measured DST boundary latency on hardware
