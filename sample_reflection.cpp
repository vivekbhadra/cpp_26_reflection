#include <iostream>
#include <meta>
#include <string_view>
#include <type_traits>

template <typename E>
requires std::is_enum_v<E>
constexpr std::string_view enum_to_string(E value)
{
    template for (constexpr auto e : std::define_static_array(std::meta::enumerators_of(^^E)))
    {
        if (value == [:e:])
        {
            return std::meta::identifier_of(e);
        }
    }

    return "<unknown>";
}

enum class Colour
{
    Red,
    Green,
    Blue,
    Yellow
};

int main()
{
    static_assert(enum_to_string(Colour::Green) == "Green");

    constexpr std::string_view enum_name = std::meta::identifier_of(^^Colour);

    constexpr std::size_t enumerator_count = std::meta::enumerators_of(^^Colour).size();

    std::cout << "Enum type: " << enum_name << "\n";
    std::cout << "Number of enumerators: " << enumerator_count << "\n";

    std::cout << "Enumerators:\n";

    template for (constexpr auto e : std::define_static_array(std::meta::enumerators_of(^^Colour)))
    {
        constexpr std::string_view name = std::meta::identifier_of(e);

        constexpr int value = static_cast<int>(std::meta::extract<Colour>(e));

        std::cout << "  Name: " << name << ", value: " << value << "\n";
    }

    std::cout << "Colour::Green converted to string: " << enum_to_string(Colour::Green) << "\n";

    return 0;
}