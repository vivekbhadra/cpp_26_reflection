// manual_serialisation.cpp
//
// Today's C++: hand-written, field-by-field JSON mapping.
//
// Build (verified against libpoco-dev on Ubuntu 24.04):
//   g++ -std=c++17 manual_serialisation.cpp -o can_config_manual \
//       -lPocoJSON -lPocoFoundation
//
// If your Poco install isn't on the default include/lib path, add
// -I/path/to/poco/include -L/path/to/poco/lib, or use pkg-config if
// your distro ships a poco-json.pc file:
//   g++ -std=c++17 manual_serialisation.cpp -o can_config_manual \
//       $(pkg-config --cflags --libs poco-json)

#include <Poco/JSON/Array.h>
#include <Poco/JSON/Object.h>
#include <iostream>
#include <sstream>
#include "domain.h"

struct ManualJson
{
    static Poco::JSON::Array toHexArray(const std::vector<unsigned int>& values)
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

    static Poco::JSON::Object toJson(const RetryPolicy& policy)
    {
        Poco::JSON::Object jsonObject;
        jsonObject.set("maxAttempts", policy.maxAttempts);
        jsonObject.set("backoffMs", policy.backoffMs);
        jsonObject.set("jitterEnabled", policy.jitterEnabled);
        return jsonObject;
    }

    static Poco::JSON::Object toJson(const CanDataSource& source)
    {
        Poco::JSON::Object jsonObject;
        jsonObject.set("id", source.m_id);
        jsonObject.set("canLine", source.m_canLine);
        jsonObject.set("bitrate", source.m_bitrate);
        jsonObject.set("extendedFrame", source.m_extendedFrame);
        jsonObject.set("pgnPassFilter", toHexArray(source.m_pgnPassFilter));
        jsonObject.set("retryPolicy", toJson(source.m_retryPolicy));
        return jsonObject;
    }
};

int main()
{
    const CanDataSource source{"can-source-1", "can1", 250000, true,
                               {0xFEF1, 0xFEEE},
                               RetryPolicy{3, 500, true}};

    ManualJson::toJson(source).stringify(std::cout, 4);
    std::cout << "\n";
    return 0;
}
