/**
 * @file test_wifi.js
 * @brief Test script for Wi-Fi functionality
 */

// Test Wi-Fi API functions
function testWiFiAPI() {
    console.log("Testing Wi-Fi API functions...");
    
    try {
        // Test getting status
        console.log("Getting Wi-Fi status...");
        const status = wifi.getStatus();
        console.log("Current Wi-Fi status:", status);
        
        // Test getting IP address
        console.log("Getting IP address...");
        const ip = wifi.getIPAddress();
        console.log("Current IP address:", ip);
        
        console.log("Wi-Fi API tests completed successfully!");
        return true;
    } catch (error) {
        console.error("Wi-Fi API tests failed:", error);
        return false;
    }
}

// Test Wi-Fi scanning
function testWiFiScan() {
    console.log("Testing Wi-Fi scanning...");
    
    try {
        console.log("Scanning for networks...");
        const networks = wifi.scan();
        console.log("Found", networks.length, "networks");
        
        for (let i = 0; i < Math.min(3, networks.length); i++) {
            console.log("Network", i, ":", networks[i].ssid, "(" + networks[i].rssi + "dBm)");
        }
        
        console.log("Wi-Fi scanning test completed successfully!");
        return true;
    } catch (error) {
        console.error("Wi-Fi scanning test failed:", error);
        return false;
    }
}

// Test Wi-Fi connection (without actual connection)
function testWiFiConnection() {
    console.log("Testing Wi-Fi connection functions...");
    
    try {
        // Test connection with invalid SSID (should fail gracefully)
        console.log("Testing connection with invalid SSID...");
        try {
            wifi.connect("");
            console.error("ERROR: Should have failed with empty SSID");
            return false;
        } catch (error) {
            console.log("Correctly rejected empty SSID");
        }
        
        console.log("Wi-Fi connection tests completed successfully!");
        return true;
    } catch (error) {
        console.error("Wi-Fi connection tests failed:", error);
        return false;
    }
}

// Test Wi-Fi AP functions
function testWiFiAP() {
    console.log("Testing Wi-Fi AP functions...");
    
    try {
        // Test starting AP with valid SSID
        console.log("Testing starting AP with valid SSID...");
        wifi.startAP("TestAP");
        console.log("AP started successfully");
        
        // Test stopping AP
        console.log("Testing stopping AP...");
        wifi.stopAP();
        console.log("AP stopped successfully");
        
        console.log("Wi-Fi AP tests completed successfully!");
        return true;
    } catch (error) {
        console.error("Wi-Fi AP tests failed:", error);
        return false;
    }
}

// Main test function
function runAllTests() {
    console.log("Running Wi-Fi tests...");
    
    let passed = 0;
    let total = 4;
    
    if (testWiFiAPI()) passed++;
    if (testWiFiScan()) passed++;
    if (testWiFiConnection()) passed++;
    if (testWiFiAP()) passed++;
    
    console.log(`Tests completed: ${passed}/${total} passed`);
    
    if (passed === total) {
        console.log("All Wi-Fi tests passed!");
        return true;
    } else {
        console.log("Some Wi-Fi tests failed!");
        return false;
    }
}

// Run the tests
runAllTests();