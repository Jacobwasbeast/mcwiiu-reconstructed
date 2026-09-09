#ifndef SOURCE_NN_NEX_CLIENTPROTOCOL_HPP
#define SOURCE_NN_NEX_CLIENTPROTOCOL_HPP

#include "nn/nex/Protocol.hpp"

namespace nn
{
namespace nex
{

class Credentials;

class ClientProtocol : public Protocol
{
public:
    explicit ClientProtocol(unsigned int uiFlags);
    virtual ~ClientProtocol();

    virtual const wchar_t* GetType() const;
    virtual bool IsAKindOf(const wchar_t* pType) const;
    virtual void EnforceDeclareSysComponentMacro();
    virtual _Type GetProtocolType() const;
    virtual void ExtractCallSpecificResults(Message* pMessage, ProtocolCallContext* pCallContext) = 0;
    virtual Protocol* CreateResponder() const = 0;
    virtual void SetDefaultCredentials(Credentials* pCredentials);

    bool SendRMCMessage(ProtocolCallContext* pCallContext, Message* pMessage);
    bool SendOverLocalLoopback(ProtocolCallContext* pCallContext, Message* pMessage);
    void ProcessResponse(Message* pMessage, EndPoint* pEndPoint);

    static void SetCallError(qResult result);

protected:
    Credentials* m_pDefaultCredentials;
};

}
}

#endif
