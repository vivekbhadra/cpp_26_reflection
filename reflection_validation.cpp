#include <iostream>
#include <meta>
#include <string>

struct Range
{
    double minimum;
    double maximum;
};

struct Required
{
};

struct MotorConfig
{
    [[= Range{0.0, 10000.0}]] int rpm;

    [[= Range{-40.0, 150.0}]] double maxTemperature;

    [[= Required{}]] std::string deviceName;
};

template <typename T>
bool validate(const T& object)
{
    bool valid = true;

    template for (constexpr std::meta::info member : std::define_static_array(
                      std::meta::nonstatic_data_members_of(^^T,
                                                           std::meta::access_context::unchecked())))
    {
        constexpr auto name = std::meta::identifier_of(member);

        constexpr auto ranges =
            std::define_static_array(std::meta::annotations_of_with_type(member, ^^Range));

        if constexpr (!ranges.empty())
        {
            constexpr Range range = std::meta::extract<Range>(ranges[0]);

            const auto& value = object.[:member:];

            if (value < range.minimum || value > range.maximum)
            {
                std::cout << "Validation failed: " << name << " = " << value
                          << " is outside range [" << range.minimum << ", " << range.maximum
                          << "]\n";

                valid = false;
            }
        }

        constexpr auto required =
            std::define_static_array(std::meta::annotations_of_with_type(member, ^^Required));

        if constexpr (!required.empty())
        {
            const auto& value = object.[:member:];

            if (value.empty())
            {
                std::cout << "Validation failed: " << name << " is required\n";

                valid = false;
            }
        }
    }

    return valid;
}

int main()
{
    MotorConfig badConfig{.rpm = 15000, .maxTemperature = 200.0, .deviceName = ""};

    std::cout << "Checking bad configuration:\n";

    if (!validate(badConfig))
    {
        std::cout << "Configuration is invalid.\n";
    }

    std::cout << '\n';

    MotorConfig goodConfig{.rpm = 5000, .maxTemperature = 85.0, .deviceName = "motor-controller-1"};

    std::cout << "Checking good configuration:\n";

    if (validate(goodConfig))
    {
        std::cout << "Configuration is valid.\n";
    }

    return 0;
}