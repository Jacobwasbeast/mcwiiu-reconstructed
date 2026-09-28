#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_PROTOCOLREQUESTBROKERINTERFACE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_PROTOCOLREQUESTBROKERINTERFACE_HPP

#include <stdint.h>

#include "nn/nex/BandwidthCounterMap.hpp"
#include "nn/nex/OperationManager.hpp"
#include "nn/nex/Protocol.hpp"

namespace nn { namespace nex {

class ClientProtocol;
class ServerProtocol;

extern unsigned int (*g_protocolCurrentPID)();
extern unsigned int (*g_protocolCurrentCID)();

struct GhsVTableEntry
{
    uint32_t adjustment;
    uint32_t function;
};

struct GhsVTable
{
    uint32_t header0;
    uint32_t typeInfo;
    uint32_t header2;
    GhsVTableEntry entries[14];
};

class ProtocolRequestBrokerInterface : public RootObject
{
public:
    ProtocolRequestBrokerInterface();
    virtual ~ProtocolRequestBrokerInterface();

    virtual void ProcessMessage(CallProtocolMethodOperation* operation, EndPoint* endPoint,
                        Buffer* buffer) = 0;
    virtual void ProcessCallRequest(unsigned short protocolId, Message* request, Message* response,
                            bool* responseRequired, EndPoint* endPoint) = 0;
    virtual void ProcessCallResponse(unsigned short protocolId, Message* response, EndPoint* endPoint) = 0;
    virtual bool RegisterProtocol(Protocol* protocol) = 0;
    virtual void UnregisterProtocol(Protocol* protocol) = 0;
    virtual Protocol* GetProtocol(unsigned short protocolId, Protocol::_Type type) = 0;
    virtual bool CloneProtocols() = 0;
    virtual bool RegisterServerProtocol(unsigned short protocolId, ServerProtocol* protocol) = 0;
    virtual bool UnregisterServerProtocol(unsigned short protocolId) = 0;
    virtual ServerProtocol* GetServerProtocol(unsigned short protocolId) = 0;
    virtual qResult DispatchLocalRMC(Message* message, ClientProtocol* client,
                             ServerProtocol* server, EndPoint* endPoint) = 0;
    virtual qResult DispatchBlockingLocalRMC(Message* message, ClientProtocol* client,
                                     ServerProtocol* server, EndPoint* endPoint) = 0;
    virtual qResult DispatchThreadSafeLocalRMC(Message* message, ClientProtocol* client,
                                       ServerProtocol* server, EndPoint* endPoint) = 0;

    static void InitMessage(Message* message, unsigned short protocolId, Protocol::_Command command);
    void Receive(EndPoint* endPoint, Buffer* buffer);

    const GhsVTable* GetTailVTable() const
    {
        return reinterpret_cast<const GhsVTable*>(static_cast<uintptr_t>(mVTable));
    }

protected:
    friend class Protocol;

    uint32_t m_reserved0;
    BandwidthCounterMap m_bandwidthCounters;
    OperationManager m_operationManager;
    uint32_t mVTable;
};

}}

#endif
