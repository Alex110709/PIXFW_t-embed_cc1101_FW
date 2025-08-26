# T-Embed CC1101 Development Guide

Complete guide for developing JavaScript applications for the T-Embed CC1101 device.

## Table of Contents

1. [Getting Started](#getting-started)
2. [Development Environment](#development-environment)
3. [Creating Your First App](#creating-your-first-app)
4. [App Structure](#app-structure)
5. [JavaScript APIs](#javascript-apis)
6. [UI Development](#ui-development)
7. [Hardware Integration](#hardware-integration)
8. [Testing and Debugging](#testing-and-debugging)
9. [Deployment](#deployment)
10. [Best Practices](#best-practices)

## Getting Started

### Prerequisites

- Node.js 14 or higher
- T-Embed CC1101 device
- USB cable for programming
- Text editor or IDE

### SDK Installation

1. Clone or download the SDK:
```bash
git clone https://github.com/your-repo/t-embed-cc1101-fw.git
cd t-embed-cc1101-fw/sdk
```

2. Install dependencies:
```bash
npm install
```

3. Verify installation:
```bash
npm run --help
```

## Development Environment

### CLI Tools

The SDK provides several command-line tools:

- `npm run create-app` - Create new app projects
- `npm run validate-app` - Validate app structure and code
- `npm run build-app` - Build apps for deployment
- `npm run install-app` - Install apps to devices

### Directory Structure

```
my-app/
├── manifest.json      # App metadata and configuration
├── app.js            # Main application code
├── assets/           # App resources (icons, images)
│   ├── app.png       # App icon
│   └── README.md     # Assets documentation
├── README.md         # App documentation
└── .gitignore        # Git ignore rules
```

## Creating Your First App

### Interactive App Creation

```bash
npm run create-app -- --interactive
```

This will guide you through creating a new app with the following options:

1. **App Name**: Identifier for your app
2. **Description**: Brief description of functionality
3. **Author**: Your name or organization
4. **Template**: Starting template (basic, rf, ui, gpio)
5. **Category**: App category for organization
6. **Permissions**: Required hardware access permissions

### Manual App Creation

```bash
npm run create-app -- --name my-awesome-app --template rf --dir ./apps
```

### App Templates

#### Basic Template
Simple app with basic UI components - ideal for learning.

#### RF Template
Radio communication app with CC1101 integration - for RF applications.

#### UI Template
Advanced user interface with multiple screens - for complex UI apps.

#### GPIO Template
Hardware control app with GPIO access - for IoT applications.

## App Structure

### Manifest File

The `manifest.json` file contains app metadata:

```json
{
  "manifest_version": "1.0",
  "name": "my-app",
  "description": "My awesome T-Embed app",
  "version": "1.0.0",
  "author": "Developer",
  "main": "app.js",
  "permissions": [
    "rf.receive",
    "rf.transmit",
    "ui.create"
  ],
  "icon": "app.png",
  "category": "utility"
}
```

### Main Application File

The main JavaScript file (typically `app.js`) contains your app logic:

```javascript
// App initialization
function init() {
    console.log("App starting...");
    createUI();
    setupEventHandlers();
}

// Create user interface
function createUI() {
    const screen = UI.getScreen();
    const container = UI.createContainer(screen);
    
    const title = UI.createLabel(container, "My App");
    const button = UI.createButton(container, "Click Me");
    
    UI.setButtonCallback(button, onButtonClick);
}

// Event handlers
function setupEventHandlers() {
    System.onBackButton(function() {
        System.exit();
    });
}

function onButtonClick() {
    Notification.show("Button clicked!", 2000);
}

// Start the app
init();
```

## JavaScript APIs

### Core APIs

The T-Embed platform provides several JavaScript APIs:

- **RF**: Radio communication (CC1101)
- **GPIO**: Digital/analog pin control
- **UI**: User interface components (LVGL)
- **Storage**: File system access
- **Notification**: System notifications
- **System**: Device and lifecycle management

See the [API Reference](API.md) for detailed documentation.

### Permission System

Apps must declare required permissions in the manifest:

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

## UI Development

### LVGL Integration

The UI system is based on LVGL (Light and Versatile Graphics Library):

```javascript
function createAdvancedUI() {
    const screen = UI.getScreen();
    
    // Create container with flex layout
    const container = UI.createContainer(screen);
    UI.setContainerLayout(container, UI.LAYOUT_FLEX_COLUMN);
    
    // Create components
    const title = UI.createLabel(container, "Advanced UI");
    const slider = UI.createSlider(container);
    const switch1 = UI.createSwitch(container);
    const button = UI.createButton(container, "Action");
    
    // Set up callbacks
    UI.setSliderCallback(slider, function(value) {
        console.log("Slider value:", value);
    });
    
    UI.setSwitchCallback(switch1, function(state) {
        console.log("Switch state:", state);
    });
}
```

### Responsive Design

Design for the T-Embed's 170x320 pixel display:

```javascript
// Use flexible layouts
UI.setContainerLayout(container, UI.LAYOUT_FLEX_COLUMN);

// Set appropriate font sizes
UI.setLabelStyle(title, {
    fontSize: 20,
    fontWeight: "bold"
});

// Ensure touch targets are large enough
UI.setButtonStyle(button, {
    minHeight: 40,
    padding: 10
});
```

## Hardware Integration

### RF Communication

```javascript
// Configure CC1101
RF.setFrequency(433.92);
RF.setModulation(RF.MOD_ASK);
RF.setDataRate(4.8);
RF.setPower(10);

// Transmit data
RF.transmit("Hello World", function(success) {
    if (success) {
        console.log("Message sent");
    }
});

// Receive data
RF.startReceive(function(data) {
    console.log("Received:", data);
    Notification.show("Message: " + data, 3000);
});
```

### GPIO Control

```javascript
// Configure pins
GPIO.pinMode(2, GPIO.OUTPUT);      // LED
GPIO.pinMode(0, GPIO.INPUT_PULLUP); // Button

// Control LED
GPIO.digitalWrite(2, GPIO.HIGH);

// Read button with interrupt
GPIO.setInterrupt(0, GPIO.FALLING, function() {
    console.log("Button pressed!");
    GPIO.digitalWrite(2, GPIO.LOW); // Turn off LED
});

// Read analog sensor
const sensorValue = GPIO.analogRead(4);
const voltage = (sensorValue / 4095.0) * 3.3;
```

### Storage Access

```javascript
// Save app settings
const settings = {
    theme: "dark",
    frequency: 433.92,
    volume: 75
};

Storage.writeFile("/config/app.json", JSON.stringify(settings, null, 2));

// Load app settings
try {
    const data = Storage.readFile("/config/app.json");
    const settings = JSON.parse(data);
    console.log("Loaded settings:", settings);
} catch (error) {
    console.log("Using default settings");
}
```

## Testing and Debugging

### App Validation

Validate your app before deployment:

```bash
npm run validate-app ./apps/my-app
```

This checks for:
- Valid manifest.json
- JavaScript syntax errors
- Missing files
- Permission usage
- Best practice compliance

### Debug Output

Use console functions for debugging:

```javascript
console.log("Info message");
console.warn("Warning message");
console.error("Error message");
```

Debug output is available through:
- Serial console (USB connection)
- Web IDE debug panel
- Log files on device

### Error Handling

Implement robust error handling:

```javascript
function safeRFOperation() {
    try {
        RF.setFrequency(433.92);
        RF.transmit("test", function(success) {
            if (!success) {
                throw new Error("Transmission failed");
            }
        });
    } catch (error) {
        console.error("RF error:", error.message);
        Notification.showError("RF communication failed");
        // Fallback behavior
    }
}
```

### Testing Strategies

1. **Unit Testing**: Test individual functions
2. **Integration Testing**: Test API interactions
3. **Device Testing**: Test on actual hardware
4. **User Testing**: Test real-world usage

## Deployment

### Building Apps

Build your app for deployment:

```bash
npm run build-app ./apps/my-app --output ./build --minify --compress
```

Build options:
- `--minify`: Compress JavaScript code
- `--compress`: Create installation package
- `--output`: Specify output directory

### Installation Methods

#### USB Serial Installation
```bash
npm run install-app ./build/my-app --port /dev/ttyUSB0
```

#### Wi-Fi Installation
```bash
npm run install-app ./build/my-app --wifi --ip 192.168.1.100
```

#### Manual Installation
Copy app files to device `/apps/` directory.

### Distribution

Create distributable packages:

1. Build the app with compression
2. Test on multiple devices
3. Create documentation
4. Package for distribution

## Best Practices

### Code Organization

```javascript
// app.js - Main application structure
function init() {
    initializeHardware();
    createUI();
    setupEventHandlers();
    loadSettings();
}

function initializeHardware() {
    // Hardware initialization
}

function createUI() {
    // UI creation
}

function setupEventHandlers() {
    // Event handling
}

function loadSettings() {
    // Load app settings
}

// Start app
init();
```

### Performance Optimization

1. **Minimize Memory Usage**
   - Clean up unused objects
   - Avoid memory leaks
   - Use efficient data structures

2. **Optimize UI Updates**
   - Batch UI changes
   - Avoid frequent redraws
   - Use appropriate widget types

3. **Efficient API Usage**
   - Cache results when possible
   - Use callbacks appropriately
   - Handle errors gracefully

### Security Considerations

1. **Permission Management**
   - Request minimal permissions
   - Validate permission usage
   - Handle permission denials

2. **Input Validation**
   - Validate all user inputs
   - Sanitize data before storage
   - Check bounds and ranges

3. **Secure Communication**
   - Use appropriate RF protocols
   - Implement encryption if needed
   - Validate received data

### User Experience

1. **Responsive Design**
   - Design for touch input
   - Use appropriate font sizes
   - Ensure good contrast

2. **Feedback and Notifications**
   - Provide operation feedback
   - Use appropriate notification types
   - Don't overwhelm with notifications

3. **Error Recovery**
   - Graceful error handling
   - Provide recovery options
   - Maintain app stability

### Code Quality

1. **Documentation**
   - Comment complex logic
   - Document API usage
   - Maintain README files

2. **Testing**
   - Test on real hardware
   - Validate edge cases
   - Test error conditions

3. **Version Control**
   - Use semantic versioning
   - Maintain change logs
   - Tag releases

## Example Projects

### Simple LED Controller

```javascript
// Simple GPIO app to control an LED
function init() {
    GPIO.pinMode(2, GPIO.OUTPUT);
    createUI();
}

function createUI() {
    const screen = UI.getScreen();
    const container = UI.createContainer(screen);
    
    const title = UI.createLabel(container, "LED Controller");
    const switch1 = UI.createSwitch(container);
    
    UI.setSwitchCallback(switch1, function(state) {
        GPIO.digitalWrite(2, state ? GPIO.HIGH : GPIO.LOW);
        Notification.show(state ? "LED On" : "LED Off", 1000);
    });
}

init();
```

### RF Scanner

```javascript
// RF scanner app
let scanning = false;

function init() {
    RF.setFrequency(433.92);
    RF.setModulation(RF.MOD_ASK);
    createUI();
}

function createUI() {
    const screen = UI.getScreen();
    const container = UI.createContainer(screen);
    
    const title = UI.createLabel(container, "RF Scanner");
    const button = UI.createButton(container, "Start Scan");
    const status = UI.createLabel(container, "Ready");
    
    UI.setButtonCallback(button, function() {
        if (!scanning) {
            startScanning();
            UI.setButtonText(button, "Stop Scan");
            UI.setLabelText(status, "Scanning...");
        } else {
            stopScanning();
            UI.setButtonText(button, "Start Scan");
            UI.setLabelText(status, "Stopped");
        }
        scanning = !scanning;
    });
    
    window.statusLabel = status;
}

function startScanning() {
    RF.startReceive(function(data) {
        UI.setLabelText(window.statusLabel, "Received: " + data);
        console.log("RF Data:", data);
    });
}

function stopScanning() {
    RF.stopReceive();
}

init();
```

## Support and Resources

- **API Reference**: [API.md](API.md)
- **GitHub Repository**: https://github.com/your-repo/t-embed-cc1101-fw
- **Issues and Support**: https://github.com/your-repo/t-embed-cc1101-fw/issues
- **Community Forum**: [Link to community forum]
- **Examples**: Check the `/examples` directory in the SDK

## Contributing

We welcome contributions to the T-Embed CC1101 SDK:

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests if applicable
5. Submit a pull request

Please read our contributing guidelines and code of conduct before submitting.