/**
 * @file main_menu.js
 * @brief Main Menu Application
 * 
 * This is the core menu application that provides navigation
 * to all other apps and system functions.
 */

// Main menu configuration
const MENU_ITEMS = [
    {
        id: 'rf_scanner',
        title: 'RF Scanner',
        icon: '📡',
        description: 'Scan for RF signals'
    },
    {
        id: 'signal_generator',
        title: 'Signal Generator', 
        icon: '📢',
        description: 'Generate RF signals'
    },
    {
        id: 'apps',
        title: 'Apps',
        icon: '📱',
        description: 'JavaScript Apps'
    },
    {
        id: 'wifi_settings',
        title: 'Wi-Fi',
        icon: '📶',
        description: 'Wi-Fi Settings'
    },
    {
        id: 'system_settings',
        title: 'Settings',
        icon: '⚙️',
        description: 'System Settings'
    },
    {
        id: 'about',
        title: 'About',
        icon: 'ℹ️',
        description: 'System Information'
    }
];

let currentMenuIndex = 0;
let menuScreen = null;
let statusBar = null;

// Initialize main menu
function initMainMenu() {
    console.log("Initializing main menu...");
    
    // Create main screen
    menuScreen = ui.createScreen();
    
    // Create status bar
    statusBar = ui.createStatusBar(menuScreen);
    
    // Create menu list
    createMenuList();
    
    // Set up input handling
    setupInputHandlers();
    
    // Update status bar
    updateStatusBar();
    
    console.log("Main menu initialized");
}

// Create menu list UI
function createMenuList() {
    const listContainer = ui.createContainer(menuScreen);
    ui.setPosition(listContainer, 0, 30); // Below status bar
    ui.setSize(listContainer, 170, 290);
    
    // Create menu items
    for (let i = 0; i < MENU_ITEMS.length; i++) {
        const item = MENU_ITEMS[i];
        const menuItem = createMenuItem(listContainer, item, i);
        
        // Highlight first item
        if (i === 0) {
            ui.setStyle(menuItem, 'bg_color', '#2196F3');
        }
    }
}

// Create individual menu item
function createMenuItem(parent, item, index) {
    const itemContainer = ui.createContainer(parent);
    ui.setSize(itemContainer, 170, 40);
    ui.setPosition(itemContainer, 0, index * 42);
    ui.setStyle(itemContainer, 'border_width', 1);
    ui.setStyle(itemContainer, 'border_color', '#333333');
    
    // Icon
    const iconLabel = ui.createLabel(itemContainer, item.icon);
    ui.setPosition(iconLabel, 10, 10);
    ui.setStyle(iconLabel, 'text_color', '#FFFFFF');
    
    // Title
    const titleLabel = ui.createLabel(itemContainer, item.title);
    ui.setPosition(titleLabel, 40, 8);
    ui.setStyle(titleLabel, 'text_color', '#FFFFFF');
    ui.setStyle(titleLabel, 'text_font', 'bold');
    
    // Description
    const descLabel = ui.createLabel(itemContainer, item.description);
    ui.setPosition(descLabel, 40, 22);
    ui.setStyle(descLabel, 'text_color', '#CCCCCC');
    ui.setStyle(descLabel, 'text_font', 'small');
    
    return itemContainer;
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
        showQuickActions();
    });
}

// Navigate menu up
function navigateUp() {
    if (currentMenuIndex > 0) {
        updateMenuSelection(currentMenuIndex - 1);
    }
}

// Navigate menu down  
function navigateDown() {
    if (currentMenuIndex < MENU_ITEMS.length - 1) {
        updateMenuSelection(currentMenuIndex + 1);
    }
}

// Update menu selection highlight
function updateMenuSelection(newIndex) {
    // Remove highlight from current item
    const currentItem = getMenuItemContainer(currentMenuIndex);
    ui.setStyle(currentItem, 'bg_color', '#1E1E1E');
    
    // Add highlight to new item
    const newItem = getMenuItemContainer(newIndex);
    ui.setStyle(newItem, 'bg_color', '#2196F3');
    
    currentMenuIndex = newIndex;
    
    // Show haptic feedback
    notify.vibrate(50);
}

// Get menu item container by index
function getMenuItemContainer(index) {
    // This would return the actual UI container object
    // Implementation depends on UI framework
    return ui.getChildByIndex(menuScreen, index + 1); // +1 for status bar
}

// Select current menu item
function selectCurrentItem() {
    const selectedItem = MENU_ITEMS[currentMenuIndex];
    console.log("Selected:", selectedItem.title);
    
    // Show selection feedback
    notify.flash(1, 100);
    
    // Launch corresponding app/function
    switch (selectedItem.id) {
        case 'rf_scanner':
            launchRFScanner();
            break;
        case 'signal_generator':
            launchSignalGenerator();
            break;
        case 'apps':
            showAppsMenu();
            break;
        case 'wifi_settings':
            showWiFiSettings();
            break;
        case 'system_settings':
            showSystemSettings();
            break;
        case 'about':
            showAboutScreen();
            break;
        default:
            notify.show("Info", "Feature not implemented yet", 2000);
    }
}

// Launch RF Scanner
function launchRFScanner() {
    console.log("Launching RF Scanner...");
    
    // Load and execute RF scanner app
    try {
        // This would load the actual RF scanner JavaScript app
        notify.show("RF Scanner", "Starting RF Scanner...", 1000);
        
        // For now, show a placeholder
        setTimeout(() => {
            showRFScannerPlaceholder();
        }, 1000);
        
    } catch (error) {
        console.log("Error launching RF Scanner:", error);
        notify.show("Error", "Failed to start RF Scanner", 3000);
    }
}

// Show RF Scanner placeholder
function showRFScannerPlaceholder() {
    const scanScreen = ui.createScreen();
    
    const titleLabel = ui.createLabel(scanScreen, "RF Scanner");
    ui.setPosition(titleLabel, 10, 50);
    ui.setStyle(titleLabel, 'text_color', '#FFFFFF');
    ui.setStyle(titleLabel, 'text_font', 'large');
    
    const statusLabel = ui.createLabel(scanScreen, "Scanning 433.92 MHz...");
    ui.setPosition(statusLabel, 10, 100);
    ui.setStyle(statusLabel, 'text_color', '#00FF00');
    
    const rssiLabel = ui.createLabel(scanScreen, "RSSI: -75 dBm");
    ui.setPosition(rssiLabel, 10, 130);
    ui.setStyle(rssiLabel, 'text_color', '#FFFF00');
    
    const backLabel = ui.createLabel(scanScreen, "Press Button 1 to return");
    ui.setPosition(backLabel, 10, 200);
    ui.setStyle(backLabel, 'text_color', '#CCCCCC');
    ui.setStyle(backLabel, 'text_font', 'small');
    
    ui.setActiveScreen(scanScreen);
    
    // Set up back navigation
    input.onButton('BUTTON1', () => {
        ui.setActiveScreen(menuScreen);
    });
}

// Update status bar information
function updateStatusBar() {
    if (!statusBar) return;
    
    // Get current time (placeholder)
    const now = new Date();
    const timeStr = now.getHours().toString().padStart(2, '0') + ':' + 
                   now.getMinutes().toString().padStart(2, '0');
    
    // Get Wi-Fi status
    const wifiConnected = false; // This would check actual Wi-Fi status
    
    // Get battery level (placeholder)
    const batteryLevel = 85;
    
    ui.updateStatusBar(statusBar, wifiConnected, batteryLevel, timeStr);
}

// Show system information
function showSystemInfo() {
    const info = [
        "T-Embed CC1101 v1.0.0",
        "ESP32-S3 @ 240MHz", 
        "Free RAM: " + system.getFreeHeap() + " bytes",
        "Uptime: " + system.getUptime() + " seconds"
    ].join("\n");
    
    notify.show("System Info", info, 5000);
}

// Show quick actions menu
function showQuickActions() {
    // Quick toggle for common functions
    notify.show("Quick Actions", "Wi-Fi | RF | Apps", 2000);
}

// Initialize the menu when app starts
initMainMenu();

// Export functions for other apps to use
if (typeof module !== 'undefined') {
    module.exports = {
        initMainMenu,
        navigateUp,
        navigateDown,
        selectCurrentItem
    };
}