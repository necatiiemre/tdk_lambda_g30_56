# Ubuntu Kullanım Kılavuzu - TDK Lambda G30

Bu kılavuz, Ubuntu sistemlerde TDK Lambda G30 kütüphanesini kullanmak için adım adım talimatlar içerir.

## 🚀 Hızlı Başlangıç

### 1. Sistem Gereksinimlerini Kur

```bash
# Paket listesini güncelle
sudo apt update

# Gerekli paketleri kur
sudo apt install -y build-essential cmake git

# Versiyonları kontrol et
gcc --version     # 7.0 veya üstü olmalı
g++ --version     # 7.0 veya üstü olmalı
cmake --version   # 3.10 veya üstü olmalı
```

### 2. Projeyi Derle

```bash
# Proje dizinine git
cd ~/tdk_lambda_g30_56

# Build dizini oluştur
mkdir -p build
cd build

# CMake ile yapılandır
cmake ..

# Tüm CPU çekirdekleriyle derle
make -j$(nproc)

# Sonucu kontrol et
ls -lh
```

Başarılı derleme sonucu:
```
-rw-r--r-- libtdk_lambda_g30.a      (static library)
-rwxr-xr-x libtdk_lambda_g30.so     (shared library)
-rwxr-xr-x example_usage            (örnek program)
-rwxr-xr-x simple_test              (basit test)
```

### 3. Seri Port Ayarları

#### 3.1. Seri Portları Bul

```bash
# USB seri portları listele
ls -l /dev/ttyUSB* /dev/ttyACM*

# Tipik çıktı:
# crw-rw---- 1 root dialout 188, 0 Nov 24 10:00 /dev/ttyUSB0
```

#### 3.2. Kullanıcı İzinlerini Ayarla

```bash
# Kullanıcını dialout grubuna ekle (bir kez yapılır)
sudo usermod -a -G dialout $USER

# Değişikliğin etkili olması için LOGOUT/LOGIN yap
# VEYA şunu çalıştır:
newgrp dialout

# İzinleri kontrol et
groups | grep dialout
```

#### 3.3. Port İzinlerini Test Et

```bash
# Port bilgilerini görüntüle
ls -l /dev/ttyUSB0

# Çıktı şöyle olmalı:
# crw-rw---- 1 root dialout 188, 0 Nov 24 10:00 /dev/ttyUSB0
#           ^     ^
#    root user  dialout group (sen bu grupta olmalısın)
```

### 4. Basit Test

```bash
# Build dizininde simple_test'i çalıştır
cd ~/tdk_lambda_g30_56/build
./simple_test
```

**Not**: Eğer port `/dev/ttyUSB0` değilse, kaynak kodda değiştir:

```bash
# Port adını bul
ls /dev/ttyUSB*

# Kaynak kodu düzenle
nano ~/tdk_lambda_g30_56/examples/simple_test.cpp
# std::string port = "/dev/ttyUSB0"; satırını değiştir

# Yeniden derle
cd build
make -j$(nproc)
./simple_test
```

## 📝 Kendi Programını Yaz

### Örnek 1: Minimal Program

Dosya: `my_test.cpp`

```cpp
#include "../include/tdk_lambda_g30.h"
#include <iostream>

int main() {
    try {
        auto psu = TDKLambda::createG30("/dev/ttyUSB0", 9600);

        psu->connect();
        std::cout << "Bağlandı: " << psu->getIdentification() << std::endl;

        psu->setVoltage(5.0);   // 5V
        psu->setCurrent(1.0);   // 1A
        psu->enableOutput(true);

        std::cout << "Voltaj: " << psu->measureVoltage() << "V" << std::endl;
        std::cout << "Akım: " << psu->measureCurrent() << "A" << std::endl;

        psu->enableOutput(false);

    } catch (const TDKLambda::G30Exception& e) {
        std::cerr << "Hata: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
```

### Derleme

```bash
# Manuel derleme
g++ -std=c++14 my_test.cpp -o my_test \
    -I../include \
    -L. -ltdk_lambda_g30 \
    -pthread

# Çalıştır
./my_test
```

### Veya CMakeLists.txt'e Ekle

`CMakeLists.txt` dosyasına ekle:

```cmake
add_executable(my_test examples/my_test.cpp)
target_link_libraries(my_test tdk_lambda_g30_static)
```

Sonra:

```bash
cd build
cmake ..
make my_test
./my_test
```

## 🔧 Sorun Giderme

### "Permission denied" Hatası

```bash
# İzin hatası alıyorsan:
sudo chmod 666 /dev/ttyUSB0

# Veya kalıcı çözüm için udev kuralı oluştur:
sudo nano /etc/udev/rules.d/99-serial.rules

# İçine şunu ekle:
SUBSYSTEM=="tty", ATTRS{idVendor}=="xxxx", MODE="0666"

# udev'i yeniden yükle
sudo udevadm control --reload-rules
sudo udevadm trigger
```

### "No such file or directory" (/dev/ttyUSB0)

```bash
# USB cihazları listele
lsusb

# Kernel loglarını kontrol et
dmesg | grep tty

# USB seri portları tekrar ara
ls /dev/tty* | grep -E "USB|ACM"
```

### "Cannot connect to device"

```bash
# 1. Cihazın açık olduğunu kontrol et
# 2. USB kabloyu çıkar-tak
# 3. Baud rate'i kontrol et (genelde 9600)
# 4. Başka bir USB port dene

# Seri port test et
sudo apt install minicom
minicom -D /dev/ttyUSB0 -b 9600

# Minicom'da test komutları:
# *IDN? [ENTER]  -> Cihaz kimliği görmeli
```

### Derleme Hataları

```bash
# C++ standart desteğini kontrol et
g++ -std=c++14 --version

# CMake cache'i temizle
cd build
rm -rf *
cmake ..
make
```

## 📊 Performans İpuçları

### 1. Timeout Ayarı

Yavaş yanıt için timeout'u artır:

```cpp
config.timeout_ms = 3000;  // 3 saniye
```

### 2. Ramping Kullan

Voltaj değişimlerinde ramping kullan:

```cpp
psu.setVoltageWithRamp(15.0, 1.0);  // 1V/s ile 15V'a çık
```

### 3. Batch İşlemler

Çok sayıda ölçüm için delay ekle:

```cpp
for (int i = 0; i < 100; i++) {
    double v = psu.measureVoltage();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}
```

## 🔐 Güvenlik İpuçları

### 1. Limitleri Ayarla

```cpp
psu.setMaxVoltage(24.0);   // Max 24V
psu.setMaxCurrent(5.0);    // Max 5A
```

### 2. OVP Kullan

```cpp
psu.setOverVoltageProtection(13.0);  // 13V'da kes
```

### 3. Hata Kontrolü

```cpp
PowerSupplyStatus status = psu.getStatus();
if (status.overVoltageProtection) {
    std::cerr << "OVP tetiklendi!" << std::endl;
    psu.clearProtection();
}
```

## 🧪 Gerçek Cihaz ile Test

### Basit LED Testi

1. LED + direnç (330Ω) bağla
2. Program çalıştır:

```cpp
psu.setVoltage(3.3);    // LED için 3.3V
psu.setCurrent(0.02);   // 20mA limit
psu.enableOutput(true);

// LED yanmalı
std::this_thread::sleep_for(std::chrono::seconds(2));

psu.enableOutput(false);
```

### Yük Testi

```cpp
// Sabit güç yükü ile test
psu.setVoltage(12.0);
psu.setCurrent(5.0);
psu.enableOutput(true);

// 10 saniye boyunca ölç
for (int i = 0; i < 10; i++) {
    std::cout << "V: " << psu.measureVoltage()
              << " I: " << psu.measureCurrent()
              << " P: " << psu.measurePower() << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
}
```

## 📚 Ek Kaynaklar

- Ana README: `README.md`
- SCPI Komutları: README.md içindeki "SCPI Commands Reference" bölümü
- Örnekler: `examples/` dizini
- API Dökümantasyonu: `include/tdk_lambda_g30.h`

## 💡 Faydalı Komutlar

```bash
# Projeyi tamamen temizle ve yeniden derle
cd ~/tdk_lambda_g30_56
rm -rf build
mkdir build && cd build
cmake .. && make -j$(nproc)

# Sadece değişen dosyaları derle
cd ~/tdk_lambda_g30_56/build
make -j$(nproc)

# Belirli bir target derle
make simple_test

# Debug modda derle
cmake -DCMAKE_BUILD_TYPE=Debug ..
make

# Release modda derle (optimizasyonlu)
cmake -DCMAKE_BUILD_TYPE=Release ..
make

# Verbose output ile derle
make VERBOSE=1

# Kütüphaneyi sistem genelinde kur
sudo make install
```

## 🎯 Sonraki Adımlar

1. ✅ `simple_test` ile bağlantıyı test et
2. ✅ Kendi basit programını yaz
3. ✅ `example_usage` örneğini incele
4. ✅ Projene entegre et
5. ✅ Gelişmiş özellikler kullan (ramping, monitoring, vb.)

---

**Ubuntu versiyonu**: 20.04 LTS veya üstü önerilir
**Son güncelleme**: 2025-11-24
