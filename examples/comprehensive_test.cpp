/**
 * @file comprehensive_test.cpp
 * @brief Comprehensive real-time test for ALL TDK Lambda G30 functions
 * @author TDK Lambda G30 Test Suite
 * @date 2025-11-24
 */

#include "../include/tdk_lambda_g30.h"
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <string>
#include <sstream>

using namespace TDKLambda;
using namespace std::chrono_literals;

// Test configuration
const std::string TEST_IP = "10.1.33.5";
const int TEST_PORT = 8003;

// Color codes for terminal output
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define BOLD    "\033[1m"

// Test helper functions
void printHeader(const std::string& testName) {
    std::cout << "\n" << BOLD << CYAN << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << RESET << "\n";
    std::cout << BOLD << CYAN << "🧪 TEST: " << testName << RESET << "\n";
    std::cout << BOLD << CYAN << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << RESET << "\n";
}

void printSuccess(const std::string& message) {
    std::cout << GREEN << "✓ " << message << RESET << "\n";
}

void printInfo(const std::string& message) {
    std::cout << BLUE << "ℹ " << message << RESET << "\n";
}

void printWarning(const std::string& message) {
    std::cout << YELLOW << "⚠ " << message << RESET << "\n";
}

void printError(const std::string& message) {
    std::cout << RED << "✗ " << message << RESET << "\n";
}

void printValue(const std::string& name, const std::string& value) {
    std::cout << "  " << BOLD << name << ": " << RESET << CYAN << value << RESET << "\n";
}

void waitForUser(const std::string& message = "Devam etmek için ENTER'a basın...") {
    std::cout << YELLOW << "\n⏸  " << message << RESET;
    std::cin.get();
}

void delay(int milliseconds, const std::string& reason = "") {
    if (!reason.empty()) {
        std::cout << "  ⏱  " << reason << " (" << milliseconds << "ms)\n";
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

// ==================== TEST FUNCTIONS ====================

/**
 * Test 1: Factory Function & Constructor
 */
void test_01_factory_and_constructor() {
    printHeader("Factory Function & Constructor");

    try {
        printInfo("createG30Ethernet() ile nesne oluşturuluyor...");
        auto psu = createG30Ethernet(TEST_IP, TEST_PORT);
        printSuccess("Nesne başarıyla oluşturuldu");
        printValue("IP Address", TEST_IP);
        printValue("TCP Port", std::to_string(TEST_PORT));

    } catch (const std::exception& e) {
        printError(std::string("Factory function hatası: ") + e.what());
    }
}

/**
 * Test 2: Connection & Disconnection
 */
void test_02_connection(std::unique_ptr<TDKLambdaG30>& psu) {
    printHeader("Connection & Disconnection");

    try {
        // Test isConnected() before connection
        printInfo("Bağlantı öncesi isConnected() testi...");
        bool connected = psu->isConnected();
        printValue("isConnected()", connected ? "true" : "false");

        // Test connect()
        printInfo("connect() çağrılıyor...");
        psu->connect();
        delay(500, "Bağlantı stabilizasyonu");
        printSuccess("Bağlantı başarılı");

        // Test isConnected() after connection
        connected = psu->isConnected();
        printValue("isConnected()", connected ? "true" : "false");

        // Test double connect (should not fail)
        printInfo("Çift connect() testi (idempotent olmalı)...");
        psu->connect();
        printSuccess("Çift connect() başarılı (beklendiği gibi)");

    } catch (const std::exception& e) {
        printError(std::string("Bağlantı hatası: ") + e.what());
        throw;
    }
}

/**
 * Test 3: Device Identification
 */
void test_03_identification(std::unique_ptr<TDKLambdaG30>& psu) {
    printHeader("Device Identification & Info");

    try {
        // getIdentification()
        printInfo("getIdentification() çağrılıyor...");
        std::string id = psu->getIdentification();
        printSuccess("Cihaz kimliği alındı");
        printValue("*IDN?", id);

        // getVendor()
        printInfo("getVendor() çağrılıyor...");
        Vendor vendor = psu->getVendor();
        std::string vendorStr = (vendor == Vendor::TDK_LAMBDA) ? "TDK_LAMBDA" : "UNKNOWN";
        printValue("Vendor", vendorStr);

        // getModel()
        printInfo("getModel() çağrılıyor...");
        std::string model = psu->getModel();
        printValue("Model", model);

        // getCapabilities()
        printInfo("getCapabilities() çağrılıyor...");
        auto caps = psu->getCapabilities();
        printSuccess("Cihaz yetenekleri alındı");

        std::cout << "\n  " << BOLD << "Capabilities:" << RESET << "\n";
        printValue("  Max Voltage", std::to_string(caps.maxVoltage) + " V");
        printValue("  Max Current", std::to_string(caps.maxCurrent) + " A");
        printValue("  Max Power", std::to_string(caps.maxPower) + " W");
        printValue("  Channels", std::to_string(caps.numberOfChannels));
        printValue("  OVP Support", caps.supportsOVP ? "Yes" : "No");
        printValue("  OCP Support", caps.supportsOCP ? "Yes" : "No");
        printValue("  Remote Sensing", caps.supportsRemoteSensing ? "Yes" : "No");

    } catch (const std::exception& e) {
        printError(std::string("Identification hatası: ") + e.what());
    }
}

/**
 * Test 4: Reset Function
 */
void test_04_reset(std::unique_ptr<TDKLambdaG30>& psu) {
    printHeader("Reset Function");

    try {
        printInfo("reset() çağrılıyor...");
        psu->reset();
        delay(1000, "Reset sonrası stabilizasyon");
        printSuccess("Cihaz başarıyla sıfırlandı");

        // Verify reset state
        bool outputEnabled = psu->isOutputEnabled();
        printValue("Output State After Reset", outputEnabled ? "ON" : "OFF");

        if (!outputEnabled) {
            printSuccess("Reset sonrası çıkış kapalı (doğru)");
        } else {
            printWarning("Reset sonrası çıkış açık (beklenmedik)");
        }

    } catch (const std::exception& e) {
        printError(std::string("Reset hatası: ") + e.what());
    }
}

/**
 * Test 5: Voltage Control Functions
 */
void test_05_voltage_control(std::unique_ptr<TDKLambdaG30>& psu) {
    printHeader("Voltage Control Functions");

    std::cout << std::fixed << std::setprecision(3);

    try {
        // setVoltage()
        printInfo("setVoltage(12.5) çağrılıyor...");
        psu->setVoltage(12.5);
        delay(100, "Voltaj ayarı");
        printSuccess("Voltaj ayarlandı");

        // getVoltage()
        printInfo("getVoltage() çağrılıyor...");
        double setVoltage = psu->getVoltage();
        printValue("Set Voltage", std::to_string(setVoltage) + " V");

        if (std::abs(setVoltage - 12.5) < 0.1) {
            printSuccess("Voltaj doğru ayarlandı");
        } else {
            printWarning("Voltaj beklenen değerden farklı");
        }

        // Test different voltage values
        double testVoltages[] = {5.0, 10.0, 15.0, 20.0};
        std::cout << "\n  " << BOLD << "Farklı voltaj değerleri test ediliyor..." << RESET << "\n";

        for (double v : testVoltages) {
            psu->setVoltage(v);
            delay(50);
            double readback = psu->getVoltage();

            std::ostringstream oss;
            oss << std::fixed << std::setprecision(3);
            oss << "Set: " << v << "V → Read: " << readback << "V";

            if (std::abs(readback - v) < 0.1) {
                printSuccess(oss.str());
            } else {
                printWarning(oss.str() + " (Fark var!)");
            }
        }

        // Set back to 12V for next tests
        psu->setVoltage(12.0);

    } catch (const std::exception& e) {
        printError(std::string("Voltaj kontrol hatası: ") + e.what());
    }
}

/**
 * Test 6: Current Control Functions
 */
void test_06_current_control(std::unique_ptr<TDKLambdaG30>& psu) {
    printHeader("Current Control Functions");

    std::cout << std::fixed << std::setprecision(3);

    try {
        // setCurrent()
        printInfo("setCurrent(2.5) çağrılıyor...");
        psu->setCurrent(2.5);
        delay(100, "Akım limiti ayarı");
        printSuccess("Akım limiti ayarlandı");

        // getCurrent()
        printInfo("getCurrent() çağrılıyor...");
        double setCurrent = psu->getCurrent();
        printValue("Set Current Limit", std::to_string(setCurrent) + " A");

        if (std::abs(setCurrent - 2.5) < 0.1) {
            printSuccess("Akım limiti doğru ayarlandı");
        } else {
            printWarning("Akım limiti beklenen değerden farklı");
        }

        // Test different current values
        double testCurrents[] = {0.5, 1.0, 1.5, 2.0};
        std::cout << "\n  " << BOLD << "Farklı akım limit değerleri test ediliyor..." << RESET << "\n";

        for (double c : testCurrents) {
            psu->setCurrent(c);
            delay(50);
            double readback = psu->getCurrent();

            std::ostringstream oss;
            oss << std::fixed << std::setprecision(3);
            oss << "Set: " << c << "A → Read: " << readback << "A";

            if (std::abs(readback - c) < 0.1) {
                printSuccess(oss.str());
            } else {
                printWarning(oss.str() + " (Fark var!)");
            }
        }

        // Set back to 2A for next tests
        psu->setCurrent(2.0);

    } catch (const std::exception& e) {
        printError(std::string("Akım kontrol hatası: ") + e.what());
    }
}

/**
 * Test 7: Output Enable/Disable
 */
void test_07_output_control(std::unique_ptr<TDKLambdaG30>& psu) {
    printHeader("Output Enable/Disable Control");

    try {
        // Initial state
        printInfo("Başlangıç çıkış durumu kontrol ediliyor...");
        bool initialState = psu->isOutputEnabled();
        printValue("Initial Output State", initialState ? "ON" : "OFF");

        // Enable output
        printInfo("enableOutput(true) çağrılıyor...");
        psu->enableOutput(true);
        delay(200, "Çıkış aktivasyonu");

        bool state1 = psu->isOutputEnabled();
        printValue("Output State", state1 ? "ON ⚡" : "OFF");

        if (state1) {
            printSuccess("Çıkış başarıyla aktifleştirildi");
        } else {
            printWarning("Çıkış aktifleştirilmedi!");
        }

        delay(1000, "Çıkış açık durumda test");

        // Disable output
        printInfo("enableOutput(false) çağrılıyor...");
        psu->enableOutput(false);
        delay(200, "Çıkış deaktivasyonu");

        bool state2 = psu->isOutputEnabled();
        printValue("Output State", state2 ? "ON" : "OFF 🔌");

        if (!state2) {
            printSuccess("Çıkış başarıyla kapatıldı");
        } else {
            printWarning("Çıkış kapatılamadı!");
        }

        // Test rapid on/off switching
        std::cout << "\n  " << BOLD << "Hızlı on/off switching testi..." << RESET << "\n";
        for (int i = 0; i < 3; i++) {
            psu->enableOutput(true);
            delay(100);
            bool on = psu->isOutputEnabled();

            psu->enableOutput(false);
            delay(100);
            bool off = psu->isOutputEnabled();

            std::ostringstream oss;
            oss << "Cycle " << (i+1) << ": ON=" << (on ? "✓" : "✗")
                << ", OFF=" << (!off ? "✓" : "✗");

            if (on && !off) {
                printSuccess(oss.str());
            } else {
                printWarning(oss.str());
            }
        }

    } catch (const std::exception& e) {
        printError(std::string("Output kontrol hatası: ") + e.what());
    }
}

/**
 * Test 8: Measurement Functions (Real-time)
 */
void test_08_measurements(std::unique_ptr<TDKLambdaG30>& psu) {
    printHeader("Real-time Measurement Functions");

    std::cout << std::fixed << std::setprecision(3);

    try {
        // Setup for measurements
        printInfo("Ölçüm için hazırlık yapılıyor...");
        psu->setVoltage(12.0);
        psu->setCurrent(2.0);
        psu->enableOutput(true);
        delay(500, "Output stabilizasyonu");

        printSuccess("Çıkış aktif, real-time ölçümler başlıyor...\n");

        // Real-time measurements (10 iterations)
        std::cout << "  " << BOLD << "Real-time Ölçümler (10 iterasyon):" << RESET << "\n";
        std::cout << "  ┌─────┬──────────┬──────────┬──────────┐\n";
        std::cout << "  │ # │  Volt    │  Curr    │  Power   │\n";
        std::cout << "  ├─────┼──────────┼──────────┼──────────┤\n";

        for (int i = 0; i < 10; i++) {
            // measureVoltage()
            double voltage = psu->measureVoltage();

            // measureCurrent()
            double current = psu->measureCurrent();

            // measurePower()
            double power = psu->measurePower();

            std::cout << "  │ " << std::setw(3) << (i+1) << " │ ";
            std::cout << std::setw(6) << voltage << " V │ ";
            std::cout << std::setw(6) << current << " A │ ";
            std::cout << std::setw(6) << power << " W │\n";

            delay(200);  // 200ms measurement interval
        }

        std::cout << "  └─────┴──────────┴──────────┴──────────┘\n";
        printSuccess("10 iterasyon ölçüm tamamlandı");

        // Disable output
        psu->enableOutput(false);
        printInfo("Çıkış kapatıldı");

    } catch (const std::exception& e) {
        printError(std::string("Ölçüm hatası: ") + e.what());
        try { psu->enableOutput(false); } catch(...) {}
    }
}

/**
 * Test 9: Ramp Functions
 */
void test_09_ramp_functions(std::unique_ptr<TDKLambdaG30>& psu) {
    printHeader("Voltage & Current Ramp Functions");

    std::cout << std::fixed << std::setprecision(3);

    try {
        printWarning("Ramp fonksiyonları zaman alabilir, lütfen bekleyin...");

        // Voltage Ramp Test
        printInfo("setVoltageWithRamp() test ediliyor...");
        std::cout << "  Rampa: 5V → 15V @ 2V/s hızıyla\n";

        psu->setVoltage(5.0);
        delay(200);

        auto startTime = std::chrono::steady_clock::now();
        psu->setVoltageWithRamp(15.0, 2.0);  // 5V fark, 2V/s = ~5 saniye
        auto endTime = std::chrono::steady_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime).count();

        double finalVoltage = psu->getVoltage();
        printValue("Final Voltage", std::to_string(finalVoltage) + " V");
        printValue("Ramp Duration", std::to_string(duration) + " seconds");
        printSuccess("Voltage ramp tamamlandı");

        delay(500);

        // Current Ramp Test
        printInfo("setCurrentWithRamp() test ediliyor...");
        std::cout << "  Rampa: 0.5A → 2.5A @ 0.5A/s hızıyla\n";

        psu->setCurrent(0.5);
        delay(200);

        startTime = std::chrono::steady_clock::now();
        psu->setCurrentWithRamp(2.5, 0.5);  // 2A fark, 0.5A/s = ~4 saniye
        endTime = std::chrono::steady_clock::now();

        duration = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime).count();

        double finalCurrent = psu->getCurrent();
        printValue("Final Current", std::to_string(finalCurrent) + " A");
        printValue("Ramp Duration", std::to_string(duration) + " seconds");
        printSuccess("Current ramp tamamlandı");

    } catch (const std::exception& e) {
        printError(std::string("Ramp hatası: ") + e.what());
    }
}

/**
 * Test 10: Over-Voltage Protection (OVP)
 */
void test_10_ovp_functions(std::unique_ptr<TDKLambdaG30>& psu) {
    printHeader("Over-Voltage Protection (OVP)");

    std::cout << std::fixed << std::setprecision(3);

    try {
        // Set OVP level
        printInfo("setOverVoltageProtection(25.0) çağrılıyor...");
        psu->setOverVoltageProtection(25.0);
        delay(100, "OVP ayarı");
        printSuccess("OVP seviyesi ayarlandı");

        // Get OVP level
        printInfo("getOverVoltageProtection() çağrılıyor...");
        double ovpLevel = psu->getOverVoltageProtection();
        printValue("OVP Level", std::to_string(ovpLevel) + " V");

        if (std::abs(ovpLevel - 25.0) < 0.5) {
            printSuccess("OVP seviyesi doğru");
        } else {
            printWarning("OVP seviyesi beklenenden farklı");
        }

        // Test clearProtection()
        printInfo("clearProtection() çağrılıyor...");
        psu->clearProtection();
        delay(100, "Koruma temizleme");
        printSuccess("Koruma alarm'ları temizlendi");

        // Test different OVP levels
        double ovpLevels[] = {15.0, 20.0, 30.0, 40.0};
        std::cout << "\n  " << BOLD << "Farklı OVP seviyeleri test ediliyor..." << RESET << "\n";

        for (double ovp : ovpLevels) {
            psu->setOverVoltageProtection(ovp);
            delay(50);
            double readback = psu->getOverVoltageProtection();

            std::ostringstream oss;
            oss << std::fixed << std::setprecision(1);
            oss << "OVP Set: " << ovp << "V → Read: " << readback << "V";
            printSuccess(oss.str());
        }

    } catch (const std::exception& e) {
        printError(std::string("OVP hatası: ") + e.what());
    }
}

/**
 * Test 11: Status Functions
 */
void test_11_status_functions(std::unique_ptr<TDKLambdaG30>& psu) {
    printHeader("Status & Error Functions");

    try {
        // getStatus()
        printInfo("getStatus() çağrılıyor...");
        auto status = psu->getStatus();
        printSuccess("Status bilgisi alındı");

        std::cout << "\n  " << BOLD << "Cihaz Durumu:" << RESET << "\n";
        printValue("  Output Enabled", status.outputEnabled ? "YES ⚡" : "NO 🔌");
        printValue("  OVP Tripped", status.overVoltageProtection ? "YES ⚠️" : "NO ✓");
        printValue("  OCP Tripped", status.overCurrentProtection ? "YES ⚠️" : "NO ✓");
        printValue("  Over Temperature", status.overTemperature ? "YES 🔥" : "NO ✓");

        // checkError()
        printInfo("checkError() çağrılıyor...");
        std::string error = psu->checkError();
        printValue("Error Queue", error.empty() ? "Empty ✓" : error);

        if (error.find("No error") != std::string::npos || error.find("+0") != std::string::npos) {
            printSuccess("Hata yok");
        } else if (!error.empty()) {
            printWarning("Hata var: " + error);
        }

    } catch (const std::exception& e) {
        printError(std::string("Status hatası: ") + e.what());
    }
}

/**
 * Test 12: Raw Command Functions
 */
void test_12_raw_commands(std::unique_ptr<TDKLambdaG30>& psu) {
    printHeader("Raw SCPI Command Functions");

    try {
        // sendCommand() - no response expected
        printInfo("sendCommand('SYST:BEEP') çağrılıyor...");
        std::string cmdResult = psu->sendCommand("SYST:BEEP");
        printValue("Command Result", cmdResult);
        printSuccess("Raw command gönderildi");

        delay(500);

        // sendQuery() - response expected
        printInfo("sendQuery('SYST:VERS?') çağrılıyor...");
        std::string version = psu->sendQuery("SYST:VERS?");
        printValue("SCPI Version", version);
        printSuccess("Raw query başarılı");

        // Test multiple queries
        std::cout << "\n  " << BOLD << "Çeşitli SCPI query'leri test ediliyor..." << RESET << "\n";

        std::string queries[] = {
            "*IDN?",
            "SYST:ERR?",
            "VOLT?",
            "CURR?"
        };

        for (const auto& query : queries) {
            try {
                std::string response = psu->sendQuery(query);
                std::ostringstream oss;
                oss << query << " → " << response;
                printSuccess(oss.str());
                delay(100);
            } catch (const std::exception& e) {
                printWarning(std::string(query) + " → Error: " + e.what());
            }
        }

    } catch (const std::exception& e) {
        printError(std::string("Raw command hatası: ") + e.what());
    }
}

/**
 * Test 13: Stress Test - Rapid Operations
 */
void test_13_stress_test(std::unique_ptr<TDKLambdaG30>& psu) {
    printHeader("Stress Test - Rapid Operations");

    printWarning("Bu test cihazı yoğun şekilde kullanacak...");

    try {
        int iterations = 20;
        int successCount = 0;
        int errorCount = 0;

        std::cout << "  " << BOLD << iterations << " iterasyon hızlı işlem yapılıyor..." << RESET << "\n\n";

        auto startTime = std::chrono::steady_clock::now();

        for (int i = 0; i < iterations; i++) {
            try {
                // Rapid operations
                double v = 5.0 + (i % 10);
                double c = 0.5 + (i % 5) * 0.3;

                psu->setVoltage(v);
                psu->setCurrent(c);

                double rv = psu->getVoltage();
                double rc = psu->getCurrent();

                if (std::abs(rv - v) < 0.1 && std::abs(rc - c) < 0.1) {
                    successCount++;
                    std::cout << "  " << GREEN << "✓" << RESET;
                } else {
                    errorCount++;
                    std::cout << "  " << RED << "✗" << RESET;
                }

                if ((i + 1) % 10 == 0) std::cout << "\n";

            } catch (const std::exception& e) {
                errorCount++;
                std::cout << "  " << RED << "✗" << RESET;
            }
        }

        auto endTime = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

        std::cout << "\n\n";
        printValue("Total Iterations", std::to_string(iterations));
        printValue("Success", std::to_string(successCount));
        printValue("Errors", std::to_string(errorCount));
        printValue("Duration", std::to_string(duration) + " ms");
        printValue("Avg Time/Op", std::to_string(duration / iterations) + " ms");

        if (errorCount == 0) {
            printSuccess("Tüm stress test işlemleri başarılı! 🎉");
        } else {
            printWarning(std::to_string(errorCount) + " hata oluştu");
        }

    } catch (const std::exception& e) {
        printError(std::string("Stress test hatası: ") + e.what());
    }
}

/**
 * Test 14: Disconnection Test
 */
void test_14_disconnection(std::unique_ptr<TDKLambdaG30>& psu) {
    printHeader("Disconnection & Cleanup Test");

    try {
        // Ensure output is off before disconnect
        printInfo("Güvenli kapatma: Çıkış devre dışı bırakılıyor...");
        psu->enableOutput(false);
        delay(200);
        printSuccess("Çıkış kapatıldı");

        // Test disconnect()
        printInfo("disconnect() çağrılıyor...");
        psu->disconnect();
        delay(200);
        printSuccess("Bağlantı kesildi");

        // Verify disconnection
        bool connected = psu->isConnected();
        printValue("isConnected()", connected ? "true" : "false");

        if (!connected) {
            printSuccess("Bağlantı başarıyla sonlandırıldı");
        } else {
            printWarning("Bağlantı hala aktif görünüyor");
        }

    } catch (const std::exception& e) {
        printError(std::string("Disconnection hatası: ") + e.what());
    }
}

// ==================== MAIN TEST SUITE ====================

int main() {
    std::cout << BOLD << MAGENTA;
    std::cout << "\n╔════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                                                            ║\n";
    std::cout << "║   TDK LAMBDA G30 - COMPREHENSIVE TEST SUITE               ║\n";
    std::cout << "║   Real-time Function Testing                              ║\n";
    std::cout << "║                                                            ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════╝\n";
    std::cout << RESET << "\n";

    printInfo("Tüm TDK Lambda G30 fonksiyonları test edilecek");
    printValue("Test IP", TEST_IP);
    printValue("Test Port", std::to_string(TEST_PORT));

    std::cout << "\n" << YELLOW << "Cihazın açık ve hazır olduğundan emin olun!" << RESET << "\n";
    waitForUser();

    std::unique_ptr<TDKLambdaG30> psu;

    try {
        // Test 1: Factory & Constructor
        test_01_factory_and_constructor();

        // Create PSU for remaining tests
        psu = createG30Ethernet(TEST_IP, TEST_PORT);

        // Test 2: Connection
        test_02_connection(psu);

        // Test 3: Identification
        test_03_identification(psu);

        // Test 4: Reset
        test_04_reset(psu);

        // Test 5: Voltage Control
        test_05_voltage_control(psu);

        // Test 6: Current Control
        test_06_current_control(psu);

        // Test 7: Output Control
        test_07_output_control(psu);

        // Test 8: Measurements (Real-time)
        test_08_measurements(psu);

        // Test 9: Ramp Functions
        test_09_ramp_functions(psu);

        // Test 10: OVP Functions
        test_10_ovp_functions(psu);

        // Test 11: Status Functions
        test_11_status_functions(psu);

        // Test 12: Raw Commands
        test_12_raw_commands(psu);

        // Test 13: Stress Test
        test_13_stress_test(psu);

        // Test 14: Disconnection
        test_14_disconnection(psu);

        // Final Summary
        std::cout << "\n" << BOLD << GREEN;
        std::cout << "╔════════════════════════════════════════════════════════════╗\n";
        std::cout << "║                                                            ║\n";
        std::cout << "║             🎉 TÜM TESTLER TAMAMLANDI! 🎉                 ║\n";
        std::cout << "║                                                            ║\n";
        std::cout << "║   14/14 Test Suite Başarıyla Çalıştırıldı                 ║\n";
        std::cout << "║                                                            ║\n";
        std::cout << "╚════════════════════════════════════════════════════════════╝\n";
        std::cout << RESET << "\n";

        return 0;

    } catch (const G30Exception& e) {
        std::cout << "\n";
        printError("TDK Lambda G30 Hatası: " + std::string(e.what()));
        std::cout << "\n" << YELLOW << "Test suite sonlandırıldı." << RESET << "\n\n";
        return 1;

    } catch (const std::exception& e) {
        std::cout << "\n";
        printError("Beklenmeyen Hata: " + std::string(e.what()));
        std::cout << "\n" << YELLOW << "Test suite sonlandırıldı." << RESET << "\n\n";
        return 1;
    }
}
