/**
 * @file ethernet_example.cpp
 * @brief Ethernet (TCP/IP) connection example for TDK Lambda G30
 * @version 1.0.0
 * @date 2025-11-24
 *
 * This example demonstrates how to control TDK Lambda G30 power supply
 * via Ethernet using TCP/IP connection (SCPI over LAN).
 */

#include "../include/tdk_lambda_g30.h"
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>

using namespace TDKLambda;

/**
 * @brief Basic Ethernet connection example
 */
void basicEthernetExample() {
    std::cout << "\n========== Basic Ethernet Connection ==========\n" << std::endl;

    try {
        // TDK Lambda G30 TCP port 8003'te SCPI komutlarını dinler
        // NOT: Standard SCPI port 5025 DEĞİL, G30 için özel port 8003!
        std::string ipAddress = "192.168.1.100";  // Cihazın IP adresini buraya girin
        int port = 8003;  // TDK Lambda G30 TCP port

        std::cout << "Bağlanıyor: " << ipAddress << ":" << port << std::endl;

        // Factory function ile Ethernet bağlantısı oluştur
        auto psu = createG30Ethernet(ipAddress, port);

        // Bağlan
        psu->connect();
        std::cout << "✓ Bağlantı başarılı!" << std::endl;

        // Cihaz bilgisi
        std::string id = psu->getIdentification();
        std::cout << "Cihaz: " << id << std::endl;

        // Voltaj ve akım ayarla
        std::cout << "\nVoltaj ve akım ayarlanıyor..." << std::endl;
        psu->setVoltage(12.0);
        psu->setCurrent(2.0);

        std::cout << std::fixed << std::setprecision(3);
        std::cout << "Ayarlanan voltaj: " << psu->getVoltage() << "V" << std::endl;
        std::cout << "Ayarlanan akım: " << psu->getCurrent() << "A" << std::endl;

        // Çıkışı aktifleştir
        std::cout << "\n⚡ Çıkış aktifleştiriliyor..." << std::endl;
        psu->enableOutput(true);

        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        // Ölçüm yap
        double voltage = psu->measureVoltage();
        double current = psu->measureCurrent();
        double power = psu->measurePower();

        std::cout << "\n📊 Ölçümler:" << std::endl;
        std::cout << "  Voltaj: " << voltage << " V" << std::endl;
        std::cout << "  Akım:   " << current << " A" << std::endl;
        std::cout << "  Güç:    " << power << " W" << std::endl;

        // Çıkışı kapat
        std::cout << "\n🔌 Çıkış kapatılıyor..." << std::endl;
        psu->enableOutput(false);

        psu->disconnect();
        std::cout << "✓ Bağlantı kesildi" << std::endl;

    } catch (const G30Exception& e) {
        std::cerr << "\n❌ HATA: " << e.what() << std::endl;
    }
}

/**
 * @brief Manuel konfigürasyon ile Ethernet bağlantısı
 */
void manualConfigExample() {
    std::cout << "\n========== Manuel Konfigürasyon ==========\n" << std::endl;

    try {
        // Konfigürasyonu manuel oluştur
        G30Config config;
        config.ipAddress = "192.168.1.100";
        config.tcpPort = 8003;
        config.timeout_ms = 2000;  // 2 saniye timeout

        std::cout << "Bağlanıyor: " << config.ipAddress << ":" << config.tcpPort << std::endl;

        // Power supply instance oluştur
        TDKLambdaG30 psu(config);

        // Bağlan ve test et
        psu.connect();
        std::cout << "✓ Bağlantı başarılı!" << std::endl;

        std::string id = psu.getIdentification();
        std::cout << "Cihaz: " << id << std::endl;

        psu.disconnect();

    } catch (const G30Exception& e) {
        std::cerr << "\n❌ HATA: " << e.what() << std::endl;
    }
}

/**
 * @brief Voltage sequencing over Ethernet
 */
void ethernetSequencingExample() {
    std::cout << "\n========== Ethernet Voltage Sequencing ==========\n" << std::endl;

    try {
        auto psu = createG30Ethernet("192.168.1.100", 8003);

        psu->connect();
        std::cout << "Bağlandı: " << psu->getIdentification() << "\n" << std::endl;

        // Voltaj dizisi
        std::vector<double> voltages = {3.3, 5.0, 9.0, 12.0, 15.0, 12.0, 5.0, 3.3};

        psu->setCurrent(2.0);
        psu->enableOutput(true);

        std::cout << std::fixed << std::setprecision(2);

        for (size_t i = 0; i < voltages.size(); ++i) {
            double targetV = voltages[i];

            std::cout << "Adım " << (i + 1) << "/" << voltages.size()
                      << ": " << targetV << "V ayarlanıyor..." << std::endl;

            psu->setVoltage(targetV);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

            double measured = psu->measureVoltage();
            std::cout << "  → Ölçülen: " << measured << "V" << std::endl;
        }

        psu->setVoltage(0.0);
        psu->enableOutput(false);
        psu->disconnect();

        std::cout << "\n✓ Sequencing tamamlandı!" << std::endl;

    } catch (const G30Exception& e) {
        std::cerr << "\n❌ HATA: " << e.what() << std::endl;
    }
}

/**
 * @brief Monitoring loop over Ethernet
 */
void ethernetMonitoringExample() {
    std::cout << "\n========== Ethernet Monitoring ==========\n" << std::endl;

    try {
        auto psu = createG30Ethernet("192.168.1.100", 8003);

        psu->connect();
        std::cout << "Bağlandı!\n" << std::endl;

        // Set up
        psu->setVoltage(12.0);
        psu->setCurrent(3.0);
        psu->setOverVoltageProtection(13.0);
        psu->enableOutput(true);

        std::cout << "10 saniye boyunca izleniyor...\n" << std::endl;
        std::cout << std::fixed << std::setprecision(3);

        for (int i = 0; i < 10; ++i) {
            PowerSupplyStatus status = psu->getStatus();
            double v = psu->measureVoltage();
            double a = psu->measureCurrent();
            double w = psu->measurePower();

            std::cout << "[" << (i + 1) << "s] "
                      << "V:" << v << "V  "
                      << "I:" << a << "A  "
                      << "P:" << w << "W  "
                      << (status.outputEnabled ? "ON" : "OFF");

            if (status.overVoltageProtection)
                std::cout << " [OVP!]";
            if (status.overCurrentProtection)
                std::cout << " [OCP!]";

            std::cout << std::endl;

            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        psu->enableOutput(false);
        psu->disconnect();

        std::cout << "\n✓ Monitoring tamamlandı!" << std::endl;

    } catch (const G30Exception& e) {
        std::cerr << "\n❌ HATA: " << e.what() << std::endl;
    }
}

/**
 * @brief SCPI command examples over Ethernet
 */
void scpiOverEthernetExample() {
    std::cout << "\n========== SCPI Over Ethernet ==========\n" << std::endl;

    try {
        auto psu = createG30Ethernet("192.168.1.100", 8003);

        psu->connect();
        std::cout << "Bağlandı!\n" << std::endl;

        // Raw SCPI komutları gönder
        std::cout << "SCPI komutları gönderiliyor..." << std::endl;

        psu->sendCommand("*RST");
        std::cout << "  → *RST gönderildi" << std::endl;

        std::string idn = psu->sendQuery("*IDN?");
        std::cout << "  → *IDN?: " << idn << std::endl;

        psu->sendCommand("VOLT 10.0");
        std::string voltResp = psu->sendQuery("VOLT?");
        std::cout << "  → VOLT?: " << voltResp << "V" << std::endl;

        psu->sendCommand("CURR 1.5");
        std::string currResp = psu->sendQuery("CURR?");
        std::cout << "  → CURR?: " << currResp << "A" << std::endl;

        psu->disconnect();
        std::cout << "\n✓ SCPI test tamamlandı!" << std::endl;

    } catch (const G30Exception& e) {
        std::cerr << "\n❌ HATA: " << e.what() << std::endl;
    }
}

/**
 * @brief Error handling example
 */
void ethernetErrorHandlingExample() {
    std::cout << "\n========== Error Handling ==========\n" << std::endl;

    try {
        auto psu = createG30Ethernet("192.168.1.100", 8003);

        // Custom error handler
        psu->setErrorHandler([](const std::string& error) {
            std::cerr << "[ETH ERROR] " << error << std::endl;
        });

        psu->connect();
        std::cout << "Bağlandı!" << std::endl;

        // Test error conditions
        psu->setMaxVoltage(20.0);

        std::cout << "\nYanlış voltaj deneniyor (25V > 20V max)..." << std::endl;
        try {
            psu->setVoltage(25.0);  // This will throw
        } catch (const G30Exception& e) {
            std::cout << "Yakalandı: " << e.what() << std::endl;
        }

        // Check device errors
        std::string error = psu->checkError();
        if (!error.empty()) {
            std::cout << "Cihaz hatası: " << error << std::endl;
        }

        psu->disconnect();

    } catch (const G30Exception& e) {
        std::cerr << "\n❌ HATA: " << e.what() << std::endl;
    }
}

int main(int argc, char* argv[]) {
    std::cout << "=======================================" << std::endl;
    std::cout << "TDK Lambda G30 Ethernet Examples" << std::endl;
    std::cout << "=======================================" << std::endl;
    std::cout << "\n⚠️  Önemli Notlar:" << std::endl;
    std::cout << "    • IP adresini kodda güncelleyin!" << std::endl;
    std::cout << "    • TDK Lambda G30 TCP Port: 8003 (NOT 5025!)" << std::endl;
    std::cout << "    • Varsayılan: 192.168.1.100:8003" << std::endl;
    std::cout << "    • Multiple Clients için web arayüzünden ayarlayın\n" << std::endl;

    if (argc > 1) {
        std::string example = argv[1];

        if (example == "basic") {
            basicEthernetExample();
        } else if (example == "config") {
            manualConfigExample();
        } else if (example == "sequence") {
            ethernetSequencingExample();
        } else if (example == "monitor") {
            ethernetMonitoringExample();
        } else if (example == "scpi") {
            scpiOverEthernetExample();
        } else if (example == "error") {
            ethernetErrorHandlingExample();
        } else {
            std::cout << "Bilinmeyen örnek: " << example << std::endl;
            std::cout << "\nKullanılabilir örnekler:" << std::endl;
            std::cout << "  basic    - Temel Ethernet bağlantısı" << std::endl;
            std::cout << "  config   - Manuel konfigürasyon" << std::endl;
            std::cout << "  sequence - Voltaj dizisi" << std::endl;
            std::cout << "  monitor  - Sürekli izleme" << std::endl;
            std::cout << "  scpi     - SCPI komutları" << std::endl;
            std::cout << "  error    - Hata yönetimi" << std::endl;
            return 1;
        }
    } else {
        // Tüm örnekleri çalıştır
        basicEthernetExample();
        manualConfigExample();
        ethernetSequencingExample();
        ethernetMonitoringExample();
        scpiOverEthernetExample();
        ethernetErrorHandlingExample();
    }

    std::cout << "\n=======================================" << std::endl;
    std::cout << "Örnekler tamamlandı!" << std::endl;
    std::cout << "=======================================" << std::endl;

    return 0;
}
