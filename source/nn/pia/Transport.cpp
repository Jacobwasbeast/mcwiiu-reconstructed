#include "nn/pia/Transport.hpp"
#include "nn/pia/transport/StationConnectionInfoTable.hpp"
#include "nn/pia/transport/StationManager.hpp"
#include "nn/pia/transport/NetworkRttManager.hpp"
#include "nn/pia/common/SessionStateMonitoringContent.hpp"
#include "nn/pia/common/SessionBeginMonitoringContent.hpp"
#include "nn/pia/common/IPacketOutput.hpp"
#include "nn/pia/PayloadSizeManager.hpp"
#include "nn/pia/common/Job.hpp"
#include "nn/pia/transport/RelayStationPacketHandler.hpp"
#include "nn/pia/transport/ResendingMessageManager.hpp"
#include "nn/pia/transport/ThreadStreamManager.hpp"
#include "nn/pia/transport/TransportAnalyzer.hpp"
#include "nn/pia/transport/TransportSupport.hpp"
namespace nn { namespace pia { namespace transport {
Transport* g_piaTransport = 0;

common::IPacketOutput* Transport::GetOutputStream()
{
    return g_piaThreadStreamManager ? g_piaThreadStreamManager->m_packetOutput : 0;
}

void Transport::OutputStreamUpdateEvent()
{
    common::IPacketOutput* output = GetOutputStream();
    if (output)
        output->OnStationConnectionEvent();
}

void Transport::SetMonitoringNetworkRtt(bool isSessionBegin)
{
    NetworkRttManager* rttManager = NetworkRttManager::sInstance;
    StationManager* stationManager = g_piaStationManager;
    if (!rttManager || !stationManager)
        return;

    unsigned int minimum = 0xffff;
    unsigned int maximum = 0;
    Station* minimumStation = 0;
    Station* maximumStation = 0;
    for (unsigned int index = 0; index < stationManager->m_stations.size; ++index)
    {
        Station* station = stationManager->m_stations.data[index];
        if (station->m_field20 != 5 || !station->m_stationAddress.IsValid())
            continue;
        unsigned int average = rttManager->GetAverage(station->m_stationAddress);
        if (average == 0)
            continue;
        if (average < minimum)
        {
            minimum = average;
            minimumStation = station;
        }
        if (maximum < average)
        {
            maximum = average;
            maximumStation = station;
        }
    }

    StationConnectionInfoTable* table = g_stationConnectionInfoTable;
    unsigned long long principal = table->GetPrincipalIdByStation(minimumStation);
    common::SessionStateMonitoringContent& state = common::g_sessionStateMonitoringContent;
    common::SessionBeginMonitoringContent& begin = common::g_sessionBeginMonitoringContent;
    unsigned int minimumHash = minimum < 0xffff && principal != 0
        ? common::hashWithMd5(principal) : 0xffffffff;
    unsigned int previousHash = state.m_metric163;
    state.m_metric163 = minimumHash;
    unsigned short minimumRtt = minimum < 0xffff
        ? static_cast<unsigned short>(minimum) : 0xffff;
    if (isSessionBegin)
    {
        begin.m_metric37 = minimumRtt;
        begin.m_metric38 = state.m_metric163;
        state.m_metric163 = previousHash;
    }
    else
        state.m_metric162 = minimumRtt;

    principal = table->GetPrincipalIdByStation(maximumStation);
    unsigned int maximumHash = maximum != 0 && principal != 0
        ? common::hashWithMd5(principal) : 0xffffffff;
    unsigned short maximumRtt = maximum != 0
        ? static_cast<unsigned short>(maximum) : 0xffff;
    if (isSessionBegin)
    {
        begin.m_metric40 = maximumHash;
        begin.m_metric39 = maximumRtt;
    }
    else
    {
        state.m_metric165 = maximumHash;
        state.m_metric164 = maximumRtt;
    }
}

Transport::Transport()
    : m_networkFactory(0), m_packetHandler(0), m_stationNum(0),
      m_startedUp(false), m_field2f(false), m_connectionError(0), m_dispatchJob(0),
      m_field38(0), m_relayRouteManager(0), m_field40(0), m_field50(false), m_field51(false),
      m_field54(0), m_field58(0), m_stationIdTable(0),
      m_transportStatus(static_cast<Result>(0x10c08)), m_localStationId(0)
{
    m_time.SetNow();
}

Result Transport::Startup(unsigned int payloadSize,
                          const common::CryptoSetting* cryptoSetting)
{
    if (IsInSetupMode() != 0)
        return static_cast<Result>(0x10c08);
    if (m_startedUp)
        return static_cast<Result>(0x10c08);

    Result result = common::g_payloadSizeManager->SetMtuSize(
        payloadSize - common::g_payloadSizeManager->m_field0c);
    if (result != 0)
        return result;

    result = g_piaThreadStreamManager->Startup();

    PacketStream::Writer* const writer =
        &g_piaThreadStreamManager->m_sendThreadStream->m_writer;
    PacketStream::Reader* const reader =
        &g_piaThreadStreamManager->m_receiveThreadStream->m_reader;
    common::IPacketInput* const packetInput = g_piaThreadStreamManager->m_packetInput;
    common::IPacketOutput* const packetOutput = g_piaThreadStreamManager->m_packetOutput;

    if (m_networkFactory->IsStarConnection())
    {
        m_packetHandler->Startup(writer, reader, packetInput, packetOutput,
                                 &m_localStationId, cryptoSetting);
    }
    else if (m_networkFactory->IsClientRelayConnectionPermitted())
    {
        static_cast<RelayStationPacketHandler*>(m_packetHandler)
            ->Startup(writer, reader, packetInput, packetOutput, &m_localStationId,
                      m_relayRouteManager, cryptoSetting);
    }
    else
    {
        static_cast<StationPacketHandler*>(m_packetHandler)
            ->Startup(writer, reader, packetInput, packetOutput, &m_localStationId,
                      cryptoSetting);
    }

    result = m_protocolManager.Startup(m_packetHandler);
    result = g_piaResendingMessageManager->Startup(m_packetHandler,
                                                   m_networkFactory->IsStarConnection());
    result = g_piaTransportAnalyzer->Startup();
    m_dispatchJob->Ready(false);

    m_connectionError = 0;
    m_startedUp = true;
    m_startupTime = m_time;
    m_field54 = 0;
    m_field2f = false;

    const Result protocolResult = m_protocolManager.StartupProtocols();
    if (protocolResult != 0)
        return protocolResult;
    return 0;
}

Result Transport::ConvertToStationIndex(StationIndex* index,
                                         std::uint64_t stationId) const
{
    if (index == 0)
        return static_cast<Result>(0x10c07);
    if (m_transportStatus == static_cast<Result>(0x10c08))
        return static_cast<Result>(0x10c08);
    if (m_transportStatus == static_cast<Result>(0x4c11))
        return static_cast<Result>(0x4c11);
    if (stationId == 0)
    {
        *index = static_cast<StationIndex>(0xfd);
        return 0;
    }
    StationIdTable::Entry entry;
    const Result result = m_stationIdTable->FindByStationId(&entry, stationId);
    if (result != 0)
        return result;
    *index = entry.stationIndex;
    return 0;
}
StationIndex Transport::ConvertToStationIndexUnsafe(std::uint64_t stationId) const
{
    return m_stationIdTable->FindUnsafe(stationId);
}

StationIndex Conv2StationIndex(std::uint64_t stationId)
{
    StationIndex index;
    if (g_piaTransport != 0 &&
        g_piaTransport->ConvertToStationIndex(&index, stationId) == 0)
        return index;
    return static_cast<StationIndex>(0xfd);
}
}}}
