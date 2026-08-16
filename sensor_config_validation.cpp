#include <iostream>
#include <meta>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

struct Range
{
    double minimum;
    double maximum;
};

struct Required
{
};

class SensorConfig
{
public:
    SensorConfig(int communicationPort,
                 int pollingIntervalMs,
                 double alarmTemperature,
                 std::string sensorId,
                 std::string gatewayAddress);

    [[nodiscard]]
    int communication_port() const noexcept
    {
        return m_communicationPort;
    }

    [[nodiscard]]
    int polling_interval_ms() const noexcept
    {
        return m_pollingIntervalMs;
    }

    [[nodiscard]]
    double alarm_temperature() const noexcept
    {
        return m_alarmTemperature;
    }

    [[nodiscard]]
    const std::string& sensor_id() const noexcept
    {
        return m_sensorId;
    }

    [[nodiscard]]
    const std::string& gateway_address() const noexcept
    {
        return m_gatewayAddress;
    }

private:
    void validate_or_throw() const;

    [[=Range{1.0, 65535.0}]]
    int m_communicationPort;

    [[=Range{1.0, 1000.0}]]
    int m_pollingIntervalMs;

    [[=Range{-40.0, 125.0}]]
    double m_alarmTemperature;

    [[=Required{}]]
    std::string m_sensorId;

    [[=Required{}]]
    std::string m_gatewayAddress;
};

SensorConfig::SensorConfig(int communicationPort,
                           int pollingIntervalMs,
                           double alarmTemperature,
                           std::string sensorId,
                           std::string gatewayAddress)
    : m_communicationPort{communicationPort},
      m_pollingIntervalMs{pollingIntervalMs},
      m_alarmTemperature{alarmTemperature},
      m_sensorId{std::move(sensorId)},
      m_gatewayAddress{std::move(gatewayAddress)}
{
    validate_or_throw();
}

void SensorConfig::validate_or_throw() const
{
    template for (
        constexpr std::meta::info member :
        std::define_static_array(
            std::meta::nonstatic_data_members_of(
                ^^SensorConfig,
                std::meta::access_context::unchecked())))
    {
        constexpr std::string_view name =
            std::meta::identifier_of(member);

        constexpr auto ranges =
            std::define_static_array(
                std::meta::annotations_of_with_type(
                    member,
                    ^^Range));

        if constexpr (!ranges.empty())
        {
            constexpr Range range =
                std::meta::extract<Range>(ranges[0]);

            const auto& value = this->[:member:];

            if (value < range.minimum ||
                value > range.maximum)
            {
                throw std::out_of_range(
                    std::string{name} +
                    " is outside the permitted range [" +
                    std::to_string(range.minimum) +
                    ", " +
                    std::to_string(range.maximum) +
                    "]");
            }
        }

        constexpr auto required =
            std::define_static_array(
                std::meta::annotations_of_with_type(
                    member,
                    ^^Required));

        if constexpr (!required.empty())
        {
            const auto& value = this->[:member:];

            if constexpr (requires { value.empty(); })
            {
                if (value.empty())
                {
                    throw std::invalid_argument(
                        std::string{name} +
                        " must not be empty");
                }
            }
            else
            {
                static_assert(
                    requires { value.empty(); },
                    "Required can only be applied to a type "
                    "that provides empty()");
            }
        }
    }
}

int main()
{
    try
    {
        const SensorConfig invalidConfig{
            70000,          // Invalid communication port
            1500,           // Invalid polling interval
            150.0,          // Invalid alarm temperature
            "",             // Missing sensor ID
            "192.168.1.10"
        };

        std::cout << "Invalid configuration was unexpectedly created.\n";
    }
    catch (const std::exception& exception)
    {
        std::cout
            << "Configuration rejected: "
            << exception.what()
            << '\n';
    }

    std::cout << '\n';

    try
    {
        const SensorConfig validConfig{
            502,
            250,
            80.0,
            "temperature-sensor-01",
            "192.168.1.10"
        };

        std::cout << "Valid sensor configuration created.\n";

        std::cout
            << "Communication port: "
            << validConfig.communication_port()
            << '\n';

        std::cout
            << "Polling interval: "
            << validConfig.polling_interval_ms()
            << " ms\n";

        std::cout
            << "Alarm temperature: "
            << validConfig.alarm_temperature()
            << " C\n";

        std::cout
            << "Sensor ID: "
            << validConfig.sensor_id()
            << '\n';

        std::cout
            << "Gateway address: "
            << validConfig.gateway_address()
            << '\n';
    }
    catch (const std::exception& exception)
    {
        std::cout
            << "Configuration rejected: "
            << exception.what()
            << '\n';
    }

    return 0;
}
