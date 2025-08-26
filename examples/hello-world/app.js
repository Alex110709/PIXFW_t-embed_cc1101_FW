/**
 * Hello World - T-Embed CC1101 JavaScript App
 * Demonstrates basic UI creation and event handling
 */

// Global variables
let clickCount = 0;
let statusLabel;

// App initialization
function init() {
    console.log("Hello World app starting...");
    
    // Initialize UI
    createUI();
    
    // Set up event handlers
    setupEventHandlers();
    
    console.log("Hello World app initialized successfully");
}

// Create user interface
function createUI() {
    // Get main screen
    const screen = UI.getScreen();
    
    // Create main container
    const container = UI.createContainer(screen);
    UI.setContainerLayout(container, UI.LAYOUT_FLEX_COLUMN);
    
    // Create title label
    const title = UI.createLabel(container, "Hello World!");
    UI.setLabelStyle(title, {
        fontSize: 24,
        fontWeight: "bold",
        textAlign: "center",
        marginBottom: 20
    });
    
    // Create description label
    const description = UI.createLabel(container, "Welcome to T-Embed CC1101");
    UI.setLabelStyle(description, {
        fontSize: 16,
        textAlign: "center",
        marginBottom: 30
    });
    
    // Create click counter button
    const clickButton = UI.createButton(container, "Click Me!");
    UI.setButtonCallback(clickButton, onButtonClick);
    UI.setButtonStyle(clickButton, {
        marginBottom: 20,
        padding: 15,
        minWidth: 120
    });
    
    // Create status label
    statusLabel = UI.createLabel(container, "Ready to interact");
    UI.setLabelStyle(statusLabel, {
        fontSize: 14,
        textAlign: "center",
        marginBottom: 20
    });
    
    // Create info section
    const infoContainer = UI.createContainer(container);
    UI.setContainerLayout(infoContainer, UI.LAYOUT_FLEX_COLUMN);
    
    const deviceInfo = UI.createLabel(infoContainer, "Device: T-Embed CC1101");
    const sdkInfo = UI.createLabel(infoContainer, "SDK: JavaScript 1.0");
    
    UI.setLabelStyle(deviceInfo, { fontSize: 12, textAlign: "center" });
    UI.setLabelStyle(sdkInfo, { fontSize: 12, textAlign: "center" });
    
    // Create action buttons
    const buttonContainer = UI.createContainer(container);
    UI.setContainerLayout(buttonContainer, UI.LAYOUT_FLEX_ROW);
    
    const showInfoButton = UI.createButton(buttonContainer, "Info");
    const resetButton = UI.createButton(buttonContainer, "Reset");
    
    UI.setButtonCallback(showInfoButton, showDeviceInfo);
    UI.setButtonCallback(resetButton, resetCounter);
    
    UI.setButtonStyle(showInfoButton, { margin: 5 });
    UI.setButtonStyle(resetButton, { margin: 5 });
}

// Event handlers
function setupEventHandlers() {
    // Handle back button
    System.onBackButton(function() {
        console.log("Back button pressed - exiting app");
        System.exit();
    });
    
    // Handle app lifecycle
    System.onPause(function() {
        console.log("App paused");
        UI.setLabelText(statusLabel, "App paused");
    });
    
    System.onResume(function() {
        console.log("App resumed");
        UI.setLabelText(statusLabel, "App resumed");
    });
}

// Button click handler
function onButtonClick() {
    clickCount++;
    console.log(`Button clicked ${clickCount} times`);
    
    // Update status
    UI.setLabelText(statusLabel, `Clicked ${clickCount} time${clickCount === 1 ? '' : 's'}`);
    
    // Show notification
    let message;
    if (clickCount === 1) {
        message = "First click!";
    } else if (clickCount === 5) {
        message = "You're getting good at this!";
    } else if (clickCount === 10) {
        message = "Click master! 🎉";
    } else if (clickCount % 10 === 0) {
        message = `${clickCount} clicks! Amazing!`;
    } else {
        message = `Click ${clickCount}!`;
    }
    
    Notification.show(message, 1500);
}

// Show device information
function showDeviceInfo() {
    try {
        const deviceInfo = System.getDeviceInfo();
        const batteryLevel = System.getBatteryLevel();
        const memoryInfo = System.getMemoryInfo();
        
        const info = [
            `Device: ${deviceInfo.model || 'T-Embed CC1101'}`,
            `Firmware: ${deviceInfo.firmware || '1.0.0'}`,
            `Battery: ${batteryLevel}%`,
            `Memory: ${Math.round(memoryInfo.free / 1024)}KB free`
        ].join('\\n');
        
        Notification.show(info, 4000);
        console.log("Device Info:", deviceInfo);
    } catch (error) {
        console.error("Failed to get device info:", error);
        Notification.show("Device info unavailable", 2000);
    }
}

// Reset click counter
function resetCounter() {
    const oldCount = clickCount;
    clickCount = 0;
    
    UI.setLabelText(statusLabel, "Counter reset");
    Notification.show(`Reset from ${oldCount} to 0`, 2000);
    
    console.log(`Click counter reset from ${oldCount} to 0`);
}

// Cleanup function
function cleanup() {
    console.log("Cleaning up Hello World app...");
    // No specific cleanup needed for this simple app
}

// App entry point
init();

// Export functions for testing (if needed)
if (typeof module !== 'undefined' && module.exports) {
    module.exports = {
        init,
        onButtonClick,
        showDeviceInfo,
        resetCounter,
        cleanup
    };
}