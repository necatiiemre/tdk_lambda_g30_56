# TDK Lambda G30 Power Supply Controller

Professional C++ library for controlling TDK Lambda G30 series programmable power supplies via serial communication.

## Features

- **Modern C++ Design**: Clean, professional C++14/17 implementation
- **RAII Compliant**: Automatic resource management
- **Exception-Based Error Handling**: Robust error management
- **Generic and Extensible**: Easy to adapt and extend
- **Full SCPI Support**: Direct access to SCPI commands
- **Safety Features**: Built-in voltage and current limits
- **Cross-Platform**: Works on Linux and Windows
- **Well-Documented**: Comprehensive API documentation

## Capabilities

### Basic Control
- ✅ Connect/disconnect to power supply
- ✅ Enable/disable output
- ✅ Set and read voltage
- ✅ Set and read current limit
- ✅ Measure actual voltage, current, and power
- ✅ Device identification and status

### Advanced Features
- ✅ Voltage and current ramping with configurable rates
- ✅ Over-voltage protection (OVP) control
- ✅ Status monitoring (OVP, OCP, over-temperature)
- ✅ Error queue checking
- ✅ Custom error handlers
- ✅ Raw SCPI command interface
- ✅ Safety limits configuration

## Project Structure

```
tdk_lambda_g30_56/
├── include/
│   └── tdk_lambda_g30.h       # Main header file
├── src/
│   └── tdk_lambda_g30.cpp     # Implementation
├── examples/
│   └── example_usage.cpp      # Usage examples
├── CMakeLists.txt             # Build configuration
└── README.md                  # This file
```

## Requirements

- C++14 or later
- CMake 3.10 or later
- Serial port access (Linux: `/dev/ttyUSB0`, Windows: `COM3`)
- TDK Lambda G30 series power supply

## Building

### Linux

```bash
mkdir build
cd build
cmake ..
make
```

### Windows (Visual Studio)

```cmd
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019"
cmake --build . --config Release
```

### Build Outputs

- `libtdk_lambda_g30.a` - Static library
- `libtdk_lambda_g30.so` (Linux) or `tdk_lambda_g30.dll` (Windows) - Shared library
- `example_usage` - Example program

## Quick Start

### Basic Usage

```cpp
#include "tdk_lambda_g30.h"
#include <iostream>

using namespace TDKLambda;

int main() {
    try {
        // Configure connection
        G30Config config;
        config.port = "/dev/ttyUSB0";  // Linux: /dev/ttyUSB0, Windows: COM3
        config.baudRate = 9600;

        // Create power supply controller
        TDKLambdaG30 psu(config);

        // Connect
        psu.connect();
        std::cout << "Connected: " << psu.getIdentification() << std::endl;

        // Set voltage and current
        psu.setVoltage(12.0);   // 12V
        psu.setCurrent(2.5);    // 2.5A current limit

        // Enable output
        psu.enableOutput(true);

        // Measure values
        std::cout << "Voltage: " << psu.measureVoltage() << "V" << std::endl;
        std::cout << "Current: " << psu.measureCurrent() << "A" << std::endl;
        std::cout << "Power: " << psu.measurePower() << "W" << std::endl;

        // Disable output
        psu.enableOutput(false);

    } catch (const G30Exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
```

### Using Factory Function

```cpp
#include "tdk_lambda_g30.h"

int main() {
    // Simpler creation using factory function
    auto psu = TDKLambda::createG30("/dev/ttyUSB0", 9600);

    psu->connect();
    psu->setVoltage(5.0);
    psu->enableOutput(true);

    return 0;
}
```

## Advanced Examples

### Voltage Ramping

```cpp
// Ramp voltage from current value to 15V at 1V/second
psu.setVoltageWithRamp(15.0, 1.0);
```

### Safety Limits

```cpp
// Set maximum allowed voltage and current
psu.setMaxVoltage(24.0);   // Don't allow more than 24V
psu.setMaxCurrent(5.0);    // Don't allow more than 5A

// These will throw exceptions if exceeded
psu.setVoltage(30.0);  // Throws G30Exception!
```

### Over-Voltage Protection

```cpp
// Set OVP level
psu.setOverVoltageProtection(15.0);  // Trip at 15V

// Check OVP setting
double ovp = psu.getOverVoltageProtection();

// Clear protection if triggered
psu.clearProtection();
```

### Status Monitoring

```cpp
PowerSupplyStatus status = psu.getStatus();

if (status.outputEnabled) {
    std::cout << "Output is ON" << std::endl;
}

if (status.overVoltageProtection) {
    std::cout << "WARNING: OVP triggered!" << std::endl;
}

if (status.overCurrentProtection) {
    std::cout << "WARNING: OCP triggered!" << std::endl;
}
```

### Custom Error Handler

```cpp
psu.setErrorHandler([](const std::string& error) {
    std::cerr << "[CUSTOM] Error: " << error << std::endl;
    // Log to file, send alert, etc.
});
```

### Raw SCPI Commands

```cpp
// Send raw SCPI command
psu.sendCommand("*RST");  // Reset device

// Send raw SCPI query
std::string response = psu.sendQuery("*IDN?");
std::cout << "Device: " << response << std::endl;
```

### Voltage Sequencing

```cpp
std::vector<double> sequence = {3.3, 5.0, 9.0, 12.0, 15.0};

psu.enableOutput(true);

for (double voltage : sequence) {
    psu.setVoltage(voltage);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Set to " << voltage << "V" << std::endl;
}

psu.enableOutput(false);
```

## Running Examples

The `examples/example_usage.cpp` file contains multiple demonstration programs:

```bash
# Run all examples
./example_usage

# Run specific example
./example_usage basic      # Basic usage
./example_usage ramp       # Voltage ramping
./example_usage status     # Status monitoring
./example_usage error      # Custom error handler
./example_usage scpi       # Raw SCPI commands
./example_usage sequence   # Voltage sequencing
```

**Note**: Before running examples, update the serial port in the code to match your system.

## API Reference

### Main Classes

#### `TDKLambdaG30`
Main controller class for power supply operations.

**Constructors:**
- `TDKLambdaG30(const G30Config& config)` - Create with configuration
- `TDKLambdaG30(unique_ptr<ISerialPort> port, const G30Config& config)` - Inject custom serial port

**Connection Methods:**
- `void connect()` - Connect to device
- `void disconnect()` - Disconnect from device
- `bool isConnected() const` - Check connection status

**Basic Control:**
- `void enableOutput(bool enable)` - Enable/disable output
- `bool isOutputEnabled() const` - Get output state
- `void reset()` - Reset device to defaults

**Voltage Control:**
- `void setVoltage(double voltage)` - Set output voltage
- `double getVoltage() const` - Get set voltage value
- `double measureVoltage() const` - Measure actual voltage
- `void setVoltageWithRamp(double voltage, double rampRate)` - Ramp to voltage

**Current Control:**
- `void setCurrent(double current)` - Set current limit
- `double getCurrent() const` - Get current limit
- `double measureCurrent() const` - Measure actual current
- `void setCurrentWithRamp(double current, double rampRate)` - Ramp to current

**Power and Protection:**
- `double measurePower() const` - Measure output power
- `void setOverVoltageProtection(double voltage)` - Set OVP level
- `double getOverVoltageProtection() const` - Get OVP level
- `void clearProtection()` - Clear protection faults

**Status and Info:**
- `string getIdentification() const` - Get device ID
- `PowerSupplyStatus getStatus() const` - Get detailed status
- `string checkError() const` - Check error queue

**Safety Limits:**
- `double getMaxVoltage() const` - Get voltage limit
- `double getMaxCurrent() const` - Get current limit
- `void setMaxVoltage(double voltage)` - Set voltage limit
- `void setMaxCurrent(double current)` - Set current limit

**Advanced:**
- `string sendCommand(const string& command)` - Send SCPI command
- `string sendQuery(const string& query) const` - Send SCPI query
- `void setErrorHandler(function<void(const string&)> handler)` - Set error callback

### Configuration Structure

#### `G30Config`
```cpp
struct G30Config {
    string port;         // Serial port (e.g., "/dev/ttyUSB0" or "COM3")
    int baudRate;        // Baud rate (default: 9600)
    int dataBits;        // Data bits (default: 8)
    int stopBits;        // Stop bits (default: 1)
    char parity;         // Parity ('N', 'E', 'O')
    int timeout_ms;      // Communication timeout (default: 1000)
};
```

### Status Structure

#### `PowerSupplyStatus`
```cpp
struct PowerSupplyStatus {
    bool outputEnabled;          // Output state
    bool overVoltageProtection;  // OVP triggered
    bool overCurrentProtection;  // OCP triggered
    bool overTemperature;        // Over temperature
    bool remoteSensing;          // Remote sensing enabled
};
```

### Exception Handling

All methods throw `G30Exception` on errors:

```cpp
try {
    psu.setVoltage(12.0);
} catch (const G30Exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
}
```

## Serial Port Configuration

### Linux
- Default port: `/dev/ttyUSB0` or `/dev/ttyACM0`
- User must have permission to access the port
- Add user to `dialout` group: `sudo usermod -a -G dialout $USER`

### Windows
- Default port: `COM3` (check Device Manager)
- No special permissions needed

### Common Settings
- Baud rate: 9600 (default for TDK Lambda G30)
- Data bits: 8
- Stop bits: 1
- Parity: None
- Flow control: None

## SCPI Commands Reference

The library uses standard SCPI commands for TDK Lambda G30:

| Command | Description |
|---------|-------------|
| `*IDN?` | Get device identification |
| `*RST` | Reset device |
| `*CLS` | Clear status |
| `OUTP ON\|OFF` | Enable/disable output |
| `OUTP?` | Query output state |
| `VOLT <value>` | Set voltage |
| `VOLT?` | Query set voltage |
| `MEAS:VOLT?` | Measure voltage |
| `CURR <value>` | Set current |
| `CURR?` | Query set current |
| `MEAS:CURR?` | Measure current |
| `VOLT:PROT <value>` | Set OVP level |
| `VOLT:PROT?` | Query OVP level |
| `STAT:QUES?` | Query status |
| `SYST:ERR?` | Query error |

## Safety Considerations

1. **Always set current limits** before enabling output
2. **Use OVP** to protect sensitive loads
3. **Monitor status** for protection events
4. **Implement proper error handling** in your application
5. **Test with safe loads** before using with expensive equipment
6. **Verify connections** before powering sensitive devices

## Troubleshooting

### Cannot connect to device
- Check serial port name (Linux: `ls /dev/tty*`, Windows: Device Manager)
- Verify user has permission (Linux: check `dialout` group)
- Ensure correct baud rate (usually 9600 for G30)
- Try another USB port
- Check USB cable quality

### Commands not working
- Verify device is powered on
- Check communication settings match device
- Increase timeout in config
- Try raw SCPI commands to test communication

### Compilation errors
- Ensure C++14 or later is enabled
- Check CMake version (3.10+)
- Verify all source files are present

## License

MIT License

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues.

## Author

Professional Power Supply Control Library

## Version

1.0.0 (2025-11-24)

## Support

For issues and questions, please open an issue on the project repository.
