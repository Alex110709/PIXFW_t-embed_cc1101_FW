/**
 * UI Showcase - T-Embed CC1101 UI Components Demo
 * Demonstrates various LVGL UI components and interactions
 */

// App state
let appState = {
    sliderValue: 50,
    switchState: false,
    dropdownSelection: "Option 1",
    progressValue: 0,
    textInput: "",
    tabIndex: 0
};

// UI elements
let ui = {};

// Animation timer
let progressTimer;

// App initialization
function init() {
    console.log("UI Showcase starting...");
    
    try {
        createUI();
        setupEventHandlers();
        startAnimations();
        
        console.log("UI Showcase initialized successfully");
        updateStatus("UI components ready for interaction");
        
    } catch (error) {
        console.error("UI Showcase initialization error:", error);
        Notification.showError("UI Showcase failed to start: " + error.message);
    }
}

// Create user interface
function createUI() {
    const screen = UI.getScreen();
    
    // Create tab view for different component categories
    const tabView = UI.createTabView(screen);
    
    // Basic Components tab
    const basicTab = UI.addTab(tabView, "Basic");
    createBasicComponentsTab(basicTab);
    
    // Input Components tab
    const inputTab = UI.addTab(tabView, "Input");
    createInputComponentsTab(inputTab);
    
    // Display Components tab
    const displayTab = UI.addTab(tabView, "Display");
    createDisplayComponentsTab(displayTab);
    
    // Interactive tab
    const interactiveTab = UI.addTab(tabView, "Interactive");
    createInteractiveTab(interactiveTab);
}

// Create basic components tab
function createBasicComponentsTab(parent) {
    const container = UI.createContainer(parent);
    UI.setContainerLayout(container, UI.LAYOUT_FLEX_COLUMN);
    
    // Status label
    ui.statusLabel = UI.createLabel(container, "Ready for interaction");
    UI.setLabelStyle(ui.statusLabel, {
        fontSize: 14,
        fontWeight: "bold",
        textAlign: "center",
        marginBottom: 15
    });
    
    // Title label
    const titleLabel = UI.createLabel(container, "UI Component Showcase");
    UI.setLabelStyle(titleLabel, {
        fontSize: 20,
        fontWeight: "bold",
        textAlign: "center",
        marginBottom: 10
    });
    
    // Description label
    const descLabel = UI.createLabel(container, "Explore T-Embed UI capabilities");
    UI.setLabelStyle(descLabel, {
        fontSize: 12,
        textAlign: "center",
        marginBottom: 20
    });
    
    // Basic buttons
    const buttonContainer = UI.createContainer(container);
    UI.setContainerLayout(buttonContainer, UI.LAYOUT_FLEX_ROW);
    
    const infoButton = UI.createButton(buttonContainer, "Info");
    const warningButton = UI.createButton(buttonContainer, "Warning");
    const successButton = UI.createButton(buttonContainer, "Success");
    
    UI.setButtonCallback(infoButton, () => {
        Notification.show("Information message", 2000);
        updateStatus("Info button pressed");
    });
    
    UI.setButtonCallback(warningButton, () => {
        Notification.showWarning("Warning message", 2000);
        updateStatus("Warning button pressed");
    });
    
    UI.setButtonCallback(successButton, () => {
        Notification.show("Success message! 🎉", 2000);
        updateStatus("Success button pressed");
    });
    
    // Switch and checkbox
    const toggleContainer = UI.createContainer(container);
    UI.setContainerLayout(toggleContainer, UI.LAYOUT_FLEX_ROW);
    
    const switchLabel = UI.createLabel(toggleContainer, "Switch:");
    ui.mainSwitch = UI.createSwitch(toggleContainer);
    ui.switchStatusLabel = UI.createLabel(toggleContainer, "OFF");
    
    UI.setSwitchCallback(ui.mainSwitch, onSwitchToggle);
    
    // Slider
    const sliderContainer = UI.createContainer(container);
    UI.setContainerLayout(sliderContainer, UI.LAYOUT_FLEX_COLUMN);
    
    const sliderLabel = UI.createLabel(sliderContainer, "Value Slider:");
    ui.valueSlider = UI.createSlider(sliderContainer);
    ui.sliderValueLabel = UI.createLabel(sliderContainer, "50");
    
    UI.setSliderRange(ui.valueSlider, 0, 100);
    UI.setSliderValue(ui.valueSlider, 50);
    UI.setSliderCallback(ui.valueSlider, onSliderChange);
}

// Create input components tab
function createInputComponentsTab(parent) {
    const container = UI.createContainer(parent);
    UI.setContainerLayout(container, UI.LAYOUT_FLEX_COLUMN);
    
    // Dropdown
    const dropdownContainer = UI.createContainer(container);
    UI.setContainerLayout(dropdownContainer, UI.LAYOUT_FLEX_COLUMN);
    
    const dropdownLabel = UI.createLabel(dropdownContainer, "Select Option:");
    ui.dropdown = UI.createDropdown(dropdownContainer);
    
    UI.addDropdownOption(ui.dropdown, "Option 1");
    UI.addDropdownOption(ui.dropdown, "Option 2");
    UI.addDropdownOption(ui.dropdown, "Option 3");
    UI.addDropdownOption(ui.dropdown, "Custom Option");
    
    UI.setDropdownCallback(ui.dropdown, onDropdownChange);
    
    ui.dropdownStatus = UI.createLabel(dropdownContainer, "Selected: Option 1");
    UI.setLabelStyle(ui.dropdownStatus, { fontSize: 12, marginTop: 5 });
    
    // Text area
    const textContainer = UI.createContainer(container);
    UI.setContainerLayout(textContainer, UI.LAYOUT_FLEX_COLUMN);
    
    const textLabel = UI.createLabel(textContainer, "Text Input:");
    ui.textArea = UI.createTextArea(textContainer);
    UI.setTextAreaPlaceholder(ui.textArea, "Type your message here...");
    
    const textButtons = UI.createContainer(textContainer);
    UI.setContainerLayout(textButtons, UI.LAYOUT_FLEX_ROW);
    
    const clearTextButton = UI.createButton(textButtons, "Clear");
    const saveTextButton = UI.createButton(textButtons, "Save");
    
    UI.setButtonCallback(clearTextButton, clearTextArea);
    UI.setButtonCallback(saveTextButton, saveTextContent);
    
    // Number input simulation
    const numberContainer = UI.createContainer(container);
    UI.setContainerLayout(numberContainer, UI.LAYOUT_FLEX_ROW);
    
    const numberLabel = UI.createLabel(numberContainer, "Number:");
    const minusButton = UI.createButton(numberContainer, "-");
    ui.numberDisplay = UI.createLabel(numberContainer, "0");
    const plusButton = UI.createButton(numberContainer, "+");
    
    let numberValue = 0;
    
    UI.setButtonCallback(minusButton, () => {
        numberValue = Math.max(0, numberValue - 1);
        UI.setLabelText(ui.numberDisplay, numberValue.toString());
        updateStatus(`Number: ${numberValue}`);
    });
    
    UI.setButtonCallback(plusButton, () => {
        numberValue = Math.min(100, numberValue + 1);
        UI.setLabelText(ui.numberDisplay, numberValue.toString());
        updateStatus(`Number: ${numberValue}`);
    });
}

// Create display components tab
function createDisplayComponentsTab(parent) {
    const container = UI.createContainer(parent);
    UI.setContainerLayout(container, UI.LAYOUT_FLEX_COLUMN);
    
    // Progress bar
    const progressContainer = UI.createContainer(container);
    UI.setContainerLayout(progressContainer, UI.LAYOUT_FLEX_COLUMN);
    
    const progressLabel = UI.createLabel(progressContainer, "Progress Bar:");
    ui.progressBar = UI.createProgressBar(progressContainer);
    ui.progressValueLabel = UI.createLabel(progressContainer, "0%");
    
    UI.setProgressBarRange(ui.progressBar, 0, 100);
    UI.setProgressBarValue(ui.progressBar, 0);
    
    const progressButtons = UI.createContainer(progressContainer);
    UI.setContainerLayout(progressButtons, UI.LAYOUT_FLEX_ROW);
    
    const startProgressButton = UI.createButton(progressButtons, "Start");
    const stopProgressButton = UI.createButton(progressButtons, "Stop");
    const resetProgressButton = UI.createButton(progressButtons, "Reset");
    
    UI.setButtonCallback(startProgressButton, startProgress);
    UI.setButtonCallback(stopProgressButton, stopProgress);
    UI.setButtonCallback(resetProgressButton, resetProgress);
    
    // List component
    const listContainer = UI.createContainer(container);
    UI.setContainerLayout(listContainer, UI.LAYOUT_FLEX_COLUMN);
    
    const listLabel = UI.createLabel(listContainer, "Dynamic List:");
    ui.itemList = UI.createList(listContainer);
    
    // Add initial items
    UI.addListItem(ui.itemList, "🎯 Basic item");
    UI.addListItem(ui.itemList, "⭐ Important item");
    UI.addListItem(ui.itemList, "📱 Device info");
    UI.addListItem(ui.itemList, "🔧 Settings");
    
    const listButtons = UI.createContainer(listContainer);
    UI.setContainerLayout(listButtons, UI.LAYOUT_FLEX_ROW);
    
    const addItemButton = UI.createButton(listButtons, "Add");
    const clearListButton = UI.createButton(listButtons, "Clear");
    
    UI.setButtonCallback(addItemButton, addListItem);
    UI.setButtonCallback(clearListButton, () => {
        UI.clearList(ui.itemList);
        updateStatus("List cleared");
    });
    
    // Chart simulation
    const chartContainer = UI.createContainer(container);
    UI.setContainerLayout(chartContainer, UI.LAYOUT_FLEX_COLUMN);
    
    const chartLabel = UI.createLabel(chartContainer, "Data Visualization:");
    ui.chartDisplay = UI.createLabel(chartContainer, "📊 ████████░░ 80%");
    UI.setLabelStyle(ui.chartDisplay, { 
        fontSize: 14, 
        fontFamily: "monospace",
        textAlign: "center",
        backgroundColor: "#f0f0f0",
        padding: 10
    });
}

// Create interactive tab
function createInteractiveTab(parent) {
    const container = UI.createContainer(parent);
    UI.setContainerLayout(container, UI.LAYOUT_FLEX_COLUMN);
    
    // Color picker simulation
    const colorContainer = UI.createContainer(container);
    UI.setContainerLayout(colorContainer, UI.LAYOUT_FLEX_COLUMN);
    
    const colorLabel = UI.createLabel(colorContainer, "Color Picker:");
    
    // RGB sliders
    ui.redSlider = createColorSlider(colorContainer, "Red", 255);
    ui.greenSlider = createColorSlider(colorContainer, "Green", 128);
    ui.blueSlider = createColorSlider(colorContainer, "Blue", 64);
    
    // Color preview
    ui.colorPreview = UI.createContainer(colorContainer);
    UI.setContainerStyle(ui.colorPreview, {
        backgroundColor: "#FF8040",
        height: 30,
        border: "2px solid #000",
        borderRadius: 5
    });
    
    // Animation controls
    const animContainer = UI.createContainer(container);
    UI.setContainerLayout(animContainer, UI.LAYOUT_FLEX_COLUMN);
    
    const animLabel = UI.createLabel(animContainer, "Animations:");
    
    const animButtons = UI.createContainer(animContainer);
    UI.setContainerLayout(animButtons, UI.LAYOUT_FLEX_ROW);
    
    const bounceButton = UI.createButton(animButtons, "Bounce");
    const fadeButton = UI.createButton(animButtons, "Fade");
    const rotateButton = UI.createButton(animButtons, "Rotate");
    
    UI.setButtonCallback(bounceButton, () => {
        updateStatus("Bounce animation started");
        Notification.show("Bouncing! 🎾", 1500);
    });
    
    UI.setButtonCallback(fadeButton, () => {
        updateStatus("Fade animation started");
        Notification.show("Fading... 👻", 1500);
    });
    
    UI.setButtonCallback(rotateButton, () => {
        updateStatus("Rotation animation started");
        Notification.show("Spinning! 🌀", 1500);
    });
    
    // Game-like interaction
    const gameContainer = UI.createContainer(container);
    UI.setContainerLayout(gameContainer, UI.LAYOUT_FLEX_COLUMN);
    
    const gameLabel = UI.createLabel(gameContainer, "Mini Game:");
    ui.gameDisplay = UI.createLabel(gameContainer, "🎯 Click to play!");
    ui.scoreLabel = UI.createLabel(gameContainer, "Score: 0");
    
    let gameScore = 0;
    
    const gameButton = UI.createButton(gameContainer, "🎮 Play");
    UI.setButtonCallback(gameButton, () => {
        gameScore++;
        UI.setLabelText(ui.scoreLabel, `Score: ${gameScore}`);
        UI.setLabelText(ui.gameDisplay, `🎉 Hit! Score: ${gameScore}`);
        updateStatus(`Game score: ${gameScore}`);
        
        if (gameScore % 5 === 0) {
            Notification.show(`Level up! Score: ${gameScore} 🏆`, 2000);
        }
    });
}

// Create color slider helper
function createColorSlider(parent, name, value) {
    const container = UI.createContainer(parent);
    UI.setContainerLayout(container, UI.LAYOUT_FLEX_ROW);
    
    const label = UI.createLabel(container, name);
    UI.setLabelStyle(label, { minWidth: 50, fontSize: 12 });
    
    const slider = UI.createSlider(container);
    UI.setSliderRange(slider, 0, 255);
    UI.setSliderValue(slider, value);
    
    const valueLabel = UI.createLabel(container, value.toString());
    UI.setLabelStyle(valueLabel, { minWidth: 30, fontSize: 12 });
    
    UI.setSliderCallback(slider, (newValue) => {
        UI.setLabelText(valueLabel, newValue.toString());
        updateColorPreview();
        updateStatus(`${name}: ${newValue}`);
    });
    
    return { slider, valueLabel };
}

// Event handlers
function setupEventHandlers() {
    System.onBackButton(function() {
        console.log("Stopping UI Showcase...");
        cleanup();
        System.exit();
    });
    
    System.onPause(function() {
        console.log("UI Showcase paused");
        stopProgress();
    });
    
    System.onResume(function() {
        console.log("UI Showcase resumed");
        updateStatus("Resumed - Ready for interaction");
    });
}

// Component event handlers
function onSwitchToggle(state) {
    appState.switchState = state;
    UI.setLabelText(ui.switchStatusLabel, state ? "ON" : "OFF");
    updateStatus(`Switch turned ${state ? 'ON' : 'OFF'}`);
    
    if (state) {
        Notification.show("Switch activated! ✅", 1500);
    }
}

function onSliderChange(value) {
    appState.sliderValue = value;
    UI.setLabelText(ui.sliderValueLabel, value.toString());
    updateStatus(`Slider value: ${value}`);
    
    // Update progress bar to match slider
    UI.setProgressBarValue(ui.progressBar, value);
    UI.setLabelText(ui.progressValueLabel, `${value}%`);
}

function onDropdownChange(selection) {
    appState.dropdownSelection = selection;
    UI.setLabelText(ui.dropdownStatus, `Selected: ${selection}`);
    updateStatus(`Dropdown: ${selection}`);
    
    Notification.show(`Selected: ${selection}`, 1500);
}

// Text area functions
function clearTextArea() {
    UI.setTextAreaValue(ui.textArea, "");
    updateStatus("Text area cleared");
}

function saveTextContent() {
    const text = UI.getTextAreaValue(ui.textArea);
    if (text.trim()) {
        try {
            Storage.writeFile("/temp/ui-showcase-text.txt", text);
            updateStatus("Text saved successfully");
            Notification.show("Text saved! 💾", 2000);
        } catch (error) {
            updateStatus("Failed to save text");
            Notification.showError("Save failed: " + error.message);
        }
    } else {
        updateStatus("No text to save");
        Notification.showWarning("Text area is empty", 1500);
    }
}

// Progress bar functions
function startProgress() {
    if (progressTimer) return;
    
    let progress = UI.getProgressBarValue(ui.progressBar);
    
    progressTimer = setInterval(() => {
        progress += 2;
        if (progress > 100) {
            progress = 100;
            stopProgress();
            Notification.show("Progress completed! 🎉", 2000);
        }
        
        UI.setProgressBarValue(ui.progressBar, progress);
        UI.setLabelText(ui.progressValueLabel, `${progress}%`);
        updateStatus(`Progress: ${progress}%`);
    }, 100);
    
    updateStatus("Progress animation started");
}

function stopProgress() {
    if (progressTimer) {
        clearInterval(progressTimer);
        progressTimer = null;
        updateStatus("Progress animation stopped");
    }
}

function resetProgress() {
    stopProgress();
    UI.setProgressBarValue(ui.progressBar, 0);
    UI.setLabelText(ui.progressValueLabel, "0%");
    updateStatus("Progress reset");
}

// List functions
function addListItem() {
    const itemCount = UI.getListItemCount(ui.itemList);
    const newItem = `📋 Item ${itemCount + 1}`;
    UI.addListItem(ui.itemList, newItem);
    updateStatus(`Added: ${newItem}`);
}

// Color functions
function updateColorPreview() {
    const r = UI.getSliderValue(ui.redSlider.slider);
    const g = UI.getSliderValue(ui.greenSlider.slider);
    const b = UI.getSliderValue(ui.blueSlider.slider);
    
    const hex = `#${r.toString(16).padStart(2, '0')}${g.toString(16).padStart(2, '0')}${b.toString(16).padStart(2, '0')}`;
    UI.setContainerStyle(ui.colorPreview, { backgroundColor: hex });
}

// Animation functions
function startAnimations() {
    // Start some subtle background animations
    setInterval(() => {
        const charts = ["📊 ████████░░ 80%", "📈 ██████████ 100%", "📉 ████░░░░░░ 40%", "📊 ███████░░░ 70%"];
        const randomChart = charts[Math.floor(Math.random() * charts.length)];
        UI.setLabelText(ui.chartDisplay, randomChart);
    }, 3000);
}

// Utility functions
function updateStatus(status) {
    UI.setLabelText(ui.statusLabel, status);
}

// Cleanup function
function cleanup() {
    console.log("Cleaning up UI Showcase...");
    stopProgress();
}

// App entry point
init();