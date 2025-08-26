/**
 * LED Controller - T-Embed CC1101 GPIO Control App
 * Simple GPIO controller with LED control and button handling
 */

// GPIO Pin Configuration
const GPIO_CONFIG = {
    LED_RED: 2,
    LED_GREEN: 3,
    LED_BLUE: 4,
    BUTTON_1: 0,
    BUTTON_2: 1,
    SENSOR: 6
};

// App state
let appState = {
    leds: { red: false, green: false, blue: false },
    buttons: { button1: 0, button2: 0 },
    sensorValue: 0
};

// UI elements
let ui = {};

// App initialization
function init() {
    console.log("LED Controller starting...");
    
    try {
        initializeGPIO();
        createUI();
        setupEventHandlers();
        startSensorReading();
        
        console.log("LED Controller initialized successfully");
        updateStatus("Ready - GPIO controller active");
        
    } catch (error) {
        console.error("LED Controller initialization error:", error);
        Notification.showError("LED Controller failed to start: " + error.message);
    }
}

// Initialize GPIO pins
function initializeGPIO() {
    // Configure LED pins as outputs
    GPIO.pinMode(GPIO_CONFIG.LED_RED, GPIO.OUTPUT);
    GPIO.pinMode(GPIO_CONFIG.LED_GREEN, GPIO.OUTPUT);
    GPIO.pinMode(GPIO_CONFIG.LED_BLUE, GPIO.OUTPUT);
    
    // Configure button pins as inputs with pullup
    GPIO.pinMode(GPIO_CONFIG.BUTTON_1, GPIO.INPUT_PULLUP);
    GPIO.pinMode(GPIO_CONFIG.BUTTON_2, GPIO.INPUT_PULLUP);
    
    // Configure sensor pin
    GPIO.pinMode(GPIO_CONFIG.SENSOR, GPIO.INPUT);
    
    // Set up button interrupts
    GPIO.setInterrupt(GPIO_CONFIG.BUTTON_1, GPIO.FALLING, onButton1Press);
    GPIO.setInterrupt(GPIO_CONFIG.BUTTON_2, GPIO.FALLING, onButton2Press);
    
    // Initialize all LEDs to OFF
    setAllLEDs(false);
    
    console.log("GPIO initialized successfully");
}

// Create user interface
function createUI() {
    const screen = UI.getScreen();
    const container = UI.createContainer(screen);
    UI.setContainerLayout(container, UI.LAYOUT_FLEX_COLUMN);
    
    // Status display
    ui.statusLabel = UI.createLabel(container, "Ready");
    UI.setLabelStyle(ui.statusLabel, {
        fontSize: 16,
        fontWeight: "bold",
        textAlign: "center",
        marginBottom: 20
    });
    
    // LED controls
    ui.redSwitch = createLEDControl(container, "Red LED", "red");
    ui.greenSwitch = createLEDControl(container, "Green LED", "green");
    ui.blueSwitch = createLEDControl(container, "Blue LED", "blue");
    
    // All LEDs control
    const allContainer = UI.createContainer(container);
    UI.setContainerLayout(allContainer, UI.LAYOUT_FLEX_ROW);
    
    const allOnButton = UI.createButton(allContainer, "All On");
    const allOffButton = UI.createButton(allContainer, "All Off");
    const testButton = UI.createButton(allContainer, "Test");
    
    UI.setButtonCallback(allOnButton, () => setAllLEDs(true));
    UI.setButtonCallback(allOffButton, () => setAllLEDs(false));
    UI.setButtonCallback(testButton, runLEDTest);
    
    // Button status
    ui.button1Status = UI.createLabel(container, "Button 1: Released (0)");
    ui.button2Status = UI.createLabel(container, "Button 2: Released (0)");
    
    // Sensor reading
    ui.sensorLabel = UI.createLabel(container, "Sensor: 0");
    UI.setLabelStyle(ui.sensorLabel, { fontSize: 14, marginTop: 10 });
}

// Create individual LED control
function createLEDControl(parent, name, color) {
    const container = UI.createContainer(parent);
    UI.setContainerLayout(container, UI.LAYOUT_FLEX_ROW);
    
    const label = UI.createLabel(container, name);
    UI.setLabelStyle(label, { minWidth: 100 });
    
    const switch1 = UI.createSwitch(container);
    UI.setSwitchCallback(switch1, (state) => setLED(color, state));
    
    return switch1;
}

// Event handlers
function setupEventHandlers() {
    System.onBackButton(function() {
        console.log("Stopping LED Controller...");
        cleanup();
        System.exit();
    });
    
    System.onPause(function() {
        console.log("LED Controller paused");
    });
    
    System.onResume(function() {
        console.log("LED Controller resumed");
        updateStatus("Resumed - Ready");
    });
}

// Button event handlers
function onButton1Press() {
    appState.buttons.button1++;
    updateButtonStatus();
    console.log("Button 1 pressed, count:", appState.buttons.button1);
    Notification.show("Button 1 pressed!", 1000);
}

function onButton2Press() {
    appState.buttons.button2++;
    updateButtonStatus();
    console.log("Button 2 pressed, count:", appState.buttons.button2);
    Notification.show("Button 2 pressed!", 1000);
    
    // Toggle all LEDs on button 2
    const anyLEDOn = appState.leds.red || appState.leds.green || appState.leds.blue;
    setAllLEDs(!anyLEDOn);
}

// LED control functions
function setLED(color, state) {
    appState.leds[color] = state;
    
    const pin = color === 'red' ? GPIO_CONFIG.LED_RED :
               color === 'green' ? GPIO_CONFIG.LED_GREEN :
               GPIO_CONFIG.LED_BLUE;
    
    try {
        GPIO.digitalWrite(pin, state ? GPIO.HIGH : GPIO.LOW);
        console.log(`${color} LED ${state ? 'ON' : 'OFF'}`);
        updateStatus(`${color} LED ${state ? 'enabled' : 'disabled'}`);
        
    } catch (error) {
        console.error(`Failed to control ${color} LED:`, error);
        Notification.showError(`LED control failed: ${error.message}`);
    }
}

function setAllLEDs(state) {
    setLED('red', state);
    setLED('green', state);
    setLED('blue', state);
    
    // Update UI switches
    UI.setSwitchState(ui.redSwitch, state);
    UI.setSwitchState(ui.greenSwitch, state);
    UI.setSwitchState(ui.blueSwitch, state);
    
    updateStatus(`All LEDs ${state ? 'enabled' : 'disabled'}`);
}

function runLEDTest() {
    updateStatus("Running LED test sequence...");
    
    // Turn off all LEDs first
    setAllLEDs(false);
    
    // Test sequence: Red -> Green -> Blue -> All -> Off
    setTimeout(() => setLED('red', true), 200);
    setTimeout(() => { setLED('red', false); setLED('green', true); }, 700);
    setTimeout(() => { setLED('green', false); setLED('blue', true); }, 1200);
    setTimeout(() => { setLED('blue', false); setAllLEDs(true); }, 1700);
    setTimeout(() => { setAllLEDs(false); updateStatus("Test completed"); }, 2500);
}

// Sensor functions
function startSensorReading() {
    setInterval(readSensor, 1000);
}

function readSensor() {
    try {
        const sensorValue = GPIO.analogRead(GPIO_CONFIG.SENSOR);
        appState.sensorValue = sensorValue;
        
        UI.setLabelText(ui.sensorLabel, `Sensor: ${sensorValue}`);
        
        // Auto-adjust LEDs based on sensor value
        if (sensorValue > 3000) {
            setLED('red', true);
        } else if (sensorValue > 2000) {
            setLED('green', true);
        } else if (sensorValue > 1000) {
            setLED('blue', true);
        }
        
    } catch (error) {
        console.error("Failed to read sensor:", error);
        UI.setLabelText(ui.sensorLabel, "Sensor: Error");
    }
}

// UI update functions
function updateStatus(status) {
    UI.setLabelText(ui.statusLabel, status);
}

function updateButtonStatus() {
    UI.setLabelText(ui.button1Status, `Button 1: Released (${appState.buttons.button1})`);
    UI.setLabelText(ui.button2Status, `Button 2: Released (${appState.buttons.button2})`);
}

// Cleanup function
function cleanup() {
    console.log("Cleaning up LED Controller...");
    setAllLEDs(false);
}

// App entry point
init();