/**
 * Spectrum Analyzer App for T-Embed CC1101 (Flipper Zero Style)
 * Implements RF spectrum analysis functionality with Flipper Zero style UI
 */

// App state
let appState = {
    isAnalyzing: false,
    frequencyRangeIndex: 1, // Default to 433 MHz range
    stepSizeIndex: 1, // Default to 100 kHz
    currentFrequency: 433920000,
    maxRSSI: -100,
    maxFrequency: 433920000
};

// Predefined frequency ranges
const FREQUENCY_RANGES = [
    { start: 315000000, stop: 316000000, name: "315 MHz" },
    { start: 433000000, stop: 434000000, name: "433 MHz" },
    { start: 868000000, stop: 869000000, name: "868 MHz" },
    { start: 915000000, stop: 916000000, name: "915 MHz" }
];

// Predefined step sizes
const STEP_SIZES = [
    { size: 50000, name: "50 kHz" },
    { size: 100000, name: "100 kHz" },
    { size: 200000, name: "200 kHz" },
    { size: 500000, name: "500 kHz" }
];

// UI elements
let ui = {};

// App initialization
function init() {
    console.log("Spectrum Analyzer App starting (Flipper Zero style)...");
    
    try {
        createUI();
        updateStatus("[Ready]");
        
        console.log("Spectrum Analyzer App initialized successfully");
    } catch (error) {
        console.error("Spectrum Analyzer App initialization error:", error);
        Notification.showError("Spectrum Analyzer failed to start: " + error.message);
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
    
    const titleLabel = UI.createLabel(titleBar, "[Spectrum Analyzer]");
    UI.setPosition(titleLabel, 5, 2);
    UI.setLabelStyle(titleLabel, { 
        text_color: '#FFFFFF', 
        font_size: 10,
        font_weight: 'bold'
    });
    
    // Frequency range display
    ui.rangeLabel = UI.createLabel(screen, `Range: ${FREQUENCY_RANGES[appState.frequencyRangeIndex].name}`);
    UI.setPosition(ui.rangeLabel, 10, 30);
    UI.setLabelStyle(ui.rangeLabel, { 
        text_color: '#00FF00',
        font_size: 14
    });
    
    // Step size display
    ui.stepLabel = UI.createLabel(screen, `Step: ${STEP_SIZES[appState.stepSizeIndex].name}`);
    UI.setPosition(ui.stepLabel, 10, 55);
    UI.setLabelStyle(ui.stepLabel, { 
        text_color: '#FFFF00',
        font_size: 14
    });
    
    // Current frequency display
    ui.currentFreqLabel = UI.createLabel(screen, `Freq: 433.92 MHz`);
    UI.setPosition(ui.currentFreqLabel, 10, 80);
    UI.setLabelStyle(ui.currentFreqLabel, { 
        text_color: '#FFFFFF',
        font_size: 14
    });
    
    // RSSI display
    ui.rssiLabel = UI.createLabel(screen, `RSSI: --- dBm`);
    UI.setPosition(ui.rssiLabel, 10, 105);
    UI.setLabelStyle(ui.rssiLabel, { 
        text_color: '#00FFFF',
        font_size: 14
    });
    
    // Max signal display
    ui.maxSignalLabel = UI.createLabel(screen, `Max: --- dBm @ --- MHz`);
    UI.setPosition(ui.maxSignalLabel, 10, 130);
    UI.setLabelStyle(ui.maxSignalLabel, { 
        text_color: '#FF00FF',
        font_size: 14
    });
    
    // Status display
    ui.statusLabel = UI.createLabel(screen, "[Stopped]");
    UI.setPosition(ui.statusLabel, 10, 160);
    UI.setLabelStyle(ui.statusLabel, { 
        text_color: '#FFFFFF',
        font_size: 14,
        font_weight: 'bold'
    });
    
    // Spectrum bar (simplified visualization)
    ui.spectrumBar = UI.createLabel(screen, "[                    ]");
    UI.setPosition(ui.spectrumBar, 10, 190);
    UI.setLabelStyle(ui.spectrumBar, { 
        text_color: '#00FF00',
        font_size: 14,
        font_family: 'monospace'
    });
    
    // Status bar
    const statusBar = UI.createContainer(screen);
    UI.setSize(statusBar, 170, 30);
    UI.setPosition(statusBar, 0, 290);
    UI.setContainerStyle(statusBar, { bg_color: '#000000' });
    
    ui.statusBarLabel = UI.createLabel(statusBar, "[←→: Range] [↑↓: Step] [OK: Start/Stop]");
    UI.setPosition(ui.statusBarLabel, 5, 10);
    UI.setLabelStyle(ui.statusBarLabel, { 
        text_color: '#888888',
        font_size: 8
    });
    
    UI.setActiveScreen(screen);
}

// Update frequency range display
function updateFrequencyRangeDisplay() {
    UI.setLabelText(ui.rangeLabel, `Range: ${FREQUENCY_RANGES[appState.frequencyRangeIndex].name}`);
}

// Update step size display
function updateStepSizeDisplay() {
    UI.setLabelText(ui.stepLabel, `Step: ${STEP_SIZES[appState.stepSizeIndex].name}`);
}

// Format frequency for display
function formatFrequency(freq) {
    if (freq >= 1000000) {
        return (freq / 1000000).toFixed(3) + " MHz";
    } else if (freq >= 1000) {
        return (freq / 1000).toFixed(3) + " kHz";
    } else {
        return freq + " Hz";
    }
}

// Start spectrum analysis
function startAnalysis() {
    try {
        const range = FREQUENCY_RANGES[appState.frequencyRangeIndex];
        const step = STEP_SIZES[appState.stepSizeIndex];
        
        // Start spectrum analyzer
        RF.startSpectrumAnalyzer(range.start, range.stop, step.size);
        
        appState.isAnalyzing = true;
        appState.currentFrequency = range.start;
        appState.maxRSSI = -100;
        appState.maxFrequency = range.start;
        
        updateStatus("[Analyzing...]");
        UI.setLabelText(ui.statusBarLabel, "[OK: Stop] [Back: Exit]");
        
        // Start scanning loop
        scanLoop();
        
        Notification.show(`Analyzing ${range.name}`, 2000);
        
    } catch (error) {
        console.error("Failed to start analysis:", error);
        Notification.showError("Failed to start analysis: " + error.message);
    }
}

// Stop spectrum analysis
function stopAnalysis() {
    try {
        RF.stopSpectrumAnalyzer();
        
        appState.isAnalyzing = false;
        updateStatus("[Stopped]");
        UI.setLabelText(ui.statusBarLabel, "[←→: Range] [↑↓: Step] [OK: Start/Stop]");
        
        Notification.show("Analysis stopped", 2000);
        
    } catch (error) {
        console.error("Failed to stop analysis:", error);
        Notification.showError("Failed to stop analysis: " + error.message);
    }
}

// Scanning loop
function scanLoop() {
    if (!appState.isAnalyzing) return;
    
    try {
        const range = FREQUENCY_RANGES[appState.frequencyRangeIndex];
        const step = STEP_SIZES[appState.stepSizeIndex];
        
        // Get RSSI at current frequency
        const rssi = RF.getRssiAtFrequency(appState.currentFrequency);
        
        // Update displays
        UI.setLabelText(ui.currentFreqLabel, `Freq: ${formatFrequency(appState.currentFrequency)}`);
        UI.setLabelText(ui.rssiLabel, `RSSI: ${rssi} dBm`);
        
        // Update max signal if needed
        if (rssi > appState.maxRSSI) {
            appState.maxRSSI = rssi;
            appState.maxFrequency = appState.currentFrequency;
            UI.setLabelText(ui.maxSignalLabel, 
                `Max: ${appState.maxRSSI} dBm @ ${formatFrequency(appState.maxFrequency)}`);
        }
        
        // Update spectrum bar visualization (simplified)
        const barLength = 20;
        const rssiRange = 50; // -100 to -50 dBm
        const normalizedRSSI = Math.max(0, Math.min(1, (rssi + 100) / rssiRange));
        const filledLength = Math.floor(normalizedRSSI * barLength);
        const bar = "[" + "=".repeat(filledLength) + " ".repeat(barLength - filledLength) + "]";
        UI.setLabelText(ui.spectrumBar, bar);
        
        // Move to next frequency
        appState.currentFrequency += step.size;
        
        // Check if we've reached the end
        if (appState.currentFrequency > range.stop) {
            appState.currentFrequency = range.start;
        }
        
        // Schedule next scan
        setTimeout(scanLoop, 50);
        
    } catch (error) {
        console.error("Error during scanning:", error);
        stopAnalysis();
    }
}

// Toggle analysis state
function toggleAnalysis() {
    if (appState.isAnalyzing) {
        stopAnalysis();
    } else {
        startAnalysis();
    }
}

// Change frequency range
function changeFrequencyRange(direction) {
    const newIndex = appState.frequencyRangeIndex + direction;
    
    if (newIndex >= 0 && newIndex < FREQUENCY_RANGES.length) {
        appState.frequencyRangeIndex = newIndex;
        updateFrequencyRangeDisplay();
        Notification.vibrate(50); // Haptic feedback
    }
}

// Change step size
function changeStepSize(direction) {
    const newIndex = appState.stepSizeIndex + direction;
    
    if (newIndex >= 0 && newIndex < STEP_SIZES.length) {
        appState.stepSizeIndex = newIndex;
        updateStepSizeDisplay();
        Notification.vibrate(50); // Haptic feedback
    }
}

// UI update functions
function updateStatus(status) {
    UI.setLabelText(ui.statusLabel, status);
}

// Cleanup function
function cleanup() {
    console.log("Cleaning up Spectrum Analyzer App...");
    if (appState.isAnalyzing) {
        stopAnalysis();
    }
}

// Event handlers
function setupEventHandlers() {
    // Encoder rotation for frequency range
    System.onEncoder((direction) => {
        if (!appState.isAnalyzing) {
            if (direction === 'CW') {
                changeFrequencyRange(1);
            } else if (direction === 'CCW') {
                changeFrequencyRange(-1);
            }
        }
    });
    
    // Encoder press for toggle
    System.onButton('ENCODER', () => {
        toggleAnalysis();
    });
    
    // Button 1 for step size change
    System.onButton('BUTTON1', () => {
        if (!appState.isAnalyzing) {
            changeStepSize(1);
        }
    });
    
    // Button 2 for back to menu
    System.onButton('BUTTON2', () => {
        console.log("Stopping Spectrum Analyzer App...");
        cleanup();
        System.exit();
    });
    
    // Up/Down buttons for step size
    System.onButton('UP', () => {
        if (!appState.isAnalyzing) {
            changeStepSize(-1);
        }
    });
    
    System.onButton('DOWN', () => {
        if (!appState.isAnalyzing) {
            changeStepSize(1);
        }
    });
    
    System.onBackButton(function() {
        console.log("Stopping Spectrum Analyzer App...");
        cleanup();
        System.exit();
    });
    
    System.onPause(function() {
        console.log("Spectrum Analyzer App paused");
        if (appState.isAnalyzing) {
            stopAnalysis();
        }
    });
    
    System.onResume(function() {
        console.log("Spectrum Analyzer App resumed");
        updateStatus(appState.isAnalyzing ? "[Analyzing...]" : "[Stopped]");
    });
}

// App entry point
init();
setupEventHandlers();