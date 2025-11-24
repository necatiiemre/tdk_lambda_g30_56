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
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <fcntl.h>
    #include <unistd.h>
    #include <termios.h>
    #include <cstring>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <errno.h>
    #include <poll.h>
#endif

namespace TDKLambda {

// ==================== TCP/IP Port Implementation ====================

/**
 * @brief TCP/IP port implementation for Ethernet communication
 */
class TcpPort : public ICommunication {
public:
    TcpPort(const G30Config& config)
        : config_(config), isOpen_(false), sockfd_(-1) {
#ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            throw G30Exception("Failed to initialize Winsock");
        }
#endif
    }

    ~TcpPort() override {
        close();
#ifdef _WIN32
        WSACleanup();
#endif
    }

    void open() {
        if (isOpen_) {
            return;
        }

        if (config_.ipAddress.empty()) {
            throw G30Exception("IP address is empty");
        }

        // Create socket
        sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd_ < 0) {
            throw G30Exception("Failed to create socket");
        }

        // Set socket timeout
        struct timeval timeout;
        timeout.tv_sec = config_.timeout_ms / 1000;
        timeout.tv_usec = (config_.timeout_ms % 1000) * 1000;

#ifdef _WIN32
        DWORD tv = config_.timeout_ms;
        setsockopt(sockfd_, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
        setsockopt(sockfd_, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof(tv));
#else
        setsockopt(sockfd_, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
        setsockopt(sockfd_, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
#endif

        // Setup server address
        struct sockaddr_in serverAddr;
        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(config_.tcpPort);

        // Convert IP address
        if (inet_pton(AF_INET, config_.ipAddress.c_str(), &serverAddr.sin_addr) <= 0) {
            closesocket(sockfd_);
            throw G30Exception("Invalid IP address: " + config_.ipAddress);
        }

        // Connect to server
        if (connect(sockfd_, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
            closesocket(sockfd_);
            throw G30Exception("Failed to connect to " + config_.ipAddress + ":" +
                             std::to_string(config_.tcpPort));
        }

        isOpen_ = true;
    }

    size_t write(const std::string& data) override {
        if (!isOpen_) {
            throw G30Exception("TCP port is not open");
        }

#ifdef _WIN32
        int result = send(sockfd_, data.c_str(), static_cast<int>(data.length()), 0);
#else
        ssize_t result = send(sockfd_, data.c_str(), data.length(), 0);
#endif

        if (result < 0) {
            throw G30Exception("Failed to send data over TCP");
        }

        return static_cast<size_t>(result);
    }

    std::string read(int timeout_ms = 1000) override {
        if (!isOpen_) {
            throw G30Exception("TCP port is not open");
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

            // Use poll/select to check for data
#ifdef _WIN32
            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(sockfd_, &readfds);

            struct timeval tv;
            tv.tv_sec = 0;
            tv.tv_usec = 10000; // 10ms

            int selectResult = select(0, &readfds, nullptr, nullptr, &tv);
            if (selectResult > 0) {
                int bytesRead = recv(sockfd_, buffer, sizeof(buffer) - 1, 0);
#else
            struct pollfd pfd;
            pfd.fd = sockfd_;
            pfd.events = POLLIN;

            int pollResult = poll(&pfd, 1, 10); // 10ms timeout
            if (pollResult > 0 && (pfd.revents & POLLIN)) {
                ssize_t bytesRead = recv(sockfd_, buffer, sizeof(buffer) - 1, 0);
#endif
                if (bytesRead > 0) {
                    buffer[bytesRead] = '\0';
                    result += buffer;
                    if (result.find('\n') != std::string::npos) {
                        break;
                    }
                } else if (bytesRead == 0) {
                    // Connection closed
                    throw G30Exception("TCP connection closed by remote host");
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(5));
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

        if (sockfd_ >= 0) {
            closesocket(sockfd_);
            sockfd_ = -1;
        }
        isOpen_ = false;
    }

private:
    G30Config config_;
    bool isOpen_;
    int sockfd_;

    void closesocket(int sock) {
#ifdef _WIN32
        ::closesocket(sock);
#else
        ::close(sock);
#endif
    }
};

// ==================== TDKLambdaG30 Implementation ====================

TDKLambdaG30::TDKLambdaG30(const G30Config& config)
    : config_(config),
      connected_(false),
      outputEnabled_(false),
      maxVoltage_(30.0),
      maxCurrent_(56.0),
      errorHandler_(nullptr) {

    // Create TCP/IP communication port
    commPort_ = std::make_unique<TcpPort>(config_);
}

TDKLambdaG30::TDKLambdaG30(std::unique_ptr<ICommunication> commPort, const G30Config& config)
    : commPort_(std::move(commPort)),
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
    : commPort_(std::move(other.commPort_)),
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
        commPort_ = std::move(other.commPort_);
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
        // Open the TCP port
        auto* tcpPort = dynamic_cast<TcpPort*>(commPort_.get());
        if (tcpPort) {
            tcpPort->open();
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
    if (commPort_) {
        commPort_->close();
    }
    connected_ = false;
}

bool TDKLambdaG30::isConnected() const {
    return connected_ && commPort_ && commPort_->isOpen();
}

void TDKLambdaG30::enableOutput(bool enable) {
    if (!isConnected()) {
        throw G30Exception("Not connected to device");
    }

    std::string command = enable ? "OUTP ON\n" : "OUTP OFF\n";
    commPort_->write(command);
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

    commPort_->write("*RST\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    outputEnabled_ = false;
}

void TDKLambdaG30::setVoltage(double voltage, int channel) {
    // G30 is single-channel, ignore channel parameter
    (void)channel;

    validateVoltage(voltage);

    if (!isConnected()) {
        throw G30Exception("Not connected to device");
    }

    std::ostringstream oss;
    oss.precision(3);
    oss << std::fixed << "VOLT " << voltage << "\n";

    commPort_->write(oss.str());
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

double TDKLambdaG30::getVoltage(int channel) const {
    // G30 is single-channel, ignore channel parameter
    (void)channel;

    if (!isConnected()) {
        throw G30Exception("Not connected to device");
    }

    std::string response = sendQuery("VOLT?");
    return parseNumericResponse(response);
}

double TDKLambdaG30::measureVoltage(int channel) const {
    // G30 is single-channel, ignore channel parameter
    (void)channel;

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

void TDKLambdaG30::setCurrent(double current, int channel) {
    // G30 is single-channel, ignore channel parameter
    (void)channel;

    validateCurrent(current);

    if (!isConnected()) {
        throw G30Exception("Not connected to device");
    }

    std::ostringstream oss;
    oss.precision(3);
    oss << std::fixed << "CURR " << current << "\n";

    commPort_->write(oss.str());
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

double TDKLambdaG30::getCurrent(int channel) const {
    // G30 is single-channel, ignore channel parameter
    (void)channel;

    if (!isConnected()) {
        throw G30Exception("Not connected to device");
    }

    std::string response = sendQuery("CURR?");
    return parseNumericResponse(response);
}

double TDKLambdaG30::measureCurrent(int channel) const {
    // G30 is single-channel, ignore channel parameter
    (void)channel;

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

double TDKLambdaG30::measurePower(int channel) const {
    // G30 is single-channel, ignore channel parameter
    (void)channel;

    if (!isConnected()) {
        throw G30Exception("Not connected to device");
    }

    double voltage = measureVoltage();
    double current = measureCurrent();
    return voltage * current;
}

void TDKLambdaG30::setOverVoltageProtection(double voltage, int channel) {
    // G30 is single-channel, ignore channel parameter
    (void)channel;

    if (!isConnected()) {
        throw G30Exception("Not connected to device");
    }

    std::ostringstream oss;
    oss.precision(3);
    oss << std::fixed << "VOLT:PROT " << voltage << "\n";

    commPort_->write(oss.str());
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

    commPort_->write("*CLS\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

std::string TDKLambdaG30::getIdentification() const {
    if (!isConnected() && !commPort_->isOpen()) {
        throw G30Exception("Not connected to device");
    }

    return sendQuery("*IDN?");
}

PowerSupplyStatus TDKLambdaG30::getStatus(int channel) const {
    // G30 is single-channel, ignore channel parameter
    (void)channel;

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

PowerSupplyCapabilities TDKLambdaG30::getCapabilities() const {
    PowerSupplyCapabilities caps;
    caps.maxVoltage = maxVoltage_;
    caps.maxCurrent = maxCurrent_;
    caps.maxPower = maxVoltage_ * maxCurrent_;
    caps.numberOfChannels = 1;  // G30 is single channel
    caps.supportsRemoteSensing = false;  // G30 doesn't support remote sensing
    caps.supportsOVP = true;
    caps.supportsOCP = true;
    caps.supportsOPP = false;
    caps.supportsSequencing = false;
    return caps;
}

Vendor TDKLambdaG30::getVendor() const {
    return Vendor::TDK_LAMBDA;
}

std::string TDKLambdaG30::getModel() const {
    return "G30";
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

    commPort_->write(cmd);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    return "OK";
}

std::string TDKLambdaG30::sendQuery(const std::string& query) const {
    if (!isConnected() && !commPort_->isOpen()) {
        throw G30Exception("Not connected to device");
    }

    std::string cmd = query;
    if (cmd.back() != '\n') {
        cmd += '\n';
    }

    commPort_->write(cmd);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    std::string response = commPort_->read(config_.timeout_ms);
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

// ==================== Factory Functions ====================

std::unique_ptr<TDKLambdaG30> createG30Ethernet(const std::string& ipAddress, int tcpPort) {
    G30Config config;
    config.ipAddress = ipAddress;
    config.tcpPort = tcpPort;
    return std::make_unique<TDKLambdaG30>(config);
}

} // namespace TDKLambda
