// reflective_serialisation.cpp
//
// C++26 (P2996 reflection). One generic function replaces every
// hand-written toJson overload in manual_serialisation.cpp.

#include "domain.h"
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Object.h>
#include <iostream>
#include <meta>
#include <sstream>
#include <string_view>
#include <type_traits>

// C++ convention here is m_-prefixed members; JSON keys shouldn't carry
// that prefix.
consteval std::string_view strip_member_prefix(std::string_view name)
{
    return name.starts_with("m_") ? name.substr(2) : name;
}

// Forward-declared.
template <typename T> Poco::JSON::Object to_json(const T &source);

Poco::JSON::Array to_hex_array(const std::vector<unsigned int> &values)
{
    Poco::JSON::Array jsonArray;
    for (const unsigned int value : values)
    {
        std::ostringstream stream;
        stream << std::uppercase << std::hex << value;
        jsonArray.add(stream.str());
    }
    return jsonArray;
}

// Dispatches on the field's type: vectors of uint get hex-formatted,
// nested structs get recursively serialized, everything else (string,
// int, bool) is passed straight to Poco.
template <typename T> void add_field(Poco::JSON::Object &jsonObject, std::string_view key, const T &value)
{
    if constexpr (std::is_same_v<T, std::vector<unsigned int>>)
        jsonObject.set(std::string(key), to_hex_array(value));
    else if constexpr (std::is_same_v<T, std::string>)
        jsonObject.set(std::string(key), value);
    else if constexpr (std::is_class_v<T>)
        jsonObject.set(std::string(key), to_json(value));
    else
        jsonObject.set(std::string(key), value);
}

// The generic serializer. Walks every non-static data member of T at
// compile time and adds it to the JSON object. Works for CanDataSource,
// RetryPolicy, or any future struct — no new overload required.
template <typename T> Poco::JSON::Object to_json(const T &source)
{
    Poco::JSON::Object jsonObject;

    // nonstatic_data_members_of returns a std::vector<std::meta::info>,
    // which is heap-allocated and can't survive past compile time.
    // define_static_array promotes it to a span backed by static
    // storage, which template for can iterate.
    static constexpr auto members =
        std::define_static_array(std::meta::nonstatic_data_members_of(^^T, std::meta::access_context::unchecked()));

    template for (constexpr auto member : members)
    {
        constexpr std::string_view key = strip_member_prefix(std::meta::identifier_of(member));
        add_field(jsonObject, key, source.[:member:]);
    }

    return jsonObject;
}

int main()
{
    const CanDataSource source{ "can-source-1", "can1", 250000, true, { 0xFEF1, 0xFEEE }, RetryPolicy{ 3, 500, true } };

    to_json(source).stringify(std::cout, 4);
    std::cout << "\n";
    return 0;
}
