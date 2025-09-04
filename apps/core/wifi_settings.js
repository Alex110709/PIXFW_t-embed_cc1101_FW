/**
 * @file wifi_settings.js
 * @brief Wi-Fi Settings Application (Flipper Zero Style)
 * 
 * Provides Wi-Fi management interface with Flipper Zero style UI
 */

// App state
let wifiState = {
    currentScreen: 'main', // main, scan, connect, ap
    networks: [],
    selectedNetwork: -1,
    status: 0, // 0=disconnected, 1=connecting, 2=connected, 3=ap_mode
    ipAddress: '0.0.0.0',
    apSSID: 'T-Embed-CC1101',
    apPassword: ''
};

// UI elements
let ui = {};

// Wi-Fi status constants
const WIFI_STATUS_DISCONNECTED = 0;
const WIFI_STATUS_CONNECTING = 1;
const WIFI_STATUS_CONNECTED = 2;
const WIFI_STATUS_FAILED = 3;
const WIFI_STATUS_AP_MODE = 4;

// Initialize Wi-Fi settings app
function initWiFiSettings() {
    console.log("Initializing Wi-Fi Settings App (Flipper Zero style)...");
    
    try {
        createUI();
        updateStatus();
        updateIPAddress();
        
        console.log("Wi-Fi Settings App initialized successfully");
    } catch (error) {
        console.error("Wi-Fi Settings App initialization error:", error);
        Notification.showError("Wi-Fi Settings failed to start: " + error.message);
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
    
    const titleLabel = UI.createLabel(titleBar, "[Wi-Fi Settings]");
    UI.setPosition(titleLabel, 5, 2);
    UI.setLabelStyle(titleLabel, { 
        text_color: '#FFFFFF', 
        font_size: 10,
        font_weight: 'bold'
    });
    
    // Main menu
    createMainMenu(screen);
    
    // Status bar
    const statusBar = UI.createContainer(screen);
    UI.setSize(statusBar, 170, 30);
    UI.setPosition(statusBar, 0, 290);
    UI.setContainerStyle(statusBar, { bg_color: '#000000' });
    
    ui.statusBarLabel = UI.createLabel(statusBar, "[OK: Select] [Back: Exit]");
    UI.setPosition(ui.statusBarLabel, 5, 10);
    UI.setLabelStyle(ui.statusBarLabel, { 
        text_color: '#888888',
        font_size: 8
    });
    
    UI.setActiveScreen(screen);
}

// Create main menu
function createMainMenu(parent) {
    ui.mainMenu = UI.createList(parent);
    UI.setPosition(ui.mainMenu, 0, 30);
    UI.setSize(ui.mainMenu, 170, 240);
    
    UI.addListItem(ui.mainMenu, "Scan Networks");
    UI.addListItem(ui.mainMenu, "Connect to Network");
    UI.addListItem(ui.mainMenu, "Start Access Point");
    UI.addListItem(ui.mainMenu, "Stop Access Point");
    UI.addListItem(ui.mainMenu, "Current Status");
    
    // Select first item
    UI.setListSelectedIndex(ui.mainMenu, 0);
}

// Create network scan screen
function createScanScreen() {
    const screen = UI.getScreen();
    UI.setScreenStyle(screen, { bg_color: '#000000' });
    
    // Title bar
    const titleBar = UI.createContainer(screen);
    UI.setSize(titleBar, 170, 20);
    UI.setPosition(titleBar, 0, 0);
    UI.setContainerStyle(titleBar, { bg_color: '#1976D2' });
    
    const titleLabel = UI.createLabel(titleBar, "[Scan Networks]");
    UI.setPosition(titleLabel, 5, 2);
    UI.setLabelStyle(titleLabel, { 
        text_color: '#FFFFFF', 
        font_size: 10,
        font_weight: 'bold'
    });
    
    // Scan button
    ui.scanButton = UI.createButton(screen, "Scan");
    UI.setPosition(ui.scanButton, 10, 30);
    UI.setButtonCallback(ui.scanButton, scanNetworks);
    
    // Network list
    ui.networkList = UI.createList(screen);
    UI.setPosition(ui.networkList, 0, 60);
    UI.setSize(ui.networkList, 170, 200);
    
    // Back button
    const backButton = UI.createButton(screen, "Back");
    UI.setPosition(backButton, 10, 270);
    UI.setButtonCallback(backButton, showMainScreen);
    
    UI.setActiveScreen(screen);
    
    // Start scanning immediately
    scanNetworks();
}

// Create connect screen
function createConnectScreen() {
    const screen = UI.getScreen();
    UI.setScreenStyle(screen, { bg_color: '#000000' });
    
    // Title bar
    const titleBar = UI.createContainer(screen);
    UI.setSize(titleBar, 170, 20);
    UI.setPosition(titleBar, 0, 0);
    UI.setContainerStyle(titleBar, { bg_color: '#1976D2' });
    
    const titleLabel = UI.createLabel(titleBar, "[Connect]");
    UI.setPosition(titleLabel, 5, 2);
    UI.setLabelStyle(titleLabel, { 
        text_color: '#FFFFFF', 
        font_size: 10,
        font_weight: 'bold'
    });
    
    // SSID input
    UI.createLabel(screen, "SSID:");
    UI.setPosition(UI.createLabel(screen, "SSID:"), 10, 30);
    
    ui.ssidInput = UI.createTextInput(screen);
    UI.setPosition(ui.ssidInput, 10, 50);
    UI.setSize(ui.ssidInput, 150, 20);
    
    // Password input
    UI.createLabel(screen, "Password (optional):");
    UI.setPosition(UI.createLabel(screen, "Password (optional):"), 10, 80);
    
    ui.passwordInput = UI.createTextInput(screen);
    UI.setPosition(ui.passwordInput, 10, 100);
    UI.setSize(ui.passwordInput, 150, 20);
    UI.setTextInputPassword(ui.passwordInput, true);
    
    // Connect button
    const connectButton = UI.createButton(screen, "Connect");
    UI.setPosition(connectButton, 10, 130);
    UI.setButtonCallback(connectButton, connectToNetwork);
    
    // Back button
    const backButton = UI.createButton(screen, "Back");
    UI.setPosition(backButton, 10, 270);
    UI.setButtonCallback(backButton, showMainScreen);
    
    UI.setActiveScreen(screen);
}

// Create AP screen
function createAPScreen() {
    const screen = UI.getScreen();
    UI.setScreenStyle(screen, { bg_color: '#000000' });
    
    // Title bar
    const titleBar = UI.createContainer(screen);
    UI.setSize(titleBar, 170, 20);
    UI.setPosition(titleBar, 0, 0);
    UI.setContainerStyle(titleBar, { bg_color: '#1976D2' });
    
    const titleLabel = UI.createLabel(titleBar, "[Access Point]");
    UI.setPosition(titleLabel, 5, 2);
    UI.setLabelStyle(titleLabel, { 
        text_color: '#FFFFFF', 
        font_size: 10,
        font_weight: 'bold'
    });
    
    // SSID input
    UI.createLabel(screen, "SSID:");
    UI.setPosition(UI.createLabel(screen, "SSID:"), 10, 30);
    
    ui.apSSIDInput = UI.createTextInput(screen);
    UI.setPosition(ui.apSSIDInput, 10, 50);
    UI.setSize(ui.apSSIDInput, 150, 20);
    UI.setTextInputValue(ui.apSSIDInput, wifiState.apSSID);
    
    // Password input
    UI.createLabel(screen, "Password (optional):");
    UI.setPosition(UI.createLabel(screen, "Password (optional):"), 10, 80);
    
    ui.apPasswordInput = UI.createTextInput(screen);
    UI.setPosition(ui.apPasswordInput, 10, 100);
    UI.setSize(ui.apPasswordInput, 150, 20);
    UI.setTextInputPassword(ui.apPasswordInput, true);
    UI.setTextInputValue(ui.apPasswordInput, wifiState.apPassword);
    
    // Start AP button
    const startAPButton = UI.createButton(screen, "Start AP");
    UI.setPosition(startAPButton, 10, 130);
    UI.setButtonCallback(startAPButton, startAccessPoint);
    
    // Back button
    const backButton = UI.createButton(screen, "Back");
    UI.setPosition(backButton, 10, 270);
    UI.setButtonCallback(backButton, showMainScreen);
    
    UI.setActiveScreen(screen);
}

// Create status screen
function createStatusScreen() {
    const screen = UI.getScreen();
    UI.setScreenStyle(screen, { bg_color: '#000000' });
    
    // Title bar
    const titleBar = UI.createContainer(screen);
    UI.setSize(titleBar, 170, 20);
    UI.setPosition(titleBar, 0, 0);
    UI.setContainerStyle(titleBar, { bg_color: '#1976D2' });
    
    const titleLabel = UI.createLabel(titleBar, "[Status]");
    UI.setPosition(titleLabel, 5, 2);
    UI.setLabelStyle(titleLabel, { 
        text_color: '#FFFFFF', 
        font_size: 10,
        font_weight: 'bold'
    });
    
    // Status info
    ui.statusLabel = UI.createLabel(screen, "Status: Unknown");
    UI.setPosition(ui.statusLabel, 10, 40);
    UI.setLabelStyle(ui.statusLabel, { 
        text_color: '#00FF00',
        font_size: 12
    });
    
    ui.ipLabel = UI.createLabel(screen, "IP: 0.0.0.0");
    UI.setPosition(ui.ipLabel, 10, 70);
    UI.setLabelStyle(ui.ipLabel, { 
        text_color: '#FFFF00',
        font_size: 12
    });
    
    // Refresh button
    const refreshButton = UI.createButton(screen, "Refresh");
    UI.setPosition(refreshButton, 10, 100);
    UI.setButtonCallback(refreshButton, updateStatusInfo);
    
    // Back button
    const backButton = UI.createButton(screen, "Back");
    UI.setPosition(backButton, 10, 270);
    UI.setButtonCallback(backButton, showMainScreen);
    
    UI.setActiveScreen(screen);
    
    // Update status info immediately
    updateStatusInfo();
}

// Show main screen
function showMainScreen() {
    const screen = UI.getScreen();
    UI.setScreenStyle(screen, { bg_color: '#000000' });
    
    // Title bar
    const titleBar = UI.createContainer(screen);
    UI.setSize(titleBar, 170, 20);
    UI.setPosition(titleBar, 0, 0);
    UI.setContainerStyle(titleBar, { bg_color: '#1976D2' });
    
    const titleLabel = UI.createLabel(titleBar, "[Wi-Fi Settings]");
    UI.setPosition(titleLabel, 5, 2);
    UI.setLabelStyle(titleLabel, { 
        text_color: '#FFFFFF', 
        font_size: 10,
        font_weight: 'bold'
    });
    
    // Main menu
    createMainMenu(screen);
    
    // Status bar
    const statusBar = UI.createContainer(screen);
    UI.setSize(statusBar, 170, 30);
    UI.setPosition(statusBar, 0, 290);
    UI.setContainerStyle(statusBar, { bg_color: '#000000' });
    
    ui.statusBarLabel = UI.createLabel(statusBar, "[OK: Select] [Back: Exit]");
    UI.setPosition(ui.statusBarLabel, 5, 10);
    UI.setLabelStyle(ui.statusBarLabel, { 
        text_color: '#888888',
        font_size: 8
    });
    
    UI.setActiveScreen(screen);
}

// Scan for Wi-Fi networks
function scanNetworks() {
    try {
        Notification.show("Scanning...", 1000);
        
        // Scan for networks
        const networks = wifi.scan();
        
        // Clear previous list
        UI.clearList(ui.networkList);
        
        // Add networks to list
        if (networks && networks.length > 0) {
            wifiState.networks = networks;
            
            for (let i = 0; i < networks.length; i++) {
                const network = networks[i];
                const displayText = `${network.ssid} (${network.rssi}dBm)`;
                UI.addListItem(ui.networkList, displayText);
            }
            
            // Select first network
            UI.setListSelectedIndex(ui.networkList, 0);
            wifiState.selectedNetwork = 0;
        } else {
            UI.addListItem(ui.networkList, "No networks found");
            wifiState.selectedNetwork = -1;
        }
        
        Notification.show(`Found ${networks ? networks.length : 0} networks`, 2000);
    } catch (error) {
        console.error("Wi-Fi scan failed:", error);
        Notification.showError("Scan failed: " + error.message);
    }
}

// Connect to selected network
function connectToNetwork() {
    try {
        const ssid = UI.getTextInputValue(ui.ssidInput);
        const password = UI.getTextInputValue(ui.passwordInput);
        
        if (!ssid || ssid.trim() === '') {
            Notification.showError("Please enter SSID");
            return;
        }
        
        Notification.show("Connecting...", 2000);
        
        // Connect to network
        wifi.connect(ssid, password);
        
        // Update status
        wifiState.status = WIFI_STATUS_CONNECTING;
        updateStatus();
        
        Notification.show("Connected to " + ssid, 2000);
    } catch (error) {
        console.error("Wi-Fi connection failed:", error);
        Notification.showError("Connection failed: " + error.message);
        wifiState.status = WIFI_STATUS_FAILED;
        updateStatus();
    }
}

// Start access point
function startAccessPoint() {
    try {
        const ssid = UI.getTextInputValue(ui.apSSIDInput);
        const password = UI.getTextInputValue(ui.apPasswordInput);
        
        if (!ssid || ssid.trim() === '') {
            Notification.showError("Please enter SSID");
            return;
        }
        
        Notification.show("Starting AP...", 2000);
        
        // Start AP
        if (password && password.trim() !== '') {
            wifi.startAP(ssid, password);
        } else {
            wifi.startAP(ssid);
        }
        
        // Update status
        wifiState.status = WIFI_STATUS_AP_MODE;
        wifiState.apSSID = ssid;
        wifiState.apPassword = password;
        updateStatus();
        
        Notification.show("AP started: " + ssid, 2000);
    } catch (error) {
        console.error("Failed to start AP:", error);
        Notification.showError("AP failed: " + error.message);
    }
}

// Stop access point
function stopAccessPoint() {
    try {
        Notification.show("Stopping AP...", 1000);
        
        // Stop AP
        wifi.stopAP();
        
        // Update status
        wifiState.status = WIFI_STATUS_DISCONNECTED;
        updateStatus();
        
        Notification.show("AP stopped", 2000);
    } catch (error) {
        console.error("Failed to stop AP:", error);
        Notification.showError("Stop AP failed: " + error.message);
    }
}

// Update status information
function updateStatusInfo() {
    try {
        // Get current status
        const status = wifi.getStatus();
        wifiState.status = status;
        
        // Update status label
        let statusText = "Unknown";
        switch (status) {
            case WIFI_STATUS_DISCONNECTED:
                statusText = "Disconnected";
                break;
            case WIFI_STATUS_CONNECTING:
                statusText = "Connecting...";
                break;
            case WIFI_STATUS_CONNECTED:
                statusText = "Connected";
                break;
            case WIFI_STATUS_FAILED:
                statusText = "Connection Failed";
                break;
            case WIFI_STATUS_AP_MODE:
                statusText = "AP Mode";
                break;
        }
        
        UI.setLabelText(ui.statusLabel, "Status: " + statusText);
        
        // Get IP address
        const ip = wifi.getIPAddress();
        if (ip) {
            wifiState.ipAddress = ip;
            UI.setLabelText(ui.ipLabel, "IP: " + ip);
        }
    } catch (error) {
        console.error("Failed to update status:", error);
        Notification.showError("Status update failed: " + error.message);
    }
}

// Update status display
function updateStatus() {
    // This would update any status indicators in the UI
    console.log("Wi-Fi status updated:", wifiState.status);
}

// Update IP address display
function updateIPAddress() {
    // This would update any IP address display in the UI
    console.log("IP address:", wifiState.ipAddress);
}

// Handle menu selection
function handleMenuSelection() {
    const selectedIndex = UI.getListSelectedIndex(ui.mainMenu);
    
    switch (selectedIndex) {
        case 0: // Scan Networks
            createScanScreen();
            break;
        case 1: // Connect to Network
            createConnectScreen();
            break;
        case 2: // Start Access Point
            createAPScreen();
            break;
        case 3: // Stop Access Point
            stopAccessPoint();
            break;
        case 4: // Current Status
            createStatusScreen();
            break;
    }
}

// Set up input event handlers
function setupInputHandlers() {
    // Encoder rotation for navigation
    System.onEncoder((direction) => {
        if (wifiState.currentScreen === 'main') {
            if (direction === 'CW') {
                // Navigate down in main menu
                let currentIndex = UI.getListSelectedIndex(ui.mainMenu);
                if (currentIndex < 4) { // 5 menu items (0-4)
                    UI.setListSelectedIndex(ui.mainMenu, currentIndex + 1);
                    Notification.vibrate(50); // Haptic feedback
                }
            } else if (direction === 'CCW') {
                // Navigate up in main menu
                let currentIndex = UI.getListSelectedIndex(ui.mainMenu);
                if (currentIndex > 0) {
                    UI.setListSelectedIndex(ui.mainMenu, currentIndex - 1);
                    Notification.vibrate(50); // Haptic feedback
                }
            }
        }
    });
    
    // Encoder press for selection
    System.onButton('ENCODER', () => {
        if (wifiState.currentScreen === 'main') {
            handleMenuSelection();
        }
    });
    
    // Button 1 for back/exit
    System.onButton('BUTTON1', () => {
        if (wifiState.currentScreen !== 'main') {
            showMainScreen();
            wifiState.currentScreen = 'main';
        } else {
            // Exit app
            System.exit();
        }
    });
    
    // Button 2 for quick actions
    System.onButton('BUTTON2', () => {
        // Refresh status
        updateStatusInfo();
    });
    
    // Back button
    System.onBackButton(() => {
        if (wifiState.currentScreen !== 'main') {
            showMainScreen();
            wifiState.currentScreen = 'main';
        } else {
            // Exit app
            System.exit();
        }
    });
}

// Cleanup function
function cleanup() {
    console.log("Cleaning up Wi-Fi Settings App...");
    // Any cleanup code would go here
}

// App entry point
initWiFiSettings();
setupInputHandlers();

// Export functions for testing
if (typeof module !== 'undefined') {
    module.exports = {
        initWiFiSettings,
        scanNetworks,
        connectToNetwork,
        startAccessPoint,
        stopAccessPoint,
        updateStatusInfo,
        cleanup
    };
}