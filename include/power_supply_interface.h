/**
 * @file power_supply_interface.h
 * @brief Generic Power Supply Interface
 * @version 1.0.0
 * @date 2025-11-24
 *
 * Generic interface for programmable power supplies.
 * Supports multiple vendors/models through common abstraction.
 *
 * @author Professional Power Supply Control Library
 * @copyright MIT License
 */

#ifndef POWER_SUPPLY_INTERFACE_H
#define POWER_SUPPLY_INTERFACE_H

#include <string>
#include <memory>
#include <stdexcept>
#include <functional>

namespace PowerSupply {

/**
 * @brief Power supply vendor enumeration
 */
enum class Vendor {
    TDK_LAMBDA,     ///< TDK Lambda
    KEYSIGHT,       ///< Keysight / Agilent
    ROHDE_SCHWARZ,  ///< Rohde & Schwarz
    RIGOL,          ///< Rigol
    SIGLENT,        ///< Siglent
    TTI,            ///< Thurlby Thandar Instruments
    BK_PRECISION,   ///< B&K Precision
    TENMA,          ///< Tenma
    CUSTOM          ///< Custom/Other vendors
};

/**
 * @brief Connection type enumeration
 */
enum class ConnectionType {
    SERIAL,      ///< Serial port connection (RS232/USB)
    ETHERNET,    ///< Ethernet TCP/IP connection
    USB,         ///< Direct USB (USBTMC)
    GPIB         ///< GPIB/IEEE-488
};

/**
 * @brief Power supply status information
 */
struct PowerSupplyStatus {
    bool outputEnabled;         ///< Output state
    bool overVoltageProtection; ///< OVP triggered
    bool overCurrentProtection; ///< OCP triggered
    bool overPowerProtection;   ///< OPP triggered
    bool overTemperature;       ///< Over temperature
    bool remoteSensing;         ///< Remote sensing enabled
    bool ccMode;                ///< Constant current mode
    bool cvMode;                ///< Constant voltage mode

    PowerSupplyStatus()
        : outputEnabled(false),
          overVoltageProtection(false),
          overCurrentProtection(false),
          overPowerProtection(false),
          overTemperature(false),
          remoteSensing(false),
          ccMode(false),
          cvMode(false) {}
};

/**
 * @brief Power supply capabilities
 */
struct PowerSupplyCapabilities {
    double maxVoltage;          ///< Maximum output voltage
    double maxCurrent;          ///< Maximum output current
    double maxPower;            ///< Maximum output power
    int numberOfChannels;       ///< Number of output channels
    bool supportsRemoteSensing; ///< Supports remote sensing
    bool supportsOVP;           ///< Supports OVP
    bool supportsOCP;           ///< Supports OCP
    bool supportsOPP;           ///< Supports OPP
    bool supportsSequencing;    ///< Supports voltage/current sequencing

    PowerSupplyCapabilities()
        : maxVoltage(0),
          maxCurrent(0),
          maxPower(0),
          numberOfChannels(1),
          supportsRemoteSensing(false),
          supportsOVP(false),
          supportsOCP(false),
          supportsOPP(false),
          supportsSequencing(false) {}
};

/**
 * @brief Generic Power Supply Interface
 *
 * Abstract base class for all programmable power supplies.
 * Defines common operations that all power supplies should support.
 */
class IPowerSupply {
public:
    virtual ~IPowerSupply() = default;

    // ==================== Connection Management ====================

    /**
     * @brief Connect to the power supply
     * @throws std::runtime_error if connection fails
     */
    virtual void connect() = 0;

    /**
     * @brief Disconnect from the power supply
     */
    virtual void disconnect() = 0;

    /**
     * @brief Check if connected
     * @return true if connected, false otherwise
     */
    virtual bool isConnected() const = 0;

    // ==================== Basic Control ====================

    /**
     * @brief Enable or disable output
     * @param enable true to enable, false to disable
     * @throws std::runtime_error on error
     */
    virtual void enableOutput(bool enable) = 0;

    /**
     * @brief Get output state
     * @return true if enabled, false if disabled
     * @throws std::runtime_error on error
     */
    virtual bool isOutputEnabled() const = 0;

    /**
     * @brief Reset the power supply to default state
     * @throws std::runtime_error on error
     */
    virtual void reset() = 0;

    // ==================== Voltage Control ====================

    /**
     * @brief Set output voltage
     * @param voltage Voltage in volts
     * @param channel Channel number (default: 1)
     * @throws std::runtime_error if out of range or communication fails
     */
    virtual void setVoltage(double voltage, int channel = 1) = 0;

    /**
     * @brief Get set voltage value
     * @param channel Channel number (default: 1)
     * @return Voltage in volts
     * @throws std::runtime_error on error
     */
    virtual double getVoltage(int channel = 1) const = 0;

    /**
     * @brief Measure actual output voltage
     * @param channel Channel number (default: 1)
     * @return Measured voltage in volts
     * @throws std::runtime_error on error
     */
    virtual double measureVoltage(int channel = 1) const = 0;

    // ==================== Current Control ====================

    /**
     * @brief Set current limit
     * @param current Current in amperes
     * @param channel Channel number (default: 1)
     * @throws std::runtime_error if out of range or communication fails
     */
    virtual void setCurrent(double current, int channel = 1) = 0;

    /**
     * @brief Get set current limit
     * @param channel Channel number (default: 1)
     * @return Current in amperes
     * @throws std::runtime_error on error
     */
    virtual double getCurrent(int channel = 1) const = 0;

    /**
     * @brief Measure actual output current
     * @param channel Channel number (default: 1)
     * @return Measured current in amperes
     * @throws std::runtime_error on error
     */
    virtual double measureCurrent(int channel = 1) const = 0;

    // ==================== Power Measurement ====================

    /**
     * @brief Measure output power
     * @param channel Channel number (default: 1)
     * @return Power in watts
     * @throws std::runtime_error on error
     */
    virtual double measurePower(int channel = 1) const = 0;

    // ==================== Status and Information ====================

    /**
     * @brief Get device identification string
     * @return ID string (vendor, model, serial, firmware)
     * @throws std::runtime_error on error
     */
    virtual std::string getIdentification() const = 0;

    /**
     * @brief Get detailed status
     * @param channel Channel number (default: 1)
     * @return PowerSupplyStatus structure
     * @throws std::runtime_error on error
     */
    virtual PowerSupplyStatus getStatus(int channel = 1) const = 0;

    /**
     * @brief Get power supply capabilities
     * @return PowerSupplyCapabilities structure
     */
    virtual PowerSupplyCapabilities getCapabilities() const = 0;

    /**
     * @brief Get vendor
     * @return Vendor enumeration
     */
    virtual Vendor getVendor() const = 0;

    /**
     * @brief Get model name
     * @return Model string
     */
    virtual std::string getModel() const = 0;

    // ==================== Optional Advanced Features ====================

    /**
     * @brief Set over-voltage protection level (if supported)
     * @param voltage OVP level in volts
     * @param channel Channel number (default: 1)
     * @throws std::runtime_error if not supported or on error
     */
    virtual void setOverVoltageProtection(double voltage, int channel = 1) {
        (void)voltage; (void)channel;
        throw std::runtime_error("OVP not supported by this power supply");
    }

    /**
     * @brief Clear protection faults (if supported)
     * @throws std::runtime_error if not supported or on error
     */
    virtual void clearProtection() {
        throw std::runtime_error("Protection clear not supported by this power supply");
    }

    /**
     * @brief Send raw command (vendor-specific)
     * @param command Command string
     * @return Response (may be empty for commands)
     * @throws std::runtime_error on error
     */
    virtual std::string sendCommand(const std::string& command) = 0;

    /**
     * @brief Send raw query (vendor-specific)
     * @param query Query string
     * @return Response from device
     * @throws std::runtime_error on error
     */
    virtual std::string sendQuery(const std::string& query) const = 0;
};

/**
 * @brief Power Supply Factory
 *
 * Factory class for creating power supply instances based on vendor/model
 */
class PowerSupplyFactory {
public:
    /**
     * @brief Create a power supply instance
     * @param vendor Vendor enumeration
     * @param model Model string
     * @param connectionType Connection type
     * @param connectionString Connection string (port, IP, etc.)
     * @return Unique pointer to IPowerSupply instance
     * @throws std::runtime_error if vendor/model not supported
     */
    static std::unique_ptr<IPowerSupply> create(
        Vendor vendor,
        const std::string& model,
        ConnectionType connectionType,
        const std::string& connectionString
    );

    /**
     * @brief Create a power supply instance from identification string
     * @param idnString *IDN? response string
     * @param connectionType Connection type
     * @param connectionString Connection string
     * @return Unique pointer to IPowerSupply instance
     * @throws std::runtime_error if cannot parse or vendor not supported
     */
    static std::unique_ptr<IPowerSupply> createFromIDN(
        const std::string& idnString,
        ConnectionType connectionType,
        const std::string& connectionString
    );
};

} // namespace PowerSupply

#endif // POWER_SUPPLY_INTERFACE_H
