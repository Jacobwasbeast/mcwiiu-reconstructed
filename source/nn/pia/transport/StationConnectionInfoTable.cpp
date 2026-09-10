#include "nn/pia/transport/StationConnectionInfoTable.hpp"

namespace nn { namespace pia { namespace transport {

StationConnectionInfoTable* g_stationConnectionInfoTable = 0;

int StationConnectionInfoTable::GetStationConnectionInfo(const Station* station, StationConnectionInfo* info) const
{
    if (station == 0 || info == 0)
        return 0x10c07;

    const_cast<common::CriticalSection&>(m_criticalSection).Lock();
    *info = m_hostStationConnectionInfo;
    const_cast<common::CriticalSection&>(m_criticalSection).Unlock();
    return 0;
}

}}}
