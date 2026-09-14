#include "nn/pia/transport/StationConnectionInfoTable.hpp"

#include "nn/pia/HeapManager.hpp"
#include "nn/pia/transport/TransportSupport.hpp"
#include "platform/wiiu/pead.hpp"

#include <new>

namespace nn { namespace pia { namespace transport {

StationConnectionInfoTable* g_stationConnectionInfoTable = 0;

namespace
{
void* g_stationConnectionInfoTableStorage = 0;

unsigned long long PrincipalId(const StationConnectionInfo& info)
{
    return (static_cast<unsigned long long>(static_cast<unsigned int>(info.m_field14)) << 32) |
           static_cast<unsigned int>(info.m_field10);
}
}

StationConnectionInfoTable::StationConnectionInfoTable()
    : StationConnectionInfo(), m_hostStationConnectionInfo(), m_hostStationAddress(),
      m_infos(0), m_count(0), m_criticalSection(-1)
{
}

StationConnectionInfoTable::~StationConnectionInfoTable()
{
    Finalize();
}

Result StationConnectionInfoTable::CreateInstance()
{
    if (IsInitialized() == 0) return 0x10c0c;
    if (IsInSetupMode() == 0) return 0x10c08;
    if (g_stationConnectionInfoTable != 0) return 0x10c0f;

    pead::Heap* heap = common::HeapManager::GetHeap();
    g_stationConnectionInfoTableStorage = pead::allocateFromSeadHeap(
        sizeof(StationConnectionInfoTable), heap, 8);
    if (g_stationConnectionInfoTableStorage == 0)
    {
        g_stationConnectionInfoTable = 0;
        return 0x10c01;
    }
    unsigned char* storage = static_cast<unsigned char*>(g_stationConnectionInfoTableStorage);
    for (unsigned int i = 0; i < sizeof(StationConnectionInfoTable); ++i)
        storage[i] = 0;
    g_stationConnectionInfoTable = new (g_stationConnectionInfoTableStorage)
        StationConnectionInfoTable();
    return 0;
}

void StationConnectionInfoTable::DestroyInstance()
{
    if (g_stationConnectionInfoTable != 0)
    {
        g_stationConnectionInfoTable->~StationConnectionInfoTable();
        g_stationConnectionInfoTable = 0;
    }
    if (g_stationConnectionInfoTableStorage != 0)
    {
        pead::getArraySize(g_stationConnectionInfoTableStorage);
        pead::freeToSeadHeap(g_stationConnectionInfoTableStorage);
        g_stationConnectionInfoTableStorage = 0;
    }
}

void StationConnectionInfoTable::Finalize()
{
    if (m_infos != 0)
    {
        const unsigned int count = pead::getArraySize(m_infos) / sizeof(StationConnectionInfo);
        for (unsigned int i = 0; i < count; ++i) m_infos[i].~StationConnectionInfo();
        pead::freeToSeadHeap(m_infos);
        m_infos = 0;
    }
}

Result StationConnectionInfoTable::Initialize(unsigned int count)
{
    if (count > 32) return 0x10c07;
    m_count = count;
    pead::Heap* heap = common::HeapManager::GetHeap();
    StationConnectionInfo* infos = static_cast<StationConnectionInfo*>(
        pead::allocateFromSeadHeap(count * sizeof(StationConnectionInfo), heap));
    if (infos == 0)
    {
        m_infos = 0;
        for (unsigned int i = 0; i < 32; ++i) m_stations[i] = 0;
        return 0;
    }
    for (unsigned int i = 0; i < count; ++i) new (infos + i) StationConnectionInfo();
    m_infos = infos;
    for (unsigned int i = 0; i < 32; ++i) m_stations[i] = 0;
    return 0;
}

Result StationConnectionInfoTable::AddToTable(const Station* station,
                                               const StationConnectionInfo& info)
{
    if (station == 0) return 0x10c07;
    info.Trace(0x8000);
    m_criticalSection.Lock();
    unsigned int index = 0;
    while (index < m_count && m_stations[index] != station) ++index;
    if (index == m_count)
    {
        index = 0;
        while (index < m_count && m_stations[index] != 0) ++index;
        if (index == m_count)
        {
            m_criticalSection.Unlock();
            return 0x10c08;
        }
        m_stations[index] = const_cast<Station*>(station);
    }
    m_infos[index] = info;
    m_criticalSection.Unlock();
    return 0;
}

Result StationConnectionInfoTable::GetStationConnectionInfo(
    const Station* station, StationConnectionInfo* info) const
{
    if (station == 0 || info == 0) return 0x10c07;
    m_criticalSection.Lock();
    for (unsigned int i = 0; i < m_count; ++i)
        if (m_stations[i] == station)
        {
            *info = m_infos[i];
            m_criticalSection.Unlock();
            return 0;
        }
    station->Trace(0x4008000);
    m_criticalSection.Unlock();
    return 0x2c09;
}

Result StationConnectionInfoTable::GetStationConnectionInfo(
    unsigned long long principalId, StationConnectionInfo* info) const
{
    if (info == 0) return 0x10c07;
    const unsigned int low = static_cast<unsigned int>(principalId);
    const unsigned int high = static_cast<unsigned int>(principalId >> 32);
    m_criticalSection.Lock();
    for (unsigned int i = 0; i < m_count; ++i)
        if (static_cast<unsigned int>(m_infos[i].m_field10) == low &&
            static_cast<unsigned int>(m_infos[i].m_field14) == high)
        {
            *info = m_infos[i];
            m_criticalSection.Unlock();
            return 0;
        }
    m_criticalSection.Unlock();
    return 0x2c09;
}

Station* StationConnectionInfoTable::GetStation(const StationConnectionInfo& info) const
{
    m_criticalSection.Lock();
    for (unsigned int i = 0; i < m_count; ++i)
        if (m_infos[i] == info)
        {
            Station* station = m_stations[i];
            m_criticalSection.Unlock();
            return station;
        }
    m_criticalSection.Unlock();
    return 0;
}

Station* StationConnectionInfoTable::GetStation(const common::StationAddress& address) const
{
    m_criticalSection.Lock();
    for (unsigned int i = 0; i < m_count; ++i)
        if (static_cast<const common::StationAddress&>(m_infos[i]) == address)
        {
            static_cast<const common::StationAddress&>(m_infos[i]).Trace(0x4008000);
            Station* station = m_stations[i];
            m_criticalSection.Unlock();
            return station;
        }
    m_criticalSection.Unlock();
    return 0;
}

Station* StationConnectionInfoTable::GetStation(unsigned int stationIndex) const
{
    m_criticalSection.Lock();
    for (unsigned int i = 0; i < m_count; ++i)
        if (m_infos[i].m_field1c == static_cast<int>(stationIndex))
        {
            m_infos[i].Trace(0x4008000);
            Station* station = m_stations[i];
            m_criticalSection.Unlock();
            return station;
        }
    m_criticalSection.Unlock();
    return 0;
}

Station* StationConnectionInfoTable::GetStation(unsigned long long principalId) const
{
    m_criticalSection.Lock();
    for (unsigned int i = 0; i < m_count; ++i)
        if (PrincipalId(m_infos[i]) == principalId)
        {
            m_infos[i].Trace(0x4008000);
            Station* station = m_stations[i];
            m_criticalSection.Unlock();
            return station;
        }
    m_criticalSection.Unlock();
    return 0;
}

Station* StationConnectionInfoTable::GetReconnectingStation(
    const StationConnectionInfo& info) const
{
    m_criticalSection.Lock();
    for (unsigned int i = 0; i < m_count; ++i)
    {
        const StationConnectionInfo& candidate = m_infos[i];
        if (candidate.m_field18 != 0 && candidate.m_field10 == info.m_field10 &&
            candidate.m_field14 == info.m_field14 && info.m_field18 != candidate.m_field18)
        {
            Station* station = m_stations[i];
            m_criticalSection.Unlock();
            return station;
        }
    }
    m_criticalSection.Unlock();
    return 0;
}

unsigned int StationConnectionInfoTable::GetStationIndexByPrincipalID(
    unsigned long long principalId) const
{
    const unsigned int low = static_cast<unsigned int>(principalId);
    const unsigned int high = static_cast<unsigned int>(principalId >> 32);
    m_criticalSection.Lock();
    for (unsigned int i = 0; i < m_count; ++i)
    {
        Station* station = m_stations[i];
        if (static_cast<unsigned int>(m_infos[i].m_field10) == low &&
            static_cast<unsigned int>(m_infos[i].m_field14) == high && station != 0 &&
            station->m_field10 < 0x20)
        {
            const unsigned int result = static_cast<unsigned int>(station->m_field10);
            m_criticalSection.Unlock();
            return result;
        }
    }
    m_criticalSection.Unlock();
    return 0xfd;
}

unsigned long long StationConnectionInfoTable::GetPrincipalIdByStation(const Station* station) const
{
    if (station == 0) return 0;
    m_criticalSection.Lock();
    for (unsigned int i = 0; i < m_count; ++i)
        if (m_stations[i] == station)
        {
            const unsigned long long result = PrincipalId(m_infos[i]);
            m_criticalSection.Unlock();
            return result;
        }
    m_criticalSection.Unlock();
    return 0;
}

void StationConnectionInfoTable::EraseFromTable(const Station* station)
{
    if (station == 0) return;
    m_criticalSection.Lock();
    for (unsigned int i = 0; i < m_count; ++i)
        if (m_stations[i] == station)
        {
            m_stations[i] = 0;
            break;
        }
    m_criticalSection.Unlock();
}

void StationConnectionInfoTable::ClearTable()
{
    m_criticalSection.Lock();
    for (unsigned int i = 0; i < 32; ++i) m_stations[i] = 0;
    m_criticalSection.Unlock();
}

void StationConnectionInfoTable::Trace(unsigned long long) const
{
}

}}}
