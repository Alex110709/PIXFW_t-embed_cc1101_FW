/**
 * @file test_rf_extensions.js
 * @brief JavaScript tests for RF Jammer and Spectrum Analyzer extensions
 */

// Test RF Jammer functionality
function testRFJammer() {
    console.log("Testing RF Jammer functionality...");
    
    try {
        // Test starting jammer
        console.log("Starting jammer at 433.92 MHz...");
        RF.startJammer(433920000);
        console.log("Jammer started successfully");
        
        // Test stopping jammer
        console.log("Stopping jammer...");
        RF.stopJammer();
        console.log("Jammer stopped successfully");
        
        console.log("RF Jammer tests passed!");
        return true;
    } catch (error) {
        console.error("RF Jammer tests failed:", error);
        return false;
    }
}

// Test Spectrum Analyzer functionality
function testSpectrumAnalyzer() {
    console.log("Testing Spectrum Analyzer functionality...");
    
    try {
        // Test starting spectrum analyzer
        console.log("Starting spectrum analyzer...");
        RF.startSpectrumAnalyzer(433000000, 434000000, 100000);
        console.log("Spectrum analyzer started successfully");
        
        // Test getting RSSI at frequency
        console.log("Getting RSSI at 433.92 MHz...");
        const rssi = RF.getRssiAtFrequency(433920000);
        console.log("RSSI:", rssi, "dBm");
        
        // Test stopping spectrum analyzer
        console.log("Stopping spectrum analyzer...");
        RF.stopSpectrumAnalyzer();
        console.log("Spectrum analyzer stopped successfully");
        
        console.log("Spectrum Analyzer tests passed!");
        return true;
    } catch (error) {
        console.error("Spectrum Analyzer tests failed:", error);
        return false;
    }
}

// Test invalid parameters
function testInvalidParameters() {
    console.log("Testing invalid parameters...");
    
    try {
        // Test invalid frequency for jammer
        try {
            RF.startJammer(-1);
            console.error("ERROR: Should have failed with invalid frequency");
            return false;
        } catch (error) {
            console.log("Correctly rejected invalid jammer frequency");
        }
        
        // Test invalid parameters for spectrum analyzer
        try {
            RF.startSpectrumAnalyzer(434000000, 433000000, 100000);
            console.error("ERROR: Should have failed with invalid frequency range");
            return false;
        } catch (error) {
            console.log("Correctly rejected invalid spectrum analyzer parameters");
        }
        
        console.log("Invalid parameters tests passed!");
        return true;
    } catch (error) {
        console.error("Invalid parameters tests failed:", error);
        return false;
    }
}

// Main test function
function runAllTests() {
    console.log("Running RF extensions tests...");
    
    let passed = 0;
    let total = 3;
    
    if (testRFJammer()) passed++;
    if (testSpectrumAnalyzer()) passed++;
    if (testInvalidParameters()) passed++;
    
    console.log(`Tests completed: ${passed}/${total} passed`);
    
    if (passed === total) {
        console.log("All tests passed!");
        return true;
    } else {
        console.log("Some tests failed!");
        return false;
    }
}

// Run the tests
runAllTests();