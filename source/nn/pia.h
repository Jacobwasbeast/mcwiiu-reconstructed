#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_H
#define LEGACY_CONSOLE_WIIU_NN_PIA_H

#include "nn/pia/Reconstruction.hpp"
#include "nn/pia/common/StationAddress.hpp"
#include "nn/pia/inet/InetProcessHostMigrationJob.hpp"
#include <coreinit/fastmutex.h>


namespace nn
{
namespace pia
{
typedef int Result;
typedef short StationIndex;

namespace common
{
class CriticalSection
{
public:
    CriticalSection(int value);
    ~CriticalSection();
    void Lock();
    bool TryLock();
    void Unlock();
    void Trace(unsigned long long value) const;
private:
    OSFastMutex m_mutex;
};
}

namespace transport
{
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

class SequenceIdController
{
public:
    SequenceIdController();
    virtual ~SequenceIdController();
    void Startup();
    unsigned short GetNextSendSequenceId();
    bool CheckReceivedSequenceId(unsigned short sequenceId);
    void StartProfile();
    void StopProfile();
    virtual void Trace(unsigned long long value) const;
    void Cleanup();
    unsigned int CountReceivedToAllPacket();
private:
    unsigned char m_data[20];
};

class Station
{
public:
    struct IdentificationToken;
    struct PlayerInfo;
    enum StationState : unsigned int;
    Station();
    virtual ~Station();
    void Initialize(NetworkFactory* networkFactory, StationProtocol* stationProtocol, RttProtocol* rttProtocol);
    void Finalize();
    void SetStationIdAndIndex(unsigned long long id, ::nn::pia::StationIndex index);
    void helperStartupCleanup(bool startup, unsigned long long id, ::nn::pia::StationIndex index,
                              const common::StationAddress& address, StationState state);
    void Startup(unsigned long long id, ::nn::pia::StationIndex index, const common::StationAddress& address);
    void Startup(const common::StationAddress& address);
    void Startup();
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
    void GetPrincipalId(unsigned long long* principalId);
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
    int m_field28;
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

}

namespace chat
{
Result Initialize();
Result BeginSetup();
Result EndSetup();
Result Finalize();
}
void Assert(const char* expression, const char* file, unsigned int line,
            const char* function, ...);
}
}

#endif
