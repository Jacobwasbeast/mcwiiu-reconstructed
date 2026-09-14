#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_STATIONMANAGER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_STATIONMANAGER_HPP

#include "nn/pia.h"
#include "nn/pia/ListBase.hpp"
#include <cstddef>

namespace nn { namespace pia {
namespace common {
template <class T, int N> struct SimpleContainer { unsigned int size; T data[N]; unsigned int reserved; };
}
namespace transport {
class StationManager {
public:
    enum { cStationNumMax = 32 };
    StationManager(); ~StationManager(); static Result CreateInstance(); void Finalize(); static void DestroyInstance();
    Result Initialize(NetworkFactory*, unsigned int, StationProtocol*, RttProtocol*);
    Station* CreateStation(); Station* CreateLocalStation();
    bool isInStationList(const Station*) const;
    bool isInPtrArray(const common::SimpleContainer<Station*, cStationNumMax>&, const Station*) const;
    void DestroyStation(Station*); void DestroyStation(StationIndex);
    Station* GetStation(StationIndex); const Station* GetStation(StationIndex) const;
    Station* GetStation(unsigned long long); const Station* GetStation(unsigned long long) const;
    Station* GetStation(const common::StationAddress&); const Station* GetStation(const common::StationAddress&) const;
    Station* GetStationByAddress(const common::InetAddress&); const Station* GetStationByAddress(const common::InetAddress&) const;
    Station* GetStationByStationIdWithoutConvert(unsigned long long); const Station* GetStationByStationIdWithoutConvert(unsigned long long) const;
    Result GetStationAddress(common::StationAddress*, StationIndex) const; Result GetStationAddress(common::StationAddress*, unsigned long long) const;
    unsigned short GetValidStationNum() const; unsigned int GetParticipatingStationBitmap(bool) const; void Trace(unsigned long long) const;
public:
    common::ListBase m_stationList; common::ListBase m_freeStationList;
    unsigned int m_stationOffset; unsigned char* m_stationPool; unsigned int m_stationNum;
    unsigned char* m_stationStorage;
    common::SimpleContainer<Station*, cStationNumMax> m_stations;
    common::SimpleContainer<Station*, cStationNumMax> m_freeStations;
    Station* m_localStation; StationIndex m_invalidStationIndex;
};
extern StationManager* g_piaStationManager;
static_assert(sizeof(StationManager) == 0x140, "StationManager size");
static_assert(offsetof(StationManager, m_stationOffset) == 0x18, "StationManager offset");
static_assert(offsetof(StationManager, m_stationPool) == 0x1c, "StationManager pool");
static_assert(offsetof(StationManager, m_stationNum) == 0x20, "StationManager count");
static_assert(offsetof(StationManager, m_stationStorage) == 0x24, "StationManager storage");
static_assert(offsetof(StationManager, m_stations) == 0x28, "StationManager stations");
static_assert(offsetof(StationManager, m_freeStations) == 0xb0, "StationManager free stations");
static_assert(offsetof(StationManager, m_localStation) == 0x138, "StationManager local");
static_assert(offsetof(StationManager, m_invalidStationIndex) == 0x13c, "StationManager invalid index");
}}}
#endif
