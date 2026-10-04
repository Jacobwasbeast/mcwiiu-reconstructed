#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_CLIENTPROTOCOLREQUESTBROKER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_CLIENTPROTOCOLREQUESTBROKER_HPP

#include "nn/nex/ProtocolRequestBrokerInterface.hpp"
#include "nn/nex/SingleThreadCallPolicy.hpp"

namespace nn { namespace nex {

class ClientProtocolRequestBroker : public ProtocolRequestBrokerInterface
{
public:
    ClientProtocolRequestBroker();
    virtual ~ClientProtocolRequestBroker();

    virtual void ProcessMessage(CallProtocolMethodOperation*, EndPoint*, Buffer*);
    virtual void ProcessCallRequest(unsigned short, Message*, Message*, bool*, EndPoint*);
    virtual void ProcessCallResponse(unsigned short, Message*, EndPoint*);
    virtual bool RegisterProtocol(Protocol*);
    virtual void UnregisterProtocol(Protocol*);
    virtual Protocol* GetProtocol(unsigned short, Protocol::_Type);
    virtual bool CloneProtocols();
    virtual bool RegisterServerProtocol(unsigned short, ServerProtocol*);
    virtual bool UnregisterServerProtocol(unsigned short);
    virtual ServerProtocol* GetServerProtocol(unsigned short);
    virtual qResult DispatchLocalRMC(Message*, ClientProtocol*, ServerProtocol*, EndPoint*);
    virtual qResult DispatchBlockingLocalRMC(Message*, ClientProtocol*, ServerProtocol*, EndPoint*);
    virtual qResult DispatchThreadSafeLocalRMC(Message*, ClientProtocol*, ServerProtocol*, EndPoint*);
    virtual void PeerDisconnected(EndPoint*);
    virtual void FaultDetection(EndPoint*, unsigned int);
    Operation* GetCurrentOperation();
    static bool SystemStateIsValid(bool);

private:
    void ProcessMessageCore(CallProtocolMethodOperation*, EndPoint*, Buffer*);
    void DispatchCallToProtocol(ServerProtocol*, Message*, Message*, bool*, EndPoint*);
    void ProcessLocalCallRequest(ServerProtocol*, Message*, Message*, bool*, EndPoint*);
    uint32_t m_serverTree;
    uint32_t m_field48;
    uint32_t m_field4c;
    uint32_t m_currentEndPoint;
    SingleThreadCallPolicy m_callPolicy;
};

}}

#endif
