// domain.h
//
// Plain data only. No serialization logic lives here on purpose —
// both manual_serialisation.cpp and reflective_serialisation.cpp
// consume this exact same definition.
#pragma once

#include <string>
#include <utility>
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
    friend struct ManualJson;   // grants the hand-written serializer access to private fields

    std::string m_id;
    std::string m_canLine;
    int m_bitrate;
    bool m_extendedFrame;
    std::vector<unsigned int> m_pgnPassFilter;
    RetryPolicy m_retryPolicy;
};
