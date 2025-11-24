/**
 * @file tdk_lambda_g30.cpp
 * @brief Implementation of TDK Lambda G30 Power Supply Controller
 * @version 1.0.0
 * @date 2025-11-24
 */

#include "../include/tdk_lambda_g30.h"
#include <sstream>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <thread>
#include <chrono>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <fcntl.h>
    #include <unistd.h>
    #include <termios.h>
    #include <cstring>
#endif

namespace TDKLambda {

// ==================== Serial Port Implementation ====================

/**
 * @brief Default serial port implementation using native OS APIs
 */
class DefaultSerialPort : public ISerialPort {
public:
    DefaultSerialPort(const G30Config& config)
        : config_(config), isOpen_(false) {
#ifdef _WIN32
        hSerial_ = INVALID_HANDLE_VALUE;
#else
        fd_ = -1;
#endif
    }

    ~DefaultSerialPort() override {
        close();
    }

    void open() {
#ifdef _WIN32
        openWindows();
#else
        openPosix();
#endif
    }

    size_t write(const std::string& data) override {
        if (!isOpen_) {
            throw G30Exception("Serial port is not open");
        }

#ifdef _WIN32
        DWORD bytesWritten;
        if (!WriteFile(hSerial_, data.c_str(), static_cast<DWORD>(data.length()),
                      &bytesWritten, nullptr)) {
            throw G30Exception("Failed to write to serial port");
        }
        return static_cast<size_t>(bytesWritten);
#else
        ssize_t result = ::write(fd_, data.c_str(), data.length());
        if (result < 0) {
            throw G30Exception("Failed to write to serial port: " + std::string(strerror(errno)));
        }
        return static_cast<size_t>(result);
#endif
    }

    std::string read(int timeout_ms = 1000) override {
        if (!isOpen_) {
            throw G30Exception("Serial port is not open");
        }

        std::string result;
        char buffer[256];
        auto start = std::chrono::steady_clock::now();

        while (true) {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();

            if (elapsed >= timeout_ms) {
                break;
            }

#ifdef _WIN32
            DWORD bytesRead;
            if (ReadFile(hSerial_, buffer, sizeof(buffer) - 1, &bytesRead, nullptr) && bytesRead > 0) {
                buffer[bytesRead] = '\0';
                result += buffer;
                if (result.find('\n') != std::string::npos) {
                    break;
                }
            }
#else
            ssize_t bytesRead = ::read(fd_, buffer, sizeof(buffer) - 1);
            if (bytesRead > 0) {
                buffer[bytesRead] = '\0';
                result += buffer;
                if (result.find('\n') != std::string::npos) {
                    break;
                }
            }
#endif
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        return result;
    }

    bool isOpen() const override {
        return isOpen_;
    }

    void close() override {
        if (!isOpen_) {
            return;
        }

#ifdef _WIN32
        if (hSerial_ != INVALID_HANDLE_VALUE) {
            CloseHandle(hSerial_);
            hSerial_ = INVALID_HANDLE_VALUE;
        }
#else
        if (fd_ != -1) {
            ::close(fd_);
            fd_ = -1;
        }
#endif
        isOpen_ = false;
    }

private:
    G30Config config_;
    bool isOpen_;

#ifdef _WIN32
    HANDLE hSerial_;

    void openWindows() {
        std::string portName = "\\\\.\\" + config_.port;
        hSerial_ = CreateFileA(
            portName.c_str(),
            GENERIC_READ | GENERIC_WRITE,
            0,
            nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            nullptr
        );

        if (hSerial_ == INVALID_HANDLE_VALUE) {
            throw G30Exception("Failed to open serial port: " + config_.port);
        }

        DCB dcbSerialParams = {0};
        dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

        if (!GetCommState(hSerial_, &dcbSerialParams)) {
            CloseHandle(hSerial_);
            throw G30Exception("Failed to get serial port state");
        }

        dcbSerialParams.BaudRate = config_.baudRate;
        dcbSerialParams.ByteSize = config_.dataBits;
        dcbSerialParams.StopBits = (config_.stopBits == 1) ? ONESTOPBIT : TWOSTOPBITS;
        dcbSerialParams.Parity = (config_.parity == 'N') ? NOPARITY :
                                 (config_.parity == 'E') ? EVENPARITY : ODDPARITY;

        if (!SetCommState(hSerial_, &dcbSerialParams)) {
            CloseHandle(hSerial_);
            throw G30Exception("Failed to set serial port parameters");
        }

        COMMTIMEOUTS timeouts = {0};
        timeouts.ReadIntervalTimeout = 50;
        timeouts.ReadTotalTimeoutConstant = config_.timeout_ms;
        timeouts.ReadTotalTimeoutMultiplier = 10;
        timeouts.WriteTotalTimeoutConstant = config_.timeout_ms;
        timeouts.WriteTotalTimeoutMultiplier = 10;

        if (!SetCommTimeouts(hSerial_, &timeouts)) {
            CloseHandle(hSerial_);
            throw G30Exception("Failed to set serial port timeouts");
        }

        isOpen_ = true;
    }
#else
    int fd_;

    void openPosix() {
        fd_ = ::open(config_.port.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
        if (fd_ == -1) {
            throw G30Exception("Failed to open serial port: " + config_.port);
        }

        struct termios options;
        if (tcgetattr(fd_, &options) != 0) {
            ::close(fd_);
            throw G30Exception("Failed to get serial port attributes");
        }

        speed_t speed;
        switch (config_.baudRate) {
            case 9600: speed = B9600; break;
            case 19200: speed = B19200; break;
            case 38400: speed = B38400; break;
            case 57600: speed = B57600; break;
            case 115200: speed = B115200; break;
            default:
                ::close(fd_);
                throw G30Exception("Unsupported baud rate: " + std::to_string(config_.baudRate));
        }

        cfsetispeed(&options, speed);
        cfsetospeed(&options, speed);

        options.c_cflag |= (CLOCAL | CREAD);
        options.c_cflag &= ~PARENB;
        options.c_cflag &= ~CSTOPB;
        options.c_cflag &= ~CSIZE;
        options.c_cflag |= CS8;
        options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
        options.c_iflag &= ~(IXON | IXOFF | IXANY);
        options.c_oflag &= ~OPOST;

        options.c_cc[VMIN] = 0;
        options.c_cc[VTIME] = config_.timeout_ms / 100;

        if (tcsetattr(fd_, TCSANOW, &options) != 0) {
            ::close(fd_);
            throw G30Exception("Failed to set serial port attributes");
        }

        tcflush(fd_, TCIOFLUSH);
        isOpen_ = true;
    }
#endif
};

// ==================== TDKLambdaG30 Implementation ====================

TDKLambdaG30::TDKLambdaG30(const G30Config& config)
    : serialPort_(std::make_unique<DefaultSerialPort>(config)),
      config_(config),
      connected_(false),
      outputEnabled_(false),
      maxVoltage_(30.0),
      maxCurrent_(56.0),
      errorHandler_(nullptr) {
}

TDKLambdaG30::TDKLambdaG30(std::unique_ptr<ISerialPort> serialPort, const G30Config& config)
    : serialPort_(std::move(serialPort)),
      config_(config),
      connected_(false),
      outputEnabled_(false),
      maxVoltage_(30.0),
      maxCurrent_(56.0),
      errorHandler_(nullptr) {
}

TDKLambdaG30::~TDKLambdaG30() {
    try {
        if (connected_) {
            enableOutput(false);
            disconnect();
        }
    } catch (...) {
        // Suppress exceptions in destructor
    }
}

TDKLambdaG30::TDKLambdaG30(TDKLambdaG30&& other) noexcept
    : serialPort_(std::move(other.serialPort_)),
      config_(std::move(other.config_)),
      connected_(other.connected_),
      outputEnabled_(other.outputEnabled_),
      maxVoltage_(other.maxVoltage_),
      maxCurrent_(other.maxCurrent_),
      errorHandler_(std::move(other.errorHandler_)) {
    other.connected_ = false;
}

TDKLambdaG30& TDKLambdaG30::operator=(TDKLambdaG30&& other) noexcept {
    if (this != &other) {
        serialPort_ = std::move(other.serialPort_);
        config_ = std::move(other.config_);
        connected_ = other.connected_;
        outputEnabled_ = other.outputEnabled_;
        maxVoltage_ = other.maxVoltage_;
        maxCurrent_ = other.maxCurrent_;
        errorHandler_ = std::move(other.errorHandler_);
        other.connected_ = false;
    }
    return *this;
}

void TDKLambdaG30::connect() {
    if (connected_) {
        return;
    }

    try {
        auto* defaultPort = dynamic_cast<DefaultSerialPort*>(serialPort_.get());
        if (defaultPort) {
            defaultPort->open();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        std::string id = getIdentification();
        if (id.empty()) {
            throw G30Exception("Failed to communicate with device");
        }

        connected_ = true;

        reset();
        clearProtection();

    } catch (const std::exception& e) {
        disconnect();
        throw G30Exception("Connection failed: " + std::string(e.what()));
    }
}

void TDKLambdaG30::disconnect() {
    if (serialPort_) {
        serialPort_->close();
    }
    connected_ = false;
}

bool TDKLambdaG30::isConnected() const {
    return connected_ && serialPort_ && serialPort_->isOpen();
}

void TDKLambdaG30::enableOutput(bool enable) {
    if (!isConnected()) {
        throw G30Exception("Not connected to device");
    }

    std::string command = enable ? "OUTP ON\n" : "OUTP OFF\n";
    serialPort_->write(command);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    outputEnabled_ = enable;
}

bool TDKLambdaG30::isOutputEnabled() const {
    if (!isConnected()) {
        throw G30Exception("Not connected to device");
    }

    std::string response = sendQuery("OUTP?");
    return (trim(response) == "1" || trim(response) == "ON");
}

void TDKLambdaG30::reset() {
    if (!isConnected()) {
        throw G30Exception("Not connected to device");
    }

    serialPort_->write("*RST\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    outputEnabled_ = false;
}

void TDKLambdaG30::setVoltage(double voltage) {
    validateVoltage(voltage);

    if (!isConnected()) {
        throw G30Exception("Not connected to device");
    }

    std::ostringstream oss;
    oss.precision(3);
    oss << std::fixed << "VOLT " << voltage << "\n";

    serialPort_->write(oss.str());
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

double TDKLambdaG30::getVoltage() const {
    if (!isConnected()) {
        throw G30Exception("Not connected to device");
    }

    std::string response = sendQuery("VOLT?");
    return parseNumericResponse(response);
}

double TDKLambdaG30::measureVoltage() const {
    if (!isConnected()) {
        throw G30Exception("Not connected to device");
    }

    std::string response = sendQuery("MEAS:VOLT?");
    return parseNumericResponse(response);
}

void TDKLambdaG30::setVoltageWithRamp(double voltage, double rampRate) {
    validateVoltage(voltage);

    if (rampRate <= 0) {
        throw G30Exception("Ramp rate must be positive");
    }

    double currentVoltage = getVoltage();
    double difference = std::abs(voltage - currentVoltage);
    double steps = difference / rampRate * 10;
    double stepVoltage = (voltage - currentVoltage) / steps;

    for (int i = 0; i < static_cast<int>(steps); ++i) {
        currentVoltage += stepVoltage;
        setVoltage(currentVoltage);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    setVoltage(voltage);
}

void TDKLambdaG30::setCurrent(double current) {
    validateCurrent(current);

    if (!isConnected()) {
        throw G30Exception("Not connected to device");
    }

    std::ostringstream oss;
    oss.precision(3);
    oss << std::fixed << "CURR " << current << "\n";

    serialPort_->write(oss.str());
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

double TDKLambdaG30::getCurrent() const {
    if (!isConnected()) {
        throw G30Exception("Not connected to device");
    }

    std::string response = sendQuery("CURR?");
    return parseNumericResponse(response);
}

double TDKLambdaG30::measureCurrent() const {
    if (!isConnected()) {
        throw G30Exception("Not connected to device");
    }

    std::string response = sendQuery("MEAS:CURR?");
    return parseNumericResponse(response);
}

void TDKLambdaG30::setCurrentWithRamp(double current, double rampRate) {
    validateCurrent(current);

    if (rampRate <= 0) {
        throw G30Exception("Ramp rate must be positive");
    }

    double currentCurrent = getCurrent();
    double difference = std::abs(current - currentCurrent);
    double steps = difference / rampRate * 10;
    double stepCurrent = (current - currentCurrent) / steps;

    for (int i = 0; i < static_cast<int>(steps); ++i) {
        currentCurrent += stepCurrent;
        setCurrent(currentCurrent);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    setCurrent(current);
}

double TDKLambdaG30::measurePower() const {
    if (!isConnected()) {
        throw G30Exception("Not connected to device");
    }

    double voltage = measureVoltage();
    double current = measureCurrent();
    return voltage * current;
}

void TDKLambdaG30::setOverVoltageProtection(double voltage) {
    if (!isConnected()) {
        throw G30Exception("Not connected to device");
    }

    std::ostringstream oss;
    oss.precision(3);
    oss << std::fixed << "VOLT:PROT " << voltage << "\n";

    serialPort_->write(oss.str());
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

double TDKLambdaG30::getOverVoltageProtection() const {
    if (!isConnected()) {
        throw G30Exception("Not connected to device");
    }

    std::string response = sendQuery("VOLT:PROT?");
    return parseNumericResponse(response);
}

void TDKLambdaG30::clearProtection() {
    if (!isConnected()) {
        throw G30Exception("Not connected to device");
    }

    serialPort_->write("*CLS\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

std::string TDKLambdaG30::getIdentification() const {
    if (!isConnected() && !serialPort_->isOpen()) {
        throw G30Exception("Not connected to device");
    }

    return sendQuery("*IDN?");
}

PowerSupplyStatus TDKLambdaG30::getStatus() const {
    if (!isConnected()) {
        throw G30Exception("Not connected to device");
    }

    PowerSupplyStatus status;

    try {
        status.outputEnabled = isOutputEnabled();

        std::string statQuery = sendQuery("STAT:QUES?");
        int statValue = static_cast<int>(parseNumericResponse(statQuery));

        status.overVoltageProtection = (statValue & 0x01) != 0;
        status.overCurrentProtection = (statValue & 0x02) != 0;
        status.overTemperature = (statValue & 0x10) != 0;

    } catch (const std::exception& e) {
        if (errorHandler_) {
            errorHandler_("Failed to get complete status: " + std::string(e.what()));
        }
    }

    return status;
}

std::string TDKLambdaG30::checkError() const {
    if (!isConnected()) {
        throw G30Exception("Not connected to device");
    }

    return sendQuery("SYST:ERR?");
}

void TDKLambdaG30::setMaxVoltage(double maxVoltage) {
    if (maxVoltage <= 0) {
        throw G30Exception("Maximum voltage must be positive");
    }
    maxVoltage_ = maxVoltage;
}

void TDKLambdaG30::setMaxCurrent(double maxCurrent) {
    if (maxCurrent <= 0) {
        throw G30Exception("Maximum current must be positive");
    }
    maxCurrent_ = maxCurrent;
}

std::string TDKLambdaG30::sendCommand(const std::string& command) {
    if (!isConnected()) {
        throw G30Exception("Not connected to device");
    }

    std::string cmd = command;
    if (cmd.back() != '\n') {
        cmd += '\n';
    }

    serialPort_->write(cmd);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    return "OK";
}

std::string TDKLambdaG30::sendQuery(const std::string& query) const {
    if (!isConnected() && !serialPort_->isOpen()) {
        throw G30Exception("Not connected to device");
    }

    std::string cmd = query;
    if (cmd.back() != '\n') {
        cmd += '\n';
    }

    serialPort_->write(cmd);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    std::string response = serialPort_->read(config_.timeout_ms);
    return trim(response);
}

void TDKLambdaG30::setErrorHandler(std::function<void(const std::string&)> handler) {
    errorHandler_ = handler;
}

void TDKLambdaG30::validateVoltage(double voltage) const {
    if (voltage < 0) {
        throw G30Exception("Voltage cannot be negative");
    }
    if (voltage > maxVoltage_) {
        throw G30Exception("Voltage " + std::to_string(voltage) +
                         "V exceeds maximum limit of " + std::to_string(maxVoltage_) + "V");
    }
}

void TDKLambdaG30::validateCurrent(double current) const {
    if (current < 0) {
        throw G30Exception("Current cannot be negative");
    }
    if (current > maxCurrent_) {
        throw G30Exception("Current " + std::to_string(current) +
                         "A exceeds maximum limit of " + std::to_string(maxCurrent_) + "A");
    }
}

double TDKLambdaG30::parseNumericResponse(const std::string& response) const {
    try {
        std::string cleaned = trim(response);
        return std::stod(cleaned);
    } catch (const std::exception& e) {
        throw G30Exception("Failed to parse numeric response: '" + response + "'");
    }
}

std::string TDKLambdaG30::trim(const std::string& str) const {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) {
        return "";
    }
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, last - first + 1);
}

void TDKLambdaG30::defaultErrorHandler(const std::string& error) {
    std::cerr << "TDK Lambda G30 Error: " << error << std::endl;
}

std::unique_ptr<TDKLambdaG30> createG30(const std::string& port, int baudRate) {
    G30Config config;
    config.port = port;
    config.baudRate = baudRate;
    return std::make_unique<TDKLambdaG30>(config);
}

} // namespace TDKLambda
