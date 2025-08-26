/**
 * @file system_settings.js
 * @brief System Settings Application
 * 
 * Provides configuration interface for system settings including
 * display, RF parameters, Wi-Fi, and app management.
 */

// Settings categories
const SETTINGS_CATEGORIES = [
    {
        id: 'display',
        title: 'Display',
        icon: '🖥️',
        settings: [
            { key: 'brightness', name: 'Brightness', type: 'slider', min: 10, max: 255, value: 128 },
            { key: 'timeout', name: 'Screen Timeout', type: 'select', options: ['30s', '1m', '5m', 'Never'], value: '1m' },
            { key: 'theme', name: 'Theme', type: 'select', options: ['Dark', 'Light', 'Auto'], value: 'Dark' }
        ]
    },
    {
        id: 'rf',
        title: 'RF Settings',
        icon: '📡',
        settings: [
            { key: 'default_freq', name: 'Default Frequency', type: 'select', 
              options: ['315 MHz', '433.92 MHz', '868.3 MHz', '915 MHz'], value: '433.92 MHz' },
            { key: 'tx_power', name: 'TX Power', type: 'slider', min: -30, max: 10, value: 0 },
            { key: 'auto_gain', name: 'Auto Gain Control', type: 'toggle', value: true }
        ]
    },
    {
        id: 'wifi',
        title: 'Wi-Fi',
        icon: '📶',
        settings: [
            { key: 'auto_connect', name: 'Auto Connect', type: 'toggle', value: false },
            { key: 'ap_mode', name: 'AP Mode on Boot', type: 'toggle', value: false },
            { key: 'hostname', name: 'Hostname', type: 'text', value: 't-embed-cc1101' }
        ]
    },
    {
        id: 'system',
        title: 'System',
        icon: '⚙️',
        settings: [
            { key: 'log_level', name: 'Log Level', type: 'select', 
              options: ['Error', 'Warning', 'Info', 'Debug'], value: 'Info' },
            { key: 'auto_save', name: 'Auto Save Settings', type: 'toggle', value: true },
            { key: 'factory_reset', name: 'Factory Reset', type: 'action' }
        ]
    }
];

let settingsState = {
    currentCategory: 0,
    currentSetting: 0,
    screen: null,
    categoryList: null,
    settingsList: null,
    isEditMode: false,
    originalValues: {}
};

// Initialize settings app
function initSystemSettings() {
    console.log("Initializing System Settings...");
    
    // Load current settings from storage
    loadCurrentSettings();
    
    // Create settings UI
    createSettingsUI();
    
    // Set up input handlers
    setupSettingsInput();
    
    console.log("System Settings initialized");
}

// Load current settings from storage
function loadCurrentSettings() {
    console.log("Loading current settings...");
    
    for (const category of SETTINGS_CATEGORIES) {
        for (const setting of category.settings) {
            try {
                const savedValue = storage.getConfig(`${category.id}.${setting.key}`, setting.value.toString());
                
                // Convert saved value to appropriate type
                if (setting.type === 'toggle') {
                    setting.value = savedValue === 'true';
                } else if (setting.type === 'slider') {
                    setting.value = parseInt(savedValue);
                } else {
                    setting.value = savedValue;
                }
                
                settingsState.originalValues[`${category.id}.${setting.key}`] = setting.value;
                
            } catch (error) {
                console.log(`Failed to load setting ${category.id}.${setting.key}:`, error);
            }
        }
    }
}

// Create settings user interface
function createSettingsUI() {
    settingsState.screen = ui.createScreen();
    ui.setStyle(settingsState.screen, 'bg_color', '#000000');
    
    // Title bar
    const titleBar = ui.createContainer(settingsState.screen);
    ui.setSize(titleBar, 170, 30);
    ui.setPosition(titleBar, 0, 0);
    ui.setStyle(titleBar, 'bg_color', '#4CAF50');
    
    const titleLabel = ui.createLabel(titleBar, "Settings");
    ui.setPosition(titleLabel, 5, 5);
    ui.setStyle(titleLabel, 'text_color', '#FFFFFF');
    ui.setStyle(titleLabel, 'text_font', 'bold');
    
    // Create two-panel layout
    createCategoryPanel();
    createSettingsPanel();
    
    // Show initial category
    showCategory(0);
    
    ui.setActiveScreen(settingsState.screen);
}

// Create category selection panel
function createCategoryPanel() {
    const categoryPanel = ui.createContainer(settingsState.screen);
    ui.setSize(categoryPanel, 80, 290);
    ui.setPosition(categoryPanel, 0, 30);
    ui.setStyle(categoryPanel, 'border_width', 1);
    ui.setStyle(categoryPanel, 'border_color', '#333333');
    
    settingsState.categoryList = categoryPanel;
    
    // Create category items
    for (let i = 0; i < SETTINGS_CATEGORIES.length; i++) {
        const category = SETTINGS_CATEGORIES[i];
        const categoryItem = createCategoryItem(categoryPanel, category, i);
        
        // Highlight first category
        if (i === 0) {
            ui.setStyle(categoryItem, 'bg_color', '#2196F3');
        }
    }
}

// Create individual category item
function createCategoryItem(parent, category, index) {
    const itemContainer = ui.createContainer(parent);
    ui.setSize(itemContainer, 78, 60);
    ui.setPosition(itemContainer, 1, index * 62 + 5);
    ui.setStyle(itemContainer, 'border_width', 1);
    ui.setStyle(itemContainer, 'border_color', '#555555');
    
    // Icon
    const iconLabel = ui.createLabel(itemContainer, category.icon);
    ui.setPosition(iconLabel, 25, 10);
    ui.setStyle(iconLabel, 'text_color', '#FFFFFF');
    
    // Title
    const titleLabel = ui.createLabel(itemContainer, category.title);
    ui.setPosition(titleLabel, 5, 35);
    ui.setStyle(titleLabel, 'text_color', '#FFFFFF');
    ui.setStyle(titleLabel, 'text_font', 'small');
    
    return itemContainer;
}

// Create settings detail panel
function createSettingsPanel() {
    const settingsPanel = ui.createContainer(settingsState.screen);
    ui.setSize(settingsPanel, 89, 290);
    ui.setPosition(settingsPanel, 81, 30);
    ui.setStyle(settingsPanel, 'border_width', 1);
    ui.setStyle(settingsPanel, 'border_color', '#333333');
    
    settingsState.settingsList = settingsPanel;
}

// Show settings for selected category
function showCategory(categoryIndex) {
    // Update category highlight
    updateCategoryHighlight(categoryIndex);
    
    // Clear settings panel
    ui.clearChildren(settingsState.settingsList);
    
    const category = SETTINGS_CATEGORIES[categoryIndex];
    
    // Create setting items
    for (let i = 0; i < category.settings.length; i++) {
        const setting = category.settings[i];
        createSettingItem(settingsState.settingsList, setting, i);
    }
    
    settingsState.currentCategory = categoryIndex;
    settingsState.currentSetting = 0;
    updateSettingHighlight(0);
}

// Create individual setting item
function createSettingItem(parent, setting, index) {
    const itemContainer = ui.createContainer(parent);
    ui.setSize(itemContainer, 87, 40);
    ui.setPosition(itemContainer, 1, index * 42 + 5);
    ui.setStyle(itemContainer, 'border_width', 1);
    ui.setStyle(itemContainer, 'border_color', '#444444');
    
    // Setting name
    const nameLabel = ui.createLabel(itemContainer, setting.name);
    ui.setPosition(nameLabel, 5, 5);
    ui.setStyle(nameLabel, 'text_color', '#FFFFFF');
    ui.setStyle(nameLabel, 'text_font', 'small');
    
    // Setting value/control
    createSettingControl(itemContainer, setting);
    
    return itemContainer;
}

// Create setting control based on type
function createSettingControl(parent, setting) {
    const valueY = 22;
    
    switch (setting.type) {
        case 'toggle':
            const toggleLabel = ui.createLabel(parent, setting.value ? 'ON' : 'OFF');
            ui.setPosition(toggleLabel, 5, valueY);
            ui.setStyle(toggleLabel, 'text_color', setting.value ? '#00FF00' : '#FF0000');
            setting.control = toggleLabel;
            break;
            
        case 'slider':
            const sliderLabel = ui.createLabel(parent, setting.value.toString());
            ui.setPosition(sliderLabel, 5, valueY);
            ui.setStyle(sliderLabel, 'text_color', '#FFFF00');
            setting.control = sliderLabel;
            break;
            
        case 'select':
            const selectLabel = ui.createLabel(parent, setting.value);
            ui.setPosition(selectLabel, 5, valueY);
            ui.setStyle(selectLabel, 'text_color', '#00FFFF');
            setting.control = selectLabel;
            break;
            
        case 'text':
            const textLabel = ui.createLabel(parent, setting.value);
            ui.setPosition(textLabel, 5, valueY);
            ui.setStyle(textLabel, 'text_color', '#FFFFFF');
            setting.control = textLabel;
            break;
            
        case 'action':
            const actionLabel = ui.createLabel(parent, 'Press to execute');
            ui.setPosition(actionLabel, 5, valueY);
            ui.setStyle(actionLabel, 'text_color', '#FF9800');
            setting.control = actionLabel;
            break;
            
        default:
            const defaultLabel = ui.createLabel(parent, setting.value.toString());
            ui.setPosition(defaultLabel, 5, valueY);
            ui.setStyle(defaultLabel, 'text_color', '#CCCCCC');
            setting.control = defaultLabel;
    }
}

// Set up input handlers
function setupSettingsInput() {
    // Encoder for navigation
    input.onEncoder((direction) => {
        if (settingsState.isEditMode) {
            editCurrentSetting(direction);
        } else {
            if (direction === 'CW') {
                navigateDown();
            } else if (direction === 'CCW') {
                navigateUp();
            }
        }
    });
    
    // Encoder press for selection/edit
    input.onButton('ENCODER', () => {
        if (settingsState.isEditMode) {
            exitEditMode();
        } else {
            enterEditMode();
        }
    });
    
    // Button 1 for category switching
    input.onButton('BUTTON1', () => {
        if (!settingsState.isEditMode) {
            switchCategory();
        }
    });
    
    // Button 2 for back/save
    input.onButton('BUTTON2', () => {
        if (settingsState.isEditMode) {
            exitEditMode();
        } else {
            saveAndExit();
        }
    });
}

// Navigate settings up
function navigateUp() {
    const category = SETTINGS_CATEGORIES[settingsState.currentCategory];
    if (settingsState.currentSetting > 0) {
        updateSettingHighlight(settingsState.currentSetting - 1);
    }
}

// Navigate settings down
function navigateDown() {
    const category = SETTINGS_CATEGORIES[settingsState.currentCategory];
    if (settingsState.currentSetting < category.settings.length - 1) {
        updateSettingHighlight(settingsState.currentSetting + 1);
    }
}

// Update setting selection highlight
function updateSettingHighlight(newIndex) {
    // Implementation would update visual highlighting
    settingsState.currentSetting = newIndex;
    notify.vibrate(30);
}

// Update category selection highlight
function updateCategoryHighlight(newIndex) {
    // Implementation would update visual highlighting
    // This is simplified for the example
}

// Enter edit mode for current setting
function enterEditMode() {
    const category = SETTINGS_CATEGORIES[settingsState.currentCategory];
    const setting = category.settings[settingsState.currentSetting];
    
    if (setting.type === 'action') {
        executeAction(setting);
        return;
    }
    
    settingsState.isEditMode = true;
    notify.show("Edit", "Editing " + setting.name, 1000);
    console.log("Entering edit mode for:", setting.name);
}

// Exit edit mode
function exitEditMode() {
    settingsState.isEditMode = false;
    saveSetting();
    notify.show("Saved", "Setting saved", 1000);
}

// Edit current setting value
function editCurrentSetting(direction) {
    const category = SETTINGS_CATEGORIES[settingsState.currentCategory];
    const setting = category.settings[settingsState.currentSetting];
    
    switch (setting.type) {
        case 'toggle':
            setting.value = !setting.value;
            ui.setText(setting.control, setting.value ? 'ON' : 'OFF');
            ui.setStyle(setting.control, 'text_color', setting.value ? '#00FF00' : '#FF0000');
            break;
            
        case 'slider':
            const delta = direction === 'CW' ? 1 : -1;
            const newValue = Math.max(setting.min, Math.min(setting.max, setting.value + delta));
            setting.value = newValue;
            ui.setText(setting.control, newValue.toString());
            break;
            
        case 'select':
            const currentIndex = setting.options.indexOf(setting.value);
            const newIndex = direction === 'CW' ? 
                (currentIndex + 1) % setting.options.length :
                (currentIndex - 1 + setting.options.length) % setting.options.length;
            setting.value = setting.options[newIndex];
            ui.setText(setting.control, setting.value);
            break;
    }
}

// Save current setting
function saveSetting() {
    const category = SETTINGS_CATEGORIES[settingsState.currentCategory];
    const setting = category.settings[settingsState.currentSetting];
    const key = `${category.id}.${setting.key}`;
    
    try {
        storage.setConfig(key, setting.value.toString());
        console.log(`Saved setting ${key}:`, setting.value);
        
        // Apply setting immediately if needed
        applySetting(category.id, setting.key, setting.value);
        
    } catch (error) {
        console.log("Failed to save setting:", error);
        notify.show("Error", "Failed to save setting", 2000);
    }
}

// Apply setting changes immediately
function applySetting(categoryId, settingKey, value) {
    switch (categoryId) {
        case 'display':
            if (settingKey === 'brightness') {
                // Apply brightness change immediately
                system.setBacklightBrightness(value);
            }
            break;
            
        case 'rf':
            if (settingKey === 'default_freq') {
                // Set default RF frequency
                const freqMap = {
                    '315 MHz': 315000000,
                    '433.92 MHz': 433920000,
                    '868.3 MHz': 868300000,
                    '915 MHz': 915000000
                };
                rf.setFrequency(freqMap[value]);
            }
            break;
    }
}

// Execute action setting
function executeAction(setting) {
    switch (setting.key) {
        case 'factory_reset':
            showFactoryResetDialog();
            break;
            
        default:
            notify.show("Action", "Action not implemented", 2000);
    }
}

// Show factory reset confirmation
function showFactoryResetDialog() {
    notify.show("Warning", "Factory reset will erase all settings. Press encoder to confirm.", 5000);
    
    // Wait for confirmation
    const confirmHandler = () => {
        performFactoryReset();
        input.removeHandler('ENCODER', confirmHandler);
    };
    
    input.onButton('ENCODER', confirmHandler);
}

// Perform factory reset
function performFactoryReset() {
    console.log("Performing factory reset...");
    
    try {
        // Clear all stored settings
        for (const category of SETTINGS_CATEGORIES) {
            for (const setting of category.settings) {
                storage.deleteConfig(`${category.id}.${setting.key}`);
            }
        }
        
        notify.show("Reset", "Factory reset complete. Restarting...", 3000);
        
        // Restart system after delay
        setTimeout(() => {
            system.restart();
        }, 3000);
        
    } catch (error) {
        console.log("Factory reset failed:", error);
        notify.show("Error", "Factory reset failed", 3000);
    }
}

// Switch between categories
function switchCategory() {
    const nextCategory = (settingsState.currentCategory + 1) % SETTINGS_CATEGORIES.length;
    showCategory(nextCategory);
    notify.vibrate(50);
}

// Save all settings and exit
function saveAndExit() {
    console.log("Saving settings and exiting...");
    
    // Save any pending changes
    if (settingsState.isEditMode) {
        saveSetting();
    }
    
    notify.show("Settings", "Settings saved", 1000);
    
    // Return to main menu
    app.exit();
}

// Initialize settings app
initSystemSettings();

// Export functions
if (typeof module !== 'undefined') {
    module.exports = {
        initSystemSettings,
        loadCurrentSettings,
        saveAndExit
    };
}