#include "nn/pia/Transport.hpp"
#include "nn/pia/PayloadSizeManager.hpp"
#include "nn/pia/common/Job.hpp"
#include "nn/pia/transport/RelayStationPacketHandler.hpp"
#include "nn/pia/transport/ResendingMessageManager.hpp"
#include "nn/pia/transport/ThreadStreamManager.hpp"
#include "nn/pia/transport/TransportAnalyzer.hpp"
#include "nn/pia/transport/TransportSupport.hpp"
namespace nn { namespace pia { namespace transport {
Transport* g_piaTransport = 0;
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
StationIndex Conv2StationIndex(std::uint64_t stationId)
{
    StationIndex index;
    if (g_piaTransport != 0 &&
        g_piaTransport->ConvertToStationIndex(&index, stationId) == 0)
        return index;
    return static_cast<StationIndex>(0xfd);
}
}}}
