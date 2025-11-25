#include "../include/tdk_lambda_g30.h"
#include <iostream>
#include <iomanip>

using namespace TDKLambda;

int main(int argc, char const *argv[])
{
    try {
        std::string ipAddress = "10.1.33.5";
        int port = 8003;

        std::cout << "TDK Lambda G30 Ethernet bağlantısı kuruluyor..." << std::endl;
        std::cout << "IP Adresi: " << ipAddress << std::endl;
        std::cout << "Port: " << port << std::endl;

        auto psu = createG30Ethernet(ipAddress, port);

        // ÖNEMLİ: Bağlantıyı kurmak için connect() çağrılmalı!
        std::cout << "\nBağlanılıyor..." << std::endl;
        psu->connect();
        std::cout << "✓ Bağlantı başarılı!" << std::endl;

        // Cihaz tanımlama bilgisini al
        std::string id = psu->getIdentification();
        std::cout << "\nCihaz: " << id << std::endl;

        std::cout << "\nVoltaj ve akım ayarlanıyor..." << std::endl;
        psu->setVoltage(12.0);
        psu->setCurrent(2.0);

        std::cout << std::fixed << std::setprecision(3);
        std::cout << "Ayarlanan voltaj: " << psu->getVoltage() << "V" << std::endl;
        std::cout << "Ayarlanan akım: " << psu->getCurrent() << "A" << std::endl;

        std::cout << "\n⚡ Çıkış aktifleştiriliyor..." << std::endl;
        psu->enableOutput(true);

        // Ölçüm yap
        double voltage = psu->measureVoltage();
        double current = psu->measureCurrent();
        double power = psu->measurePower();

        std::cout << "\n📊 Ölçümler:" << std::endl;
        std::cout << "  Voltaj: " << voltage << " V" << std::endl;
        std::cout << "  Akım:   " << current << " A" << std::endl;
        std::cout << "  Güç:    " << power << " W" << std::endl;

        std::cout << "\n🔌 Çıkış kapatılıyor..." << std::endl;
        psu->enableOutput(false);

        psu->disconnect();
        std::cout << "✓ Bağlantı kesildi" << std::endl;

        return 0;

    } catch (const G30Exception& e) {
        std::cerr << "\n❌ Hata: " << e.what() << std::endl;
        std::cerr << "\nOlası nedenler:" << std::endl;
        std::cerr << "  1. IP adresi yanlış (şu an: 10.1.33.5)" << std::endl;
        std::cerr << "  2. Cihaz açık değil" << std::endl;
        std::cerr << "  3. Ağ bağlantısı yok" << std::endl;
        std::cerr << "  4. Port numarası yanlış (şu an: 8003)" << std::endl;
        std::cerr << "  5. Firewall port 8003'ü engelliyor" << std::endl;
        std::cerr << "\nTest için şunu deneyin:" << std::endl;
        std::cerr << "  ping 10.1.33.5" << std::endl;
        std::cerr << "  nc -zv 10.1.33.5 8003" << std::endl;
        return 1;

    } catch (const std::exception& e) {
        std::cerr << "\n❌ Beklenmeyen hata: " << e.what() << std::endl;
        return 1;
    }
}
