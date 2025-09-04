/**
 * @file main_menu.js
 * @brief Main Menu Application (Flipper Zero Style)
 * 
 * This is the core menu application that provides navigation
 * to all other apps and system functions with Flipper Zero style UI.
 */

// Main menu configuration
const MENU_ITEMS = [
    {
        id: 'rf_scanner',
        title: 'RF Scanner'
    },
    {
        id: 'rf_jammer',
        title: 'RF Jammer'
    },
    {
        id: 'spectrum_analyzer',
        title: 'Spectrum Analyzer'
    },
    {
        id: 'signal_generator',
        title: 'Signal Generator'
    },
    {
        id: 'apps',
        title: 'Apps'
    },
    {
        id: 'wifi_settings',
        title: 'Wi-Fi'
    },
    {
        id: 'system_settings',
        title: 'Settings'
    },
    {
        id: 'about',
        title: 'About'
    }
];

let currentMenuIndex = 0;
let menuScreen = null;
let menuList = null;

// Initialize main menu
function initMainMenu() {
    console.log("Initializing main menu (Flipper Zero style)...");
    
    // Create main screen
    menuScreen = ui.createScreen();
    ui.setStyle(menuScreen, 'bg_color', '#000000');
    
    // Create title bar
    const titleBar = ui.createContainer(menuScreen);
    ui.setSize(titleBar, 170, 20);
    ui.setPosition(titleBar, 0, 0);
    ui.setStyle(titleBar, 'bg_color', '#1976D2');
    
    const titleLabel = ui.createLabel(titleBar, "[Main Menu]");
    ui.setPosition(titleLabel, 5, 2);
    ui.setStyle(titleLabel, 'text_color', '#FFFFFF');
    ui.setStyle(titleLabel, 'text_font', 'bold');
    
    // Create menu list
    createMenuList();
    
    // Create status bar
    createStatusBar();
    
    // Set up input handling
    setupInputHandlers();
    
    ui.setActiveScreen(menuScreen);
    console.log("Main menu initialized");
}

// Create menu list UI
function createMenuList() {
    menuList = ui.createList(menuScreen);
    ui.setSize(menuList, 170, 240);
    ui.setPosition(menuList, 0, 20);
    ui.setStyle(menuList, 'bg_color', '#000000');
    
    // Add menu items to list
    for (let i = 0; i < MENU_ITEMS.length; i++) {
        const item = MENU_ITEMS[i];
        ui.addListItem(menuList, item.title);
    }
    
    // Select first item
    ui.setListSelectedIndex(menuList, 0);
}

// Create status bar
function createStatusBar() {
    const statusBar = ui.createContainer(menuScreen);
    ui.setSize(statusBar, 170, 30);
    ui.setPosition(statusBar, 0, 290);
    ui.setStyle(statusBar, 'bg_color', '#000000');
    
    const statusLabel = ui.createLabel(statusBar, "[↑↓: Navigate] [OK: Select]");
    ui.setPosition(statusLabel, 5, 10);
    ui.setStyle(statusLabel, 'text_color', '#888888');
    ui.setStyle(statusLabel, 'text_font', 'small');
}

// Set up input event handlers
function setupInputHandlers() {
    // Encoder rotation for navigation
    input.onEncoder((direction) => {
        if (direction === 'CW') {
            navigateDown();
        } else if (direction === 'CCW') {
            navigateUp();
        }
    });
    
    // Encoder press for selection
    input.onButton('ENCODER', () => {
        selectCurrentItem();
    });
    
    // Button 1 for back/exit
    input.onButton('BUTTON1', () => {
        // In main menu, button 1 could show system info
        showSystemInfo();
    });
    
    // Button 2 for quick actions
    input.onButton('BUTTON2', () => {
        // For now, exit the app
        app.exit();
    });
}

// Navigate menu up
function navigateUp() {
    let currentIndex = ui.getListSelectedIndex(menuList);
    if (currentIndex > 0) {
        ui.setListSelectedIndex(menuList, currentIndex - 1);
        notify.vibrate(50); // Haptic feedback
    }
}

// Navigate menu down  
function navigateDown() {
    let currentIndex = ui.getListSelectedIndex(menuList);
    if (currentIndex < MENU_ITEMS.length - 1) {
        ui.setListSelectedIndex(menuList, currentIndex + 1);
        notify.vibrate(50); // Haptic feedback
    }
}

// Select current menu item
function selectCurrentItem() {
    const selectedIndex = ui.getListSelectedIndex(menuList);
    const selectedItem = MENU_ITEMS[selectedIndex];
    
    console.log("Selected:", selectedItem.title);
    
    // Show selection feedback
    notify.flash(1, 100);
    
    // Launch corresponding app/function
    switch (selectedItem.id) {
        case 'rf_scanner':
            launchRFScanner();
            break;
        case 'rf_jammer':
            launchRFJammer();
            break;
        case 'spectrum_analyzer':
            launchSpectrumAnalyzer();
            break;
        case 'signal_generator':
            launchSignalGenerator();
            break;
        case 'apps':
            launchApps();
            break;
        case 'wifi_settings':
            launchWiFiSettings();
            break;
        case 'system_settings':
            launchSystemSettings();
            break;
        case 'about':
            showSystemInfo();
            break;
        default:
            console.log("Unknown menu item:", selectedItem.id);
            notify.showError("Feature not implemented");
    }
}

// Launch RF Scanner app
function launchRFScanner() {
    try {
        // Load and run the RF scanner app
        const scannerApp = require('/apps/core/rf_scanner.js');
        // In a real implementation, we would start the app in a new context
        console.log("Launching RF Scanner app");
        notify.show("Launching RF Scanner");
    } catch (error) {
        console.error("Failed to launch RF Scanner:", error);
        notify.showError("Failed to launch RF Scanner: " + error.message);
    }
}

// Launch RF Jammer app
function launchRFJammer() {
    try {
        // Load and run the jammer app
        const jammerApp = require('/apps/core/jammer.js');
        // In a real implementation, we would start the app in a new context
        console.log("Launching RF Jammer app");
        notify.show("Launching RF Jammer");
    } catch (error) {
        console.error("Failed to launch RF Jammer:", error);
        notify.showError("Failed to launch RF Jammer: " + error.message);
    }
}

// Launch Spectrum Analyzer app
function launchSpectrumAnalyzer() {
    try {
        // Load and run the spectrum analyzer app
        const spectrumApp = require('/apps/core/spectrum_analyzer.js');
        // In a real implementation, we would start the app in a new context
        console.log("Launching Spectrum Analyzer app");
        notify.show("Launching Spectrum Analyzer");
    } catch (error) {
        console.error("Failed to launch Spectrum Analyzer:", error);
        notify.showError("Failed to launch Spectrum Analyzer: " + error.message);
    }
}

// Launch Signal Generator app
function launchSignalGenerator() {
    try {
        // Load and run the signal generator app
        const generatorApp = require('/apps/core/signal_generator.js');
        // In a real implementation, we would start the app in a new context
        console.log("Launching Signal Generator app");
        notify.show("Launching Signal Generator");
    } catch (error) {
        console.error("Failed to launch Signal Generator:", error);
        notify.showError("Failed to launch Signal Generator: " + error.message);
    }
}

// Launch Apps menu
function launchApps() {
    try {
        // Load and run the apps menu
        const appsMenu = require('/apps/core/apps_menu.js');
        // In a real implementation, we would start the app in a new context
        console.log("Launching Apps menu");
        notify.show("Launching Apps");
    } catch (error) {
        console.error("Failed to launch Apps menu:", error);
        notify.showError("Failed to launch Apps: " + error.message);
    }
}

// Launch Wi-Fi settings
function launchWiFiSettings() {
    try {
        // Load and run the Wi-Fi settings app
        const wifiSettings = require('/apps/core/wifi_settings.js');
        // In a real implementation, we would start the app in a new context
        console.log("Launching Wi-Fi Settings");
        notify.show("Launching Wi-Fi Settings");
    } catch (error) {
        console.error("Failed to launch Wi-Fi Settings:", error);
        notify.showError("Failed to launch Wi-Fi Settings: " + error.message);
    }
}

// Launch system settings
function launchSystemSettings() {
    try {
        // Load and run the system settings app
        const systemSettings = require('/apps/core/system_settings.js');
        // In a real implementation, we would start the app in a new context
        console.log("Launching System Settings");
        notify.show("Launching System Settings");
    } catch (error) {
        console.error("Failed to launch System Settings:", error);
        notify.showError("Failed to launch System Settings: " + error.message);
    }
}

// Show system information
function showSystemInfo() {
    try {
        // Load and run the about app
        const aboutApp = require('/apps/core/about.js');
        // In a real implementation, we would start the app in a new context
        console.log("Showing System Info");
        notify.show("Showing System Info");
    } catch (error) {
        console.error("Failed to show System Info:", error);
        notify.showError("Failed to show System Info: " + error.message);
    }
}

// App entry point
initMainMenu();