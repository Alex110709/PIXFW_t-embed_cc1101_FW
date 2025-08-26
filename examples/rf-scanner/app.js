/**
 * RF Scanner - T-Embed CC1101 RF Signal Scanner
 * Advanced scanner with frequency sweeping and signal analysis
 */

// Configuration
const SCANNER_CONFIG = {
    frequencies: [315.0, 433.92, 868.0, 915.0],
    customFrequency: 433.92,
    sweepStep: 0.1,
    sweepRange: 5.0, // MHz around center frequency
    defaultModulation: 'ASK',
    scanTimeout: 5000, // 5 seconds per frequency
    signalThreshold: -80 // dBm
};

// Global state
let scannerState = {
    isScanning: false,
    currentFrequency: SCANNER_CONFIG.frequencies[0],
    scanResults: [],
    sweepMode: false,
    signalHistory: []
};

// UI elements
let ui = {};

// App initialization
function init() {
    console.log("RF Scanner starting...");
    
    try {
        // Initialize RF module
        if (!initializeRF()) {
            throw new Error("RF initialization failed");
        }
        
        // Create UI
        createUI();
        
        // Setup event handlers
        setupEventHandlers();
        
        // Load previous settings
        loadSettings();
        
        console.log("RF Scanner initialized successfully");
        updateStatus("Ready to scan");
        
    } catch (error) {
        console.error("RF Scanner initialization error:", error);
        Notification.showError("RF Scanner failed to start: " + error.message);
    }
}

// Initialize RF module
function initializeRF() {
    try {
        // Set initial configuration
        RF.setFrequency(scannerState.currentFrequency);
        RF.setModulation(RF.MOD_ASK);
        RF.setDataRate(4.8);
        RF.setPower(10);
        
        console.log("RF module initialized:", {
            frequency: scannerState.currentFrequency,
            modulation: "ASK",
            dataRate: 4.8,
            power: 10
        });
        
        return true;
    } catch (error) {
        console.error("RF initialization failed:", error);
        return false;
    }
}

// Create user interface
function createUI() {
    const screen = UI.getScreen();
    
    // Create tab view for different modes
    const tabView = UI.createTabView(screen);
    
    // Scanner tab
    const scannerTab = UI.addTab(tabView, "Scanner");
    createScannerTab(scannerTab);
    
    // Frequency tab
    const frequencyTab = UI.addTab(tabView, "Frequency");
    createFrequencyTab(frequencyTab);
    
    // Results tab
    const resultsTab = UI.addTab(tabView, "Results");
    createResultsTab(resultsTab);
    
    // Settings tab
    const settingsTab = UI.addTab(tabView, "Settings");
    createSettingsTab(settingsTab);
}

// Create scanner tab
function createScannerTab(parent) {
    const container = UI.createContainer(parent);
    UI.setContainerLayout(container, UI.LAYOUT_FLEX_COLUMN);
    
    // Status section
    ui.statusLabel = UI.createLabel(container, "Ready");
    UI.setLabelStyle(ui.statusLabel, {
        fontSize: 16,
        fontWeight: "bold",
        textAlign: "center",
        marginBottom: 10
    });
    
    // Frequency display
    ui.frequencyLabel = UI.createLabel(container, `${scannerState.currentFrequency.toFixed(2)} MHz`);
    UI.setLabelStyle(ui.frequencyLabel, {
        fontSize: 20,
        textAlign: "center",
        marginBottom: 20
    });
    
    // Control buttons
    const buttonContainer = UI.createContainer(container);
    UI.setContainerLayout(buttonContainer, UI.LAYOUT_FLEX_ROW);
    
    ui.scanButton = UI.createButton(buttonContainer, "Start Scan");
    ui.sweepButton = UI.createButton(buttonContainer, "Sweep");
    ui.stopButton = UI.createButton(buttonContainer, "Stop");
    
    UI.setButtonCallback(ui.scanButton, toggleScanning);
    UI.setButtonCallback(ui.sweepButton, startFrequencySweep);
    UI.setButtonCallback(ui.stopButton, stopAllScanning);
    
    // Signal strength indicator
    const signalContainer = UI.createContainer(container);
    UI.setContainerLayout(signalContainer, UI.LAYOUT_FLEX_COLUMN);
    
    const signalLabel = UI.createLabel(signalContainer, "Signal Strength:");
    ui.signalMeter = UI.createProgressBar(signalContainer);
    ui.signalValue = UI.createLabel(signalContainer, "-120 dBm");
    
    UI.setProgressBarRange(ui.signalMeter, -120, -20);
    UI.setProgressBarValue(ui.signalMeter, -120);
    
    // Activity log
    ui.activityLog = UI.createTextArea(container);
    UI.setTextAreaPlaceholder(ui.activityLog, "Scan activity will appear here...");
    UI.setTextAreaReadOnly(ui.activityLog, true);
}

// Create frequency tab
function createFrequencyTab(parent) {
    const container = UI.createContainer(parent);
    UI.setContainerLayout(container, UI.LAYOUT_FLEX_COLUMN);
    
    // Frequency presets
    const presetsLabel = UI.createLabel(container, "Frequency Presets:");
    UI.setLabelStyle(presetsLabel, { fontSize: 16, fontWeight: "bold" });
    
    const presetsContainer = UI.createContainer(container);
    UI.setContainerLayout(presetsContainer, UI.LAYOUT_GRID);
    
    SCANNER_CONFIG.frequencies.forEach(freq => {
        const button = UI.createButton(presetsContainer, `${freq} MHz`);
        UI.setButtonCallback(button, () => setFrequency(freq));
        UI.setButtonStyle(button, { margin: 5 });
    });
    
    // Custom frequency input
    const customLabel = UI.createLabel(container, "Custom Frequency (MHz):");
    ui.frequencySlider = UI.createSlider(container);
    UI.setSliderRange(ui.frequencySlider, 300, 928);
    UI.setSliderValue(ui.frequencySlider, scannerState.currentFrequency);
    UI.setSliderCallback(ui.frequencySlider, onFrequencySliderChange);
    
    ui.customFreqLabel = UI.createLabel(container, `${scannerState.currentFrequency.toFixed(2)} MHz`);
    UI.setLabelStyle(ui.customFreqLabel, { textAlign: "center" });
    
    // Set custom frequency button
    const setFreqButton = UI.createButton(container, "Set Frequency");
    UI.setButtonCallback(setFreqButton, setCustomFrequency);
    
    // Modulation settings
    const modLabel = UI.createLabel(container, "Modulation:");
    ui.modulationDropdown = UI.createDropdown(container);
    UI.addDropdownOption(ui.modulationDropdown, "ASK");
    UI.addDropdownOption(ui.modulationDropdown, "FSK");
    UI.addDropdownOption(ui.modulationDropdown, "MSK");
    UI.setDropdownCallback(ui.modulationDropdown, onModulationChange);
}

// Create results tab
function createResultsTab(parent) {
    const container = UI.createContainer(parent);
    UI.setContainerLayout(container, UI.LAYOUT_FLEX_COLUMN);
    
    // Results header
    const headerContainer = UI.createContainer(container);
    UI.setContainerLayout(headerContainer, UI.LAYOUT_FLEX_ROW);
    
    const resultsLabel = UI.createLabel(headerContainer, "Scan Results");
    UI.setLabelStyle(resultsLabel, { fontSize: 16, fontWeight: "bold" });
    
    const clearButton = UI.createButton(headerContainer, "Clear");
    UI.setButtonCallback(clearButton, clearResults);
    
    // Results list
    ui.resultsList = UI.createList(container);
    
    // Export button
    const exportButton = UI.createButton(container, "Export Results");
    UI.setButtonCallback(exportButton, exportResults);
}

// Create settings tab
function createSettingsTab(parent) {
    const container = UI.createContainer(parent);
    UI.setContainerLayout(container, UI.LAYOUT_FLEX_COLUMN);
    
    // Scan timeout setting
    const timeoutLabel = UI.createLabel(container, "Scan Timeout (seconds):");
    ui.timeoutSlider = UI.createSlider(container);
    UI.setSliderRange(ui.timeoutSlider, 1, 30);
    UI.setSliderValue(ui.timeoutSlider, SCANNER_CONFIG.scanTimeout / 1000);
    UI.setSliderCallback(ui.timeoutSlider, onTimeoutChange);
    
    ui.timeoutValueLabel = UI.createLabel(container, `${SCANNER_CONFIG.scanTimeout / 1000}s`);
    
    // Sweep settings
    const sweepLabel = UI.createLabel(container, "Sweep Range (MHz):");
    ui.sweepRangeSlider = UI.createSlider(container);
    UI.setSliderRange(ui.sweepRangeSlider, 0.5, 20);
    UI.setSliderValue(ui.sweepRangeSlider, SCANNER_CONFIG.sweepRange);
    UI.setSliderCallback(ui.sweepRangeSlider, onSweepRangeChange);
    
    ui.sweepRangeLabel = UI.createLabel(container, `${SCANNER_CONFIG.sweepRange} MHz`);
    
    // Signal threshold
    const thresholdLabel = UI.createLabel(container, "Signal Threshold (dBm):");
    ui.thresholdSlider = UI.createSlider(container);
    UI.setSliderRange(ui.thresholdSlider, -120, -30);
    UI.setSliderValue(ui.thresholdSlider, SCANNER_CONFIG.signalThreshold);
    UI.setSliderCallback(ui.thresholdSlider, onThresholdChange);
    
    ui.thresholdLabel = UI.createLabel(container, `${SCANNER_CONFIG.signalThreshold} dBm`);
    
    // Save/Load settings
    const settingsButtonContainer = UI.createContainer(container);
    UI.setContainerLayout(settingsButtonContainer, UI.LAYOUT_FLEX_ROW);
    
    const saveButton = UI.createButton(settingsButtonContainer, "Save");
    const loadButton = UI.createButton(settingsButtonContainer, "Load");
    const resetButton = UI.createButton(settingsButtonContainer, "Reset");
    
    UI.setButtonCallback(saveButton, saveSettings);
    UI.setButtonCallback(loadButton, loadSettings);
    UI.setButtonCallback(resetButton, resetSettings);
}

// Event handlers
function setupEventHandlers() {
    // Handle back button
    System.onBackButton(function() {
        console.log("Stopping RF Scanner...");
        stopAllScanning();
        cleanup();
        System.exit();
    });
    
    // Handle app lifecycle
    System.onPause(function() {
        console.log("RF Scanner paused");
        if (scannerState.isScanning) {
            stopAllScanning();
        }
    });
    
    System.onResume(function() {
        console.log("RF Scanner resumed");
        updateStatus("Resumed - Ready to scan");
    });
}

// Scanning functions
function toggleScanning() {
    if (scannerState.isScanning) {
        stopScanning();
    } else {
        startScanning();
    }
}

function startScanning() {
    if (scannerState.isScanning) return;
    
    try {
        scannerState.isScanning = true;
        UI.setButtonText(ui.scanButton, "Scanning...");
        
        updateStatus(`Scanning ${scannerState.currentFrequency.toFixed(2)} MHz`);
        logActivity(`Starting scan on ${scannerState.currentFrequency.toFixed(2)} MHz`);
        
        // Start RF receiving
        RF.startReceive(onSignalReceived);
        
        // Set scan timeout
        setTimeout(() => {
            if (scannerState.isScanning && !scannerState.sweepMode) {
                stopScanning();
                updateStatus("Scan timeout - No signals detected");
            }
        }, SCANNER_CONFIG.scanTimeout);
        
        console.log("Started scanning on", scannerState.currentFrequency, "MHz");
        
    } catch (error) {
        console.error("Failed to start scanning:", error);
        Notification.showError("Failed to start scan: " + error.message);
        stopScanning();
    }
}

function stopScanning() {
    if (!scannerState.isScanning) return;
    
    try {
        scannerState.isScanning = false;
        UI.setButtonText(ui.scanButton, "Start Scan");
        
        RF.stopReceive();
        
        updateStatus("Scan stopped");
        logActivity("Scan stopped");
        
        console.log("Stopped scanning");
        
    } catch (error) {
        console.error("Error stopping scan:", error);
    }
}

function startFrequencySweep() {
    if (scannerState.isScanning) return;
    
    scannerState.sweepMode = true;
    updateStatus("Starting frequency sweep...");
    logActivity("Starting frequency sweep");
    
    const centerFreq = scannerState.currentFrequency;
    const startFreq = centerFreq - SCANNER_CONFIG.sweepRange / 2;
    const endFreq = centerFreq + SCANNER_CONFIG.sweepRange / 2;
    
    sweepFrequencyRange(startFreq, endFreq, SCANNER_CONFIG.sweepStep);
}

function sweepFrequencyRange(startFreq, endFreq, step) {
    let currentFreq = startFreq;
    
    function sweepNext() {
        if (!scannerState.sweepMode || currentFreq > endFreq) {
            finishSweep();
            return;
        }
        
        setFrequency(currentFreq);
        updateStatus(`Sweeping ${currentFreq.toFixed(2)} MHz`);
        
        startScanning();
        
        setTimeout(() => {
            stopScanning();
            currentFreq += step;
            setTimeout(sweepNext, 100); // Small delay between frequencies
        }, 500); // 500ms per frequency
    }
    
    sweepNext();
}

function finishSweep() {
    scannerState.sweepMode = false;
    updateStatus("Frequency sweep completed");
    logActivity("Frequency sweep completed");
    
    // Return to original frequency
    setFrequency(scannerState.currentFrequency);
}

function stopAllScanning() {
    scannerState.sweepMode = false;
    stopScanning();
}

// Signal handling
function onSignalReceived(data) {
    try {
        const signalInfo = {
            frequency: scannerState.currentFrequency,
            data: data,
            timestamp: new Date().toISOString(),
            strength: -60 // Simulated RSSI
        };
        
        // Add to results
        scannerState.signalHistory.push(signalInfo);
        
        // Update UI
        updateSignalStrength(signalInfo.strength);
        logActivity(`Signal detected: ${data} (${signalInfo.strength} dBm)`);
        
        // Add to results list
        addResultToList(signalInfo);
        
        console.log("Signal received:", signalInfo);
        
        Notification.show(`Signal: ${data}`, 2000);
        
    } catch (error) {
        console.error("Error processing signal:", error);
    }
}

// UI update functions
function updateStatus(status) {
    UI.setLabelText(ui.statusLabel, status);
}

function updateSignalStrength(strength) {
    UI.setProgressBarValue(ui.signalMeter, strength);
    UI.setLabelText(ui.signalValue, `${strength} dBm`);
}

function logActivity(message) {
    const timestamp = new Date().toLocaleTimeString();
    const logLine = `[${timestamp}] ${message}\\n`;
    const currentLog = UI.getTextAreaValue(ui.activityLog);
    UI.setTextAreaValue(ui.activityLog, currentLog + logLine);
}

function addResultToList(signalInfo) {
    const resultText = `${signalInfo.frequency.toFixed(2)} MHz: ${signalInfo.data} (${signalInfo.strength} dBm)`;
    UI.addListItem(ui.resultsList, resultText);
}

// Frequency management
function setFrequency(frequency) {
    try {
        scannerState.currentFrequency = frequency;
        RF.setFrequency(frequency);
        
        // Update UI
        UI.setLabelText(ui.frequencyLabel, `${frequency.toFixed(2)} MHz`);
        UI.setLabelText(ui.customFreqLabel, `${frequency.toFixed(2)} MHz`);
        UI.setSliderValue(ui.frequencySlider, frequency);
        
        console.log("Frequency set to", frequency, "MHz");
        logActivity(`Frequency set to ${frequency.toFixed(2)} MHz`);
        
    } catch (error) {
        console.error("Failed to set frequency:", error);
        Notification.showError("Failed to set frequency: " + error.message);
    }
}

function onFrequencySliderChange(value) {
    UI.setLabelText(ui.customFreqLabel, `${value.toFixed(2)} MHz`);
}

function setCustomFrequency() {
    const frequency = UI.getSliderValue(ui.frequencySlider);
    setFrequency(frequency);
}

function onModulationChange(modulation) {
    try {
        let modType;
        switch (modulation) {
            case "ASK": modType = RF.MOD_ASK; break;
            case "FSK": modType = RF.MOD_FSK; break;
            case "MSK": modType = RF.MOD_MSK; break;
            default: modType = RF.MOD_ASK;
        }
        
        RF.setModulation(modType);
        logActivity(`Modulation set to ${modulation}`);
        console.log("Modulation set to", modulation);
        
    } catch (error) {
        console.error("Failed to set modulation:", error);
        Notification.showError("Failed to set modulation: " + error.message);
    }
}

// Settings functions
function onTimeoutChange(value) {
    SCANNER_CONFIG.scanTimeout = value * 1000;
    UI.setLabelText(ui.timeoutValueLabel, `${value}s`);
}

function onSweepRangeChange(value) {
    SCANNER_CONFIG.sweepRange = value;
    UI.setLabelText(ui.sweepRangeLabel, `${value} MHz`);
}

function onThresholdChange(value) {
    SCANNER_CONFIG.signalThreshold = value;
    UI.setLabelText(ui.thresholdLabel, `${value} dBm`);
}

function saveSettings() {
    try {
        const settings = {
            scanTimeout: SCANNER_CONFIG.scanTimeout,
            sweepRange: SCANNER_CONFIG.sweepRange,
            signalThreshold: SCANNER_CONFIG.signalThreshold,
            currentFrequency: scannerState.currentFrequency
        };
        
        Storage.writeFile("/config/rf-scanner.json", JSON.stringify(settings, null, 2));
        Notification.show("Settings saved", 1500);
        console.log("Settings saved:", settings);
        
    } catch (error) {
        console.error("Failed to save settings:", error);
        Notification.showError("Failed to save settings");
    }
}

function loadSettings() {
    try {
        const data = Storage.readFile("/config/rf-scanner.json");
        const settings = JSON.parse(data);
        
        SCANNER_CONFIG.scanTimeout = settings.scanTimeout || SCANNER_CONFIG.scanTimeout;
        SCANNER_CONFIG.sweepRange = settings.sweepRange || SCANNER_CONFIG.sweepRange;
        SCANNER_CONFIG.signalThreshold = settings.signalThreshold || SCANNER_CONFIG.signalThreshold;
        
        if (settings.currentFrequency) {
            setFrequency(settings.currentFrequency);
        }
        
        // Update UI
        UI.setSliderValue(ui.timeoutSlider, SCANNER_CONFIG.scanTimeout / 1000);
        UI.setSliderValue(ui.sweepRangeSlider, SCANNER_CONFIG.sweepRange);
        UI.setSliderValue(ui.thresholdSlider, SCANNER_CONFIG.signalThreshold);
        
        Notification.show("Settings loaded", 1500);
        console.log("Settings loaded:", settings);
        
    } catch (error) {
        console.log("No saved settings found, using defaults");
    }
}

function resetSettings() {
    SCANNER_CONFIG.scanTimeout = 5000;
    SCANNER_CONFIG.sweepRange = 5.0;
    SCANNER_CONFIG.signalThreshold = -80;
    
    // Update UI
    UI.setSliderValue(ui.timeoutSlider, 5);
    UI.setSliderValue(ui.sweepRangeSlider, 5.0);
    UI.setSliderValue(ui.thresholdSlider, -80);
    
    setFrequency(433.92);
    
    Notification.show("Settings reset to defaults", 1500);
}

// Results management
function clearResults() {
    scannerState.signalHistory = [];
    UI.clearList(ui.resultsList);
    UI.setTextAreaValue(ui.activityLog, "");
    Notification.show("Results cleared", 1000);
}

function exportResults() {
    try {
        const exportData = {
            timestamp: new Date().toISOString(),
            scannerConfig: SCANNER_CONFIG,
            results: scannerState.signalHistory
        };
        
        const filename = `/logs/rf-scan-${Date.now()}.json`;
        Storage.writeFile(filename, JSON.stringify(exportData, null, 2));
        
        Notification.show(`Results exported to ${filename}`, 3000);
        console.log("Results exported to", filename);
        
    } catch (error) {
        console.error("Failed to export results:", error);
        Notification.showError("Failed to export results");
    }
}

// Cleanup function
function cleanup() {
    console.log("Cleaning up RF Scanner...");
    stopAllScanning();
    
    try {
        RF.stopReceive();
    } catch (error) {
        console.log("Error during RF cleanup:", error);
    }
}

// App entry point
init();