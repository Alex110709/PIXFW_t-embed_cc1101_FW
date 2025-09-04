/**
 * @file rf_scanner.js
 * @brief RF Scanner Application
 * 
 * Scans for RF signals across different frequencies and displays
 * signal information including RSSI, frequency, and data.
 */

// Scanner configuration
const SCAN_FREQUENCIES = [
    { freq: 315000000, name: "315 MHz" },
    { freq: 433920000, name: "433.92 MHz" },
    { freq: 868300000, name: "868.3 MHz" },
    { freq: 915000000, name: "915 MHz" }
];

const SCAN_MODES = [
    { id: 'auto', name: 'Auto Scan', description: 'Scan all frequencies' },
    { id: 'manual', name: 'Manual', description: 'Select frequency manually' },
    { id: 'analyze', name: 'Analyze', description: 'Analyze current frequency' }
];

let scannerState = {
    isScanning: false,
    currentFreqIndex: 0,
    currentMode: 'auto',
    signalHistory: [],
    screen: null,
    lastRSSI: -100,
    scanStartTime: 0
};

// Initialize RF Scanner
function initRFScanner() {
    console.log("Initializing RF Scanner...");
    
    // Check if RF module is available
    if (!rf.isPresent()) {
        notify.show("Error", "CC1101 module not detected", 3000);
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
    scannerState.screen = ui.createScreen();
    ui.setStyle(scannerState.screen, 'bg_color', '#000000');
    
    // Title bar
    const titleBar = ui.createContainer(scannerState.screen);
    ui.setSize(titleBar, 170, 30);
    ui.setPosition(titleBar, 0, 0);
    ui.setStyle(titleBar, 'bg_color', '#1976D2');
    
    const titleLabel = ui.createLabel(titleBar, "RF Scanner");
    ui.setPosition(titleLabel, 5, 5);
    ui.setStyle(titleLabel, 'text_color', '#FFFFFF');
    ui.setStyle(titleLabel, 'text_font', 'bold');
    
    // Frequency display
    const freqContainer = ui.createContainer(scannerState.screen);
    ui.setSize(freqContainer, 170, 40);
    ui.setPosition(freqContainer, 0, 35);
    ui.setStyle(freqContainer, 'border_width', 1);
    ui.setStyle(freqContainer, 'border_color', '#333333');
    
    const freqLabel = ui.createLabel(freqContainer, "433.92 MHz");
    ui.setPosition(freqLabel, 10, 10);
    ui.setStyle(freqLabel, 'text_color', '#00FF00');
    ui.setStyle(freqLabel, 'text_font', 'large');
    scannerState.freqLabel = freqLabel;
    
    // RSSI display
    const rssiContainer = ui.createContainer(scannerState.screen);
    ui.setSize(rssiContainer, 170, 60);
    ui.setPosition(rssiContainer, 0, 80);
    ui.setStyle(rssiContainer, 'border_width', 1);
    ui.setStyle(rssiContainer, 'border_color', '#333333');
    
    const rssiLabel = ui.createLabel(rssiContainer, "RSSI: --- dBm");
    ui.setPosition(rssiLabel, 10, 10);
    ui.setStyle(rssiLabel, 'text_color', '#FFFF00');
    scannerState.rssiLabel = rssiLabel;
    
    // RSSI bar
    const rssiBar = ui.createProgressBar(rssiContainer, 140);
    ui.setPosition(rssiBar, 10, 35);
    scannerState.rssiBar = rssiBar;
    
    // Signal info display
    const infoContainer = ui.createContainer(scannerState.screen);
    ui.setSize(infoContainer, 170, 80);
    ui.setPosition(infoContainer, 0, 145);
    ui.setStyle(infoContainer, 'border_width', 1);
    ui.setStyle(infoContainer, 'border_color', '#333333');
    
    const statusLabel = ui.createLabel(infoContainer, "Status: Ready");
    ui.setPosition(statusLabel, 10, 10);
    ui.setStyle(statusLabel, 'text_color', '#FFFFFF');
    scannerState.statusLabel = statusLabel;
    
    const signalLabel = ui.createLabel(infoContainer, "Signals: 0");
    ui.setPosition(signalLabel, 10, 30);
    ui.setStyle(signalLabel, 'text_color', '#CCCCCC');
    scannerState.signalLabel = signalLabel;
    
    const timeLabel = ui.createLabel(infoContainer, "Time: 00:00");
    ui.setPosition(timeLabel, 10, 50);
    ui.setStyle(timeLabel, 'text_color', '#CCCCCC');
    scannerState.timeLabel = timeLabel;
    
    // Control instructions
    const controlsContainer = ui.createContainer(scannerState.screen);
    ui.setSize(controlsContainer, 170, 50);
    ui.setPosition(controlsContainer, 0, 230);
    
    const controlsLabel = ui.createLabel(controlsContainer, 
        "🔄 Encoder: Frequency\n" +
        "🔘 Press: Start/Stop\n" +
        "📝 Btn1: Mode  📤 Btn2: Menu");
    ui.setPosition(controlsLabel, 5, 5);
    ui.setStyle(controlsLabel, 'text_color', '#888888');
    ui.setStyle(controlsLabel, 'text_font', 'small');
    
    ui.setActiveScreen(scannerState.screen);
}

// Setup RF module configuration
function setupRFModule() {
    console.log("Setting up RF module...");
    
    // Load default preset
    rf.loadPreset("gfsk_433");
    
    // Set initial frequency
    rf.setFrequency(SCAN_FREQUENCIES[0].freq);
    
    // Set up receive callback
    rf.setReceiveCallback((signal) => {
        handleReceivedSignal(signal);
    });
    
    updateFrequencyDisplay();
}

// Set up input handlers
function setupScannerInput() {
    // Encoder for frequency selection
    input.onEncoder((direction) => {
        if (!scannerState.isScanning) {
            if (direction === 'CW') {
                changeFrequency(1);
            } else if (direction === 'CCW') {
                changeFrequency(-1);
            }
        }
    });
    
    // Encoder press to start/stop scanning
    input.onButton('ENCODER', () => {
        if (scannerState.isScanning) {
            stopScanning();
        } else {
            startScanning();
        }
    });
    
    // Button 1 for mode change
    input.onButton('BUTTON1', () => {
        changeScanMode();
    });
    
    // Button 2 for back to menu
    input.onButton('BUTTON2', () => {
        stopScanning();
        // Return to main menu
        app.exit();
    });
}

// Change frequency selection
function changeFrequency(direction) {
    const newIndex = scannerState.currentFreqIndex + direction;
    
    if (newIndex >= 0 && newIndex < SCAN_FREQUENCIES.length) {
        scannerState.currentFreqIndex = newIndex;
        const newFreq = SCAN_FREQUENCIES[newIndex];
        
        rf.setFrequency(newFreq.freq);
        updateFrequencyDisplay();
        
        notify.vibrate(30);
        console.log("Changed to frequency:", newFreq.name);
    }
}

// Update frequency display
function updateFrequencyDisplay() {
    const currentFreq = SCAN_FREQUENCIES[scannerState.currentFreqIndex];
    ui.setText(scannerState.freqLabel, currentFreq.name);
}

// Start scanning for signals
function startScanning() {
    console.log("Starting RF scan...");
    
    scannerState.isScanning = true;
    scannerState.scanStartTime = Date.now();
    scannerState.signalHistory = [];
    
    // Update UI
    ui.setText(scannerState.statusLabel, "Status: Scanning...");
    ui.setStyle(scannerState.freqLabel, 'text_color', '#FF9800');
    
    // Start receiving
    rf.startReceive();
    
    // Start RSSI monitoring
    startRSSIMonitoring();
    
    notify.show("Scanner", "Scanning started", 1000);
}

// Stop scanning
function stopScanning() {
    console.log("Stopping RF scan...");
    
    scannerState.isScanning = false;
    
    // Stop receiving
    rf.stopReceive();
    
    // Update UI
    ui.setText(scannerState.statusLabel, "Status: Stopped");
    ui.setStyle(scannerState.freqLabel, 'text_color', '#00FF00');
    
    // Show scan results
    showScanResults();
    
    notify.show("Scanner", "Scanning stopped", 1000);
}

// Start RSSI monitoring
function startRSSIMonitoring() {
    const updateRSSI = () => {
        if (!scannerState.isScanning) return;
        
        const rssi = rf.getRssi();
        scannerState.lastRSSI = rssi;
        
        // Update RSSI display
        ui.setText(scannerState.rssiLabel, `RSSI: ${rssi} dBm`);
        
        // Update RSSI bar (convert -100 to 0 dBm to 0-100%)
        const rssiPercent = Math.max(0, Math.min(100, (rssi + 100) / 0.6));
        ui.setProgress(scannerState.rssiBar, rssiPercent);
        
        // Color code RSSI
        let color = '#FF0000'; // Red for weak
        if (rssi > -80) color = '#FF9800'; // Orange for medium
        if (rssi > -60) color = '#FFFF00'; // Yellow for good
        if (rssi > -40) color = '#00FF00'; // Green for strong
        
        ui.setStyle(scannerState.rssiLabel, 'text_color', color);
        
        // Update scan time
        const elapsed = Math.floor((Date.now() - scannerState.scanStartTime) / 1000);
        const minutes = Math.floor(elapsed / 60);
        const seconds = elapsed % 60;
        ui.setText(scannerState.timeLabel, 
            `Time: ${minutes.toString().padStart(2, '0')}:${seconds.toString().padStart(2, '0')}`);
        
        // Schedule next update
        setTimeout(updateRSSI, 100);
    };
    
    updateRSSI();
}

// Handle received signal
function handleReceivedSignal(signal) {
    console.log("Signal received:", signal);
    
    // Add to history
    const signalInfo = {
        frequency: signal.frequency,
        rssi: signal.rssi,
        data: signal.data,
        timestamp: Date.now()
    };
    
    scannerState.signalHistory.push(signalInfo);
    
    // Update signal counter
    ui.setText(scannerState.signalLabel, `Signals: ${scannerState.signalHistory.length}`);
    
    // Show signal notification
    notify.flash(2, 100);
    notify.show("Signal", `RSSI: ${signal.rssi} dBm`, 1500);
    
    // Analyze signal if in analyze mode
    if (scannerState.currentMode === 'analyze') {
        analyzeSignal(signalInfo);
    }
}

// Change scan mode
function changeScanMode() {
    const modeIndex = SCAN_MODES.findIndex(mode => mode.id === scannerState.currentMode);
    const nextIndex = (modeIndex + 1) % SCAN_MODES.length;
    
    scannerState.currentMode = SCAN_MODES[nextIndex].id;
    
    notify.show("Mode", SCAN_MODES[nextIndex].name, 1500);
    console.log("Changed scan mode to:", SCAN_MODES[nextIndex].name);
}

// Analyze received signal
function analyzeSignal(signalInfo) {
    // Simple signal analysis
    const analysis = {
        duration: "Unknown",
        encoding: "Unknown", 
        protocol: "Unknown"
    };
    
    // Basic pattern detection
    if (signalInfo.data && signalInfo.data.length > 0) {
        analysis.duration = `${signalInfo.data.length} bytes`;
        
        // Simple protocol detection
        const firstByte = signalInfo.data[0];
        if (firstByte === 0xAA || firstByte === 0x55) {
            analysis.encoding = "Manchester";
        } else if (firstByte === 0xFF) {
            analysis.encoding = "PWM";
        }
    }
    
    console.log("Signal analysis:", analysis);
}

// Show scan results summary
function showScanResults() {
    if (scannerState.signalHistory.length === 0) {
        notify.show("Results", "No signals detected", 2000);
        return;
    }
    
    const totalSignals = scannerState.signalHistory.length;
    const avgRSSI = scannerState.signalHistory.reduce((sum, s) => sum + s.rssi, 0) / totalSignals;
    const scanDuration = Math.floor((Date.now() - scannerState.scanStartTime) / 1000);
    
    const results = `Signals: ${totalSignals}\nAvg RSSI: ${avgRSSI.toFixed(1)} dBm\nDuration: ${scanDuration}s`;
    
    notify.show("Scan Results", results, 4000);
}

// Clean up when exiting
function cleanup() {
    console.log("Cleaning up RF Scanner...");
    
    if (scannerState.isScanning) {
        stopScanning();
    }
    
    rf.stopReceive();
}

// Initialize scanner
if (initRFScanner()) {
    console.log("RF Scanner ready");
} else {
    console.log("RF Scanner initialization failed");
}

// Export functions
if (typeof module !== 'undefined') {
    module.exports = {
        initRFScanner,
        startScanning,
        stopScanning,
        cleanup
    };
}