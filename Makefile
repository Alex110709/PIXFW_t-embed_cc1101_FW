# LilyGO T-Embed CC1101 Firmware Makefile
# ESP-IDF based build system

PROJECT_NAME := t-embed-cc1101-fw

# ESP-IDF configuration
IDF_TARGET := esp32s3
BOARD := lilygo-t-embed

# Include ESP-IDF build system
include $(IDF_PATH)/tools/cmake/project.cmake

# Default target
.PHONY: all build flash monitor clean fullclean configure menuconfig

all: build

# Build the firmware
build:
	@echo "Building T-Embed CC1101 firmware..."
	idf.py build

# Flash firmware to device
flash: build
	@echo "Flashing firmware to device..."
	idf.py -p $(PORT) flash

# Monitor serial output
monitor:
	@echo "Starting serial monitor..."
	idf.py -p $(PORT) monitor

# Flash and monitor in one command
flash-monitor: flash monitor

# Clean build files
clean:
	@echo "Cleaning build files..."
	idf.py clean

# Full clean (including downloaded components)
fullclean:
	@echo "Full clean..."
	idf.py fullclean

# Configure project
configure:
	@echo "Configuring project..."
	idf.py set-target $(IDF_TARGET)

# Menu configuration
menuconfig:
	@echo "Opening menuconfig..."
	idf.py menuconfig

# Size analysis
size:
	@echo "Analyzing firmware size..."
	idf.py size

# Erase flash completely
erase:
	@echo "Erasing flash..."
	idf.py -p $(PORT) erase-flash

# Create firmware release package
release: build
	@echo "Creating release package..."
	@mkdir -p release
	@cp build/$(PROJECT_NAME).bin release/
	@cp build/$(PROJECT_NAME).elf release/
	@cp build/bootloader/bootloader.bin release/
	@cp build/partition_table/partition-table.bin release/
	@cp partitions.csv release/
	@echo "Release package created in release/ directory"

# Format code (if tools available)
format:
	@echo "Formatting code..."
	@find . -name "*.c" -o -name "*.h" | grep -v build | xargs clang-format -i || echo "clang-format not available"

# Run static analysis (if tools available)  
analyze:
	@echo "Running static analysis..."
	@cppcheck --enable=all --suppress=missingIncludeSystem . 2>analysis.txt || echo "cppcheck not available"
	@echo "Analysis results in analysis.txt"

# Help
help:
	@echo "LilyGO T-Embed CC1101 Firmware Build System"
	@echo ""
	@echo "Available targets:"
	@echo "  build         - Build the firmware"
	@echo "  flash         - Flash firmware to device (requires PORT=<device>)"
	@echo "  monitor       - Start serial monitor (requires PORT=<device>)"
	@echo "  flash-monitor - Flash and start monitor"
	@echo "  clean         - Clean build files"
	@echo "  fullclean     - Full clean including components"
	@echo "  configure     - Configure project for ESP32-S3"
	@echo "  menuconfig    - Open configuration menu"
	@echo "  size          - Analyze firmware size"
	@echo "  erase         - Erase device flash completely"
	@echo "  release       - Create release package"
	@echo "  format        - Format source code"
	@echo "  analyze       - Run static analysis"
	@echo "  help          - Show this help"
	@echo ""
	@echo "Examples:"
	@echo "  make flash PORT=/dev/ttyUSB0"
	@echo "  make flash-monitor PORT=/dev/cu.usbserial-*"
	@echo ""
	@echo "Environment variables:"
	@echo "  PORT          - Serial port for flashing/monitoring"
	@echo "  IDF_PATH      - Path to ESP-IDF framework"

# Auto-detect port on different systems
detect-port:
	@echo "Detecting available serial ports..."
	@if [ "$(shell uname)" = "Darwin" ]; then \
		ls /dev/cu.usbserial-* 2>/dev/null || echo "No USB serial ports found on macOS"; \
	elif [ "$(shell uname)" = "Linux" ]; then \
		ls /dev/ttyUSB* /dev/ttyACM* 2>/dev/null || echo "No USB serial ports found on Linux"; \
	else \
		echo "Port detection not implemented for this OS"; \
	fi