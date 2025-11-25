/**
 * @file tdk_lambda_g30.h
 * @brief TDK Lambda G30 Power Supply Controller
 * @version 1.0.0
 * @date 2025-11-24
 *
 * Professional C++ interface for controlling TDK Lambda G30 series power supplies.
 * Supports voltage and current control via Ethernet (TCP/IP) communication using SCPI commands.
 *
 * @author Professional Power Supply Control Library
 * @copyright MIT License
 */

#ifndef TDK_LAMBDA_G30_H
#define TDK_LAMBDA_G30_H

#include "power_supply_interface.h"
#include <string>
#include <memory>
#include <stdexcept>
#include <functional>
#include <vector>

namespace TDKLambda {

// Import types from generic PowerSupply namespace
using PowerSupply::PowerSupplyStatus;
using PowerSupply::PowerSupplyCapabilities;
using PowerSupply::Vendor;

// Type alias for backward compatibility
using ConnectionType = PowerSupply::ConnectionType;

/**
 * @brief Custom exception class for TDK Lambda G30 errors
 */
class G30Exception : public std::runtime_error {
public:
    explicit G30Exception(const std::string& message)
        : std::runtime_error(message) {}
};

/**
 * @brief Communication interface for abstraction
 *
 * This allows for dependency injection and easier testing.
 * Supports Ethernet (TCP/IP) communication.
 */
class ICommunication {
public:
    virtual ~ICommunication() = default;

    /**
     * @brief Write data to communication port
     * @param data Data to write
     * @return Number of bytes written
     */
    virtual size_t write(const std::string& data) = 0;

    /**
     * @brief Read data from communication port
     * @param timeout_ms Timeout in milliseconds
     * @return Read data
     */
    virtual std::string read(int timeout_ms = 1000) = 0;

    /**
     * @brief Check if port is open
     * @return true if open, false otherwise
     */
    virtual bool isOpen() const = 0;

    /**
     * @brief Close the port
     */
    virtual void close() = 0;
};

/**
 * @brief Configuration structure for TDK Lambda G30
 */
struct G30Config {
    // Ethernet settings
    std::string ipAddress;      ///< IP address (e.g., "192.168.1.100")
    int tcpPort;                ///< TCP port (default: 8003 for TDK Lambda G30)

    // Common settings
    int timeout_ms;             ///< Communication timeout in milliseconds

    G30Config()
        : ipAddress(""),
          tcpPort(8003),  // TDK Lambda G30 default TCP port
          timeout_ms(1000) {}
};

/**
 * @brief Main controller class for TDK Lambda G30 Power Supply
 *
 * This class provides a clean, modern C++ interface for controlling
 * TDK Lambda G30 series programmable power supplies via Ethernet.
 *
 * Implements the generic IPowerSupply interface while providing
 * TDK Lambda G30 specific features and optimizations.
 *
 * Features:
 * - RAII-compliant resource management
 * - Exception-based error handling
 * - Ethernet (TCP/IP) communication
 * - Thread-safe operations (when used with proper locking)
 * - Full SCPI command support
 * - Generic PowerSupply interface compliance
 *
 * Example usage:
 * @code
 * auto psu = createG30Ethernet("192.168.1.100", 8003);
 * psu->connect();
 *
 * TDKLambdaG30 psu(config);
 * psu.connect();
 * psu.setVoltage(12.5);
 * psu.setCurrent(2.0);
 * psu.enableOutput(true);
 * @endcode
 */
class TDKLambdaG30 : public PowerSupply::IPowerSupply {
public:
    /**
     * @brief Construct a new TDKLambdaG30 object
     * @param config Configuration parameters
     */
    explicit TDKLambdaG30(const G30Config& config);

    /**
     * @brief Construct with custom communication implementation
     * @param commPort Custom communication port implementation
     * @param config Configuration parameters
     */
    TDKLambdaG30(std::unique_ptr<ICommunication> commPort, const G30Config& config);

    /**
     * @brief Destructor - ensures proper cleanup
     */
    ~TDKLambdaG30();

    // Delete copy operations (non-copyable due to unique resource)
    TDKLambdaG30(const TDKLambdaG30&) = delete;
    TDKLambdaG30& operator=(const TDKLambdaG30&) = delete;

    // Allow move operations
    TDKLambdaG30(TDKLambdaG30&&) noexcept;
    TDKLambdaG30& operator=(TDKLambdaG30&&) noexcept;

    /**
     * @brief Connect to the power supply
     * @throws G30Exception if connection fails
     */
    void connect() override;

    /**
     * @brief Disconnect from the power supply
     */
    void disconnect() override;

    /**
     * @brief Check if connected
     * @return true if connected, false otherwise
     */
    bool isConnected() const override;

    // ==================== Basic Control ====================

    /**
     * @brief Enable or disable output
     * @param enable true to enable, false to disable
     * @throws G30Exception on communication error
     */
    void enableOutput(bool enable) override;

    /**
     * @brief Get output state
     * @return true if enabled, false if disabled
     * @throws G30Exception on communication error
     */
    bool isOutputEnabled() const override;

    /**
     * @brief Reset the power supply to default state
     * @throws G30Exception on communication error
     */
    void reset() override;

    // ==================== Voltage Control ====================

    /**
     * @brief Set output voltage
     * @param voltage Voltage in volts
     * @param channel Channel number (ignored for single-channel G30, default: 1)
     * @throws G30Exception if voltage is out of range or communication fails
     */
    void setVoltage(double voltage, int channel = 1) override;

    /**
     * @brief Get set voltage value
     * @param channel Channel number (ignored for single-channel G30, default: 1)
     * @return Voltage in volts
     * @throws G30Exception on communication error
     */
    double getVoltage(int channel = 1) const override;

    /**
     * @brief Measure actual output voltage
     * @param channel Channel number (ignored for single-channel G30, default: 1)
     * @return Measured voltage in volts
     * @throws G30Exception on communication error
     */
    double measureVoltage(int channel = 1) const override;

    /**
     * @brief Set voltage with ramp rate
     * @param voltage Target voltage in volts
     * @param rampRate Ramp rate in V/s
     * @throws G30Exception on error
     */
    void setVoltageWithRamp(double voltage, double rampRate);

    // ==================== Current Control ====================

    /**
     * @brief Set current limit
     * @param current Current in amperes
     * @param channel Channel number (ignored for single-channel G30, default: 1)
     * @throws G30Exception if current is out of range or communication fails
     */
    void setCurrent(double current, int channel = 1) override;

    /**
     * @brief Get set current limit
     * @param channel Channel number (ignored for single-channel G30, default: 1)
     * @return Current in amperes
     * @throws G30Exception on communication error
     */
    double getCurrent(int channel = 1) const override;

    /**
     * @brief Measure actual output current
     * @param channel Channel number (ignored for single-channel G30, default: 1)
     * @return Measured current in amperes
     * @throws G30Exception on communication error
     */
    double measureCurrent(int channel = 1) const override;

    /**
     * @brief Set current with ramp rate
     * @param current Target current in amperes
     * @param rampRate Ramp rate in A/s
     * @throws G30Exception on error
     */
    void setCurrentWithRamp(double current, double rampRate);

    // ==================== Power and Limits ====================

    /**
     * @brief Measure output power
     * @param channel Channel number (ignored for single-channel G30, default: 1)
     * @return Power in watts
     * @throws G30Exception on communication error
     */
    double measurePower(int channel = 1) const override;

    /**
     * @brief Set over-voltage protection level
     * @param voltage OVP level in volts
     * @param channel Channel number (ignored for single-channel G30, default: 1)
     * @throws G30Exception on error
     */
    void setOverVoltageProtection(double voltage, int channel = 1) override;

    /**
     * @brief Get over-voltage protection level
     * @return OVP level in volts
     * @throws G30Exception on error
     */
    double getOverVoltageProtection() const;

    /**
     * @brief Clear protection faults
     * @throws G30Exception on error
     */
    void clearProtection() override;

    // ==================== Status and Information ====================

    /**
     * @brief Get device identification string
     * @return ID string (manufacturer, model, serial, firmware)
     * @throws G30Exception on communication error
     */
    std::string getIdentification() const override;

    /**
     * @brief Get detailed status
     * @param channel Channel number (ignored for single-channel G30, default: 1)
     * @return PowerSupplyStatus structure
     * @throws G30Exception on communication error
     */
    PowerSupplyStatus getStatus(int channel = 1) const override;

    /**
     * @brief Get power supply capabilities
     * @return PowerSupplyCapabilities structure
     */
    PowerSupplyCapabilities getCapabilities() const override;

    /**
     * @brief Get vendor
     * @return Vendor enumeration
     */
    Vendor getVendor() const override;

    /**
     * @brief Get model name
     * @return Model string
     */
    std::string getModel() const override;

    /**
     * @brief Check for errors in error queue
     * @return Error message, empty if no errors
     * @throws G30Exception on communication error
     */
    std::string checkError() const;

    /**
     * @brief Get maximum voltage rating
     * @return Maximum voltage in volts
     */
    double getMaxVoltage() const { return maxVoltage_; }

    /**
     * @brief Get maximum current rating
     * @return Maximum current in amperes
     */
    double getMaxCurrent() const { return maxCurrent_; }

    /**
     * @brief Set maximum voltage limit (safety feature)
     * @param maxVoltage Maximum allowed voltage
     */
    void setMaxVoltage(double maxVoltage);

    /**
     * @brief Set maximum current limit (safety feature)
     * @param maxCurrent Maximum allowed current
     */
    void setMaxCurrent(double maxCurrent);

    // ==================== Advanced Features ====================

    /**
     * @brief Send raw SCPI command
     * @param command SCPI command string
     * @return Response from device
     * @throws G30Exception on communication error
     */
    std::string sendCommand(const std::string& command) override;

    /**
     * @brief Send raw SCPI query
     * @param query SCPI query string
     * @return Response from device
     * @throws G30Exception on communication error
     */
    std::string sendQuery(const std::string& query) const override;

    /**
     * @brief Set custom error handler callback
     * @param handler Error handler function
     */
    void setErrorHandler(std::function<void(const std::string&)> handler);

private:
    std::unique_ptr<ICommunication> commPort_;
    G30Config config_;
    bool connected_;
    mutable bool outputEnabled_;

    // Safety limits
    double maxVoltage_;
    double maxCurrent_;

    // Error handling
    std::function<void(const std::string&)> errorHandler_;

    /**
     * @brief Validate voltage is within limits
     * @param voltage Voltage to validate
     * @throws G30Exception if out of range
     */
    void validateVoltage(double voltage) const;

    /**
     * @brief Validate current is within limits
     * @param current Current to validate
     * @throws G30Exception if out of range
     */
    void validateCurrent(double current) const;

    /**
     * @brief Parse numeric response from device
     * @param response Response string
     * @return Parsed numeric value
     * @throws G30Exception if parsing fails
     */
    double parseNumericResponse(const std::string& response) const;

    /**
     * @brief Trim whitespace from string
     * @param str String to trim
     * @return Trimmed string
     */
    std::string trim(const std::string& str) const;

    /**
     * @brief Default error handler
     * @param error Error message
     */
    void defaultErrorHandler(const std::string& error);
};

// ==================== Factory Functions ====================

/**
 * @brief Factory function to create TDKLambdaG30 instance with Ethernet
 * @param ipAddress IP address (e.g., "192.168.1.100")
 * @param tcpPort TCP port (default: 8003 for TDK Lambda G30)
 * @return Unique pointer to TDKLambdaG30 instance
 */
std::unique_ptr<TDKLambdaG30> createG30Ethernet(const std::string& ipAddress, int tcpPort = 8003);

} // namespace TDKLambda

#endif // TDK_LAMBDA_G30_H
