#include "nn/pia/transport/TransportAnalyzer.hpp"

#include <new>
#include <stdint.h>
#include <string.h>

#include "nn/pia/HeapManager.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/common/SessionStateMonitoringContent.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/transport/PacketHandler.hpp"
#include "nn/pia/transport/Station.hpp"
#include "nn/pia/transport/StationManager.hpp"
#include "nn/pia/transport/ThreadStreamManager.hpp"
#include "nn/pia/transport/TransportSupport.hpp"
#include "platform/wiiu/pead.hpp"

namespace nn { namespace pia { namespace transport {

TransportAnalyzer* g_piaTransportAnalyzer = 0;

namespace
{
void* g_transportAnalyzerStorage = 0;
}

TransportAnalyzer::TransportAnalyzer()
    : m_startedUp(false), m_field110c(0)
{
}

TransportAnalyzer::~TransportAnalyzer()
{
}

Result TransportAnalyzer::CreateInstance()
{
    if (IsInitialized() == 0)
        return static_cast<Result>(0x10c0c);
    if (IsInSetupMode() == 0)
        return static_cast<Result>(0x10c08);
    if (g_piaTransportAnalyzer != 0)
        return static_cast<Result>(0x10c0f);

    pead::Heap* heap = common::HeapManager::GetHeap();
    g_transportAnalyzerStorage =
        pead::allocateFromSeadHeap(sizeof(TransportAnalyzer), heap, 8);
    if (g_transportAnalyzerStorage == 0)
        return static_cast<Result>(0x10c01);

    memset(g_transportAnalyzerStorage, 0, sizeof(TransportAnalyzer));
    g_piaTransportAnalyzer = new (g_transportAnalyzerStorage) TransportAnalyzer();
    return 0;
}

void TransportAnalyzer::DestroyInstance()
{
    if (g_piaTransportAnalyzer != 0)
    {
        g_piaTransportAnalyzer->~TransportAnalyzer();
        g_piaTransportAnalyzer = 0;
    }
    if (g_transportAnalyzerStorage != 0)
    {
        pead::freeToSeadHeap(g_transportAnalyzerStorage);
        g_transportAnalyzerStorage = 0;
    }
}

Result TransportAnalyzer::Initialize()
{
    return 0;
}

void TransportAnalyzer::Finalize()
{
}

Result TransportAnalyzer::Startup()
{
    if (m_startedUp)
        return static_cast<Result>(0x10c08);
    if (g_piaThreadStreamManager == 0)
        return static_cast<Result>(0x10c08);

    m_connectionAnalysisData.Clear();
    m_startTime = g_piaTransport->m_time;
    m_startedUp = true;
    return 0;
}

void TransportAnalyzer::Cleanup()
{
    if (!m_startedUp)
        return;
    m_startedUp = false;
}

Result TransportAnalyzer::Update()
{
    if (!m_startedUp || g_piaThreadStreamManager == 0)
        return static_cast<Result>(0x10c08);
    Result result = UpdateConnectionAnalysisData();
    if (result != 0)
        return result;
    PacketHandler* handler = g_piaTransport->m_packetHandler;
    result = handler->GetPacketAnalysisData(&m_packetAnalysisData[0],
        &m_packetAnalysisData[1], &m_packetAnalysisData[2], &m_packetAnalysisData[3]);
    if (result != 0)
        return result;
    handler->ClearPacketAnalysisData();
    m_field110c = g_piaTransport->m_dispatchCount;
    g_piaTransport->m_dispatchCount = 0;
    return 0;
}

Result TransportAnalyzer::UpdateConnectionAnalysisData()
{
    if (!m_startedUp || g_piaStationManager == 0)
        return static_cast<Result>(0x10c08);

    unsigned char seen[ConnectionAnalysisData::cStationNumMax] = {};
    StationManager* const manager = g_piaStationManager;
    Station* const localStation = manager->m_localStation;
    for (unsigned int index = 0; index < manager->m_stations.size; ++index)
    {
        Station* const station = manager->m_stations.data[index];
        if (station == localStation ||
            station->m_field10 >= ConnectionAnalysisData::cStationNumMax)
            continue;

        const unsigned int stationIndex = station->m_field10;
        seen[stationIndex] = 1;
        ConnectionAnalysisData::Entry& entry = m_connectionAnalysisData.m_entries[stationIndex];
        entry.m_rtt = static_cast<unsigned int>(station->GetRtt());
        entry.m_rttMin = static_cast<unsigned int>(station->GetRttMin());
        entry.m_rttMax = static_cast<unsigned int>(station->GetRttMax());
        station->ResetRttMinMax();

        const SequenceIdController& sequence = station->m_sequenceIdController;
        const unsigned int sequenceCount = sequence.m_sequenceCount;
        const unsigned int missingSequenceCount = sequence.m_missingSequenceCount;
        if (!entry.m_isValid)
        {
            entry.m_isValid = true;
            entry.m_prevSendPacketNum = sequenceCount;
            entry.m_sendPacketNum = sequenceCount;
            entry.m_prevLostPacketNum = missingSequenceCount;
            entry.m_lostPacketNum = missingSequenceCount;
        }
        else
        {
            entry.m_prevSendPacketNum = entry.m_sendPacketNum;
            entry.m_sendPacketNum = sequenceCount;
            entry.m_prevLostPacketNum = entry.m_lostPacketNum;
            entry.m_lostPacketNum = missingSequenceCount;
        }
    }

    for (unsigned int index = 0; index < ConnectionAnalysisData::cStationNumMax; ++index)
    {
        if (!seen[index])
            m_connectionAnalysisData.m_entries[index].m_isValid = false;
    }

    common::Time current;
    if (g_piaTransport != 0)
        current = g_piaTransport->m_time;
    else
        current.SetNow();
    const uint64_t deltaBits = current.m_tick - m_startTime.m_tick;
    const int64_t signedDelta = static_cast<int64_t>(deltaBits);
    const int64_t ticksPerMSec = common::TimeSpan::GetTicksPerMSec().m_tick;
    m_connectionAnalysisData.m_passedMilliSecond =
        static_cast<unsigned int>(signedDelta / ticksPerMSec);
    return 0;
}

void TransportAnalyzer::SetMonitoringData()
{
    PacketHandler* const handler = g_piaTransport->m_packetHandler;
    PacketAnalysisData data;
    if (handler->GetPacketAnalysisData(&data, 0, 0, 0) != 0)
        return;

    common::SessionStateMonitoringContent& state = common::g_sessionStateMonitoringContent;
    state.m_metric1[3] = 0;
    state.m_metric20[0] = data.m_totalPacketNum;
    state.m_metric36[0] = data.m_totalPacketSize;
    const int sendEntryNum = data.m_entryNum < 15 ? data.m_entryNum : 15;
    for (int index = 0; index < sendEntryNum; ++index)
    {
        const PacketAnalysisData::Entry& entry = data.m_entries[index];
        state.m_metric1[4 + index] = entry.m_protocolId.value();
        state.m_metric20[1 + index] = entry.m_totalPacketNum;
        state.m_metric36[1 + index] = entry.m_totalPacketSize;
    }

    if (handler->GetPacketAnalysisData(0, 0, 0, &data) != 0)
        return;

    state.m_metric52[0] = 0;
    state.m_metric68[0] = data.m_totalPacketNum;
    state.m_metric84[0] = data.m_totalPacketSize;
    const int receiveEntryNum = data.m_entryNum < 15 ? data.m_entryNum : 15;
    for (int index = 0; index < receiveEntryNum; ++index)
    {
        const PacketAnalysisData::Entry& entry = data.m_entries[index];
        state.m_metric52[1 + index] = entry.m_protocolId.value();
        state.m_metric68[1 + index] = entry.m_totalPacketNum;
        state.m_metric84[1 + index] = entry.m_totalPacketSize;
    }

    StationManager* const manager = g_piaStationManager;
    if (manager == 0 || manager->m_localStation == 0)
        return;

    Station* const localStation = manager->m_localStation;
    unsigned int remoteIndex = 0;
    for (unsigned int index = 0; index < manager->m_stations.size && remoteIndex < 15; ++index)
    {
        Station* const station = manager->m_stations.data[index];
        if (station == localStation)
            continue;

        const unsigned char stationIndex = static_cast<unsigned char>(station->m_field10);
        const unsigned short rtt = static_cast<unsigned short>(station->GetRtt());
        const unsigned int sequenceCount = station->m_sequenceIdController.m_sequenceCount;
        const unsigned int missingSequenceCount = station->m_sequenceIdController.m_missingSequenceCount;
        const unsigned short loss = sequenceCount == 0
            ? static_cast<unsigned short>(0xffff)
            : static_cast<unsigned short>(static_cast<uint64_t>(missingSequenceCount) * 10000 / sequenceCount);
        unsigned long long principalId = 0;
        const Result principalResult = station->GetPrincipalId(&principalId);
        state.m_metric100[remoteIndex] = principalResult == 0
            ? common::hashWithMd5(principalId)
            : common::hashWithMd5(0xffffffffffffffffULL);
        state.m_metric115[remoteIndex] = stationIndex;
        state.m_metric130[remoteIndex] = rtt;
        state.m_metric145[remoteIndex] = loss;
        ++remoteIndex;
    }
}

}}}
