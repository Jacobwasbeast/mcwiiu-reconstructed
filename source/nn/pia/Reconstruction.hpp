#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_RECONSTRUCTION_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_RECONSTRUCTION_HPP

#include <stdint.h>
#include "nn/pia/ListBase.hpp"
#include "nn/pia/ProtocolMessageFilteringManager.hpp"
#include "nn/pia/NetworkFactory.hpp"

namespace nn { namespace pia {

typedef int Result;

namespace common { class StationAddress; }

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

}}}

#endif
