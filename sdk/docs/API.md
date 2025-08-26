# T-Embed CC1101 JavaScript API Reference

This document describes the JavaScript APIs available for T-Embed CC1101 app development.

## Overview

The T-Embed CC1101 provides a comprehensive JavaScript API that allows developers to create powerful apps with access to hardware features including RF communication, GPIO control, UI components, and system services.

## API Categories

### RF (Radio Frequency)
Control the CC1101 radio module for Sub-GHz communication.

### GPIO (General Purpose I/O)
Interface with digital and analog pins.

### UI (User Interface)
Create graphical user interfaces using LVGL components.

### Storage
Read and write files to the device filesystem.

### Notification
Display system notifications and alerts.

### System
Access system functions and app lifecycle events.

---

## RF API

The RF API provides access to the CC1101 radio module for Sub-GHz communication.

**Required Permission:** `rf.receive`, `rf.transmit`

### RF.setFrequency(frequency)

Set the radio frequency in MHz.

**Parameters:**
- `frequency` (number): Frequency in MHz (315, 433, 868, 915, or custom)

**Returns:** `boolean` - Success status

**Example:**
```javascript
RF.setFrequency(433.92); // Set to 433.92 MHz
```

### RF.setModulation(modulation)

Set the modulation type.

**Parameters:**
- `modulation` (number): Modulation type
  - `RF.MOD_ASK` - Amplitude Shift Keying
  - `RF.MOD_FSK` - Frequency Shift Keying
  - `RF.MOD_MSK` - Minimum Shift Keying

**Example:**
```javascript
RF.setModulation(RF.MOD_ASK);
```

### RF.setDataRate(rate)

Set the data transmission rate.

**Parameters:**
- `rate` (number): Data rate in kBaud

**Example:**
```javascript
RF.setDataRate(4.8); // 4.8 kBaud
```

### RF.setPower(power)

Set the transmission power.

**Parameters:**
- `power` (number): Power level in dBm (-30 to 10)

**Example:**
```javascript
RF.setPower(10); // Maximum power
```

### RF.transmit(data, callback)

Transmit data over radio.

**Parameters:**
- `data` (string): Data to transmit
- `callback` (function): Callback function with success status

**Example:**
```javascript
RF.transmit("Hello World", function(success) {
    if (success) {
        console.log("Transmission successful");
    } else {
        console.log("Transmission failed");
    }
});
```

### RF.startReceive(callback)

Start receiving radio data.

**Parameters:**
- `callback` (function): Callback function called when data is received

**Example:**
```javascript
RF.startReceive(function(data) {
    console.log("Received:", data);
    Notification.show("Data received: " + data, 3000);
});
```

### RF.stopReceive()

Stop receiving radio data.

**Example:**
```javascript
RF.stopReceive();
```

---

## GPIO API

The GPIO API provides access to digital and analog pins.

**Required Permission:** `gpio.read`, `gpio.write`

### Constants

```javascript
// Pin modes
GPIO.INPUT          = 0
GPIO.OUTPUT         = 1
GPIO.INPUT_PULLUP   = 2
GPIO.INPUT_PULLDOWN = 3

// Digital values
GPIO.LOW  = 0
GPIO.HIGH = 1

// Interrupt types
GPIO.RISING  = 1
GPIO.FALLING = 2
GPIO.CHANGE  = 3
```

### GPIO.pinMode(pin, mode)

Configure pin mode.

**Parameters:**
- `pin` (number): GPIO pin number
- `mode` (number): Pin mode (INPUT, OUTPUT, INPUT_PULLUP, INPUT_PULLDOWN)

**Example:**
```javascript
GPIO.pinMode(2, GPIO.OUTPUT);      // LED pin
GPIO.pinMode(0, GPIO.INPUT_PULLUP); // Button pin
```

### GPIO.digitalWrite(pin, value)

Write digital value to pin.

**Parameters:**
- `pin` (number): GPIO pin number
- `value` (number): Value to write (LOW or HIGH)

**Example:**
```javascript
GPIO.digitalWrite(2, GPIO.HIGH); // Turn LED on
GPIO.digitalWrite(2, GPIO.LOW);  // Turn LED off
```

### GPIO.digitalRead(pin)

Read digital value from pin.

**Parameters:**
- `pin` (number): GPIO pin number

**Returns:** `number` - Pin value (LOW or HIGH)

**Example:**
```javascript
let buttonState = GPIO.digitalRead(0);
if (buttonState === GPIO.LOW) {
    console.log("Button pressed");
}
```

### GPIO.analogRead(pin)

Read analog value from pin.

**Parameters:**
- `pin` (number): Analog pin number

**Returns:** `number` - Analog value (0-4095)

**Example:**
```javascript
let sensorValue = GPIO.analogRead(4);
let voltage = (sensorValue / 4095.0) * 3.3;
console.log("Sensor voltage:", voltage);
```

### GPIO.setInterrupt(pin, type, callback)

Set up pin interrupt.

**Parameters:**
- `pin` (number): GPIO pin number
- `type` (number): Interrupt type (RISING, FALLING, CHANGE)
- `callback` (function): Interrupt callback function

**Example:**
```javascript
GPIO.setInterrupt(0, GPIO.FALLING, function() {
    console.log("Button pressed!");
    Notification.show("Button pressed", 1000);
});
```

---

## UI API

The UI API provides access to LVGL user interface components.

**Required Permission:** `ui.create`

### UI.getScreen()

Get the main screen object.

**Returns:** `object` - Screen object

### UI.createContainer(parent)

Create a container widget.

**Parameters:**
- `parent` (object): Parent widget

**Returns:** `object` - Container widget

### UI.createLabel(parent, text)

Create a label widget.

**Parameters:**
- `parent` (object): Parent widget
- `text` (string): Label text

**Returns:** `object` - Label widget

**Example:**
```javascript
const screen = UI.getScreen();
const label = UI.createLabel(screen, "Hello World");
```

### UI.createButton(parent, text)

Create a button widget.

**Parameters:**
- `parent` (object): Parent widget
- `text` (string): Button text

**Returns:** `object` - Button widget

**Example:**
```javascript
const button = UI.createButton(screen, "Click Me");
UI.setButtonCallback(button, function() {
    console.log("Button clicked!");
});
```

### UI.createSlider(parent)

Create a slider widget.

**Parameters:**
- `parent` (object): Parent widget

**Returns:** `object` - Slider widget

**Example:**
```javascript
const slider = UI.createSlider(screen);
UI.setSliderRange(slider, 0, 100);
UI.setSliderValue(slider, 50);
```

### UI.createSwitch(parent)

Create a switch widget.

**Parameters:**
- `parent` (object): Parent widget

**Returns:** `object` - Switch widget

### UI.setButtonCallback(button, callback)

Set button click callback.

**Parameters:**
- `button` (object): Button widget
- `callback` (function): Click callback

### UI.setSliderCallback(slider, callback)

Set slider value change callback.

**Parameters:**
- `slider` (object): Slider widget
- `callback` (function): Value change callback

### UI.setLabelText(label, text)

Set label text.

**Parameters:**
- `label` (object): Label widget
- `text` (string): New text

### UI.setSliderRange(slider, min, max)

Set slider range.

**Parameters:**
- `slider` (object): Slider widget
- `min` (number): Minimum value
- `max` (number): Maximum value

### UI.setSliderValue(slider, value)

Set slider value.

**Parameters:**
- `slider` (object): Slider widget
- `value` (number): Slider value

---

## Storage API

The Storage API provides file system access.

**Required Permission:** `storage.read`, `storage.write`

### Storage.readFile(path)

Read file contents.

**Parameters:**
- `path` (string): File path

**Returns:** `string` - File contents

**Example:**
```javascript
try {
    let config = Storage.readFile("/config/app.json");
    let settings = JSON.parse(config);
} catch (error) {
    console.error("Failed to read config:", error);
}
```

### Storage.writeFile(path, data)

Write data to file.

**Parameters:**
- `path` (string): File path
- `data` (string): Data to write

**Example:**
```javascript
let settings = { theme: "dark", volume: 50 };
let config = JSON.stringify(settings, null, 2);
Storage.writeFile("/config/app.json", config);
```

### Storage.deleteFile(path)

Delete a file.

**Parameters:**
- `path` (string): File path

### Storage.listFiles(path)

List files in directory.

**Parameters:**
- `path` (string): Directory path

**Returns:** `array` - Array of file names

**Example:**
```javascript
let files = Storage.listFiles("/apps");
console.log("Installed apps:", files);
```

### Storage.fileExists(path)

Check if file exists.

**Parameters:**
- `path` (string): File path

**Returns:** `boolean` - File exists

### Storage.makeDirectory(path)

Create directory.

**Parameters:**
- `path` (string): Directory path

---

## Notification API

The Notification API provides system notifications.

**Required Permission:** None

### Notification.show(message, duration)

Show a notification message.

**Parameters:**
- `message` (string): Notification message
- `duration` (number): Duration in milliseconds (optional, default: 3000)

**Example:**
```javascript
Notification.show("App started successfully", 2000);
```

### Notification.showError(message, duration)

Show an error notification.

**Parameters:**
- `message` (string): Error message
- `duration` (number): Duration in milliseconds (optional, default: 5000)

### Notification.showWarning(message, duration)

Show a warning notification.

**Parameters:**
- `message` (string): Warning message
- `duration` (number): Duration in milliseconds (optional, default: 4000)

---

## System API

The System API provides access to system functions and app lifecycle.

**Required Permission:** `system`

### System.exit()

Exit the current app.

**Example:**
```javascript
System.onBackButton(function() {
    System.exit();
});
```

### System.restart()

Restart the device.

### System.getDeviceInfo()

Get device information.

**Returns:** `object` - Device info

**Example:**
```javascript
let info = System.getDeviceInfo();
console.log("Device:", info.model);
console.log("Firmware:", info.firmware);
```

### System.onBackButton(callback)

Set back button handler.

**Parameters:**
- `callback` (function): Back button callback

### System.onPause(callback)

Set app pause handler.

**Parameters:**
- `callback` (function): Pause callback

### System.onResume(callback)

Set app resume handler.

**Parameters:**
- `callback` (function): Resume callback

### System.getBatteryLevel()

Get battery level percentage.

**Returns:** `number` - Battery level (0-100)

### System.getMemoryInfo()

Get memory information.

**Returns:** `object` - Memory info with free/total bytes

---

## Error Handling

Always use try-catch blocks when calling API functions that may fail:

```javascript
try {
    RF.setFrequency(433.92);
    RF.transmit("Hello", function(success) {
        if (!success) {
            throw new Error("Transmission failed");
        }
    });
} catch (error) {
    console.error("RF error:", error.message);
    Notification.showError("RF communication failed");
}
```

## App Lifecycle

Implement proper app lifecycle management:

```javascript
function init() {
    console.log("App starting...");
    setupUI();
    setupEventHandlers();
}

function setupEventHandlers() {
    System.onBackButton(function() {
        cleanup();
        System.exit();
    });
    
    System.onPause(function() {
        console.log("App paused");
        // Save state, stop timers, etc.
    });
    
    System.onResume(function() {
        console.log("App resumed");
        // Restore state, restart timers, etc.
    });
}

function cleanup() {
    // Clean up resources
    RF.stopReceive();
    // Clear intervals, close files, etc.
}

// Start the app
init();
```

## Best Practices

1. **Error Handling:** Always handle errors gracefully
2. **Resource Cleanup:** Clean up resources when app exits
3. **Permission Requests:** Only request necessary permissions
4. **Performance:** Avoid blocking operations on the main thread
5. **Memory Management:** Be mindful of memory usage
6. **User Experience:** Provide feedback for long operations

## Debugging

Use `console.log()` for debugging output:

```javascript
console.log("Debug message");
console.error("Error message");
console.warn("Warning message");
```

Debug output is available through the serial console and web IDE.