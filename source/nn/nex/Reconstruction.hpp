#ifndef SOURCE_NN_NEX_RECONSTRUCTION_HPP
#define SOURCE_NN_NEX_RECONSTRUCTION_HPP

#include <cstdint>

#include "nn/nex/ConnectivityManager.hpp"
#include "nn/nex/ProviderSupport.hpp"
#include "nn/nex/ServiceClient.hpp"
#include "nn/nex/GlobalVariables.hpp"
#include "nn/nex/NotificationEventManager.hpp"

namespace nn
{
namespace nex
{
class CallContext;
class Credentials;

class MatchMakingClient : public ServiceClient
{
public:
    MatchMakingClient();
    virtual ~MatchMakingClient();

    virtual void UpdateProtocolsDefaultCredentials(Credentials* pCredentials);

protected:
    MatchMakingProtocolClient m_matchMakingProtocolClient;
    MatchMakingProtocolExtClient m_matchMakingProtocolExtClient;
};

class MatchmakeExtensionProtocolClient : public ClientProtocol
{
public:
    explicit MatchmakeExtensionProtocolClient(unsigned short protocolId);
    virtual ~MatchmakeExtensionProtocolClient();

    virtual void ExtractCallSpecificResults(Message* pMessage, ProtocolCallContext* pCallContext);
    virtual Protocol* CreateResponder() const;
};

class MatchmakeExtensionClient : public MatchMakingClient
{
public:
    MatchmakeExtensionClient();
    virtual ~MatchmakeExtensionClient();

    virtual bool Bind(Credentials* pCredentials);
    virtual bool Unbind();

private:
    bool m_protocolRegistered;
    MatchmakeExtensionProtocolClient m_matchmakeExtensionProtocolClient;
    unsigned long long m_uiLastProgressScoreTime;
};

class BackEndServices
{
public:
    std::uint32_t RegisterNotificationEventHandler(NotificationEventHandler* handler);
    std::uint32_t UnregisterNotificationEventHandler(NotificationEventHandler* handler);
    Credentials* GetCredentials();

private:
    NotificationEventManager* m_notificationEventManager;
    Credentials* m_credentials;
};

class RendezVous : public BackEndServices
{
public:
    RendezVous();
    virtual ~RendezVous();
};

class NgsFacade : public RendezVous
{
public:
};

class EventLog
{
public:
    static EventLog* GetInstance();
    void SetLogLevel(int level);
};

class TraceLog
{
public:
    static TraceLog* GetInstance();
};

}
}

#endif
