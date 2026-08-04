// reflective_serialisation.cpp
//
// C++26 (P2996 reflection). One generic function replaces every
// hand-written toJson overload in manual_serialisation.cpp.
//
// Requires GCC 16.1+ built with reflection support (-freflection).
// NOTE: this file has NOT been compiled in this environment — GCC 16.1
// with reflection isn't available here. It has been reviewed for API
// consistency with GCC 16.1's documented P2996 support, but verify it
// builds cleanly against your own /opt/gcc-16.1 before publishing.
//
// Build:
//   export PATH=/opt/gcc-16.1/bin:$PATH
//   g++ -std=c++26 -freflection -Wl,-rpath,/opt/gcc-16.1/lib64 \
//       reflective_serialisation.cpp -o can_config_reflection \
//       -lPocoJSON -lPocoFoundation

#include <Poco/JSON/Array.h>
#include <Poco/JSON/Object.h>
#include <iostream>
#include <meta>
#include <sstream>
#include <string_view>
#include <type_traits>
#include "domain.h"

// C++ convention here is m_-prefixed members; JSON keys shouldn't carry
// that prefix. Ordinary consteval code — reflection just hands it the
// member's name as a std::string_view to operate on.
consteval std::string_view strip_member_prefix(std::string_view name)
{
    return name.starts_with("m_") ? name.substr(2) : name;
}

// Forward-declared so add_field can recurse into it for nested struct fields.
template <typename T>
Poco::JSON::Object to_json(const T& source);

Poco::JSON::Array to_hex_array(const std::vector<unsigned int>& values)
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
template <typename T>
void add_field(Poco::JSON::Object& jsonObject, std::string_view key, const T& value)
{
    if constexpr (std::is_same_v<T, std::vector<unsigned int>>)
        jsonObject.set(std::string(key), to_hex_array(value));
    else if constexpr (std::is_class_v<T>)
        jsonObject.set(std::string(key), to_json(value));
    else
        jsonObject.set(std::string(key), value);
}

// The generic serializer. Walks every non-static data member of T at
// compile time and adds it to the JSON object. Works for CanDataSource,
// RetryPolicy, or any future struct — no new overload required.
template <typename T>
Poco::JSON::Object to_json(const T& source)
{
    Poco::JSON::Object jsonObject;

    template for (constexpr auto member :
                  std::meta::nonstatic_data_members_of(
                      ^^T, std::meta::access_context::unchecked()))
    {
        constexpr std::string_view key = strip_member_prefix(std::meta::identifier_of(member));
        add_field(jsonObject, key, source.[:member:]);
    }

    return jsonObject;
}

int main()
{
    const CanDataSource source{"can-source-1", "can1", 250000, true,
                               {0xFEF1, 0xFEEE},
                               RetryPolicy{3, 500, true}};

    to_json(source).stringify(std::cout, 4);
    std::cout << "\n";
    return 0;
}
