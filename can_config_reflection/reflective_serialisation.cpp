// reflective_serialisation.cpp
// build: g++ -std=c++26 -freflection -Wl,-rpath,/opt/gcc-16.1/lib64 \
//        reflective_serialisation.cpp -o can_config_reflection \
//        $(pkg-config --cflags --libs poco-json) -lPocoJSON -lPocoFoundation

#include <Poco/JSON/Array.h>
#include <Poco/JSON/Object.h>
#include <iostream>
#include <meta>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

struct RetryPolicy
{
    int maxAttempts;
    int backoffMs;
    bool jitterEnabled;
};

class CanDataSource
{
public:
    CanDataSource(std::string id,
                  std::string canLine,
                  int bitrate,
                  bool extendedFrame,
                  std::vector<unsigned int> pgnPassFilter,
                  RetryPolicy retryPolicy)
        : m_id{std::move(id)},
          m_canLine{std::move(canLine)},
          m_bitrate{bitrate},
          m_extendedFrame{extendedFrame},
          m_pgnPassFilter{std::move(pgnPassFilter)},
          m_retryPolicy{retryPolicy}
    {
    }

private:
    std::string m_id;
    std::string m_canLine;
    int m_bitrate;
    bool m_extendedFrame;
    std::vector<unsigned int> m_pgnPassFilter;
    RetryPolicy m_retryPolicy;
};

consteval std::string_view strip_member_prefix(std::string_view name)
{
    return name.starts_with("m_") ? name.substr(2) : name;
}

template <typename T> Poco::JSON::Object to_json(const T& source);

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

template <typename T> Poco::JSON::Object to_json(const T& source)
{
    Poco::JSON::Object jsonObject;

    template for (constexpr auto member :
                  std::meta::nonstatic_data_members_of(^^T, std::meta::access_context::unchecked()))
    {
        constexpr std::string_view key = strip_member_prefix(std::meta::identifier_of(member));
        add_field(jsonObject, key, source.[:member:]);
    }

    return jsonObject;
}

int main()
{
    const CanDataSource source{"can-source-1",
                               "can1",
                               250000,
                               true,
                               {0xFEF1, 0xFEEE},
                               RetryPolicy{3, 500, true}};

    to_json(source).stringify(std::cout, 4);
    std::cout << "\n";

    return 0;
}