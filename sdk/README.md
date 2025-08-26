# T-Embed CC1101 JavaScript SDK

JavaScript development kit for creating apps on the LilyGO T-Embed CC1101 device.

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Node.js Version](https://img.shields.io/badge/node-%3E%3D14.0.0-brightgreen)](https://nodejs.org/)
[![Platform](https://img.shields.io/badge/platform-T--Embed%20CC1101-blue)](https://github.com/Lilygo/T-Embed)

## Overview

The T-Embed CC1101 SDK enables developers to create JavaScript applications for the LilyGO T-Embed ESP32-S3 device with CC1101 radio module. Build powerful IoT applications with radio communication, GPIO control, and rich user interfaces.

## Features

- 🚀 **JavaScript Runtime** - mJS engine for ECMAScript execution
- 📡 **RF Communication** - CC1101 Sub-GHz radio (315/433/868/915 MHz)
- 🎨 **Rich UI** - LVGL-based graphical interface components
- 🔧 **GPIO Control** - Digital/analog pin access
- 💾 **File System** - Storage API for data persistence
- 🛡️ **Sandboxed Apps** - Secure permission-based execution
- 🌐 **Wi-Fi Connectivity** - Network features and web IDE
- ⚡ **Hardware Accelerated** - ESP32-S3 240MHz dual-core performance

## Quick Start

### Prerequisites

- Node.js 14 or higher
- LilyGO T-Embed CC1101 device
- USB-C cable

### Installation

1. **Clone the SDK:**
```bash
git clone https://github.com/your-repo/t-embed-cc1101-fw.git
cd t-embed-cc1101-fw/sdk
```

2. **Install dependencies:**
```bash
npm install
```

3. **Verify installation:**
```bash
npm run --help
```

### Create Your First App

```bash
# Interactive app creation
npm run create-app -- --interactive

# Or create directly
npm run create-app -- --name hello-world --template basic
```

### Build and Deploy

```bash
# Validate your app
npm run validate-app ./apps/hello-world

# Build for deployment
npm run build-app ./apps/hello-world --minify

# Install to device
npm run install-app ./build/hello-world --port /dev/ttyUSB0
```

## CLI Commands

The SDK provides a comprehensive command-line interface:

### App Management
```bash
npm run create-app      # Create new app projects
npm run validate-app    # Validate app structure and code
npm run build-app       # Build apps for deployment
npm run install-app     # Install apps to devices
```

### Development Tools
```bash
npm run test           # Run test suite
npm run docs           # Open documentation
npm run examples       # List example projects
```

## App Structure

```
my-app/
├── manifest.json      # App metadata and configuration
├── app.js            # Main application code
├── assets/           # App resources
│   ├── app.png       # App icon (64x64)
│   └── ...           # Other assets
├── README.md         # App documentation
└── .gitignore        # Git ignore rules
```

### Manifest Example

```json
{
  "manifest_version": "1.0",
  "name": "hello-world",
  "description": "My first T-Embed app",
  "version": "1.0.0",
  "author": "Developer",
  "main": "app.js",
  "permissions": [
    "ui.create",
    "rf.receive"
  ],
  "icon": "app.png",
  "category": "utility"
}
```

### Application Example

```javascript
// Simple T-Embed app
function init() {
    console.log("Hello World app starting...");
    createUI();
    setupEventHandlers();
}

function createUI() {
    const screen = UI.getScreen();
    const container = UI.createContainer(screen);
    
    const title = UI.createLabel(container, "Hello World!");
    const button = UI.createButton(container, "Say Hello");
    
    UI.setButtonCallback(button, function() {
        Notification.show("Hello from T-Embed!", 2000);
    });
}

function setupEventHandlers() {
    System.onBackButton(function() {
        System.exit();
    });
}

init();
```

## JavaScript APIs

The SDK provides comprehensive JavaScript APIs:

### RF Communication
```javascript
// Configure and use CC1101 radio
RF.setFrequency(433.92);
RF.setModulation(RF.MOD_ASK);
RF.transmit("Hello RF!", callback);
RF.startReceive(onDataReceived);
```

### GPIO Control
```javascript
// Control hardware pins
GPIO.pinMode(2, GPIO.OUTPUT);
GPIO.digitalWrite(2, GPIO.HIGH);
const value = GPIO.analogRead(4);
```

### User Interface
```javascript
// Create rich UIs with LVGL
const screen = UI.getScreen();
const button = UI.createButton(screen, "Click Me");
const slider = UI.createSlider(screen);
UI.setSliderRange(slider, 0, 100);
```

### Storage System
```javascript
// File system operations
Storage.writeFile("/config/settings.json", data);
const config = Storage.readFile("/config/settings.json");
const files = Storage.listFiles("/apps");
```

See the [API Reference](docs/API.md) for complete documentation.

## Templates

The SDK includes several app templates:

### Basic Template
Simple app with UI components - perfect for beginners.

### RF Template
Radio communication app with CC1101 integration.

### UI Template
Advanced interface with multiple screens and complex layouts.

### GPIO Template
Hardware control app with digital/analog pin access.

## Examples

### LED Controller
```javascript
// Control an LED with a switch
GPIO.pinMode(2, GPIO.OUTPUT);

const switch1 = UI.createSwitch(screen);
UI.setSwitchCallback(switch1, function(state) {
    GPIO.digitalWrite(2, state ? GPIO.HIGH : GPIO.LOW);
});
```

### RF Scanner
```javascript
// Scan for RF signals
RF.setFrequency(433.92);
RF.startReceive(function(data) {
    console.log("Received:", data);
    UI.setLabelText(statusLabel, "Data: " + data);
});
```

### Sensor Monitor
```javascript
// Read analog sensor
setInterval(function() {
    const value = GPIO.analogRead(4);
    const voltage = (value / 4095.0) * 3.3;
    UI.setLabelText(sensorLabel, voltage.toFixed(2) + "V");
}, 1000);
```

## Hardware Specifications

### T-Embed CC1101 Device
- **MCU**: ESP32-S3 (240MHz dual-core)
- **Memory**: 512KB SRAM, 8MB Flash
- **Display**: 170x320 pixel color LCD
- **Radio**: CC1101 Sub-GHz (315/433/868/915 MHz)
- **Connectivity**: Wi-Fi, Bluetooth LE
- **Storage**: MicroSD card slot
- **GPIO**: Multiple digital/analog pins
- **Power**: USB-C, battery connector

### Supported Frequencies
- 315 MHz (ISM band)
- 433 MHz (ISM band) 
- 868 MHz (Europe)
- 915 MHz (Americas)
- Custom frequencies (300-348, 387-464, 779-928 MHz)

## Development Workflow

### 1. Create and Develop
```bash
npm run create-app -- --name my-sensor-app --template gpio
cd apps/my-sensor-app
# Edit app.js with your logic
```

### 2. Validate and Test
```bash
npm run validate-app ./apps/my-sensor-app --verbose
# Fix any issues reported
```

### 3. Build for Production
```bash
npm run build-app ./apps/my-sensor-app --minify --compress
```

### 4. Deploy to Device
```bash
# Via USB
npm run install-app ./build/my-sensor-app --port /dev/ttyUSB0

# Via Wi-Fi
npm run install-app ./build/my-sensor-app --wifi --ip 192.168.1.100
```

## Permissions System

Apps must declare required permissions in their manifest:

```json
{
  "permissions": [
    "rf.receive",     // Read radio signals
    "rf.transmit",    // Send radio signals  
    "gpio.read",      // Read GPIO pins
    "gpio.write",     // Write GPIO pins
    "storage.read",   // Read files
    "storage.write",  // Write files
    "ui.create",      // Create UI components
    "network",        // Network access
    "system"          // System functions
  ]
}
```

Users can grant/revoke permissions through the device settings.

## Best Practices

### Code Quality
- Use proper error handling with try-catch blocks
- Implement app lifecycle management (init, pause, resume, exit)
- Clean up resources when app exits
- Follow JavaScript best practices

### Performance
- Minimize memory usage
- Optimize UI updates
- Use efficient algorithms
- Cache frequently accessed data

### User Experience
- Design for touch input
- Provide visual feedback
- Handle errors gracefully
- Test on actual hardware

### Security
- Request minimal permissions
- Validate all inputs
- Sanitize data before storage
- Use secure communication protocols

## Debugging

### Debug Output
```javascript
console.log("Info message");
console.warn("Warning message");
console.error("Error message");
```

### Debug Access
- Serial console (USB connection)
- Web IDE debug panel
- Log files on device storage

### Common Issues
- Permission denied errors
- Invalid manifest format
- JavaScript syntax errors
- Hardware initialization failures

## Contributing

We welcome contributions! Please see our [Contributing Guide](CONTRIBUTING.md) for details.

### Development Setup
```bash
git clone https://github.com/your-repo/t-embed-cc1101-fw.git
cd t-embed-cc1101-fw
npm install
npm run build
```

### Testing
```bash
npm test                    # Run test suite
npm run test:apps          # Test example apps
npm run test:integration   # Integration tests
```

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Support

- **Documentation**: [Development Guide](docs/GUIDE.md), [API Reference](docs/API.md)
- **GitHub Issues**: [Report bugs and request features](https://github.com/your-repo/t-embed-cc1101-fw/issues)
- **Discussions**: [Community forum](https://github.com/your-repo/t-embed-cc1101-fw/discussions)
- **Wiki**: [Additional documentation](https://github.com/your-repo/t-embed-cc1101-fw/wiki)

## Acknowledgments

- [LilyGO](https://github.com/Lilygo) for the T-Embed hardware platform
- [Espressif](https://www.espressif.com/) for the ESP32-S3 MCU
- [LVGL](https://lvgl.io/) for the graphics library
- [Mongoose OS](https://mongoose-os.com/) for the mJS JavaScript engine
- [Texas Instruments](https://www.ti.com/) for the CC1101 radio chip

## Roadmap

### Version 1.1 (Planned)
- [ ] WebBluetooth API support
- [ ] Audio playback APIs
- [ ] Advanced crypto functions
- [ ] Cloud integration APIs

### Version 1.2 (Future)
- [ ] Machine learning inference
- [ ] Advanced networking protocols
- [ ] Real-time collaboration features
- [ ] Plugin system

---

**Ready to build amazing IoT applications with JavaScript?** 

Start with: `npm run create-app -- --interactive`