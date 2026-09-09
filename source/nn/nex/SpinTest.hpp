#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_SPINTEST_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_SPINTEST_HPP

#include <cstdint>

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class SpinTest : public RootObject
{
public:
    SpinTest(std::uint32_t sleepInterval, std::uint32_t timeout);
    virtual ~SpinTest();

    void SetTrace(std::uint32_t trace, std::uint64_t flags);
    void AssertOnTimeout();
    void LeaveOnTimeout();

    std::uint32_t GetElapsed();
    std::uint32_t GetTimeout();
    std::int32_t GetRemainingTime();
    std::uint32_t SpinOnce(wchar_t* file, std::uint32_t line, wchar_t* state);

private:
    std::uint16_t m_state;
    std::uint32_t m_timeout;
    std::uint64_t m_started;
    std::uint32_t m_elapsed;
    std::uint32_t m_sleepInterval;
    std::uint32_t m_limit;
    std::uint32_t m_reserved;
    std::uint32_t m_trace;
    std::uint32_t m_traceCount;
    bool m_timedOut;
};

}
}

#endif
