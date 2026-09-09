#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_RECONSTRUCTION_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_RECONSTRUCTION_HPP

#include <stdint.h>
#include "nn/pia/ListBase.hpp"
#include "nn/pia/ProtocolMessageFilteringManager.hpp"
#include "nn/pia/NetworkFactory.hpp"

namespace nn { namespace pia { namespace transport {

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
    virtual void CleanupWithStationAddress();
    virtual unsigned int IsProtocolFilteringEnabled() const;
    void SetPort(unsigned char port);
    unsigned int GetStationPacketHandler();
    void Cleanup();
protected:
    friend class ProtocolManager;
    ProtocolId mProtocolId;
    unsigned int mStationPacketHandler;
};

class ReceivedDataContainer;

class UnreliableProtocol : public Protocol
{
public:
    UnreliableProtocol();
    virtual ~UnreliableProtocol();
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
