/**
 * @file index.js
 * @brief T-Embed CC1101 JavaScript SDK Main Entry Point
 */

const fs = require('fs');
const path = require('path');

// SDK Constants
const SDK_VERSION = '1.0.0';
const SUPPORTED_API_VERSION = '1.0';

// Default manifest template
const DEFAULT_MANIFEST = {
  manifest_version: "1.0",
  name: "",
  description: "",
  version: "1.0.0",
  author: "",
  main: "app.js",
  permissions: [],
  icon: "app.png",
  category: "utility"
};

// Available permissions
const PERMISSIONS = {
  RF_RECEIVE: 'rf.receive',
  RF_TRANSMIT: 'rf.transmit', 
  GPIO_READ: 'gpio.read',
  GPIO_WRITE: 'gpio.write',
  STORAGE_READ: 'storage.read',
  STORAGE_WRITE: 'storage.write',
  UI_CREATE: 'ui.create',
  NETWORK: 'network',
  SYSTEM: 'system'
};

// API Categories
const API_CATEGORIES = {
  RF: 'rf',
  GPIO: 'gpio', 
  UI: 'ui',
  STORAGE: 'storage',
  NOTIFICATION: 'notification',
  SYSTEM: 'system'
};

class TEmbedSDK {
  constructor() {
    this.version = SDK_VERSION;
    this.apiVersion = SUPPORTED_API_VERSION;
  }

  /**
   * Create a new app project structure
   * @param {string} appName - Name of the app
   * @param {string} targetDir - Target directory
   * @param {object} options - Creation options
   */
  createApp(appName, targetDir, options = {}) {
    const appDir = path.join(targetDir, appName);
    
    // Create directory structure
    fs.mkdirSync(appDir, { recursive: true });
    fs.mkdirSync(path.join(appDir, 'assets'), { recursive: true });
    
    // Create manifest
    const manifest = {
      ...DEFAULT_MANIFEST,
      name: appName,
      description: options.description || `${appName} application`,
      author: options.author || 'Developer',
      permissions: options.permissions || [],
      category: options.category || 'utility'
    };
    
    fs.writeFileSync(
      path.join(appDir, 'manifest.json'),
      JSON.stringify(manifest, null, 2)
    );
    
    // Create main app file
    const appTemplate = this.generateAppTemplate(appName, options);
    fs.writeFileSync(path.join(appDir, 'app.js'), appTemplate);
    
    // Create README
    const readme = this.generateReadme(appName, manifest);
    fs.writeFileSync(path.join(appDir, 'README.md'), readme);
    
    console.log(`App '${appName}' created successfully in ${appDir}`);
    return appDir;
  }

  /**
   * Generate app template based on type
   */
  generateAppTemplate(appName, options) {
    const template = options.template || 'basic';
    
    switch (template) {
      case 'rf':
        return this.generateRfTemplate(appName);
      case 'ui':
        return this.generateUiTemplate(appName);
      case 'gpio':
        return this.generateGpioTemplate(appName);
      default:
        return this.generateBasicTemplate(appName);
    }
  }

  generateBasicTemplate(appName) {
    return `/**
 * ${appName} - T-Embed CC1101 JavaScript App
 * Generated with T-Embed SDK v${SDK_VERSION}
 */

// App initialization
function init() {
    console.log("${appName} starting...");
    
    // Initialize UI
    createUI();
    
    // Set up event handlers
    setupEventHandlers();
    
    console.log("${appName} initialized successfully");
}

// Create user interface
function createUI() {
    // Create main container
    const screen = UI.getScreen();
    const container = UI.createContainer(screen);
    UI.setContainerLayout(container, UI.LAYOUT_FLEX_COLUMN);
    
    // Create title label
    const title = UI.createLabel(container, "${appName}");
    UI.setLabelStyle(title, {
        fontSize: 24,
        fontWeight: "bold",
        textAlign: "center"
    });
    
    // Create info label
    const info = UI.createLabel(container, "Welcome to ${appName}!");
    UI.setLabelStyle(info, {
        fontSize: 16,
        textAlign: "center",
        marginTop: 20
    });
    
    // Create button
    const button = UI.createButton(container, "Click Me");
    UI.setButtonCallback(button, onButtonClick);
    UI.setButtonStyle(button, {
        marginTop: 30,
        padding: 10
    });
}

// Event handlers
function setupEventHandlers() {
    // Handle back button
    System.onBackButton(function() {
        console.log("Back button pressed");
        System.exit();
    });
    
    // Handle app lifecycle
    System.onPause(function() {
        console.log("App paused");
    });
    
    System.onResume(function() {
        console.log("App resumed");
    });
}

function onButtonClick() {
    console.log("Button clicked!");
    Notification.show("Hello from ${appName}!", 2000);
}

// App entry point
init();
`;
  }

  generateRfTemplate(appName) {
    return `/**
 * ${appName} - RF Application
 * Generated with T-Embed SDK v${SDK_VERSION}
 */

// RF Configuration
const RF_CONFIG = {
    frequency: 433.92, // MHz
    modulation: RF.MOD_ASK,
    dataRate: 4.8,     // kBaud
    power: 10          // dBm
};

function init() {
    console.log("${appName} (RF App) starting...");
    
    // Initialize RF module
    if (!initRF()) {
        console.error("Failed to initialize RF module");
        return;
    }
    
    createUI();
    setupEventHandlers();
    
    console.log("${appName} initialized successfully");
}

function initRF() {
    try {
        // Configure RF parameters
        RF.setFrequency(RF_CONFIG.frequency);
        RF.setModulation(RF_CONFIG.modulation);
        RF.setDataRate(RF_CONFIG.dataRate);
        RF.setPower(RF_CONFIG.power);
        
        console.log("RF configured:", RF_CONFIG);
        return true;
    } catch (error) {
        console.error("RF initialization failed:", error);
        return false;
    }
}

function createUI() {
    const screen = UI.getScreen();
    const container = UI.createContainer(screen);
    UI.setContainerLayout(container, UI.LAYOUT_FLEX_COLUMN);
    
    // Title
    const title = UI.createLabel(container, "${appName}");
    UI.setLabelStyle(title, { fontSize: 20, fontWeight: "bold" });
    
    // Frequency display
    const freqLabel = UI.createLabel(container, \`Frequency: \${RF_CONFIG.frequency} MHz\`);
    UI.setLabelStyle(freqLabel, { fontSize: 14, marginTop: 10 });
    
    // Transmit button
    const txButton = UI.createButton(container, "Transmit Test");
    UI.setButtonCallback(txButton, transmitTest);
    UI.setButtonStyle(txButton, { marginTop: 20 });
    
    // Receive button
    const rxButton = UI.createButton(container, "Start Receiving");
    UI.setButtonCallback(rxButton, startReceiving);
    UI.setButtonStyle(rxButton, { marginTop: 10 });
    
    // Status label
    window.statusLabel = UI.createLabel(container, "Ready");
    UI.setLabelStyle(window.statusLabel, { 
        fontSize: 12, 
        marginTop: 20,
        textAlign: "center"
    });
}

function transmitTest() {
    try {
        const testData = "Hello from ${appName}!";
        UI.setLabelText(window.statusLabel, "Transmitting...");
        
        RF.transmit(testData, function(success) {
            if (success) {
                UI.setLabelText(window.statusLabel, "Transmitted successfully");
                console.log("Transmitted:", testData);
            } else {
                UI.setLabelText(window.statusLabel, "Transmission failed");
                console.error("Transmission failed");
            }
        });
    } catch (error) {
        console.error("Transmit error:", error);
        UI.setLabelText(window.statusLabel, "Error: " + error.message);
    }
}

function startReceiving() {
    try {
        UI.setLabelText(window.statusLabel, "Receiving...");
        
        RF.startReceive(function(data) {
            console.log("Received:", data);
            UI.setLabelText(window.statusLabel, \`Received: \${data}\`);
            Notification.show(\`Received: \${data}\`, 3000);
        });
    } catch (error) {
        console.error("Receive error:", error);
        UI.setLabelText(window.statusLabel, "Error: " + error.message);
    }
}

function setupEventHandlers() {
    System.onBackButton(function() {
        RF.stopReceive();
        System.exit();
    });
}

init();
`;
  }

  generateUiTemplate(appName) {
    return `/**
 * ${appName} - UI Application
 * Generated with T-Embed SDK v${SDK_VERSION}
 */

function init() {
    console.log("${appName} (UI App) starting...");
    createAdvancedUI();
    setupEventHandlers();
    console.log("${appName} initialized successfully");
}

function createAdvancedUI() {
    const screen = UI.getScreen();
    
    // Create main container with tabs
    const tabView = UI.createTabView(screen);
    
    // Tab 1: Controls
    const tab1 = UI.addTab(tabView, "Controls");
    createControlsTab(tab1);
    
    // Tab 2: Display
    const tab2 = UI.addTab(tabView, "Display");
    createDisplayTab(tab2);
    
    // Tab 3: Settings
    const tab3 = UI.addTab(tabView, "Settings");
    createSettingsTab(tab3);
}

function createControlsTab(parent) {
    const container = UI.createContainer(parent);
    UI.setContainerLayout(container, UI.LAYOUT_FLEX_COLUMN);
    
    // Slider
    const sliderLabel = UI.createLabel(container, "Value: 50");
    const slider = UI.createSlider(container);
    UI.setSliderRange(slider, 0, 100);
    UI.setSliderValue(slider, 50);
    UI.setSliderCallback(slider, function(value) {
        UI.setLabelText(sliderLabel, \`Value: \${value}\`);
    });
    
    // Switch
    const switchLabel = UI.createLabel(container, "Feature: OFF");
    const switch1 = UI.createSwitch(container);
    UI.setSwitchCallback(switch1, function(state) {
        UI.setLabelText(switchLabel, \`Feature: \${state ? "ON" : "OFF"}\`);
    });
    
    // Button group
    const btnContainer = UI.createContainer(container);
    UI.setContainerLayout(btnContainer, UI.LAYOUT_FLEX_ROW);
    
    const btn1 = UI.createButton(btnContainer, "Action 1");
    const btn2 = UI.createButton(btnContainer, "Action 2");
    
    UI.setButtonCallback(btn1, () => Notification.show("Action 1 executed", 1500));
    UI.setButtonCallback(btn2, () => Notification.show("Action 2 executed", 1500));
}

function createDisplayTab(parent) {
    const container = UI.createContainer(parent);
    UI.setContainerLayout(container, UI.LAYOUT_FLEX_COLUMN);
    
    // Chart area
    const chartLabel = UI.createLabel(container, "Data Visualization");
    UI.setLabelStyle(chartLabel, { fontSize: 16, fontWeight: "bold" });
    
    // Progress bar
    const progressLabel = UI.createLabel(container, "Progress: 0%");
    window.progressBar = UI.createProgressBar(container);
    UI.setProgressBarValue(window.progressBar, 0);
    
    // Start animation button
    const animateBtn = UI.createButton(container, "Start Animation");
    UI.setButtonCallback(animateBtn, startProgressAnimation);
}

function createSettingsTab(parent) {
    const container = UI.createContainer(parent);
    UI.setContainerLayout(container, UI.LAYOUT_FLEX_COLUMN);
    
    // Dropdown
    const dropdown = UI.createDropdown(container);
    UI.addDropdownOption(dropdown, "Option 1");
    UI.addDropdownOption(dropdown, "Option 2");
    UI.addDropdownOption(dropdown, "Option 3");
    
    // Text area
    const textArea = UI.createTextArea(container);
    UI.setTextAreaPlaceholder(textArea, "Enter your notes here...");
    
    // Save button
    const saveBtn = UI.createButton(container, "Save Settings");
    UI.setButtonCallback(saveBtn, function() {
        const text = UI.getTextAreaValue(textArea);
        Storage.writeFile("settings.txt", text);
        Notification.show("Settings saved!", 2000);
    });
}

function startProgressAnimation() {
    let progress = 0;
    const interval = setInterval(function() {
        progress += 5;
        UI.setProgressBarValue(window.progressBar, progress);
        
        if (progress >= 100) {
            clearInterval(interval);
            Notification.show("Animation completed!", 2000);
        }
    }, 100);
}

function setupEventHandlers() {
    System.onBackButton(System.exit);
}

init();
`;
  }

  generateGpioTemplate(appName) {
    return `/**
 * ${appName} - GPIO Application  
 * Generated with T-Embed SDK v${SDK_VERSION}
 */

// GPIO Pin Configuration
const GPIO_PINS = {
    LED: 2,
    BUTTON: 0,
    SENSOR: 4
};

function init() {
    console.log("${appName} (GPIO App) starting...");
    
    if (!initGPIO()) {
        console.error("Failed to initialize GPIO");
        return;
    }
    
    createUI();
    setupEventHandlers();
    startSensorReading();
    
    console.log("${appName} initialized successfully");
}

function initGPIO() {
    try {
        // Configure LED pin as output
        GPIO.pinMode(GPIO_PINS.LED, GPIO.OUTPUT);
        GPIO.digitalWrite(GPIO_PINS.LED, GPIO.LOW);
        
        // Configure button pin as input with pullup
        GPIO.pinMode(GPIO_PINS.BUTTON, GPIO.INPUT_PULLUP);
        GPIO.setInterrupt(GPIO_PINS.BUTTON, GPIO.FALLING, onButtonPress);
        
        // Configure sensor pin as input
        GPIO.pinMode(GPIO_PINS.SENSOR, GPIO.INPUT);
        
        console.log("GPIO initialized successfully");
        return true;
    } catch (error) {
        console.error("GPIO initialization failed:", error);
        return false;
    }
}

function createUI() {
    const screen = UI.getScreen();
    const container = UI.createContainer(screen);
    UI.setContainerLayout(container, UI.LAYOUT_FLEX_COLUMN);
    
    // Title
    const title = UI.createLabel(container, "${appName}");
    UI.setLabelStyle(title, { fontSize: 20, fontWeight: "bold" });
    
    // LED control
    const ledContainer = UI.createContainer(container);
    UI.setContainerLayout(ledContainer, UI.LAYOUT_FLEX_ROW);
    
    const ledLabel = UI.createLabel(ledContainer, "LED:");
    window.ledSwitch = UI.createSwitch(ledContainer);
    UI.setSwitchCallback(window.ledSwitch, onLedToggle);
    
    // Button status
    window.buttonStatus = UI.createLabel(container, "Button: Released");
    UI.setLabelStyle(window.buttonStatus, { marginTop: 20 });
    
    // Sensor reading
    window.sensorReading = UI.createLabel(container, "Sensor: Reading...");
    UI.setLabelStyle(window.sensorReading, { marginTop: 10 });
    
    // Manual controls
    const controlContainer = UI.createContainer(container);
    UI.setContainerLayout(controlContainer, UI.LAYOUT_FLEX_ROW);
    
    const readBtn = UI.createButton(controlContainer, "Read All");
    UI.setButtonCallback(readBtn, readAllPins);
    
    const testBtn = UI.createButton(controlContainer, "LED Test");
    UI.setButtonCallback(testBtn, ledBlinkTest);
}

function onLedToggle(state) {
    try {
        GPIO.digitalWrite(GPIO_PINS.LED, state ? GPIO.HIGH : GPIO.LOW);
        console.log(\`LED turned \${state ? "ON" : "OFF"}\`);
    } catch (error) {
        console.error("LED control error:", error);
    }
}

function onButtonPress() {
    console.log("Button pressed!");
    UI.setLabelText(window.buttonStatus, "Button: Pressed");
    Notification.show("Button pressed!", 1000);
    
    // Reset status after delay
    setTimeout(function() {
        UI.setLabelText(window.buttonStatus, "Button: Released");
    }, 1000);
}

function startSensorReading() {
    setInterval(function() {
        try {
            const value = GPIO.analogRead(GPIO_PINS.SENSOR);
            UI.setLabelText(window.sensorReading, \`Sensor: \${value}\`);
        } catch (error) {
            UI.setLabelText(window.sensorReading, "Sensor: Error");
            console.error("Sensor reading error:", error);
        }
    }, 1000);
}

function readAllPins() {
    try {
        const ledState = GPIO.digitalRead(GPIO_PINS.LED);
        const buttonState = GPIO.digitalRead(GPIO_PINS.BUTTON);
        const sensorValue = GPIO.analogRead(GPIO_PINS.SENSOR);
        
        console.log("Pin states:", {
            LED: ledState,
            BUTTON: buttonState,
            SENSOR: sensorValue
        });
        
        Notification.show("Pin states logged to console", 2000);
    } catch (error) {
        console.error("Pin reading error:", error);
    }
}

function ledBlinkTest() {
    let count = 0;
    const blinkInterval = setInterval(function() {
        const state = count % 2 === 0;
        GPIO.digitalWrite(GPIO_PINS.LED, state ? GPIO.HIGH : GPIO.LOW);
        count++;
        
        if (count >= 6) {
            clearInterval(blinkInterval);
            GPIO.digitalWrite(GPIO_PINS.LED, GPIO.LOW);
            Notification.show("Blink test completed", 2000);
        }
    }, 200);
}

function setupEventHandlers() {
    System.onBackButton(function() {
        // Clean up GPIO
        GPIO.digitalWrite(GPIO_PINS.LED, GPIO.LOW);
        System.exit();
    });
}

init();
`;
  }

  generateReadme(appName, manifest) {
    return `# ${appName}

${manifest.description}

## Description

This is a JavaScript application for the T-Embed CC1101 device.

## Features

- Cross-platform JavaScript execution
- Hardware abstraction layer
- Event-driven architecture
- Permission-based security

## Development

This app was created using the T-Embed CC1101 SDK v${SDK_VERSION}.

### Required Permissions

${manifest.permissions.length > 0 ? 
  manifest.permissions.map(p => `- \`${p}\``).join('\n') : 
  'No special permissions required.'}

### Files

- \`app.js\` - Main application logic
- \`manifest.json\` - App metadata and configuration
- \`assets/\` - App resources (icons, images, etc.)

## Installation

1. Copy the app folder to the device's \`/apps\` directory
2. Grant required permissions through the device settings
3. Launch the app from the main menu

## API Reference

See the [T-Embed CC1101 API Documentation](https://github.com/your-repo/t-embed-cc1101-fw/docs/api) for available JavaScript APIs.

## License

${manifest.license || 'MIT'}

---

Generated with T-Embed SDK v${SDK_VERSION}
`;
  }

  /**
   * Validate an app project
   */
  validateApp(appPath) {
    const errors = [];
    const warnings = [];
    
    // Check manifest
    const manifestPath = path.join(appPath, 'manifest.json');
    if (!fs.existsSync(manifestPath)) {
      errors.push('Missing manifest.json file');
    } else {
      try {
        const manifest = JSON.parse(fs.readFileSync(manifestPath, 'utf8'));
        
        // Validate required fields
        const required = ['manifest_version', 'name', 'version', 'main'];
        for (const field of required) {
          if (!manifest[field]) {
            errors.push(\`Missing required field in manifest: \${field}\`);
          }
        }
        
        // Check main file exists
        if (manifest.main) {
          const mainPath = path.join(appPath, manifest.main);
          if (!fs.existsSync(mainPath)) {
            errors.push(\`Main file not found: \${manifest.main}\`);
          }
        }
        
        // Validate permissions
        if (manifest.permissions) {
          const validPermissions = Object.values(PERMISSIONS);
          for (const perm of manifest.permissions) {
            if (!validPermissions.includes(perm)) {
              warnings.push(\`Unknown permission: \${perm}\`);
            }
          }
        }
        
      } catch (e) {
        errors.push(\`Invalid manifest.json: \${e.message}\`);
      }
    }
    
    return { errors, warnings };
  }

  /**
   * Get available API documentation
   */
  getAPIDocumentation() {
    return {
      RF: {
        description: "Radio Frequency communication",
        methods: [
          "setFrequency(freq)",
          "setModulation(mod)",
          "transmit(data, callback)",
          "startReceive(callback)",
          "stopReceive()"
        ]
      },
      GPIO: {
        description: "General Purpose Input/Output",
        methods: [
          "pinMode(pin, mode)",
          "digitalWrite(pin, value)",
          "digitalRead(pin)",
          "analogRead(pin)"
        ]
      },
      UI: {
        description: "User Interface components",
        methods: [
          "createLabel(parent, text)",
          "createButton(parent, text)",
          "createSlider(parent)",
          "setCallback(element, callback)"
        ]
      },
      Storage: {
        description: "File system operations",
        methods: [
          "readFile(path)",
          "writeFile(path, data)",
          "deleteFile(path)",
          "listFiles(path)"
        ]
      }
    };
  }
}

// Export SDK
module.exports = {
  TEmbedSDK,
  PERMISSIONS,
  API_CATEGORIES,
  SDK_VERSION
};

// CLI interface
if (require.main === module) {
  console.log(\`T-Embed CC1101 SDK v\${SDK_VERSION}\`);
  console.log('Use the CLI tools in ./tools/ directory for development');
}