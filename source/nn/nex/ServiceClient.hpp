#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_SERVICECLIENT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_SERVICECLIENT_HPP

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class Credentials;
class EndPoint;
class Protocol;
class ProtocolCallContext;
class ProtocolRegistry;
class StreamManager;

class ServiceClient : public RootObject
{
public:
    explicit ServiceClient(unsigned int uiServiceID);
    virtual ~ServiceClient();

    virtual void UseLocalLoopback(unsigned int uiCallerPID, unsigned int uiCallerCID);
    virtual bool Bind(Credentials* pCredentials);
    virtual bool Unbind();
    virtual bool IsConnected() const;
    virtual bool IsFaulty() const;
    virtual void ConnectionStateHasChanged();
    virtual void UpdateProtocolsDefaultCredentials(Credentials* pCredentials) = 0;
    virtual bool SetDependentRvConnection(ProtocolCallContext* pCallContext) const;

    void SetCredentials(Credentials* pCredentials);
    void SetDefaultCredentials(Credentials* pCredentials);
    bool PrepareForConnection(Credentials* pCredentials);
    bool BindLocal(Credentials* pCredentials);
    ProtocolRegistry* GetProtocolRegistry() const;
    void RegisterProtocol(Protocol* pProtocol);
    void RegisterExtraProtocol(Protocol* pProtocol);
    void RegisterExtraProtocol(Protocol* pProtocol, unsigned short uiProtocolID);
    bool UnregisterProtocol(Protocol* pProtocol);
    bool UnregisterExtraProtocol(Protocol* pProtocol);
    EndPoint* GetConnection(unsigned short uiConnectionType) const;
    StreamManager* GetStreamManager() const;

protected:
    friend class BackEndServices;
    friend class AuthenticationClient;
    unsigned int m_uiServiceID;
    Credentials* m_pCredentials;
    bool m_bLocalLoopback;
};

}
}

#endif
