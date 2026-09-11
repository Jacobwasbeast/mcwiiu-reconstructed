#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_RECONSTRUCTION_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_RECONSTRUCTION_HPP

#include <stdint.h>
#include "nn/pia/ListBase.hpp"
#include "nn/pia/ProtocolMessageFilteringManager.hpp"
#include "nn/pia/NetworkFactory.hpp"

namespace nn { namespace pia {

typedef int Result;
typedef short StationIndex;

namespace common {

class StationAddress;

class Watermark
{
public:
    void SetName(const char* name);
    void Update(long long value);
};

class WatermarkManager
{
public:
    Watermark* GetWatermark(int index);
};

}

namespace transport {

class PacketHandler;
class ProtocolEvent;

extern unsigned short g_piaProtocolDefaultType;

class ProtocolId
{
public:
    ProtocolId(unsigned short value = 0) : mValue(value) {}
    operator unsigned short() const { return mValue; }
    unsigned short value() const { return mValue; }
private:
    unsigned short mValue;
};

class Protocol : public common::ListNode
{
public:
    Protocol();
    virtual ~Protocol();
    virtual void Trace(unsigned long long value) const;
    virtual unsigned short GetProtocolType() const;
    virtual Result StartupWithStationAddress(const common::StationAddress& address);
    virtual Result Startup();
    virtual void CleanupWithStationAddress();
    virtual void Cleanup();
    virtual Result Dispatch();
    virtual Result UpdateProtocolEvent(const ProtocolEvent& event);
    virtual unsigned int IsProtocolFilteringEnabled() const;
    void SetPort(unsigned char port);
    unsigned int GetStationPacketHandler();
protected:
    friend class ProtocolManager;
    ProtocolId mProtocolId;
    unsigned int mStationPacketHandler;
};

class ReceivedDataContainer;

class UnreliableProtocol : public Protocol
{
public:
    enum { cProtocolType = 0x68 };

    UnreliableProtocol();
    virtual ~UnreliableProtocol();
    virtual void Trace(unsigned long long value) const;
    virtual unsigned short GetProtocolType() const;
    void Finalize();
    unsigned int Initialize(unsigned int count);
private:
    unsigned char* m_receivedData;
    ReceivedDataContainer* m_receivedDataContainer;
};

class ReliableSlidingWindow;
class StationManager;

extern common::WatermarkManager* g_piaWatermarkManager;
extern StationManager* g_piaStationManager;

class ReliableProtocol : public Protocol
{
public:
    enum { cProtocolType = 0x7c };

    ReliableProtocol();
    virtual ~ReliableProtocol();
    virtual void Trace(unsigned long long value) const;
    virtual unsigned short GetProtocolType() const;
    virtual Result StartupWithStationAddress(const common::StationAddress& address);
    virtual void CleanupWithStationAddress();
    virtual Result UpdateProtocolEvent(const ProtocolEvent& event);

    void Finalize();
    Result Initialize(unsigned int a1, unsigned int a2);
    ReliableSlidingWindow* GetSlidingWindow(StationIndex stationIndex);
    const ReliableSlidingWindow* GetSlidingWindow(StationIndex stationIndex) const;
    Result Send(StationIndex stationIndex, const void* data, unsigned int size);
    Result Send(unsigned long long stationId, const void* data, unsigned int size);
    void SendToAll(const void* data, unsigned int size);
    Result Receive(unsigned long long* stationId, void* buffer, unsigned int* size, unsigned int bufferSize);
    bool IsInCommunication(StationIndex stationIndex) const;
    bool IsInCommunication(unsigned long long stationId) const;
    void SetThroughputLimit(unsigned int limit);
    static void RegisterResendTimeCallback(int (*callback)(int, int), int arg);

private:
    int m_field14;
    unsigned int m_field18;
    unsigned int m_field1c;
    unsigned int m_field20;
    unsigned char* m_field24;
    unsigned int m_field28;
    unsigned int m_field2c;
    unsigned int m_field30;
};

class ProtocolManager
{
public:
    ProtocolManager();
    virtual ~ProtocolManager();
    void Finalize();
    void Initialize(NetworkFactory* networkFactory);
    Result Startup(PacketHandler* packetHandler);
    Result StartupProtocols();
    void CleanupProtocols();
    UnreliableProtocol* AllocProtocol(unsigned int size);
    ProtocolId CreateProtocolImpl(Protocol* protocol, ProtocolId id);
    Protocol* SearchProtocol(ProtocolId id, unsigned char type);
    void DestroyProtocol(ProtocolId id);
private:
    NetworkFactory* m_networkFactory;
    common::ListBase m_protocols;
    int m_nodeOffset;
    unsigned char m_started;
};
extern ProtocolManager* g_piaProtocolManager;

}

namespace session {

class SessionSearchCriteria
{
public:
    SessionSearchCriteria();
    virtual ~SessionSearchCriteria();
    Result SetResultRange(unsigned int offset, unsigned int size);

    unsigned int m_field00;
    unsigned int m_field04;
    int m_field08;
    int m_field0c;
};

class SessionStationInfo
{
public:
    enum StationInfoRole
    {
        cStationInfoRole_Invalid = 0
    };

    SessionStationInfo();
    ~SessionStationInfo();
    StationInfoRole GetRole() const;
    void SetRole(StationInfoRole role);
    void Clear();
    static unsigned int GetSerializedSize();

    unsigned int m_field00;
    int m_role;
};

}

}}

#endif
