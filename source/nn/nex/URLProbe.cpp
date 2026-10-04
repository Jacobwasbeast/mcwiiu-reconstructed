#include "nn/nex/URLProbe.hpp"

namespace nn
{
namespace nex
{

URLProbe::URLProbe(const StationURL& rStationURL, int iTimeout, int iInterval,
    unsigned int uiMaxProbes)
    : m_stationURL(rStationURL), m_iPingTime(-1), m_iInterval(iInterval), m_uiProbeCount(0),
      m_uiMaxProbes(uiMaxProbes)
{
    m_expirationTime = Time(Time::GetTime() + iTimeout);
}

URLProbe::~URLProbe()
{
}

bool URLProbe::UpdateIsNeeded(Time currentTime) const
{
    if (m_iPingTime == -1)
        return m_uiProbeCount < m_uiMaxProbes;

    return static_cast<long long>(currentTime - m_lastUpdateTime) > m_iInterval;
}

void URLProbe::Trace(unsigned long long)
{
}

}
}
