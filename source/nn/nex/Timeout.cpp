#include "nn/nex/Timeout.hpp"

#include "nn/nex/Core.hpp"

namespace nn
{
namespace nex
{

Timeout::Timeout()
    : m_uiRTO(0)
    , m_awaitedTime()
    , m_expirationTime()
    , m_relativeExpirationTime(0)
    , m_pCore(Core::GetInstance())
{
}

Timeout::~Timeout()
{
}

void Timeout::SetRTO(unsigned int uiRTO)
{
    m_uiRTO = uiRTO;
}

void Timeout::Start()
{
    m_awaitedTime = Time(Time::GetTime()) + static_cast<int>(m_uiRTO);
}

void Timeout::SetExpirationTime(Time expirationTime)
{
    m_expirationTime = expirationTime;
}

Time Timeout::GetAwaitedTime()
{
    return m_awaitedTime;
}

void Timeout::SetRelativeExpirationTime(int iRelativeExpirationTime)
{
    long long relativeExpirationTime = iRelativeExpirationTime;
    SetExpirationTime(Time(Time::GetTime()) + iRelativeExpirationTime);
    m_relativeExpirationTime = relativeExpirationTime;
}

bool Timeout::IsAwaited() const
{
    return static_cast<long long>(m_awaitedTime) < static_cast<long long>(g_lastTime);
}

}
}
