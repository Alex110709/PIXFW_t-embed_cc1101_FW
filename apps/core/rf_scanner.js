/**
 * @file rf_scanner.js
 * @brief RF Scanner Application (Flipper Zero Style)
 * 
 * Scans for RF signals across different frequencies and displays
 * signal information with Flipper Zero style UI.
 */

// Scanner configuration
const SCAN_FREQUENCIES = [
    { freq: 315000000, name: "315 MHz" },
    { freq: 433920000, name: "433.92 MHz" },
    { freq: 868300000, name: "868.3 MHz" },
    { freq: 915000000, name: "915 MHz" }
];

let scannerState = {
    isScanning: false,
    currentFreqIndex: 1, // Default to 433.92 MHz
    signalHistory: [],
    screen: null,
    lastRSSI: -100,
    scanStartTime: 0
};

// Initialize RF Scanner
function initRFScanner() {
    console.log("Initializing RF Scanner (Flipper Zero style)...");
    
    // Check if RF module is available
    if (!RF.isPresent()) {
        Notification.show("Error: CC1101 not detected", 3000);
        return false;
    }
    
    // Create scanner UI
    createScannerUI();
    
    // Initialize RF module
    setupRFModule();
    
    // Set up input handlers
    setupScannerInput();
    
    console.log("RF Scanner initialized");
    return true;
}

// Create scanner user interface
function createScannerUI() {
    scannerState.screen = UI.getScreen();
    UI.setScreenStyle(scannerState.screen, { bg_color: '#000000' });
    
    // Title bar
    const titleBar = UI.createContainer(scannerState.screen);
    UI.setSize(titleBar, 170, 20);
    UI.setPosition(titleBar, 0, 0);
    UI.setContainerStyle(titleBar, { bg_color: '#1976D2' });
    
    const titleLabel = UI.createLabel(titleBar, "[RF Scanner]");
    UI.setPosition(titleLabel, 5, 2);
    UI.setLabelStyle(titleLabel, { 
        text_color: '#FFFFFF', 
        font_size: 10,
        font_weight: 'bold'
    });
    
    // Frequency display
    scannerState.freqLabel = UI.createLabel(scannerState.screen, SCAN_FREQUENCIES[scannerState.currentFreqIndex].name);
    UI.setPosition(scannerState.freqLabel, 10, 30);
    UI.setLabelStyle(scannerState.freqLabel, { 
        text_color: '#00FF00',
        font_size: 16,
        font_weight: 'bold'
    });
    
    // RSSI display
    scannerState.rssiLabel = UI.createLabel(scannerState.screen, "RSSI: --- dBm");
    UI.setPosition(scannerState.rssiLabel, 10, 60);
    UI.setLabelStyle(scannerState.rssiLabel, { 
        text_color: '#FFFF00',
        font_size: 14
    });
    
    // Status display
    scannerState.statusLabel = UI.createLabel(scannerState.screen, "[Stopped]");
    UI.setPosition(scannerState.statusLabel, 10, 90);
    UI.setLabelStyle(scannerState.statusLabel, { 
        text_color: '#FFFFFF',
        font_size: 14
    });
    
    // Signal count
    scannerState.signalLabel = UI.createLabel(scannerState.screen, "Signals: 0");
    UI.setPosition(scannerState.signalLabel, 10, 120);
    UI.setLabelStyle(scannerState.signalLabel, { 
        text_color: '#CCCCCC',
        font_size: 14
    });
    
    // Status bar
    const statusBar = UI.createContainer(scannerState.screen);
    UI.setSize(statusBar, 170, 30);
    UI.setPosition(statusBar, 0, 290);
    UI.setContainerStyle(statusBar, { bg_color: '#000000' });
    
    scannerState.statusBarLabel = UI.createLabel(statusBar, "[←→: Frequency] [OK: Start/Stop]");
    UI.setPosition(scannerState.statusBarLabel, 5, 10);
    UI.setLabelStyle(scannerState.statusBarLabel, { 
        text_color: '#888888',
        font_size: 8
    });
    
    UI.setActiveScreen(scannerState.screen);
}

// Setup RF module configuration
function setupRFModule() {
    console.log("Setting up RF module...");
    
    // Load default preset
    RF.loadPreset("gfsk_433");
    
    // Set initial frequency
    const currentFreq = SCAN_FREQUENCIES[scannerState.currentFreqIndex];
    RF.setFrequency(currentFreq.freq);
    
    updateFrequencyDisplay();
}

// Set up input handlers
function setupScannerInput() {
    // Encoder for frequency selection
    System.onEncoder((direction) => {
        if (!scannerState.isScanning) {
            if (direction === 'CW') {
                changeFrequency(1);
            } else if (direction === 'CCW') {
                changeFrequency(-1);
            }
        }
    });
    
    // Encoder press to start/stop scanning
    System.onButton('ENCODER', () => {
        if (scannerState.isScanning) {
            stopScanning();
        } else {
            startScanning();
        }
    });
    
    // Button 1 for mode change
    System.onButton('BUTTON1', () => {
        // For now, just show a notification
        Notification.show("Mode change not implemented", 1000);
    });
    
    // Button 2 for back to menu
    System.onButton('BUTTON2', () => {
        stopScanning();
        // Return to main menu
        System.exit();
    });
}

// Change frequency selection
function changeFrequency(direction) {
    const newIndex = scannerState.currentFreqIndex + direction;
    
    if (newIndex >= 0 && newIndex < SCAN_FREQUENCIES.length) {
        scannerState.currentFreqIndex = newIndex;
        const newFreq = SCAN_FREQUENCIES[newIndex];
        
        // Update RF module frequency
        RF.setFrequency(newFreq.freq);
        
        // Update display
        updateFrequencyDisplay();
        
        // Haptic feedback
        Notification.vibrate(50);
    }
}

// Update frequency display
function updateFrequencyDisplay() {
    const currentFreq = SCAN_FREQUENCIES[scannerState.currentFreqIndex];
    UI.setLabelText(scannerState.freqLabel, currentFreq.name);
}

// Start scanning
function startScanning() {
    if (scannerState.isScanning) return;
    
    scannerState.isScanning = true;
    scannerState.scanStartTime = Date.now();
    
    // Update UI
    UI.setLabelText(scannerState.statusLabel, "[Scanning...]");
    UI.setLabelText(scannerState.statusBarLabel, "[OK: Stop] [Back: Exit]");
    
    // Start receiving
    RF.startReceive();
    
    // Start scanning loop
    scanLoop();
    
    console.log("Started scanning");
}

// Stop scanning
function stopScanning() {
    if (!scannerState.isScanning) return;
    
    scannerState.isScanning = false;
    
    // Update UI
    UI.setLabelText(scannerState.statusLabel, "[Stopped]");
    UI.setLabelText(scannerState.statusBarLabel, "[←→: Frequency] [OK: Start/Stop]");
    
    // Stop receiving
    RF.stopReceive();
    
    console.log("Stopped scanning");
}

// Scanning loop
function scanLoop() {
    if (!scannerState.isScanning) return;
    
    try {
        // Get RSSI
        const rssi = RF.getRssi();
        scannerState.lastRSSI = rssi;
        
        // Update RSSI display
        UI.setLabelText(scannerState.rssiLabel, `RSSI: ${rssi} dBm`);
        
        // Check for signals
        const signal = RF.readSignal();
        if (signal !== null) {
            // Add to signal history
            scannerState.signalHistory.push({
                frequency: SCAN_FREQUENCIES[scannerState.currentFreqIndex].freq,
                rssi: rssi,
                timestamp: Date.now()
            });
            
            // Update signal count
            UI.setLabelText(scannerState.signalLabel, `Signals: ${scannerState.signalHistory.length}`);
            
            // Show notification
            Notification.show(`Signal detected: ${rssi} dBm`, 1000);
        }
        
        // Schedule next scan
        setTimeout(scanLoop, 100);
        
    } catch (error) {
        console.error("Error during scanning:", error);
        stopScanning();
    }
}

// Handle received signal
function handleReceivedSignal(signal) {
    // Add to signal history
    scannerState.signalHistory.push({
        frequency: signal.frequency,
        rssi: signal.rssi,
        data: signal.data,
        timestamp: signal.timestamp
    });
    
    // Update signal count
    UI.setLabelText(scannerState.signalLabel, `Signals: ${scannerState.signalHistory.length}`);
    
    // Show notification
    Notification.show(`Signal: ${signal.rssi} dBm`, 1000);
}

// Cleanup function
function cleanup() {
    console.log("Cleaning up RF Scanner...");
    stopScanning();
}

// Event handlers
function setupEventHandlers() {
    System.onBackButton(function() {
        console.log("Stopping RF Scanner...");
        cleanup();
        System.exit();
    });
    
    System.onPause(function() {
        console.log("RF Scanner paused");
        stopScanning();
    });
    
    System.onResume(function() {
        console.log("RF Scanner resumed");
    });
}

// App entry point
initRFScanner();
setupEventHandlers();