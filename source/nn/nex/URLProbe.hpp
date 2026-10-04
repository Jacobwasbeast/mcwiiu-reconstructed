#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_URLPROBE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_URLPROBE_HPP

#include "nn/nex/RootObject.hpp"
#include "nn/nex/StationURL.hpp"
#include "nn/nex/Time.hpp"

namespace nn
{
namespace nex
{

class URLProbe : public RootObject
{
public:
    URLProbe(const StationURL& rStationURL, int iTimeout, int iInterval, unsigned int uiMaxProbes);
    virtual ~URLProbe();

    bool UpdateIsNeeded(Time currentTime) const;
    void Trace(unsigned long long uiFilter);

    StationURL m_stationURL;
    Time m_expirationTime;
    int m_iPingTime;
    Time m_lastUpdateTime;
    int m_iInterval;
    unsigned int m_uiProbeCount;
    unsigned int m_uiMaxProbes;
};

}
}

#endif
