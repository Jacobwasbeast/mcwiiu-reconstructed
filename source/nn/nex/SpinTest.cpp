#include "nn/nex/SpinTest.hpp"

#include "nn/nex/Platform.hpp"
#include "nn/nex/Time.hpp"

namespace nn
{
namespace nex
{

SpinTest::SpinTest(uint32_t sleepInterval, uint32_t timeout)
    : m_state(1)
    , m_timeout(timeout)
    , m_started(Time::GetTime())
    , m_elapsed(0)
    , m_sleepInterval(sleepInterval)
    , m_limit(1000)
    , m_trace(8)
    , m_traceCount(0)
    , m_timedOut(false)
{
}

void SpinTest::LeaveOnTimeout()
{
    m_state = 2;
}

int32_t SpinTest::GetRemainingTime()
{
    if (m_timeout == 0xffffffffu)
        return static_cast<int32_t>(m_timeout);

    m_elapsed = static_cast<uint32_t>(Time::GetTime() - m_started);

    if (m_timeout > m_elapsed)
        return static_cast<int32_t>(m_timeout - m_elapsed);

    return 0;
}

uint32_t SpinTest::SpinOnce(wchar_t*, uint32_t, wchar_t*)
{
    Platform::Sleep(m_sleepInterval);

    m_elapsed = static_cast<uint32_t>(Time::GetTime() - m_started);

    if (m_elapsed > m_timeout && m_timeout != 0xffffffffu)
    {
        m_timedOut = true;

        if (m_state == 2)
            return 0;
    }

    return 1;
}

}
}
