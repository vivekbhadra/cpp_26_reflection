// domain.h — C++17
#pragma once

#include <Poco/JSON/Array.h>
#include <Poco/JSON/Object.h>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

struct RetryPolicy
{
    int maxAttempts;
    int backoffMs;
    bool jitterEnabled;

    [[nodiscard]] Poco::JSON::Object toJson() const
    {
        Poco::JSON::Object jsonObject;
        jsonObject.set("maxAttempts", maxAttempts);
        jsonObject.set("backoffMs", backoffMs);
        jsonObject.set("jitterEnabled", jitterEnabled);
        return jsonObject;
    }
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

    [[nodiscard]] Poco::JSON::Object toJson() const
    {
        Poco::JSON::Object jsonObject;

        jsonObject.set("id", m_id);
        jsonObject.set("canLine", m_canLine);
        jsonObject.set("bitrate", m_bitrate);
        jsonObject.set("extendedFrame", m_extendedFrame);
        jsonObject.set("pgnPassFilter", toHexArray(m_pgnPassFilter));
        jsonObject.set("retryPolicy", m_retryPolicy.toJson());

        return jsonObject;
    }

private:
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

    friend struct DomainReflectionAccess;

    std::string m_id;
    std::string m_canLine;
    int m_bitrate;
    bool m_extendedFrame;
    std::vector<unsigned int> m_pgnPassFilter;
    RetryPolicy m_retryPolicy;
};