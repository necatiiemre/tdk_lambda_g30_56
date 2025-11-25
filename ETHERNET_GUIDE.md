# TDK Lambda G30 Ethernet Kullanım Kılavuzu

## 🌐 Port Bilgileri

TDK Lambda G30 güç kaynağı **özel portlar** kullanır:

| Protocol | Port | Kullanım |
|----------|------|----------|
| **TCP** | **8003** | SCPI komutları (connection-based) |
| **UDP** | **8005** | SCPI komutları (connectionless) |

⚠️ **ÖNEMLİ**: Standard SCPI-over-LAN port **5025** DEĞİLDİR!

## 📡 TCP vs UDP

### TCP Socket (Port 8003) - ÖNERİLEN

**Özellikleri:**
- ✅ Güvenilir bağlantı
- ✅ Mesaj onayı (acknowledgement)
- ✅ Hata tespiti ve düzeltme
- ✅ Sıralı iletim
- ✅ Bu kütüphane tarafından destekleniyor

**Kullanım:**
```cpp
auto psu = TDKLambda::createG30Ethernet("192.168.1.100", 8003);
psu->connect();
psu->setVoltage(12.0);
```

**Multiple Clients:**
- Web arayüzünden "Multiple Clients" ayarını açın
- Aynı anda **3 TCP client** bağlanabilir

### UDP Socket (Port 8005) - OPSİYONEL

**Özellikleri:**
- ✅ Daha az network trafiği
- ⚠️ Connectionless (mesaj onayı yok)
- ⚠️ Mesajların alındığı garanti edilmez
- ❌ Şu an bu kütüphane tarafından desteklenmiyor (gelecek sürümde eklenebilir)

**Gereksinim:**
- UDP kullanmadan önce web arayüzünden "Multiple Clients" aktif edilmeli

## 🔧 Hızlı Başlangıç

### 1. IP Adresini Bulun

```bash
# Network taraması
sudo nmap -sP 192.168.1.0/24

# Veya router DHCP listesine bakın
```

### 2. Bağlantıyı Test Edin

```bash
# Ping testi
ping 192.168.1.100

# TCP port 8003 testi
telnet 192.168.1.100 8003

# Netcat ile test
nc -zv 192.168.1.100 8003

# UDP port 8005 testi (opsiyonel)
nc -zvu 192.168.1.100 8005
```

### 3. Kod ile Bağlan

```cpp
#include "tdk_lambda_g30.h"

int main() {
    // Port 8003 kullanarak bağlan
    auto psu = TDKLambda::createG30Ethernet("192.168.1.100", 8003);

    psu->connect();
    std::cout << "Bağlandı: " << psu->getIdentification() << std::endl;

    // Kontrol komutları
    psu->setVoltage(12.0);
    psu->setCurrent(2.5);
    psu->enableOutput(true);

    // Ölçüm
    std::cout << "V: " << psu->measureVoltage() << "V" << std::endl;
    std::cout << "I: " << psu->measureCurrent() << "A" << std::endl;

    psu->enableOutput(false);
    return 0;
}
```

## 🔍 Sorun Giderme

### Problem: "Failed to connect"

**Çözümler:**
1. IP adresini kontrol edin
2. **Port 8003** kullandığınızdan emin olun (5025 değil!)
3. Ping ile cihaza ulaşabildiğinizi test edin
4. Firewall kurallarını kontrol edin

```bash
# Doğru port ile test
telnet 192.168.1.100 8003   # ✅ Doğru
telnet 192.168.1.100 5025   # ❌ Yanlış!
```

### Problem: "Connection timeout"

**Çözümler:**
1. Timeout değerini artırın:
```cpp
G30Config config;
config.connectionType = ConnectionType::ETHERNET;
config.ipAddress = "192.168.1.100";
config.tcpPort = 8003;
config.timeout_ms = 3000;  // 3 saniye

TDKLambdaG30 psu(config);
```

2. Network gecikmesini kontrol edin:
```bash
ping -c 10 192.168.1.100
```

### Problem: "Multiple clients needed"

**Çözüm:**
1. Web tarayıcıdan cihazın IP adresine gidin
2. LAN controller ayarlarını bulun
3. "Multiple Clients" seçeneğini aktifleştirin
4. En fazla 3 TCP client aynı anda bağlanabilir

## 📊 Network Konfigürasyonu

### Static IP Ayarlama

TDK Lambda G30 web arayüzünden:
1. Tarayıcıda cihazın mevcut IP'sine gidin
2. Network Settings bölümünü açın
3. Static IP ve subnet mask girin
4. Gateway ayarlayın
5. Kaydedin ve cihazı yeniden başlatın

### DHCP Kullanımı

- Varsayılan olarak DHCP aktif
- Router'ın DHCP client listesinden IP'yi bulabilirsiniz
- Static IP önerilir (IP değişimi sorunlarını önler)

## 🔐 Güvenlik

### Firewall Ayarları

```bash
# Ubuntu - Giden TCP bağlantısına izin ver
sudo ufw allow out 8003/tcp

# UDP kullanıyorsanız
sudo ufw allow out 8005/udp

# Kontrol
sudo ufw status
```

### Network Güvenliği

⚠️ **Öneriler:**
- Güç kaynağını güvenli bir network'e bağlayın
- Gereksiz portları kapatın
- Güçlü şifreler kullanın
- Firmware'i güncel tutun

## 📚 Ek Kaynaklar

- Ana README: `README.md`
- Ubuntu Kılavuzu: `UBUNTU_GUIDE.md`
- Örnek Kod: `examples/ethernet_example.cpp`
- API Referansı: `include/tdk_lambda_g30.h`

## 🎯 Özet Checklist

- [ ] IP adresini biliyorum
- [ ] **Port 8003** kullanıyorum (5025 değil!)
- [ ] `ping` ile cihaza ulaşabiliyorum
- [ ] `telnet 192.168.1.100 8003` çalışıyor
- [ ] Firewall kuralları doğru
- [ ] Multiple clients gerekiyorsa web'den aktifleştirdim
- [ ] Timeout değeri yeterli (en az 1000ms, network için 2000-3000ms önerilir)

---

**Son Güncelleme**: 2025-11-24
**Versiyon**: 1.0.0
