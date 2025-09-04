/**
 * @file run_all_tests.js
 * @brief Run all tests for improved apps
 */

// Test functions
function runTest(testName, testFunction) {
    console.log(`\n=== Running ${testName} ===`);
    try {
        const result = testFunction();
        if (result) {
            console.log(`✅ ${testName} passed!`);
            return true;
        } else {
            console.log(`❌ ${testName} failed!`);
            return false;
        }
    } catch (error) {
        console.error(`❌ ${testName} failed with error:`, error);
        return false;
    }
}

// Import and run individual tests
function runAllTests() {
    console.log("Running all tests for improved apps...");
    
    let passed = 0;
    let total = 0;
    
    // Test main menu
    total++;
    if (runTest("Main Menu Tests", () => {
        try {
            require('./test_main_menu.js');
            return true;
        } catch (error) {
            console.error("Main Menu tests failed:", error);
            return false;
        }
    })) passed++;
    
    // Test RF Scanner
    total++;
    if (runTest("RF Scanner Tests", () => {
        try {
            require('./test_rf_scanner.js');
            return true;
        } catch (error) {
            console.error("RF Scanner tests failed:", error);
            return false;
        }
    })) passed++;
    
    // Test RF Jammer
    total++;
    if (runTest("RF Jammer Tests", () => {
        try {
            require('./test_jammer.js');
            return true;
        } catch (error) {
            console.error("RF Jammer tests failed:", error);
            return false;
        }
    })) passed++;
    
    // Test Spectrum Analyzer
    total++;
    if (runTest("Spectrum Analyzer Tests", () => {
        try {
            require('./test_spectrum_analyzer.js');
            return true;
        } catch (error) {
            console.error("Spectrum Analyzer tests failed:", error);
            return false;
        }
    })) passed++;
    
    // Test Signal Generator
    total++;
    if (runTest("Signal Generator Tests", () => {
        try {
            require('./test_signal_generator.js');
            return true;
        } catch (error) {
            console.error("Signal Generator tests failed:", error);
            return false;
        }
    })) passed++;
    
    console.log(`\n=== Test Results ===`);
    console.log(`Total tests: ${total}`);
    console.log(`Passed: ${passed}`);
    console.log(`Failed: ${total - passed}`);
    
    if (passed === total) {
        console.log("🎉 All tests passed!");
        return true;
    } else {
        console.log("😞 Some tests failed!");
        return false;
    }
}

// Run the tests
runAllTests();