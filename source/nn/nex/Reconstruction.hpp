#ifndef SOURCE_NN_NEX_RECONSTRUCTION_HPP
#define SOURCE_NN_NEX_RECONSTRUCTION_HPP

#include <stdint.h>

#include "nn/nex/ConnectivityManager.hpp"
#include "nn/nex/ProviderSupport.hpp"
#include "nn/nex/ServiceClient.hpp"
#include "nn/nex/GlobalVariables.hpp"
#include "nn/nex/NotificationEvent.hpp"
#include "nn/nex/NotificationEventManager.hpp"
#include "nn/nex/NotificationEvents.hpp"
#include "nn/nex/ProtocolCallContext.hpp"
#include "nn/nex/StationURL.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/qList.hpp"
#include "nn/nex/qResult.hpp"

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
    bool UpdateNotificationData(ProtocolCallContext* pCallContext, unsigned int uiType,
                                unsigned int uiParam1, unsigned int uiParam2, String strText);
    static void SetPersistentGatheringCodeKey(const String& rKey);
    static void ClearPersistentGatheringCodeKey();
    bool GetFriendNotificationData(ProtocolCallContext* pCallContext,
                                   qList<NotificationEvent>* pEvents,
                                   const qList<unsigned int>& rTypes);

private:
    bool m_protocolRegistered;
    MatchmakeExtensionProtocolClient m_matchmakeExtensionProtocolClient;
    unsigned long long m_uiLastProgressScoreTime;
};

class ErrorCodeConverter
{
public:
    static unsigned int ConvertToNetworkErrorCode(const qResult& rResult);
};

class SecureConnectionClient
{
public:
    bool ReplaceURL(ProtocolCallContext* pCallContext, const StationURL& rCurrentURL,
                    const StationURL& rNewURL);
    void SendReport(unsigned int uiReportId, const void* pData, unsigned int uiSize);
};

class BackEndServices
{
public:
    SecureConnectionClient* GetSecureConnectionClient() const;
    qResult IsConnected() const;
    bool RegisterNotificationEventHandler(NotificationEventHandler* handler);
    bool UnregisterNotificationEventHandler(NotificationEventHandler* handler);
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

}
}

#endif
