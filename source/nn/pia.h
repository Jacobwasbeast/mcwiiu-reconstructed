#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_H
#define LEGACY_CONSOLE_WIIU_NN_PIA_H

#include "nn/pia/Reconstruction.hpp"
#include <coreinit/fastmutex.h>


namespace nn
{
namespace pia
{
typedef int Result;
typedef short StationIndex;

namespace common
{
class String;

class InetAddress
{
public:
    InetAddress();
    InetAddress(unsigned int address, unsigned short port);
    InetAddress(const InetAddress& other);
    ~InetAddress();
    void operator=(const InetAddress& other);
    unsigned long long GetKey() const;
    void GetAddressString(String* string) const;
    void Trace(unsigned long long value) const;
    void Serialize(unsigned char* buffer, unsigned int* size, unsigned int maxSize) const;
    void Deserialize(const unsigned char* buffer);
    unsigned int IsValid() const;
    bool IsValidAddress() const;
    unsigned int IsPrivate() const;
    bool IsValidPort() const;
private:
    unsigned int m_address;
    unsigned short m_port;
};

class StationAddress : public InetAddress
{
public:
    StationAddress();
    StationAddress(const StationAddress& other);
    virtual ~StationAddress();
    void SetInetAddress(const InetAddress& address);
    void Clear();
    void Serialize(unsigned char* buffer, unsigned int* size, unsigned int maxSize) const;
    void Deserialize(const unsigned char* buffer);
    bool IsValid() const;
    bool operator==(const StationAddress& other) const;
    StationAddress& operator=(const StationAddress& other);
    bool operator<(const StationAddress& other) const;
    bool operator>(const StationAddress& other) const;
    static int Compare(const StationAddress& lhs, const StationAddress& rhs);
    void GetAddressString(String* string) const;
    virtual void Trace(unsigned long long value) const;
    unsigned int GetSerializedSize() const;
};

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

class StationLocation : public common::StationAddress
{
public:
    unsigned int m_field0c;
    int m_field10;
    int m_field14;
    int m_field18;
    int m_field1c;
    unsigned char m_field20;
    unsigned char m_field21;
    unsigned char m_field22;
    unsigned char m_field23;
    unsigned char m_field24;
    unsigned char m_field25;
    unsigned char m_field26;
    common::InetAddress m_inetAddress;
    StationLocation();
    StationLocation(const StationLocation& other);
    virtual ~StationLocation();
    StationLocation& operator=(const StationLocation& other);
    void SetStationLocation(const StationLocation& other);
    void SetStationAddress(const common::StationAddress& address);
    bool operator==(const StationLocation& other) const;
    void GetLocationString(common::String* string) const;
    virtual unsigned int GetSerializedSize() const;
    virtual void Serialize(unsigned char* buffer, unsigned int* size, unsigned int maxSize) const;
    virtual void Deserialize(const unsigned char* buffer);
    virtual void Trace(unsigned long long value) const;
};

class StationConnectionInfo : public StationLocation
{
public:
    StationLocation m_localStationLocation;
    StationConnectionInfo();
    StationConnectionInfo(const StationConnectionInfo& other);
    virtual ~StationConnectionInfo();
    StationConnectionInfo& operator=(const StationConnectionInfo& other);
    void SetStationConnectionInfo(const StationConnectionInfo& other);
    bool operator==(const StationConnectionInfo& other) const;
    virtual unsigned int GetSerializedSize() const;
    virtual void Serialize(unsigned char* buffer, unsigned int* size, unsigned int maxSize) const;
    virtual void Deserialize(const unsigned char* buffer);
    virtual unsigned int GetStreamlinedSerializedSize() const;
    virtual void StreamlinedSerialize(unsigned char* buffer, unsigned int* size, unsigned int maxSize) const;
    virtual void StreamlinedDeserialize(const unsigned char* buffer);
    virtual void Trace(unsigned long long value) const;
    void LittleMembersSerialize(unsigned char* buffer, const StationLocation& location) const;
    void LittleMembersDeserialize(StationLocation* location, const unsigned char* buffer);
};

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

class StationPacketHandler
{
public:
    static StationConnectionInfoTable* GetStationConnectionInfoTable();
};
}

namespace inet
{
class InetProcessHostMigrationJob
{
public:
    void HostChangeEventNotice(unsigned int param);
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
