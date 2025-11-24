/**
 * @file tdk_lambda_g30.h
 * @brief TDK Lambda G30 Power Supply Controller
 * @version 1.0.0
 * @date 2025-11-24
 *
 * Professional C++ interface for controlling TDK Lambda G30 series power supplies.
 * Supports voltage and current control via serial communication using SCPI commands.
 *
 * @author Professional Power Supply Control Library
 * @copyright MIT License
 */

#ifndef TDK_LAMBDA_G30_H
#define TDK_LAMBDA_G30_H

#include <string>
#include <memory>
#include <stdexcept>
#include <functional>
#include <vector>

namespace TDKLambda {

/**
 * @brief Custom exception class for TDK Lambda G30 errors
 */
class G30Exception : public std::runtime_error {
public:
    explicit G30Exception(const std::string& message)
        : std::runtime_error(message) {}
};

/**
 * @brief Communication interface for serial port abstraction
 *
 * This allows for dependency injection and easier testing
 */
class ISerialPort {
public:
    virtual ~ISerialPort() = default;

    /**
     * @brief Write data to serial port
     * @param data Data to write
     * @return Number of bytes written
     */
    virtual size_t write(const std::string& data) = 0;

    /**
     * @brief Read data from serial port
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
    std::string port;           ///< Serial port (e.g., "/dev/ttyUSB0" or "COM3")
    int baudRate;               ///< Baud rate (default: 9600)
    int dataBits;               ///< Data bits (default: 8)
    int stopBits;               ///< Stop bits (default: 1)
    char parity;                ///< Parity ('N': none, 'E': even, 'O': odd)
    int timeout_ms;             ///< Communication timeout in milliseconds

    G30Config()
        : port(""),
          baudRate(9600),
          dataBits(8),
          stopBits(1),
          parity('N'),
          timeout_ms(1000) {}
};

/**
 * @brief Power supply status information
 */
struct PowerSupplyStatus {
    bool outputEnabled;         ///< Output state
    bool overVoltageProtection; ///< OVP triggered
    bool overCurrentProtection; ///< OCP triggered
    bool overTemperature;       ///< Over temperature
    bool remoteSensing;         ///< Remote sensing enabled

    PowerSupplyStatus()
        : outputEnabled(false),
          overVoltageProtection(false),
          overCurrentProtection(false),
          overTemperature(false),
          remoteSensing(false) {}
};

/**
 * @brief Main controller class for TDK Lambda G30 Power Supply
 *
 * This class provides a clean, modern C++ interface for controlling
 * TDK Lambda G30 series programmable power supplies.
 *
 * Features:
 * - RAII-compliant resource management
 * - Exception-based error handling
 * - Dependency injection for serial communication
 * - Thread-safe operations (when used with proper locking)
 * - Full SCPI command support
 *
 * Example usage:
 * @code
 * G30Config config;
 * config.port = "/dev/ttyUSB0";
 * config.baudRate = 9600;
 *
 * TDKLambdaG30 psu(config);
 * psu.connect();
 * psu.setVoltage(12.5);
 * psu.setCurrent(2.0);
 * psu.enableOutput(true);
 * @endcode
 */
class TDKLambdaG30 {
public:
    /**
     * @brief Construct a new TDKLambdaG30 object
     * @param config Configuration parameters
     */
    explicit TDKLambdaG30(const G30Config& config);

    /**
     * @brief Construct with custom serial port implementation
     * @param serialPort Custom serial port implementation
     * @param config Configuration parameters
     */
    TDKLambdaG30(std::unique_ptr<ISerialPort> serialPort, const G30Config& config);

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
    void connect();

    /**
     * @brief Disconnect from the power supply
     */
    void disconnect();

    /**
     * @brief Check if connected
     * @return true if connected, false otherwise
     */
    bool isConnected() const;

    // ==================== Basic Control ====================

    /**
     * @brief Enable or disable output
     * @param enable true to enable, false to disable
     * @throws G30Exception on communication error
     */
    void enableOutput(bool enable);

    /**
     * @brief Get output state
     * @return true if enabled, false if disabled
     * @throws G30Exception on communication error
     */
    bool isOutputEnabled() const;

    /**
     * @brief Reset the power supply to default state
     * @throws G30Exception on communication error
     */
    void reset();

    // ==================== Voltage Control ====================

    /**
     * @brief Set output voltage
     * @param voltage Voltage in volts
     * @throws G30Exception if voltage is out of range or communication fails
     */
    void setVoltage(double voltage);

    /**
     * @brief Get set voltage value
     * @return Voltage in volts
     * @throws G30Exception on communication error
     */
    double getVoltage() const;

    /**
     * @brief Measure actual output voltage
     * @return Measured voltage in volts
     * @throws G30Exception on communication error
     */
    double measureVoltage() const;

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
     * @throws G30Exception if current is out of range or communication fails
     */
    void setCurrent(double current);

    /**
     * @brief Get set current limit
     * @return Current in amperes
     * @throws G30Exception on communication error
     */
    double getCurrent() const;

    /**
     * @brief Measure actual output current
     * @return Measured current in amperes
     * @throws G30Exception on communication error
     */
    double measureCurrent() const;

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
     * @return Power in watts
     * @throws G30Exception on communication error
     */
    double measurePower() const;

    /**
     * @brief Set over-voltage protection level
     * @param voltage OVP level in volts
     * @throws G30Exception on error
     */
    void setOverVoltageProtection(double voltage);

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
    void clearProtection();

    // ==================== Status and Information ====================

    /**
     * @brief Get device identification string
     * @return ID string (manufacturer, model, serial, firmware)
     * @throws G30Exception on communication error
     */
    std::string getIdentification() const;

    /**
     * @brief Get detailed status
     * @return PowerSupplyStatus structure
     * @throws G30Exception on communication error
     */
    PowerSupplyStatus getStatus() const;

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
    std::string sendCommand(const std::string& command);

    /**
     * @brief Send raw SCPI query
     * @param query SCPI query string
     * @return Response from device
     * @throws G30Exception on communication error
     */
    std::string sendQuery(const std::string& query) const;

    /**
     * @brief Set custom error handler callback
     * @param handler Error handler function
     */
    void setErrorHandler(std::function<void(const std::string&)> handler);

private:
    std::unique_ptr<ISerialPort> serialPort_;
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

/**
 * @brief Factory function to create TDKLambdaG30 instance
 * @param port Serial port name
 * @param baudRate Baud rate (default: 9600)
 * @return Unique pointer to TDKLambdaG30 instance
 */
std::unique_ptr<TDKLambdaG30> createG30(const std::string& port, int baudRate = 9600);

} // namespace TDKLambda

#endif // TDK_LAMBDA_G30_H
