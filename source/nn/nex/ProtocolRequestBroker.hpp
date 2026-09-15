#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_PROTOCOLREQUESTBROKER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_PROTOCOLREQUESTBROKER_HPP

#include "nn/nex/ProtocolRequestBrokerInterface.hpp"
#include "nn/nex/ProtocolRegistry.hpp"
#include <cstdint>

namespace nn { namespace nex {

class ProtocolRequestBroker : public ProtocolRequestBrokerInterface
{
public:
    ProtocolRequestBroker(); /* address-zero declaration in the Wii U image */
    virtual ~ProtocolRequestBroker();

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
    void PeerDisconnected(EndPoint*);
    void FaultDetection(EndPoint*, unsigned int);

    ProtocolRegistry& GetRegistry();

protected:
    void ProcessMessageCore(CallProtocolMethodOperation*, EndPoint*, Buffer*);
    void DispatchCallToProtocol(ServerProtocol*, Message*, Message*, bool*, EndPoint*);
    void ProcessLocalCallRequest(ServerProtocol*, Message*, Message*, bool*, EndPoint*);
    Operation* GetCurrentOperation() const;
    void TraceProtocols(unsigned long long);
    static bool SystemStateIsValid(bool);
    std::uint32_t m_field44;
    std::uint32_t m_field48;
    std::uint32_t m_field4c;
    std::uint32_t m_field50;
    std::uint32_t m_field54;
    std::uint32_t m_field58;
    std::uint32_t m_field5c;
    std::uint32_t m_field60;
    std::uint32_t m_field64;
    unsigned char m_padding68[84];
    std::uint32_t m_fieldbc;
};

}}

#endif
