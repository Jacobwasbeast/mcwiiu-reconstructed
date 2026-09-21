#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_STATION_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_STATION_HPP

#include "nn/pia/Common.hpp"
#include "nn/pia/common/StationAddress.hpp"
#include "nn/pia/String.hpp"
#include "nn/pia/transport/SequenceIdController.hpp"

namespace nn { namespace pia { namespace transport {

class NetworkFactory;
class StationProtocol;
class RttProtocol;
class ReliableSlidingWindow;
class ConnectStationJob;
class DisconnectStationJob;
class ProcessConnectionRequestJob;
class StationLocation;
class StationConnectionInfo;
class StationConnectionInfoTable;

class Station
{
public:
    struct IdentificationToken;
    struct PlayerInfo;
    enum StationState : unsigned int;
    Station();
    virtual ~Station();
    Result Initialize(NetworkFactory* networkFactory, StationProtocol* stationProtocol, RttProtocol* rttProtocol);
    void Finalize();
    void SetStationIdAndIndex(unsigned long long id, ::nn::pia::StationIndex index);
    void helperStartupCleanup(bool startup, unsigned long long id, ::nn::pia::StationIndex index,
                              const common::StationAddress& address, StationState state);
    bool Startup(unsigned long long id, ::nn::pia::StationIndex index, const common::StationAddress& address);
    bool Startup(const common::StationAddress& address);
    bool Startup();
    void Cleanup();
    int GetRtt() const;
    int GetRtt(unsigned int index) const;
    int GetLatestRtt() const;
    void ResetRttMinMax();
    int GetRttMin() const;
    int GetRttMax() const;
    void CleanupJobs();
    static void SetRttPulseInterval(int interval);
    static int GetRttPulseInterval();
    static void SetInitialRttPulseInterval(int interval);
    static int GetInitialRttPulseInterval();
    int GetRttSamplingNum() const;
    void GetIdentificationToken(IdentificationToken* token);
    void GetPlayerInfo(unsigned char index, PlayerInfo* info);
    Result GetPrincipalId(unsigned long long* principalId);
    void GetPlayerCount(unsigned char* count);
    void GetParticipantCount(unsigned char* count);
    bool IsConnectionRouteRelay() const;
    bool IsConnectionRouteDirect() const;
    unsigned char m_field0;
    common::StationAddress m_stationAddress;
    unsigned int m_field10;
    unsigned int m_field14;
    unsigned int m_field18;
    unsigned int m_field1c;
    unsigned int m_field20;
    ReliableSlidingWindow* m_reliableSlidingWindow;
    StationProtocol* m_field28;
    RttProtocol* m_rttProtocol;
    ConnectStationJob* m_connectStationJob;
    DisconnectStationJob* m_disconnectStationJob;
    ProcessConnectionRequestJob* m_processConnectionRequestJob;
    SequenceIdController m_sequenceIdController;
    unsigned char m_field54;
    unsigned char m_field55;
    unsigned char m_field56;
    int m_field58;
    int m_field5c;
    int m_field60;
    int m_field64;
    unsigned char m_field68;
    virtual void Trace(unsigned long long value) const;
};

struct Station::IdentificationToken
{
    char m_token[32];
};

struct Station::PlayerInfo
{
    PlayerInfo();

    common::FixedString<20> m_name;
    unsigned char m_language;
    unsigned long long m_field70;
    unsigned long long m_field78;
};

}}}

#endif
