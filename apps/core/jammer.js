/**
 * RF Jammer App for T-Embed CC1101 (Flipper Zero Style)
 * Implements continuous RF jamming functionality with Flipper Zero style UI
 */

// App state
let appState = {
    isJamming: false,
    frequencyIndex: 1, // Default to 433.92 MHz
    jammerType: "full" // "full" or "intermittent"
};

// Predefined frequencies
const FREQUENCIES = [
    { freq: 315000000, name: "315 MHz" },
    { freq: 433920000, name: "433.92 MHz" },
    { freq: 868300000, name: "868.3 MHz" },
    { freq: 915000000, name: "915 MHz" }
];

// Jammer types
const JAMMER_TYPES = [
    { id: "full", name: "Full" },
    { id: "intermittent", name: "Intermittent" }
];

// UI elements
let ui = {};

// App initialization
function init() {
    console.log("RF Jammer App starting (Flipper Zero style)...");
    
    try {
        createUI();
        updateStatus("[Ready]");
        
        console.log("RF Jammer App initialized successfully");
    } catch (error) {
        console.error("RF Jammer App initialization error:", error);
        Notification.showError("RF Jammer failed to start: " + error.message);
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
    
    const titleLabel = UI.createLabel(titleBar, "[RF Jammer]");
    UI.setPosition(titleLabel, 5, 2);
    UI.setLabelStyle(titleLabel, { 
        text_color: '#FFFFFF', 
        font_size: 10,
        font_weight: 'bold'
    });
    
    // Frequency display
    ui.freqLabel = UI.createLabel(screen, `Frequency: ${FREQUENCIES[appState.frequencyIndex].name}`);
    UI.setPosition(ui.freqLabel, 10, 40);
    UI.setLabelStyle(ui.freqLabel, { 
        text_color: '#00FF00',
        font_size: 14
    });
    
    // Jammer type display
    ui.typeLabel = UI.createLabel(screen, `Mode: ${JAMMER_TYPES[0].name}`);
    UI.setPosition(ui.typeLabel, 10, 70);
    UI.setLabelStyle(ui.typeLabel, { 
        text_color: '#FFFF00',
        font_size: 14
    });
    
    // Status display
    ui.statusLabel = UI.createLabel(screen, "[OFF]");
    UI.setPosition(ui.statusLabel, 10, 100);
    UI.setLabelStyle(ui.statusLabel, { 
        text_color: '#FFFFFF',
        font_size: 16,
        font_weight: 'bold'
    });
    
    // Status bar
    const statusBar = UI.createContainer(screen);
    UI.setSize(statusBar, 170, 30);
    UI.setPosition(statusBar, 0, 290);
    UI.setContainerStyle(statusBar, { bg_color: '#000000' });
    
    ui.statusBarLabel = UI.createLabel(statusBar, "[←→: Frequency] [↑↓: Mode] [OK: Toggle]");
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

// Update jammer type display
function updateJammerTypeDisplay() {
    const type = JAMMER_TYPES.find(t => t.id === appState.jammerType);
    UI.setLabelText(ui.typeLabel, `Mode: ${type.name}`);
}

// Start jamming
function startJamming() {
    try {
        const frequency = FREQUENCIES[appState.frequencyIndex].freq;
        
        // Start jammer
        RF.startJammer(frequency);
        
        appState.isJamming = true;
        updateStatus("[ON]");
        UI.setLabelText(ui.statusBarLabel, "[OK: Stop] [Back: Exit]");
        
        Notification.show(`Jamming at ${FREQUENCIES[appState.frequencyIndex].name}`, 2000);
        
    } catch (error) {
        console.error("Failed to start jamming:", error);
        Notification.showError("Failed to start jamming: " + error.message);
    }
}

// Stop jamming
function stopJamming() {
    try {
        RF.stopJammer();
        
        appState.isJamming = false;
        updateStatus("[OFF]");
        UI.setLabelText(ui.statusBarLabel, "[←→: Frequency] [↑↓: Mode] [OK: Toggle]");
        
        Notification.show("Jamming stopped", 2000);
        
    } catch (error) {
        console.error("Failed to stop jamming:", error);
        Notification.showError("Failed to stop jamming: " + error.message);
    }
}

// Toggle jammer state
function toggleJammer() {
    if (appState.isJamming) {
        stopJamming();
    } else {
        startJamming();
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

// Change jammer type
function changeJammerType(direction) {
    const currentIndex = JAMMER_TYPES.findIndex(t => t.id === appState.jammerType);
    const newIndex = currentIndex + direction;
    
    if (newIndex >= 0 && newIndex < JAMMER_TYPES.length) {
        appState.jammerType = JAMMER_TYPES[newIndex].id;
        updateJammerTypeDisplay();
        Notification.vibrate(50); // Haptic feedback
    }
}

// UI update functions
function updateStatus(status) {
    UI.setLabelText(ui.statusLabel, status);
}

// Cleanup function
function cleanup() {
    console.log("Cleaning up RF Jammer App...");
    if (appState.isJamming) {
        stopJamming();
    }
}

// Event handlers
function setupEventHandlers() {
    // Encoder rotation
    System.onEncoder((direction) => {
        if (!appState.isJamming) {
            if (direction === 'CW') {
                changeFrequency(1);
            } else if (direction === 'CCW') {
                changeFrequency(-1);
            }
        }
    });
    
    // Encoder press for toggle
    System.onButton('ENCODER', () => {
        toggleJammer();
    });
    
    // Button 1 for jammer type change
    System.onButton('BUTTON1', () => {
        if (!appState.isJamming) {
            changeJammerType(1);
        }
    });
    
    // Button 2 for back to menu
    System.onButton('BUTTON2', () => {
        console.log("Stopping RF Jammer App...");
        cleanup();
        System.exit();
    });
    
    // Up/Down buttons for jammer type
    System.onButton('UP', () => {
        if (!appState.isJamming) {
            changeJammerType(-1);
        }
    });
    
    System.onButton('DOWN', () => {
        if (!appState.isJamming) {
            changeJammerType(1);
        }
    });
    
    System.onBackButton(function() {
        console.log("Stopping RF Jammer App...");
        cleanup();
        System.exit();
    });
    
    System.onPause(function() {
        console.log("RF Jammer App paused");
        if (appState.isJamming) {
            stopJamming();
        }
    });
    
    System.onResume(function() {
        console.log("RF Jammer App resumed");
        updateStatus(appState.isJamming ? "[ON]" : "[OFF]");
    });
}

// App entry point
init();
setupEventHandlers();