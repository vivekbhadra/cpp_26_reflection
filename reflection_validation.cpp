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

template <typename T>
void validate_or_throw(const T& object)
{
    template for (constexpr std::meta::info member : std::define_static_array(
                      std::meta::nonstatic_data_members_of(^^T,
                                                           std::meta::access_context::unchecked())))
    {
        std::cout << "Validating member: " << std::meta::identifier_of(member) << '\n';
        constexpr std::string_view name = std::meta::identifier_of(member);

        constexpr auto ranges =
            std::define_static_array(std::meta::annotations_of_with_type(member, ^^Range));

        constexpr auto range_count = ranges.size();

        std::cout << "Found " << range_count << " range annotations for member: " << name << '\n';

        if constexpr (!ranges.empty())
        {
            constexpr Range range = std::meta::extract<Range>(ranges[0]);

            const auto& value = object.[:member:];

            if (value < range.minimum || value > range.maximum)
            {
                throw std::out_of_range(std::string{name} + " is outside the permitted range ["
                                        + std::to_string(range.minimum) + ", "
                                        + std::to_string(range.maximum) + "]");
            }
        }

        constexpr auto required =
            std::define_static_array(std::meta::annotations_of_with_type(member, ^^Required));

        if constexpr (!required.empty())
        {
            const auto& value = object.[:member:];

            if constexpr (requires { value.empty(); })
            {
                if (value.empty())
                {
                    throw std::invalid_argument(std::string{name} + " is required");
                }
            }
            else
            {
                static_assert(
                    requires { value.empty(); },
                    "Required annotation can only be applied to a type "
                    "that provides empty()");
            }
        }
    }
}

class MotorConfig
{
public:
    MotorConfig(int rpm, double maxTemperature, std::string deviceName);

    [[nodiscard]]
    int rpm() const noexcept
    {
        return m_rpm;
    }

    [[nodiscard]]
    double max_temperature() const noexcept
    {
        return m_maxTemperature;
    }

    [[nodiscard]]
    const std::string& device_name() const noexcept
    {
        return m_deviceName;
    }

private:
    [[= Range{0.0, 10000.0}]] int m_rpm;

    [[= Range{-40.0, 150.0}]] double m_maxTemperature;

    [[= Required{}]] std::string m_deviceName;
};

MotorConfig::MotorConfig(int rpm, double maxTemperature, std::string deviceName)
    : m_rpm{rpm},
      m_maxTemperature{maxTemperature},
      m_deviceName{std::move(deviceName)}
{
    validate_or_throw(*this);
}

int main()
{
    try
    {
        const MotorConfig badConfig{15000, 200.0, ""};

        std::cout << "Unexpectedly constructed invalid configuration.\n";
    }
    catch (const std::exception& exception)
    {
        std::cout << "Construction failed: " << exception.what() << '\n';
    }

    try
    {
        const MotorConfig goodConfig{5000, 85.0, "motor-controller-1"};

        std::cout << "Valid configuration created.\n"
                  << "RPM: " << goodConfig.rpm() << '\n'
                  << "Maximum temperature: " << goodConfig.max_temperature() << '\n'
                  << "Device name: " << goodConfig.device_name() << '\n';
    }
    catch (const std::exception& exception)
    {
        std::cout << "Construction failed: " << exception.what() << '\n';
    }

    return 0;
}