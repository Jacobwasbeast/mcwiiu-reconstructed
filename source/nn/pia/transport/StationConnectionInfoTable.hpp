#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_STATION_CONNECTION_INFO_TABLE_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_STATION_CONNECTION_INFO_TABLE_HPP

#include "nn/pia.h"
#include "nn/pia/transport/StationConnectionInfo.hpp"

namespace nn { namespace pia { namespace transport {

class Station;

class StationConnectionInfoTable
{
public:
    StationConnectionInfoTable();
    ~StationConnectionInfoTable();
    static StationConnectionInfoTable* CreateInstance();
    static void DestroyInstance();
    void Finalize();
    void Initialize(unsigned int maxStations);
    void AddToTable(const Station* station, const StationConnectionInfo& info);
    int GetStationConnectionInfo(const Station* station, StationConnectionInfo* info) const;
    int GetStationConnectionInfo(unsigned long long id, StationConnectionInfo* info) const;
    Station* GetStation(const StationConnectionInfo& info) const;
    Station* GetStation(const StationLocation& location) const;
    Station* GetStation(const common::StationAddress& address) const;
    Station* GetStation(unsigned int index) const;
    Station* GetStation(unsigned long long id) const;
    Station* GetReconnectingStation(const StationConnectionInfo& info) const;
    unsigned int GetStationIndexByPrincipalID(unsigned long long principalId) const;
    unsigned long long GetPrincipalIdByStation(const Station* station) const;
    void EraseFromTable(const Station* station);
    void ClearTable();
    void Trace(unsigned long long value) const;

    StationConnectionInfo m_localStationConnectionInfo;
    StationConnectionInfo m_hostStationConnectionInfo;
    common::StationAddress m_hostStationAddress;

private:
    unsigned int m_scalarStorage[34];
    common::CriticalSection m_criticalSection;
};

extern StationConnectionInfoTable* g_stationConnectionInfoTable;

}}}

#endif
