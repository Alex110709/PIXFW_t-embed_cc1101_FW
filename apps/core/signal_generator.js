/**
 * Signal Generator App for T-Embed CC1101 (Flipper Zero Style)
 * Implements RF signal generation functionality with Flipper Zero style UI
 */

// App state
let appState = {
    frequencyIndex: 1, // Default to 433.92 MHz
    modulationIndex: 0, // Default to ASK
    data: [0x01, 0x02, 0x03, 0x04],
    isTransmitting: false
};

// Predefined frequencies
const FREQUENCIES = [
    { freq: 315000000, name: "315 MHz" },
    { freq: 433920000, name: "433.92 MHz" },
    { freq: 868300000, name: "868.3 MHz" },
    { freq: 915000000, name: "915 MHz" }
];

// Modulation types
const MODULATIONS = [
    { id: "ASK_OOK", name: "ASK/OOK" },
    { id: "GFSK", name: "GFSK" },
    { id: "MSK", name: "MSK" },
    { id: "2FSK", name: "2FSK" }
];

// UI elements
let ui = {};

// App initialization
function init() {
    console.log("Signal Generator App starting (Flipper Zero style)...");
    
    try {
        createUI();
        updateStatus("[Ready]");
        
        console.log("Signal Generator App initialized successfully");
    } catch (error) {
        console.error("Signal Generator App initialization error:", error);
        Notification.showError("Signal Generator failed to start: " + error.message);
    }
}

// Create user interface
function createUI() {
    const screen = UI.getScreen();
    UI.setScreenStyle(screen, { bg_color: '#000000' });
    
    // Title bar
    const titleBar = UI.createContainer(screen);
    UI.setSize(titleBar, 170, 20);
    UI.setPosition(titleBar, 0, 0);
    UI.setContainerStyle(titleBar, { bg_color: '#1976D2' });
    
    const titleLabel = UI.createLabel(titleBar, "[Signal Generator]");
    UI.setPosition(titleLabel, 5, 2);
    UI.setLabelStyle(titleLabel, { 
        text_color: '#FFFFFF', 
        font_size: 10,
        font_weight: 'bold'
    });
    
    // Frequency display
    ui.freqLabel = UI.createLabel(screen, `Frequency: ${FREQUENCIES[appState.frequencyIndex].name}`);
    UI.setPosition(ui.freqLabel, 10, 30);
    UI.setLabelStyle(ui.freqLabel, { 
        text_color: '#00FF00',
        font_size: 14
    });
    
    // Modulation display
    ui.modLabel = UI.createLabel(screen, `Modulation: ${MODULATIONS[appState.modulationIndex].name}`);
    UI.setPosition(ui.modLabel, 10, 55);
    UI.setLabelStyle(ui.modLabel, { 
        text_color: '#FFFF00',
        font_size: 14
    });
    
    // Data display
    const dataStr = appState.data.map(b => "0x" + b.toString(16).padStart(2, '0')).join(" ");
    ui.dataLabel = UI.createLabel(screen, `Data: ${dataStr}`);
    UI.setPosition(ui.dataLabel, 10, 80);
    UI.setLabelStyle(ui.dataLabel, { 
        text_color: '#00FFFF',
        font_size: 14
    });
    
    // Status display
    ui.statusLabel = UI.createLabel(screen, "[Ready]");
    UI.setPosition(ui.statusLabel, 10, 110);
    UI.setLabelStyle(ui.statusLabel, { 
        text_color: '#FFFFFF',
        font_size: 14,
        font_weight: 'bold'
    });
    
    // Status bar
    const statusBar = UI.createContainer(screen);
    UI.setSize(statusBar, 170, 30);
    UI.setPosition(statusBar, 0, 290);
    UI.setContainerStyle(statusBar, { bg_color: '#000000' });
    
    ui.statusBarLabel = UI.createLabel(statusBar, "[←→: Frequency] [↑↓: Mod] [OK: Transmit]");
    UI.setPosition(ui.statusBarLabel, 5, 10);
    UI.setLabelStyle(ui.statusBarLabel, { 
        text_color: '#888888',
        font_size: 8
    });
    
    UI.setActiveScreen(screen);
}

// Update frequency display
function updateFrequencyDisplay() {
    UI.setLabelText(ui.freqLabel, `Frequency: ${FREQUENCIES[appState.frequencyIndex].name}`);
}

// Update modulation display
function updateModulationDisplay() {
    UI.setLabelText(ui.modLabel, `Modulation: ${MODULATIONS[appState.modulationIndex].name}`);
}

// Update data display
function updateDataDisplay() {
    const dataStr = appState.data.map(b => "0x" + b.toString(16).padStart(2, '0')).join(" ");
    UI.setLabelText(ui.dataLabel, `Data: ${dataStr}`);
}

// Transmit signal
function transmitSignal() {
    if (appState.isTransmitting) return;
    
    try {
        appState.isTransmitting = true;
        updateStatus("[Transmitting...]");
        UI.setLabelText(ui.statusBarLabel, "[Transmitting...] [Back: Exit]");
        
        // Set frequency
        const frequency = FREQUENCIES[appState.frequencyIndex].freq;
        RF.setFrequency(frequency);
        
        // Set modulation
        const modulation = MODULATIONS[appState.modulationIndex].id;
        RF.setModulation(modulation);
        
        // Transmit data
        RF.transmit(appState.data);
        
        // Show notification
        Notification.show(`Transmitted on ${FREQUENCIES[appState.frequencyIndex].name}`, 2000);
        
        // Reset status after a short delay
        setTimeout(() => {
            appState.isTransmitting = false;
            updateStatus("[Ready]");
            UI.setLabelText(ui.statusBarLabel, "[←→: Frequency] [↑↓: Mod] [OK: Transmit]");
        }, 1000);
        
    } catch (error) {
        console.error("Failed to transmit signal:", error);
        Notification.showError("Failed to transmit: " + error.message);
        appState.isTransmitting = false;
        updateStatus("[Error]");
    }
}

// Change frequency
function changeFrequency(direction) {
    const newIndex = appState.frequencyIndex + direction;
    
    if (newIndex >= 0 && newIndex < FREQUENCIES.length) {
        appState.frequencyIndex = newIndex;
        updateFrequencyDisplay();
        Notification.vibrate(50); // Haptic feedback
    }
}

// Change modulation
function changeModulation(direction) {
    const newIndex = appState.modulationIndex + direction;
    
    if (newIndex >= 0 && newIndex < MODULATIONS.length) {
        appState.modulationIndex = newIndex;
        updateModulationDisplay();
        Notification.vibrate(50); // Haptic feedback
    }
}

// UI update functions
function updateStatus(status) {
    UI.setLabelText(ui.statusLabel, status);
}

// Cleanup function
function cleanup() {
    console.log("Cleaning up Signal Generator App...");
}

// Event handlers
function setupEventHandlers() {
    // Encoder rotation for frequency
    System.onEncoder((direction) => {
        if (!appState.isTransmitting) {
            if (direction === 'CW') {
                changeFrequency(1);
            } else if (direction === 'CCW') {
                changeFrequency(-1);
            }
        }
    });
    
    // Encoder press for transmit
    System.onButton('ENCODER', () => {
        if (!appState.isTransmitting) {
            transmitSignal();
        }
    });
    
    // Button 1 for modulation change
    System.onButton('BUTTON1', () => {
        if (!appState.isTransmitting) {
            changeModulation(1);
        }
    });
    
    // Button 2 for back to menu
    System.onButton('BUTTON2', () => {
        console.log("Stopping Signal Generator App...");
        cleanup();
        System.exit();
    });
    
    // Up/Down buttons for modulation
    System.onButton('UP', () => {
        if (!appState.isTransmitting) {
            changeModulation(-1);
        }
    });
    
    System.onButton('DOWN', () => {
        if (!appState.isTransmitting) {
            changeModulation(1);
        }
    });
    
    System.onBackButton(function() {
        console.log("Stopping Signal Generator App...");
        cleanup();
        System.exit();
    });
    
    System.onPause(function() {
        console.log("Signal Generator App paused");
    });
    
    System.onResume(function() {
        console.log("Signal Generator App resumed");
        updateStatus("[Ready]");
    });
}

// App entry point
init();
setupEventHandlers();