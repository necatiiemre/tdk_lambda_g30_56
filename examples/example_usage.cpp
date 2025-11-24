/**
 * @file example_usage.cpp
 * @brief Example usage of TDK Lambda G30 Power Supply Controller
 * @version 1.0.0
 * @date 2025-11-24
 *
 * This file demonstrates various ways to use the TDK Lambda G30 library
 */

#include "../include/tdk_lambda_g30.h"
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>

using namespace TDKLambda;

/**
 * @brief Basic usage example
 */
void basicUsageExample() {
    std::cout << "\n========== Basic Usage Example ==========\n" << std::endl;

    try {
        // Configure the power supply
        G30Config config;
        config.port = "/dev/ttyUSB0";  // Linux: /dev/ttyUSB0, Windows: COM3
        config.baudRate = 9600;
        config.timeout_ms = 1000;

        // Create power supply instance
        TDKLambdaG30 psu(config);

        // Connect to device
        std::cout << "Connecting to power supply..." << std::endl;
        psu.connect();
        std::cout << "Connected successfully!" << std::endl;

        // Get device identification
        std::string id = psu.getIdentification();
        std::cout << "Device ID: " << id << std::endl;

        // Set voltage and current
        std::cout << "\nSetting voltage to 12.0V..." << std::endl;
        psu.setVoltage(12.0);

        std::cout << "Setting current limit to 2.5A..." << std::endl;
        psu.setCurrent(2.5);

        // Verify settings
        double setVoltage = psu.getVoltage();
        double setCurrent = psu.getCurrent();
        std::cout << std::fixed << std::setprecision(3);
        std::cout << "Set voltage: " << setVoltage << "V" << std::endl;
        std::cout << "Set current: " << setCurrent << "A" << std::endl;

        // Enable output
        std::cout << "\nEnabling output..." << std::endl;
        psu.enableOutput(true);

        // Wait a bit for stabilization
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        // Measure actual values
        double measuredVoltage = psu.measureVoltage();
        double measuredCurrent = psu.measureCurrent();
        double measuredPower = psu.measurePower();

        std::cout << "\nMeasured values:" << std::endl;
        std::cout << "  Voltage: " << measuredVoltage << "V" << std::endl;
        std::cout << "  Current: " << measuredCurrent << "A" << std::endl;
        std::cout << "  Power:   " << measuredPower << "W" << std::endl;

        // Disable output
        std::cout << "\nDisabling output..." << std::endl;
        psu.enableOutput(false);

        // Disconnect
        psu.disconnect();
        std::cout << "Disconnected successfully!" << std::endl;

    } catch (const G30Exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

/**
 * @brief Advanced usage with ramp control
 */
void advancedRampExample() {
    std::cout << "\n========== Advanced Ramp Example ==========\n" << std::endl;

    try {
        // Use factory function for simpler creation
        auto psu = createG30("/dev/ttyUSB0", 9600);

        std::cout << "Connecting..." << std::endl;
        psu->connect();

        // Set safety limits
        psu->setMaxVoltage(24.0);  // Don't allow more than 24V
        psu->setMaxCurrent(5.0);   // Don't allow more than 5A

        std::cout << "Safety limits set:" << std::endl;
        std::cout << "  Max voltage: " << psu->getMaxVoltage() << "V" << std::endl;
        std::cout << "  Max current: " << psu->getMaxCurrent() << "A" << std::endl;

        // Start from 0V
        psu->setVoltage(0.0);
        psu->setCurrent(3.0);
        psu->enableOutput(true);

        std::cout << "\nRamping voltage from 0V to 15V at 1V/s..." << std::endl;
        psu->setVoltageWithRamp(15.0, 1.0);  // Ramp to 15V at 1V/s

        std::cout << "Voltage ramp completed!" << std::endl;
        std::cout << "Current voltage: " << psu->measureVoltage() << "V" << std::endl;

        // Ramp down
        std::cout << "\nRamping voltage down to 5V at 2V/s..." << std::endl;
        psu->setVoltageWithRamp(5.0, 2.0);  // Ramp down at 2V/s

        // Disable output
        psu->enableOutput(false);
        psu->disconnect();

        std::cout << "Advanced ramp example completed!" << std::endl;

    } catch (const G30Exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

/**
 * @brief Status monitoring example
 */
void statusMonitoringExample() {
    std::cout << "\n========== Status Monitoring Example ==========\n" << std::endl;

    try {
        G30Config config;
        config.port = "/dev/ttyUSB0";
        config.baudRate = 9600;

        TDKLambdaG30 psu(config);
        psu.connect();

        // Set up over-voltage protection
        std::cout << "Setting OVP to 15V..." << std::endl;
        psu.setOverVoltageProtection(15.0);
        std::cout << "OVP level: " << psu.getOverVoltageProtection() << "V" << std::endl;

        // Configure output
        psu.setVoltage(12.0);
        psu.setCurrent(1.0);
        psu.enableOutput(true);

        // Monitor for 5 seconds
        std::cout << "\nMonitoring for 5 seconds..." << std::endl;
        std::cout << std::fixed << std::setprecision(3);

        for (int i = 0; i < 5; ++i) {
            std::this_thread::sleep_for(std::chrono::seconds(1));

            PowerSupplyStatus status = psu.getStatus();
            double voltage = psu.measureVoltage();
            double current = psu.measureCurrent();

            std::cout << "\n[" << (i + 1) << "s]" << std::endl;
            std::cout << "  V: " << voltage << "V, I: " << current << "A" << std::endl;
            std::cout << "  Output: " << (status.outputEnabled ? "ON" : "OFF") << std::endl;

            if (status.overVoltageProtection) {
                std::cout << "  WARNING: Over-voltage protection triggered!" << std::endl;
            }
            if (status.overCurrentProtection) {
                std::cout << "  WARNING: Over-current protection triggered!" << std::endl;
            }
            if (status.overTemperature) {
                std::cout << "  WARNING: Over-temperature condition!" << std::endl;
            }

            // Check for errors
            std::string error = psu.checkError();
            if (!error.empty() && error.find("No error") == std::string::npos) {
                std::cout << "  Error: " << error << std::endl;
            }
        }

        psu.enableOutput(false);
        psu.disconnect();

        std::cout << "\nMonitoring completed!" << std::endl;

    } catch (const G30Exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

/**
 * @brief Custom error handler example
 */
void customErrorHandlerExample() {
    std::cout << "\n========== Custom Error Handler Example ==========\n" << std::endl;

    try {
        auto psu = createG30("/dev/ttyUSB0", 9600);

        // Set custom error handler
        psu->setErrorHandler([](const std::string& error) {
            std::cerr << "[CUSTOM ERROR HANDLER] " << error << std::endl;
        });

        psu->connect();

        // Try to set invalid voltage (will trigger error handler)
        try {
            psu->setMaxVoltage(20.0);
            psu->setVoltage(25.0);  // This exceeds max voltage
        } catch (const G30Exception& e) {
            std::cout << "Caught exception: " << e.what() << std::endl;
        }

        psu->disconnect();

    } catch (const G30Exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

/**
 * @brief Raw SCPI command example
 */
void rawScpiCommandExample() {
    std::cout << "\n========== Raw SCPI Command Example ==========\n" << std::endl;

    try {
        auto psu = createG30("/dev/ttyUSB0", 9600);
        psu->connect();

        std::cout << "Sending raw SCPI commands..." << std::endl;

        // Send raw commands
        psu->sendCommand("*RST");  // Reset
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        // Send raw query
        std::string model = psu->sendQuery("*IDN?");
        std::cout << "Device identification: " << model << std::endl;

        // More SCPI commands
        psu->sendCommand("VOLT 10.0");
        std::string voltageStr = psu->sendQuery("VOLT?");
        std::cout << "Set voltage: " << voltageStr << "V" << std::endl;

        psu->disconnect();
        std::cout << "Raw SCPI example completed!" << std::endl;

    } catch (const G30Exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

/**
 * @brief Sequencing example - multiple voltage steps
 */
void sequencingExample() {
    std::cout << "\n========== Sequencing Example ==========\n" << std::endl;

    try {
        auto psu = createG30("/dev/ttyUSB0", 9600);
        psu->connect();

        // Define voltage sequence
        std::vector<double> voltageSequence = {3.3, 5.0, 9.0, 12.0, 15.0, 12.0, 5.0, 3.3, 0.0};
        int stepDuration_ms = 1000;  // 1 second per step

        psu->setCurrent(2.0);
        psu->enableOutput(true);

        std::cout << "Running voltage sequence..." << std::endl;
        std::cout << std::fixed << std::setprecision(1);

        for (size_t i = 0; i < voltageSequence.size(); ++i) {
            double targetVoltage = voltageSequence[i];

            std::cout << "\nStep " << (i + 1) << "/" << voltageSequence.size()
                      << ": Setting voltage to " << targetVoltage << "V" << std::endl;

            psu->setVoltage(targetVoltage);
            std::this_thread::sleep_for(std::chrono::milliseconds(stepDuration_ms));

            double measured = psu->measureVoltage();
            std::cout << "  Measured: " << measured << "V" << std::endl;
        }

        psu->enableOutput(false);
        psu->disconnect();

        std::cout << "\nSequencing completed!" << std::endl;

    } catch (const G30Exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

/**
 * @brief Main function - run all examples
 */
int main(int argc, char* argv[]) {
    std::cout << "=======================================" << std::endl;
    std::cout << "TDK Lambda G30 Usage Examples" << std::endl;
    std::cout << "=======================================" << std::endl;

    if (argc > 1) {
        std::string example = argv[1];

        if (example == "basic") {
            basicUsageExample();
        } else if (example == "ramp") {
            advancedRampExample();
        } else if (example == "status") {
            statusMonitoringExample();
        } else if (example == "error") {
            customErrorHandlerExample();
        } else if (example == "scpi") {
            rawScpiCommandExample();
        } else if (example == "sequence") {
            sequencingExample();
        } else {
            std::cout << "Unknown example: " << example << std::endl;
            std::cout << "\nAvailable examples:" << std::endl;
            std::cout << "  basic    - Basic usage" << std::endl;
            std::cout << "  ramp     - Voltage ramping" << std::endl;
            std::cout << "  status   - Status monitoring" << std::endl;
            std::cout << "  error    - Custom error handler" << std::endl;
            std::cout << "  scpi     - Raw SCPI commands" << std::endl;
            std::cout << "  sequence - Voltage sequencing" << std::endl;
            return 1;
        }
    } else {
        // Run all examples
        std::cout << "\nRunning all examples..." << std::endl;
        std::cout << "(Note: Adjust serial port in code before running)\n" << std::endl;

        basicUsageExample();
        advancedRampExample();
        statusMonitoringExample();
        customErrorHandlerExample();
        rawScpiCommandExample();
        sequencingExample();
    }

    std::cout << "\n=======================================" << std::endl;
    std::cout << "Examples completed!" << std::endl;
    std::cout << "=======================================" << std::endl;

    return 0;
}
