# Generic Power Supply Architecture

## 📐 Mimari Tasarım

Bu kütüphane **generic (genel)** bir yapıya sahiptir ve farklı üreticilerin güç kaynaklarını destekleyebilir.

### Tasarım Prensipleri

1. **Interface-Based**: Tüm PSU'lar `IPowerSupply` interface'ini implement eder
2. **Vendor-Agnostic**: Ortak işlemler generic interface ile yapılır
3. **Extensible**: Yeni PSU markaları kolayca eklenebilir
4. **Backward Compatible**: Mevcut TDK Lambda G30 API'si korunur
5. **Factory Pattern**: PSU oluşturma merkezi factory üzerinden

## 🏗️ Katman Yapısı

```
┌─────────────────────────────────────────────┐
│         Application Layer                    │
│  (User code using PSU controllers)          │
└─────────────────────────────────────────────┘
                    ↓
┌─────────────────────────────────────────────┐
│     Generic Interface Layer                  │
│  ┌─────────────────────────────────────┐   │
│  │  IPowerSupply (Abstract Interface)   │   │
│  │  - Common methods for all PSUs       │   │
│  │  - Voltage/Current control           │   │
│  │  - Status/Capabilities                │   │
│  └─────────────────────────────────────┘   │
└─────────────────────────────────────────────┘
                    ↓
┌─────────────────────────────────────────────┐
│    Vendor-Specific Implementations           │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  │
│  │   TDK    │  │ Keysight │  │  Rigol   │  │
│  │ Lambda   │  │  E36xx   │  │  DP8xx   │  │
│  │   G30    │  │          │  │          │  │
│  └──────────┘  └──────────┘  └──────────┘  │
└─────────────────────────────────────────────┘
                    ↓
┌─────────────────────────────────────────────┐
│     Communication Layer                      │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  │
│  │  Serial  │  │ Ethernet │  │   USB    │  │
│  │  RS232   │  │  TCP/IP  │  │  USBTMC  │  │
│  └──────────┘  └──────────┘  └──────────┘  │
│  ┌──────────┐                               │
│  │   GPIB   │                               │
│  │ IEEE-488 │                               │
│  └──────────┘                               │
└─────────────────────────────────────────────┘
```

## 📦 Sınıf Diyagramı

```cpp
// Generic Interface
class IPowerSupply {
public:
    virtual void connect() = 0;
    virtual void setVoltage(double v, int ch) = 0;
    virtual double measureVoltage(int ch) const = 0;
    // ... diğer metodlar
};

// TDK Lambda Implementation
class TDKLambdaG30 : public IPowerSupply {
public:
    void connect() override { /* TDK specific */ }
    void setVoltage(double v, int ch) override { /* SCPI: VOLT <v> */ }
    // ... TDK-specific implementation
};

// Keysight Implementation
class KeysightE36xx : public IPowerSupply {
public:
    void connect() override { /* Keysight specific */ }
    void setVoltage(double v, int ch) override { /* SCPI with channel */ }
    // ... Keysight-specific implementation
};
```

## 🔌 Desteklenen Bağlantı Tipleri

| Bağlantı Tipi | Açıklama | Durum |
|---------------|----------|-------|
| **SERIAL** | RS232/USB-Serial | ✅ Implement edildi |
| **ETHERNET** | TCP/IP (SCPI over LAN) | ✅ Implement edildi |
| **USB** | Direct USB (USBTMC) | ⏳ Gelecek sürüm |
| **GPIB** | IEEE-488 | ⏳ Gelecek sürüm |

## 🏭 Desteklenen / Planl anan Üreticiler

| Üretici | Modeller | Durum | Notlar |
|---------|----------|-------|--------|
| **TDK Lambda** | G30, G60, G100 | ✅ G30 Tam | Multi-channel gelecek |
| **Keysight** | E36xx, N67xx | 📝 Skeleton | Örnek kod var |
| **Rigol** | DP8xx, DP7xx | 📋 Planned | - |
| **Rohde & Schwarz** | HMC804x | 📋 Planned | - |
| **Siglent** | SPD3303X | 📋 Planned | - |

## 🎯 Yeni PSU Ekleme Rehberi

### Adım 1: Header Dosyası Oluştur

```cpp
// include/vendor_model.h
#include "power_supply_interface.h"

namespace Vendor {
    class ModelXYZ : public PowerSupply::IPowerSupply {
    public:
        // Constructor
        ModelXYZ(ConnectionType type, const std::string& conn);

        // IPowerSupply interface implementation
        void connect() override;
        void setVoltage(double v, int ch) override;
        // ... tüm pure virtual metodları implement et

        // Vendor-specific metodlar
        void vendorSpecificFeature();
    };
}
```

### Adım 2: Implementation Dosyası

```cpp
// src/vendor_model.cpp
#include "vendor_model.h"

namespace Vendor {
    void ModelXYZ::connect() {
        // Bağlantı implementasyonu
        // 1. Communication port aç
        // 2. *IDN? ile cihazı kontrol et
        // 3. Reset veya initialize
    }

    void ModelXYZ::setVoltage(double v, int ch) {
        // SCPI komutu gönder
        // Örnek: "VOLT 12.5, (@1)"
    }
}
```

### Adım 3: Factory'ye Ekle

```cpp
// power_supply_interface.cpp
std::unique_ptr<IPowerSupply> PowerSupplyFactory::create(
    Vendor vendor,
    const std::string& model,
    ConnectionType type,
    const std::string& conn
) {
    switch(vendor) {
        case Vendor::TDK_LAMBDA:
            return std::make_unique<TDKLambdaG30>(...);
        case Vendor::YOUR_VENDOR:
            return std::make_unique<YourModel>(...);
        default:
            throw std::runtime_error("Unsupported vendor");
    }
}
```

### Adım 4: Örnek Kod Yaz

```cpp
// examples/vendor_example.cpp
#include "vendor_model.h"

int main() {
    auto psu = Vendor::createModelXYZ(
        ConnectionType::ETHERNET,
        "192.168.1.100"
    );

    psu->connect();
    psu->setVoltage(12.0);
    psu->enableOutput(true);
}
```

## 💡 Örnekler

### Generic Interface Kullanımı

```cpp
#include "power_supply_interface.h"
#include "tdk_lambda_g30.h"

// Generic pointer ile çalış
std::unique_ptr<PowerSupply::IPowerSupply> psu;

// TDK Lambda oluştur
psu = TDKLambda::createG30Ethernet("192.168.1.100", 8003);

// Generic interface metodlarını kullan
psu->connect();
psu->setVoltage(12.0);  // Channel 1 (default)
psu->setCurrent(2.5);
psu->enableOutput(true);

// Capabilities al
auto caps = psu->getCapabilities();
std::cout << "Max Voltage: " << caps.maxVoltage << "V" << std::endl;
std::cout << "Channels: " << caps.numberOfChannels << std::endl;
```

### Factory Pattern ile

```cpp
// Factory ile oluştur
auto psu = PowerSupply::PowerSupplyFactory::create(
    PowerSupply::Vendor::TDK_LAMBDA,
    "G30",
    PowerSupply::ConnectionType::ETHERNET,
    "192.168.1.100:8003"
);

psu->connect();
// ... kullan
```

### Multi-Vendor Uygulama

```cpp
void controlPowerSupply(PowerSupply::IPowerSupply* psu) {
    // Generic kod - tüm PSU'larla çalışır
    psu->setVoltage(12.0);
    psu->setCurrent(3.0);
    psu->enableOutput(true);

    // Ölçüm
    double v = psu->measureVoltage();
    double i = psu->measureCurrent();
    std::cout << "V=" << v << " I=" << i << std::endl;
}

int main() {
    // TDK Lambda
    auto tdkPsu = TDKLambda::createG30Ethernet("192.168.1.10", 8003);
    controlPowerSupply(tdkPsu.get());

    // Keysight (gelecekte)
    // auto keysightPsu = Keysight::createE3631A(...);
    // controlPowerSupply(keysightPsu.get());

    // Aynı kod her iki PSU ile de çalışır!
}
```

## 🔍 Önemli Noktalar

### 1. Channel Management

- **Single-channel PSU'lar** (TDK Lambda G30): Channel parametresi ignore edilir
- **Multi-channel PSU'lar** (Keysight E3631A): Channel parametresi kullanılır
- Default channel = 1

### 2. Capability Reporting

Her PSU kendi yeteneklerini raporlar:
```cpp
auto caps = psu->getCapabilities();
if (caps.supportsOVP) {
    psu->setOverVoltageProtection(15.0);
}
```

### 3. Vendor-Specific Features

Generic interface'de olmayan özellikler için:
```cpp
// Downcast gerekirse
auto* tdkPsu = dynamic_cast<TDKLambdaG30*>(psu.get());
if (tdkPsu) {
    tdkPsu->setVoltageWithRamp(15.0, 1.0); // TDK-specific
}
```

### 4. Error Handling

Tüm PSU'lar `std::runtime_error` veya türevleri fırlatır:
```cpp
try {
    psu->setVoltage(100.0); // Limit aşımı
} catch (const std::runtime_error& e) {
    std::cerr << "Error: " << e.what() << std::endl;
}
```

## 📁 Dosya Organizasyonu

```
tdk_lambda_g30_56/
├── include/
│   ├── power_supply_interface.h       # Generic interface (YENİ!)
│   ├── tdk_lambda_g30.h               # TDK Lambda G30 (IPowerSupply implement eder)
│   ├── keysight_e36xx_skeleton.h      # Örnek skeleton (YENİ!)
│   └── ... (diğer vendor'lar)
├── src/
│   ├── power_supply_interface.cpp     # Factory implementation
│   ├── tdk_lambda_g30.cpp             # TDK implementation
│   └── ... (diğer vendor'lar)
├── examples/
│   ├── generic_usage_example.cpp      # Generic interface örneği (YENİ!)
│   ├── ethernet_example.cpp           # TDK Lambda Ethernet
│   └── simple_test.cpp                # TDK Lambda Serial
└── docs/
    ├── ARCHITECTURE.md                # Bu dosya
    ├── ADDING_NEW_PSU.md              # Yeni PSU ekleme rehberi
    └── README.md                      # Ana dökümantasyon
```

## 🚀 Gelecek Planları

### v1.1.0 - Multi-Channel Support
- [ ] TDK Lambda multi-channel modeller (G60, G100)
- [ ] Generic channel management
- [ ] Channel synchronization

### v1.2.0 - Additional Vendors
- [ ] Keysight E36xx tam implementasyon
- [ ] Rigol DP8xx serisi
- [ ] Siglent SPD3303X

### v1.3.0 - Advanced Features
- [ ] Sequencing support
- [ ] Data logging
- [ ] Remote sensing API
- [ ] Protection event callbacks

### v2.0.0 - Communication Expansion
- [ ] USBTMC support (direct USB)
- [ ] GPIB/IEEE-488 support
- [ ] VISA layer integration

## 📞 Destek

Yeni PSU ekleme konusunda yardım için:
- GitHub Issues açın
- `keysight_e36xx_skeleton.h` dosyasını template olarak kullanın
- Documentation'ı okuyun

---

**Versiyon**: 1.0.0
**Son Güncelleme**: 2025-11-24
