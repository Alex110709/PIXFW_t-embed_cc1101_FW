#!/bin/bash

# Build script for T-Embed CC1101 Firmware
# This script provides convenient build commands and validation

set -e  # Exit on any error

# Project configuration
PROJECT_NAME="t-embed-cc1101-fw"
TARGET_DEVICE="esp32s3"
MIN_IDF_VERSION="v4.4"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Logging functions
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if ESP-IDF is properly installed
check_idf_environment() {
    log_info "Checking ESP-IDF environment..."
    
    if [ -z "$IDF_PATH" ]; then
        log_error "IDF_PATH is not set. Please install and configure ESP-IDF."
        exit 1
    fi
    
    if ! command -v idf.py &> /dev/null; then
        log_error "idf.py not found in PATH. Please source ESP-IDF setup script."
        exit 1
    fi
    
    log_success "ESP-IDF environment is properly configured"
}

# Validate project structure
validate_project() {
    log_info "Validating project structure..."
    
    # Check required files
    required_files=(
        "CMakeLists.txt"
        "main/CMakeLists.txt"
        "main/main.c"
        "sdkconfig.defaults"
        "partitions.csv"
    )
    
    for file in "${required_files[@]}"; do
        if [ ! -f "$file" ]; then
            log_error "Required file missing: $file"
            exit 1
        fi
    done
    
    # Check component directories
    required_components=(
        "components/cc1101"
        "components/mjs_engine"
        "components/lvgl_port"
        "components/js_api"
        "components/app_manager"
        "components/storage_service"
        "components/network_service"
    )
    
    for component in "${required_components[@]}"; do
        if [ ! -d "$component" ]; then
            log_error "Required component missing: $component"
            exit 1
        fi
    done
    
    log_success "Project structure validation passed"
}

# Set target device
set_target() {
    log_info "Setting target device to $TARGET_DEVICE..."
    idf.py set-target $TARGET_DEVICE
    log_success "Target set to $TARGET_DEVICE"
}

# Configure project
configure() {
    log_info "Configuring project..."
    check_idf_environment
    validate_project
    set_target
    log_success "Project configuration completed"
}

# Build firmware
build() {
    log_info "Building firmware..."
    configure
    
    # Run the build
    idf.py build
    
    if [ $? -eq 0 ]; then
        log_success "Build completed successfully"
        show_build_summary
    else
        log_error "Build failed"
        exit 1
    fi
}

# Show build summary
show_build_summary() {
    if [ -f "build/$PROJECT_NAME.bin" ]; then
        local size=$(du -h "build/$PROJECT_NAME.bin" | cut -f1)
        log_info "Firmware size: $size"
    fi
    
    if [ -f "build/project_description.json" ]; then
        log_info "Build details available in build/project_description.json"
    fi
}

# Flash firmware to device
flash() {
    local port="$1"
    
    if [ -z "$port" ]; then
        port=$(detect_port)
        if [ -z "$port" ]; then
            log_error "No serial port specified or detected. Use: $0 flash <port>"
            exit 1
        fi
    fi
    
    log_info "Flashing firmware to $port..."
    build
    
    idf.py -p "$port" flash
    
    if [ $? -eq 0 ]; then
        log_success "Firmware flashed successfully to $port"
    else
        log_error "Flash operation failed"
        exit 1
    fi
}

# Monitor serial output
monitor() {
    local port="$1"
    
    if [ -z "$port" ]; then
        port=$(detect_port)
        if [ -z "$port" ]; then
            log_error "No serial port specified or detected. Use: $0 monitor <port>"
            exit 1
        fi
    fi
    
    log_info "Starting monitor on $port..."
    log_info "Press Ctrl+] to exit monitor"
    
    idf.py -p "$port" monitor
}

# Flash and monitor
flash_monitor() {
    local port="$1"
    
    if [ -z "$port" ]; then
        port=$(detect_port)
        if [ -z "$port" ]; then
            log_error "No serial port specified or detected. Use: $0 flash-monitor <port>"
            exit 1
        fi
    fi
    
    flash "$port"
    monitor "$port"
}

# Detect available serial ports
detect_port() {
    local ports=""
    
    case "$(uname)" in
        "Darwin")
            ports=$(ls /dev/cu.usbserial-* 2>/dev/null | head -1)
            ;;
        "Linux")
            ports=$(ls /dev/ttyUSB* /dev/ttyACM* 2>/dev/null | head -1)
            ;;
        *)
            log_warning "Port auto-detection not supported on this OS"
            ;;
    esac
    
    if [ -n "$ports" ]; then
        log_info "Detected port: $ports"
        echo "$ports"
    else
        log_warning "No serial ports detected"
    fi
}

# Clean build files
clean() {
    log_info "Cleaning build files..."
    
    if [ -d "build" ]; then
        idf.py clean
        log_success "Build files cleaned"
    else
        log_info "No build files to clean"
    fi
}

# Create release package
create_release() {
    local version="$1"
    
    if [ -z "$version" ]; then
        version="dev-$(date +%Y%m%d-%H%M%S)"
    fi
    
    log_info "Creating release package v$version..."
    
    # Build first
    build
    
    # Create release directory
    local release_dir="release/v$version"
    mkdir -p "$release_dir"
    
    # Copy firmware files
    cp "build/$PROJECT_NAME.bin" "$release_dir/"
    cp "build/$PROJECT_NAME.elf" "$release_dir/"
    cp "build/bootloader/bootloader.bin" "$release_dir/"
    cp "build/partition_table/partition-table.bin" "$release_dir/"
    cp "partitions.csv" "$release_dir/"
    
    # Create flash script
    create_flash_script "$release_dir"
    
    # Create README
    create_release_readme "$release_dir" "$version"
    
    # Calculate checksums
    cd "$release_dir"
    sha256sum *.bin *.elf > checksums.txt
    cd - > /dev/null
    
    log_success "Release package created: $release_dir"
}

# Create flash script for release
create_flash_script() {
    local release_dir="$1"
    
    cat > "$release_dir/flash.sh" << 'EOF'
#!/bin/bash
# Flash script for T-Embed CC1101 Firmware

PORT=${1:-/dev/ttyUSB0}

echo "Flashing T-Embed CC1101 Firmware to $PORT..."

# Flash bootloader, partition table, and firmware
esptool.py --chip esp32s3 --port "$PORT" --baud 921600 write_flash \
    0x0 bootloader.bin \
    0x8000 partition-table.bin \
    0x10000 t-embed-cc1101-fw.bin

echo "Flash completed. Reset device to start firmware."
EOF
    
    chmod +x "$release_dir/flash.sh"
}

# Create release README
create_release_readme() {
    local release_dir="$1"
    local version="$2"
    
    cat > "$release_dir/README.md" << EOF
# T-Embed CC1101 Firmware v$version

LilyGO T-Embed CC1101 JavaScript App Support Firmware

## Features

- CC1101 Sub-GHz RF communication (315/433/868/915 MHz)
- JavaScript app development environment (mJS engine)
- LVGL-based graphical user interface
- Wi-Fi connectivity and web IDE
- App management and sandboxing
- File system and configuration storage

## Flashing Instructions

### Using included script (Linux/macOS):
\`\`\`bash
./flash.sh [port]
\`\`\`

### Manual flashing:
\`\`\`bash
esptool.py --chip esp32s3 --port /dev/ttyUSB0 --baud 921600 write_flash \\
    0x0 bootloader.bin \\
    0x8000 partition-table.bin \\
    0x10000 t-embed-cc1101-fw.bin
\`\`\`

## Files

- \`t-embed-cc1101-fw.bin\` - Main firmware binary
- \`bootloader.bin\` - ESP32-S3 bootloader
- \`partition-table.bin\` - Partition table
- \`checksums.txt\` - SHA256 checksums for verification

## Hardware Requirements

- LilyGO T-Embed ESP32-S3 with CC1101 module
- USB-C cable for programming/power

## Documentation

Full documentation available at: https://github.com/your-repo/cc1101-fw

Build date: $(date)
EOF
}

# Run tests (placeholder)
test() {
    log_info "Running tests..."
    log_warning "Test framework not implemented yet"
}

# Show help
show_help() {
    echo "T-Embed CC1101 Firmware Build Script"
    echo ""
    echo "Usage: $0 <command> [options]"
    echo ""
    echo "Commands:"
    echo "  configure         - Configure project for ESP32-S3"
    echo "  build            - Build firmware"
    echo "  flash [port]     - Flash firmware to device"
    echo "  monitor [port]   - Start serial monitor"
    echo "  flash-monitor [port] - Flash and start monitor"
    echo "  clean            - Clean build files"
    echo "  release [version] - Create release package"
    echo "  test             - Run tests"
    echo "  detect-port      - Detect available serial ports"
    echo "  help             - Show this help"
    echo ""
    echo "Examples:"
    echo "  $0 build"
    echo "  $0 flash /dev/ttyUSB0"
    echo "  $0 flash-monitor"
    echo "  $0 release v1.0.0"
}

# Main script logic
case "$1" in
    "configure")
        configure
        ;;
    "build")
        build
        ;;
    "flash")
        flash "$2"
        ;;
    "monitor")
        monitor "$2"
        ;;
    "flash-monitor")
        flash_monitor "$2"
        ;;
    "clean")
        clean
        ;;
    "release")
        create_release "$2"
        ;;
    "test")
        test
        ;;
    "detect-port")
        detect_port
        ;;
    "help"|"--help"|"-h"|"")
        show_help
        ;;
    *)
        log_error "Unknown command: $1"
        show_help
        exit 1
        ;;
esac