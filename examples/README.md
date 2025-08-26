# T-Embed CC1101 Example Apps

This directory contains example JavaScript applications demonstrating various features and capabilities of the T-Embed CC1101 platform.

## Available Examples

### 1. Hello World (`hello-world/`)
**Category:** Basic  
**Description:** Simple demonstration of basic UI components and event handling.

**Features:**
- Basic UI creation
- Button interactions
- Notifications
- Device information display
- App lifecycle management

**Permissions:** `ui.create`

**Learning Goals:**
- Understand app structure
- Learn basic UI components
- Handle user interactions
- Manage app lifecycle

### 2. RF Scanner (`rf-scanner/`)
**Category:** RF Communication  
**Description:** Advanced RF signal scanner with frequency sweeping and signal analysis.

**Features:**
- CC1101 radio configuration
- Signal reception and transmission
- Frequency sweeping
- Signal strength monitoring
- Scan result logging
- Settings persistence

**Permissions:** `rf.receive`, `rf.transmit`, `ui.create`, `storage.write`

**Learning Goals:**
- Control RF hardware
- Implement frequency management
- Handle radio communications
- Create complex multi-tab UIs
- Save and load application data

### 3. LED Controller (`led-controller/`)
**Category:** Hardware Control  
**Description:** GPIO controller for LEDs, buttons, and sensors.

**Features:**
- GPIO pin control
- LED on/off control
- Button interrupt handling
- Analog sensor reading
- Hardware state management

**Permissions:** `gpio.read`, `gpio.write`, `ui.create`, `storage.read`, `storage.write`

**Learning Goals:**
- Control digital outputs
- Read digital/analog inputs
- Handle hardware interrupts
- Implement sensor monitoring
- Create hardware control interfaces

### 4. UI Showcase (`ui-showcase/`)
**Category:** User Interface  
**Description:** Comprehensive demonstration of UI components and interactions.

**Features:**
- Multiple UI component types
- Tabbed interface
- Progress bars and sliders
- Text input and dropdowns
- Color pickers
- Interactive animations
- Mini-game example

**Permissions:** `ui.create`, `storage.write`

**Learning Goals:**
- Master LVGL components
- Create complex layouts
- Handle various input types
- Implement animations
- Design user-friendly interfaces

## Getting Started

### Prerequisites
- T-Embed CC1101 device
- T-Embed SDK installed
- Node.js development environment

### Running Examples

1. **Navigate to SDK directory:**
   ```bash
   cd t-embed-cc1101-fw/sdk
   ```

2. **Build an example:**
   ```bash
   npm run build-app ../examples/hello-world --output ./build
   ```

3. **Install to device:**
   ```bash
   npm run install-app ./build/hello-world --port /dev/ttyUSB0
   ```

### Creating Your Own App

Use the SDK tools to create a new app based on these examples:

```bash
# Create app based on a template
npm run create-app -- --name my-app --template basic

# Or use interactive mode
npm run create-app -- --interactive
```

## Example Structure

Each example follows the standard T-Embed app structure:

```
example-name/
├── manifest.json      # App metadata and permissions
├── app.js            # Main application code
├── README.md         # Example documentation
└── assets/           # App resources (if any)
    └── app.png       # App icon
```

## Development Tips

### Best Practices Demonstrated

1. **Error Handling**
   - All examples include proper try-catch blocks
   - Graceful error recovery
   - User feedback for errors

2. **Resource Management**
   - Proper cleanup on app exit
   - Timer management
   - Hardware resource release

3. **User Experience**
   - Responsive UI design
   - Clear status indicators
   - Helpful notifications
   - Intuitive navigation

4. **Code Organization**
   - Modular function structure
   - Clear separation of concerns
   - Consistent naming conventions
   - Comprehensive comments

### Common Patterns

#### App Initialization
```javascript
function init() {
    console.log("App starting...");
    try {
        initializeHardware();
        createUI();
        setupEventHandlers();
        loadSettings();
    } catch (error) {
        console.error("Initialization error:", error);
        Notification.showError("App failed to start");
    }
}
```

#### UI Creation
```javascript
function createUI() {
    const screen = UI.getScreen();
    const container = UI.createContainer(screen);
    UI.setContainerLayout(container, UI.LAYOUT_FLEX_COLUMN);
    
    // Create components...
}
```

#### Event Handling
```javascript
function setupEventHandlers() {
    System.onBackButton(function() {
        cleanup();
        System.exit();
    });
    
    System.onPause(function() {
        // Pause app activities
    });
    
    System.onResume(function() {
        // Resume app activities
    });
}
```

#### Cleanup
```javascript
function cleanup() {
    console.log("Cleaning up...");
    // Stop timers
    // Release hardware resources
    // Save state if needed
}
```

## Testing Examples

### Validation
Validate any example before deployment:
```bash
npm run validate-app ../examples/hello-world --verbose
```

### Device Testing
Test examples on actual hardware to verify:
- UI responsiveness
- Hardware interactions
- Performance characteristics
- Battery usage

### Debugging
Use console output for debugging:
```javascript
console.log("Debug info:", data);
console.error("Error:", error);
```

Debug output is available through:
- Serial console (USB connection)
- Web IDE debug panel
- Device log files

## Extending Examples

### Customization Ideas

1. **Hello World Extensions:**
   - Add more interactive elements
   - Implement a simple counter game
   - Add sound effects
   - Create custom animations

2. **RF Scanner Enhancements:**
   - Add protocol decoding
   - Implement signal recording
   - Create frequency database
   - Add spectrum analysis

3. **LED Controller Improvements:**
   - Add PWM brightness control
   - Implement color mixing
   - Create light patterns
   - Add sensor-based automation

4. **UI Showcase Additions:**
   - Add more game elements
   - Implement data visualization
   - Create custom themes
   - Add gesture recognition

### Contributing

We welcome contributions to the example collection:

1. Fork the repository
2. Create a new example directory
3. Follow the existing structure and patterns
4. Add comprehensive documentation
5. Test thoroughly on hardware
6. Submit a pull request

## Support

- **Documentation:** [API Reference](../sdk/docs/API.md), [Development Guide](../sdk/docs/GUIDE.md)
- **GitHub Issues:** Report bugs or request features
- **Community:** Join discussions and share your creations

## License

These examples are provided under the MIT License as part of the T-Embed CC1101 SDK.

---

**Ready to explore T-Embed development?** Start with the Hello World example and work your way up to more advanced applications!