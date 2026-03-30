# PlatformIO helper Makefile for clockclock24 master firmware
#
# Usage examples:
#   make all               - complete test loop (clean, check, build, upload)
#   make build
#   make ports
#   make upload
#   make upload PORT=/dev/cu.usbserial-0001
#   make monitor
#   make erase-eeprom
#   make flash-clean
#
# Notes:
# - Ports are auto-discovered from pio device list
# - upload_port / monitor_port are provided via sysenv values used by platformio.ini

ENV ?= d1_mini_lite
BAUD ?= 115200

PIO ?= pio

# Keep the regex broad enough for common macOS/Linux USB serial adapters.
DISCOVER_PORT_CMD = $(PIO) device list 2>/dev/null | \
	awk '/^\/dev\// {print $$1}' | \
	grep -E '(/dev/(cu|tty)\.(usb|usbserial|usbmodem|wchusbserial|SLAB)|/dev/tty(USB|ACM))' | \
	head -n1

.PHONY: help all ports build upload monitor upload-monitor erase-eeprom flash-clean clean check

help:
	@echo "clockclock24 master firmware targets"
	@echo ""
	@echo "  make all              - complete test loop: clean, check, build, erase, upload"
	@echo "  make build            - compile firmware"
	@echo "  make ports            - list serial ports from PlatformIO"
	@echo "  make upload           - upload firmware (auto-discover port)"
	@echo "  make monitor          - open serial monitor (auto-discover port)"
	@echo "  make upload-monitor   - upload then open monitor"
	@echo "  make erase-eeprom     - erase flash/EEPROM using pio run -t erase"
	@echo "  make flash-clean      - erase flash then upload"
	@echo "  make check            - run static checks (cppcheck)"
	@echo "  make clean            - clean PlatformIO build artifacts"
	@echo ""
	@echo "Variables:"
	@echo "  ENV=$(ENV)"
	@echo "  BAUD=$(BAUD)"
	@echo "  PORT=<override serial port>"
	@echo ""
	@echo "Environment wiring used by platformio.ini:"
	@echo "  PIO_UPLOAD_PORT=<auto-detected-or-PORT>"
	@echo "  PIO_MONITOR_PORT=<auto-detected-or-PORT>"

all: clean check build upload
	@echo "===== Complete test cycle finished ====="
	@echo "Run 'make monitor' to view device logs"

ports:
	@$(PIO) device list

build:
	@$(PIO) run --environment $(ENV)

upload:
	@PORT_VAL="$(PORT)"; \
	if [ -z "$$PORT_VAL" ]; then \
	  PORT_VAL="$$($(DISCOVER_PORT_CMD))"; \
	fi; \
	if [ -z "$$PORT_VAL" ]; then \
	  echo "No USB serial port found. Run 'make ports' and set PORT=..."; \
	  exit 1; \
	fi; \
	echo "Using upload port: $$PORT_VAL"; \
	PIO_UPLOAD_PORT="$$PORT_VAL" PIO_MONITOR_PORT="$$PORT_VAL" \
	$(PIO) run --environment $(ENV) --target upload

monitor:
	@PORT_VAL="$(PORT)"; \
	if [ -z "$$PORT_VAL" ]; then \
	  PORT_VAL="$$($(DISCOVER_PORT_CMD))"; \
	fi; \
	if [ -z "$$PORT_VAL" ]; then \
	  echo "No USB serial port found. Run 'make ports' and set PORT=..."; \
	  exit 1; \
	fi; \
	echo "Using monitor port: $$PORT_VAL (baud $(BAUD))"; \
	PIO_MONITOR_PORT="$$PORT_VAL" \
	$(PIO) device monitor --environment $(ENV) --baud $(BAUD)

upload-monitor: upload monitor

erase-eeprom:
	@PORT_VAL="$(PORT)"; \
	if [ -z "$$PORT_VAL" ]; then \
	  PORT_VAL="$$($(DISCOVER_PORT_CMD))"; \
	fi; \
	if [ -z "$$PORT_VAL" ]; then \
	  echo "No USB serial port found. Run 'make ports' and set PORT=..."; \
	  exit 1; \
	fi; \
	echo "Erasing flash (EEPROM included) on: $$PORT_VAL"; \
	PIO_UPLOAD_PORT="$$PORT_VAL" PIO_MONITOR_PORT="$$PORT_VAL" \
	$(PIO) run --environment $(ENV) --target erase

flash-clean: erase-eeprom upload

check:
	@$(PIO) check --environment $(ENV)

clean:
	@$(PIO) run --environment $(ENV) --target clean
