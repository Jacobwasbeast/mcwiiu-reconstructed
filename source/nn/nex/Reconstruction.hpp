#ifndef SOURCE_NN_NEX_RECONSTRUCTION_HPP
#define SOURCE_NN_NEX_RECONSTRUCTION_HPP

#include <cstdint>

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

protected:
    MatchMakingProtocolClient m_matchMakingProtocolClient;
    MatchMakingProtocolExtClient m_matchMakingProtocolExtClient;
};

class MatchmakeExtensionProtocolClient : public ClientProtocol
{
public:
    explicit MatchmakeExtensionProtocolClient(unsigned short protocolId);
    virtual ~MatchmakeExtensionProtocolClient();
};

class ConnectivityManager
{
public:
    static ConnectivityManager* GetInstanceWithoutNetworkCheck();
    void OnStartNATSession(CallContext* context);
};

class MatchmakeExtensionClient : public MatchMakingClient
{
public:
    MatchmakeExtensionClient();
    int Bind(Credentials* credentials);
    void Unbind();

private:
    bool m_protocolRegistered;
    MatchmakeExtensionProtocolClient m_matchmakeExtensionProtocolClient;
    std::uint32_t m_stateF0;
    std::uint32_t m_stateF4;
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
