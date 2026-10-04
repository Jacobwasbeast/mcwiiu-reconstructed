#include "nn/nex/URLProbeList.hpp"

namespace nn
{
namespace nex
{

URLProbeList::URLProbeList()
{
}

URLProbeList::~URLProbeList()
{
}

URLProbe* URLProbeList::FindProbe(const StationURL& rStationURL)
{
    for (ProbeList::iterator it = m_probes.begin(); it != m_probes.end(); ++it)
    {
        if (String::IsEqual(it->m_stationURL.GetAddress(), rStationURL.GetAddress()) &&
            it->m_stationURL.GetRVConnectionID() == rStationURL.GetRVConnectionID())
            return &*it;
    }
    return 0;
}

URLProbe* URLProbeList::FindProbe(unsigned int uiConnectionID)
{
    ProbeList::iterator it = m_probes.begin();
    while (it != m_probes.end())
    {
        if (it->m_stationURL.GetRVConnectionID() == uiConnectionID)
            return &*it;
        ++it;
    }
    return 0;
}

}
}
