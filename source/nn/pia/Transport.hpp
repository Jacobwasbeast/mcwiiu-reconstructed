#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_HPP
#include <cstdint>
#include "nn/pia.h"
#include "nn/pia/StationIdTable.hpp"
#include "nn/pia/common/CryptoSetting.hpp"
#include "nn/pia/common/Time.hpp"
namespace nn { namespace pia { namespace common {

class Job;
class IPacketOutput;
}}}
namespace nn { namespace pia { namespace transport {
class NetworkFactory;
class ProtocolManager;
class PacketHandler;
class RelayRouteManager;
class Transport {
public:
    class Setting
    {
    public:
        Setting(NetworkFactory*, unsigned short, unsigned int, unsigned int,
                int, bool, bool);
    };
    Transport();
    ~Transport();
    Result Startup(unsigned int payloadSize, const common::CryptoSetting* cryptoSetting);
    Result UpdateSetting(const common::CryptoSetting* cryptoSetting);
    Result dispatch();
    void updateTransportAnalyzer();
    common::IPacketOutput* GetOutputStream();
    void OutputStreamUpdateEvent();
    void Cleanup();
    Result Restartup(const common::CryptoSetting* cryptoSetting);
    void SetKeepAliveInterval(int interval);
    int GetKeepAliveInterval() const;
    void EnableKeepAlive();
    void DisableKeepAlive();
    bool IsKeepAliveEnabled() const;
    Result GetLocalStationConnectionInfo(StationConnectionInfo* stationConnectionInfo);
    Result GetHostStationConnectionInfo(StationConnectionInfo* stationConnectionInfo);
    void SetMonitoringNetworkRtt(bool isSessionBegin);
    Result ConvertToStationIndex(StationIndex*, std::uint64_t) const;
    StationIndex ConvertToStationIndexUnsafe(std::uint64_t stationId) const;
    Result ConvertToStationId(std::uint64_t* stationId, StationIndex stationIndex) const;
    std::uint64_t ConvertToStationIdUnsafe(StationIndex stationIndex) const;
    void SetTransportStatus(Result transportStatus);
    void Trace(unsigned long long value) const;

    common::Time m_time;
    NetworkFactory* m_networkFactory;
    ProtocolManager m_protocolManager;
    PacketHandler* m_packetHandler;
    unsigned short m_stationNum;
    bool m_startedUp;
    bool m_connectionRouteRelay;
    Result m_connectionError;
    common::Job* m_dispatchJob;
    unsigned int m_connectionState;
    RelayRouteManager* m_relayRouteManager;
    int m_analysisInterval;
    common::Time m_lastAnalysisTime;
    bool m_keepAliveEnabled;
    bool m_monitoringEnabled;
    unsigned int m_dispatchCount;
    unsigned int m_lastConnectionResult;
    StationIdTable* m_stationIdTable;
    unsigned int m_stationConnectionState;
    Result m_transportStatus;
    std::uint64_t m_localStationId;
};
extern Transport* g_piaTransport;
StationIndex Conv2StationIndex(std::uint64_t);
}}}
#endif
