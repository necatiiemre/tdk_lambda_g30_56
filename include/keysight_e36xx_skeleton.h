/**
 * @file keysight_e36xx_skeleton.h
 * @brief Keysight E36xx Power Supply Skeleton (Example for adding new vendors)
 * @version 1.0.0
 * @date 2025-11-24
 *
 * This is a SKELETON/TEMPLATE showing how to add support for other power supplies.
 * Implement all pure virtual methods from IPowerSupply interface.
 *
 * @author Professional Power Supply Control Library
 * @copyright MIT License
 */

#ifndef KEYSIGHT_E36XX_SKELETON_H
#define KEYSIGHT_E36XX_SKELETON_H

#include "power_supply_interface.h"
#include <string>
#include <memory>

namespace Keysight {

/**
 * @brief Keysight E36xx Series Power Supply Controller (SKELETON/EXAMPLE)
 *
 * This is a skeleton implementation showing how to add support for
 * Keysight E3631A, E3632A, E3633A, E3634A power supplies.
 *
 * Steps to implement:
 * 1. Implement all pure virtual methods from IPowerSupply
 * 2. Add vendor-specific SCPI commands
 * 3. Handle multi-channel if supported
 * 4. Add to factory pattern
 *
 * Example for developers adding new PSU support.
 */
class KeysightE36xx : public PowerSupply::IPowerSupply {
public:
    /**
     * @brief Construct Keysight E36xx controller
     * @param model Model string (e.g., "E3631A")
     * @param connectionType Connection type (SERIAL, ETHERNET, USB, GPIB)
     * @param connectionString Connection string (port, IP, etc.)
     */
    KeysightE36xx(
        const std::string& model,
        PowerSupply::ConnectionType connectionType,
        const std::string& connectionString
    );

    ~KeysightE36xx() override = default;

    // ==================== IPowerSupply Interface Implementation ====================

    // Connection Management
    void connect() override;
    void disconnect() override;
    bool isConnected() const override;

    // Basic Control
    void enableOutput(bool enable) override;
    bool isOutputEnabled() const override;
    void reset() override;

    // Voltage Control
    void setVoltage(double voltage, int channel = 1) override;
    double getVoltage(int channel = 1) const override;
    double measureVoltage(int channel = 1) const override;

    // Current Control
    void setCurrent(double current, int channel = 1) override;
    double getCurrent(int channel = 1) const override;
    double measureCurrent(int channel = 1) const override;

    // Power Measurement
    double measurePower(int channel = 1) const override;

    // Status and Information
    std::string getIdentification() const override;
    PowerSupply::PowerSupplyStatus getStatus(int channel = 1) const override;
    PowerSupply::PowerSupplyCapabilities getCapabilities() const override;
    PowerSupply::Vendor getVendor() const override { return PowerSupply::Vendor::KEYSIGHT; }
    std::string getModel() const override { return model_; }

    // Advanced Features (optional - implement if supported)
    void setOverVoltageProtection(double voltage, int channel = 1) override;
    void clearProtection() override;

    // Raw Commands
    std::string sendCommand(const std::string& command) override;
    std::string sendQuery(const std::string& query) const override;

    // ==================== Keysight-Specific Methods ====================

    /**
     * @brief Set tracking mode (for multi-output models)
     * @param mode Tracking mode ("INDEP", "TRACK", "SYNC")
     */
    void setTrackingMode(const std::string& mode);

    /**
     * @brief Enable remote sensing (if supported)
     * @param enable true to enable, false to disable
     * @param channel Channel number
     */
    void enableRemoteSensing(bool enable, int channel = 1);

private:
    std::string model_;
    PowerSupply::ConnectionType connectionType_;
    std::string connectionString_;
    bool connected_;
    PowerSupply::PowerSupplyCapabilities capabilities_;

    // TODO: Add communication port member
    // std::unique_ptr<ICommunication> commPort_;

    /**
     * @brief Initialize capabilities based on model
     */
    void initializeCapabilities();

    /**
     * @brief Validate channel number
     * @param channel Channel number to validate
     * @throws std::runtime_error if channel is invalid
     */
    void validateChannel(int channel) const;
};

/**
 * @brief Factory function for Keysight E36xx
 * @param model Model string (e.g., "E3631A")
 * @param connectionType Connection type
 * @param connectionString Connection string
 * @return Unique pointer to Keysight E36xx instance
 */
inline std::unique_ptr<KeysightE36xx> createKeysightE36xx(
    const std::string& model,
    PowerSupply::ConnectionType connectionType,
    const std::string& connectionString
) {
    return std::make_unique<KeysightE36xx>(model, connectionType, connectionString);
}

} // namespace Keysight

/*
 * IMPLEMENTATION NOTES FOR DEVELOPERS:
 *
 * 1. Communication Layer:
 *    - Reuse ICommunication interface from TDK Lambda
 *    - Implement USBTMC support if needed
 *    - Handle GPIB via appropriate library
 *
 * 2. Multi-Channel Support:
 *    - E3631A has 3 outputs: +6V/5A, +25V/1A, -25V/1A
 *    - Use channel parameter to select output
 *    - Validate channel range in each method
 *
 * 3. SCPI Commands (Keysight-specific):
 *    - Output control: "OUTP ON|OFF, (@<channel>)"
 *    - Voltage: "VOLT <value>, (@<channel>)"
 *    - Current: "CURR <value>, (@<channel>)"
 *    - Measure: "MEAS:VOLT? (@<channel>)"
 *    - Tracking: "OUTP:TRAC INDEP|TRACK"
 *
 * 4. Capabilities:
 *    - Implement getCapabilities() to report:
 *      * Number of channels
 *      * Max voltage/current per channel
 *      * Supported features
 *
 * 5. Factory Integration:
 *    - Add to PowerSupplyFactory::create()
 *    - Add IDN string parsing in createFromIDN()
 *
 * 6. Testing:
 *    - Create unit tests for each method
 *    - Test with real hardware if available
 *    - Use mock communication for CI/CD
 *
 * 7. Documentation:
 *    - Add usage examples
 *    - Document model-specific quirks
 *    - Add to README.md
 */

#endif // KEYSIGHT_E36XX_SKELETON_H
