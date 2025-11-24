/**
 * @file simple_test.cpp
 * @brief Simple test program for TDK Lambda G30 on Ubuntu
 */

#include "../include/tdk_lambda_g30.h"
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>

using namespace TDKLambda;

int main() {
    std::cout << "==================================" << std::endl;
    std::cout << "TDK Lambda G30 Simple Test (Ethernet)" << std::endl;
    std::cout << "==================================" << std::endl;

    try {
        // Ethernet bağlantısı
        std::string ipAddress = "192.168.1.100";
        int port = 8003;

        std::cout << "\nIP Adresi: " << ipAddress << ":" << port << std::endl;
        std::cout << "Bağlanıyor..." << std::endl;

        // Power supply oluştur
        auto psu = createG30Ethernet(ipAddress, port);

        // Bağlan
        psu->connect();
        std::cout << "✓ Bağlantı başarılı!" << std::endl;

        // Cihaz bilgilerini al
        std::string id = psu->getIdentification();
        std::cout << "\nCihaz: " << id << std::endl;

        // Güvenlik limitleri ayarla
        psu->setMaxVoltage(30.0);   // G30 için max 30V
        psu->setMaxCurrent(56.0);   // G30 için max 56A (modele göre)

        std::cout << "\nGüvenlik limitleri:" << std::endl;
        std::cout << "  Max Voltaj: " << psu->getMaxVoltage() << "V" << std::endl;
        std::cout << "  Max Akım: " << psu->getMaxCurrent() << "A" << std::endl;

        // Test parametreleri
        double testVoltage = 12.0;  // 12V
        double testCurrent = 2.0;   // 2A limit

        std::cout << "\n--- Test Parametreleri ---" << std::endl;
        std::cout << "Voltaj ayarı: " << testVoltage << "V" << std::endl;
        std::cout << "Akım limiti: " << testCurrent << "A" << std::endl;

        // Voltaj ve akım ayarla
        psu->setVoltage(testVoltage);
        psu->setCurrent(testCurrent);

        // Ayarlanan değerleri kontrol et
        std::cout << std::fixed << std::setprecision(3);
        std::cout << "\nAyarlanan değerler:" << std::endl;
        std::cout << "  Voltaj: " << psu->getVoltage() << "V" << std::endl;
        std::cout << "  Akım: " << psu->getCurrent() << "A" << std::endl;

        // OVP ayarla
        psu->setOverVoltageProtection(testVoltage + 2.0);
        std::cout << "  OVP: " << psu->getOverVoltageProtection() << "V" << std::endl;

        // Çıkışı aç
        std::cout << "\n⚡ Çıkış aktifleştiriliyor..." << std::endl;
        psu->enableOutput(true);

        // Kısa bir bekleme
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        // Ölçümler
        double measuredV = psu->measureVoltage();
        double measuredI = psu->measureCurrent();
        double measuredP = psu->measurePower();

        std::cout << "\n📊 Ölçülen Değerler:" << std::endl;
        std::cout << "  Voltaj: " << measuredV << " V" << std::endl;
        std::cout << "  Akım:   " << measuredI << " A" << std::endl;
        std::cout << "  Güç:    " << measuredP << " W" << std::endl;

        // Durum kontrolü
        PowerSupplyStatus status = psu->getStatus();
        std::cout << "\n🔍 Durum:" << std::endl;
        std::cout << "  Çıkış: " << (status.outputEnabled ? "AÇIK" : "KAPALI") << std::endl;

        if (status.overVoltageProtection) {
            std::cout << "  ⚠️ UYARI: Aşırı voltaj koruması aktif!" << std::endl;
        }
        if (status.overCurrentProtection) {
            std::cout << "  ⚠️ UYARI: Aşırı akım koruması aktif!" << std::endl;
        }
        if (status.overTemperature) {
            std::cout << "  ⚠️ UYARI: Aşırı sıcaklık!" << std::endl;
        }

        // Hata kontrolü
        std::string error = psu->checkError();
        if (!error.empty()) {
            std::cout << "\nCihaz hata mesajı: " << error << std::endl;
        }

        // Çıkışı kapat
        std::cout << "\n🔌 Çıkış kapatılıyor..." << std::endl;
        psu->enableOutput(false);

        // Bağlantıyı kes
        psu->disconnect();
        std::cout << "✓ Bağlantı kesildi" << std::endl;

        std::cout << "\n==================================" << std::endl;
        std::cout << "✓ Test başarıyla tamamlandı!" << std::endl;
        std::cout << "==================================" << std::endl;

    } catch (const G30Exception& e) {
        std::cerr << "\n❌ HATA: " << e.what() << std::endl;
        std::cerr << "\nÖneriler:" << std::endl;
        std::cerr << "  1. Seri portu kontrol et: ls -l /dev/ttyUSB*" << std::endl;
        std::cerr << "  2. İzinleri kontrol et: groups | grep dialout" << std::endl;
        std::cerr << "  3. Cihazın bağlı ve açık olduğunu kontrol et" << std::endl;
        std::cerr << "  4. Baud rate'i kontrol et (genelde 9600)" << std::endl;
        return 1;
    }

    return 0;
}
