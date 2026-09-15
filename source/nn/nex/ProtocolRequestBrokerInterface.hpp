#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_PROTOCOLREQUESTBROKERINTERFACE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_PROTOCOLREQUESTBROKERINTERFACE_HPP

#include <cstdint>

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
    std::uint32_t adjustment;
    std::uint32_t function;
};

struct GhsVTable
{
    std::uint32_t header0;
    std::uint32_t typeInfo;
    std::uint32_t header2;
    GhsVTableEntry entries[14];
};

class ProtocolRequestBrokerInterface : public RootObject
{
public:
    ProtocolRequestBrokerInterface();
    ~ProtocolRequestBrokerInterface();

    void ProcessMessage(CallProtocolMethodOperation* operation, EndPoint* endPoint,
                        Buffer* buffer);
    void ProcessCallRequest(unsigned short protocolId, Message* request, Message* response,
                            bool* responseRequired, EndPoint* endPoint);
    void ProcessCallResponse(unsigned short protocolId, Message* response, EndPoint* endPoint);
    bool RegisterProtocol(Protocol* protocol);
    void UnregisterProtocol(Protocol* protocol);
    Protocol* GetProtocol(unsigned short protocolId, Protocol::_Type type);
    bool CloneProtocols();
    bool RegisterServerProtocol(unsigned short protocolId, ServerProtocol* protocol);
    bool UnregisterServerProtocol(unsigned short protocolId);
    ServerProtocol* GetServerProtocol(unsigned short protocolId);
    qResult DispatchLocalRMC(Message* message, ClientProtocol* client,
                             ServerProtocol* server, EndPoint* endPoint);
    qResult DispatchBlockingLocalRMC(Message* message, ClientProtocol* client,
                                     ServerProtocol* server, EndPoint* endPoint);
    qResult DispatchThreadSafeLocalRMC(Message* message, ClientProtocol* client,
                                       ServerProtocol* server, EndPoint* endPoint);

    static void InitMessage(Message* message, unsigned short protocolId, Protocol::_Command command);
    void Receive(EndPoint* endPoint, Buffer* buffer);

    const GhsVTable* GetTailVTable() const
    {
        return reinterpret_cast<const GhsVTable*>(static_cast<std::uintptr_t>(mVTable));
    }

protected:
    friend class Protocol;

    std::uint32_t m_reserved0;
    BandwidthCounterMap m_bandwidthCounters;
    OperationManager m_operationManager;
    std::uint32_t mVTable;
};

}}

#endif
