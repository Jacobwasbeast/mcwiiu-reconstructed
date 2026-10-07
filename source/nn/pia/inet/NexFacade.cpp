#include "nn/pia/inet/NexFacade.hpp"
#include "nn/nex/InetAddress.hpp"
#include "nn/nex/StationURL.hpp"
#include "nn/nex/qList.hpp"
#include "nn/pia/common/InetAddress.hpp"
#include "nn/nex/NgsBridgeInterface.hpp"
#include "nn/nex/Reconstruction.hpp"
#include "nn/pia/inet/NexNatRelay.hpp"
#include "nn/pia/inet/NexNatServerAddressResolveJob.hpp"
#include "nn/pia/inet/NexNotificationDataJob.hpp"
#include "nn/pia/nat/NatTraverser.hpp"
#include "nn/pia/session/Mesh.hpp"
#include "nn/pia/transport/StationConnectionInfo.hpp"

namespace nn { namespace pia { namespace inet {

namespace {
enum
{
    cRandomPortRangeMin = 0xc000,
    cRandomPortRangeMax = 0xffff,
    cResultInvalidArgument = 0x10c07,
    cResultInvalidState = 0x10c08,
    cTraceStationLocation = 0x1000
};

common::InetAddress s_noRelayServerAddress;
}

NexFacade* NexFacade::s_instance = 0;
unsigned char* NexFacade::s_instanceMemory = 0;

NexFacade::NexFacade()
    : m_natSessionContext(), m_natTraverserStartupContext(), m_natServerAddressResolveContext(),
      m_started(false), m_natSessionActive(false), m_relayServiceEnabled(false),
      m_loginValue(0), m_ngsBridge(0), m_gatheringId(0), m_matchmakeExtensionMemory(0),
      m_matchmakeExtensionClient(0), m_matchmakeSession(0), m_natRelayClientMemory(0),
      m_natRelayClient(0), m_natRelay(0), m_natTraverser(0), m_socket(0), m_outputStream(0),
      m_inputStream(0), m_natTraversalTimeout(5000), m_natTraversalTimeoutEdmExtension(1500),
      m_bossDaemonSuspendEnabled(true), m_bossDaemonSuspendCount(0), m_notificationEventHandler(0),
      m_notificationDataJob(0), m_asyncProcess(0), m_natServerAddressResolveJob(0),
      m_localStationUrl(0), m_publicStationUrl(0), m_replaceUrlContext(0),
      m_ignoreNatProbeRequest(true), m_localServiceConnectionId(0)
{
}

unsigned int NexFacade::GetRandomPortRangeMin() const
{
    return cRandomPortRangeMin;
}

unsigned int NexFacade::GetRandomPortRangeMax() const
{
    return cRandomPortRangeMax;
}

void NexFacade::RelayProbeRequest(const transport::StationLocation& source,
                                  const transport::StationLocation& destination)
{
    if (m_natRelay != 0)
        m_natRelay->RelayProbeRequest(source, destination);
}

void NexFacade::UpdateRelayTable()
{
    if (m_natRelay != 0)
        m_natRelay->UpdateRelayTable();
}

unsigned int NexFacade::GetLocalServiceConnectionId() const
{
    return m_localServiceConnectionId;
}

bool NexFacade::IsBeginMonitoringDataAvailable() const
{
    bool available = true;
    session::Mesh* mesh = session::g_piaMesh;
    if (mesh != 0 && (mesh->IsProcessedAtMeshBegin() || mesh->IsProcessingLeaveMesh()))
        available = false;
    return available;
}

const common::InetAddress& NexFacade::GetRelayServerAddress() const
{
    if (m_natRelay != 0)
        return m_natRelay->GetRelayServerAddress();
    return s_noRelayServerAddress;
}

bool NexFacade::IsStartNatSessionCompleted() const
{
    if (m_asyncProcess == 1)
        return static_cast<unsigned int>(m_natSessionContext.m_state) - 2 <= 2;
    return false;
}

Result NexFacade::GetStartNatSessionResult() const
{
    if (static_cast<unsigned int>(m_natSessionContext.m_state) - 2 > 2 || m_asyncProcess != 1)
        return cResultInvalidState;
    return m_natSessionContext.m_result;
}

Result NexFacade::CancelStartNatSessionAsync()
{
    if (m_natSessionContext.m_state != common::CallContext::cStateInProgress || m_asyncProcess != 1)
        return cResultInvalidState;
    m_natSessionContext.Cancel();
    if (m_natTraverserStartupContext.m_state == common::CallContext::cStateInProgress)
        m_natTraverserStartupContext.Cancel();
    if (m_natServerAddressResolveContext.m_state == common::CallContext::cStateInProgress)
        m_natServerAddressResolveContext.Cancel();
    return 0;
}

void NexFacade::Unbind()
{
    m_loginValue = 0;
    m_gatheringCodeKey.Clear();
    if (m_matchmakeExtensionClient != 0)
    {
        m_matchmakeExtensionClient->Unbind();
        m_matchmakeExtensionClient->~MatchmakeExtensionClient();
        m_matchmakeExtensionClient = 0;
    }
    m_ngsBridge = 0;
}

unsigned long long NexFacade::GetLocalPrincipalId() const
{
    unsigned int principalId = 0;
    if (m_ngsBridge != 0)
        principalId = m_ngsBridge->GetPrincipalID();
    return principalId;
}

bool NexFacade::RegisterNexNotificationEventHandler4Pia(nex::NotificationEventHandler* handler)
{
    if (m_ngsBridge != 0 && m_ngsBridge->RegisterNotificationEventHandler(handler))
    {
        m_notificationEventHandler = handler;
        return true;
    }
    return false;
}

bool NexFacade::UnregisterNexNotificationEventHandler4Pia(nex::NotificationEventHandler* handler)
{
    if (m_ngsBridge != 0 && m_ngsBridge->UnregisterNotificationEventHandler(handler))
    {
        m_notificationEventHandler = 0;
        return true;
    }
    return false;
}

void NexFacade::SetPersistentGatheringCodeKey()
{
    if (m_gatheringCodeKey.GetEncodingType() == common::String::EncodingType_Utf16)
    {
        nex::MatchmakeExtensionClient::SetPersistentGatheringCodeKey(
            nex::String(static_cast<const wchar_t*>(m_gatheringCodeKey.GetBuffer())));
    }
    else
    {
        nex::MatchmakeExtensionClient::SetPersistentGatheringCodeKey(
            nex::String(static_cast<const char*>(m_gatheringCodeKey.GetBuffer())));
    }
}

void NexFacade::ClearPersistentGatheringCodeKey()
{
    nex::MatchmakeExtensionClient::ClearPersistentGatheringCodeKey();
}

Result NexFacade::UpdateNotificationDataAsync(nex::NotificationEvents::NotificationEvents type,
                                              unsigned int param1, unsigned int param2,
                                              const common::String& text)
{
    if (m_ngsBridge == 0)
        return cResultInvalidState;
    if (m_natSessionContext.m_state == common::CallContext::cStateInProgress)
        return cResultInvalidState;
    m_natSessionContext.Reset();
    const Result result = m_notificationDataJob->Startup(&m_natSessionContext, type, param1, param2, text);
    if (result != 0)
        return result;
    m_asyncProcess = 2;
    m_notificationDataJob->Ready(false);
    return 0;
}

Result NexFacade::RetrieveNotificationDataAsync(unsigned char typeMask,
                                                nex::qList<nex::NotificationEvent>* events)
{
    if (m_ngsBridge == 0)
        return cResultInvalidState;
    if (events == 0)
        return cResultInvalidArgument;
    if (typeMask == 0)
        return cResultInvalidArgument;
    if (m_natSessionContext.m_state == common::CallContext::cStateInProgress)
        return cResultInvalidState;
    m_natSessionContext.Reset();
    const Result result = m_notificationDataJob->Startup(&m_natSessionContext, typeMask, events);
    if (result != 0)
        return result;
    m_asyncProcess = 3;
    m_notificationDataJob->Ready(false);
    return 0;
}

void NexFacade::CancelCleanupNatServerAddressResolution()
{
    if (m_natServerAddressResolveJob->IsRunning())
    {
        if (m_natServerAddressResolveJob->IsBackground())
        {
            m_natServerAddressResolveJob->m_cancelRequested = true;
            do
            {
                m_natServerAddressResolveJob->WaitForCompletion(5);
                if (!m_natServerAddressResolveJob->IsRunning())
                    break;
            } while (m_natServerAddressResolveJob->IsBackground());
        }
        m_natServerAddressResolveJob->CancelCleanup();
        m_natServerAddressResolveJob->Reset(false);
    }
}

void NexFacade::CompletedNatServerAddressResolution()
{
    if (m_natServerAddressResolveContext.m_state == common::CallContext::cStateCancelled)
    {
        if (m_natSessionContext.m_state == common::CallContext::cStateInProgress)
            m_natSessionContext.SignalCancel();
    }
    else
    {
        const Result resolveResult = m_natServerAddressResolveContext.m_result;
        if (resolveResult == 0)
        {
            m_natTraverserStartupContext.RegisterCompletionCallback(NatTraverserStartupCompletionCallback, 0);
            const Result startupResult = m_natTraverser->Startup(this, &m_natTraverserStartupContext);
            if (startupResult != 0)
            {
                m_natSessionContext.SignalFailure(startupResult);
                return;
            }
            m_natTraverserStartupContext.InitiateCall();
        }
        else if (m_natSessionContext.m_state == common::CallContext::cStateInProgress)
        {
            m_natSessionContext.SignalFailure(resolveResult);
        }
    }
}

void NexFacade::UpdateGatheringID(unsigned int gatheringId)
{
    m_gatheringId = gatheringId;
}

bool NexFacade::IsRelayServiceEnabled() const
{
    return m_relayServiceEnabled;
}

void NexFacade::SetIgnoreNatProbeRequest(bool ignore)
{
    m_ignoreNatProbeRequest = ignore;
}

void NexFacade::NotifyLocalServiceConnectionId(unsigned int connectionId)
{
    m_localServiceConnectionId = connectionId;
}

void NexFacade::NotifyProbeRequest(const transport::StationLocation& location)
{
    if (m_ignoreNatProbeRequest)
        return;
    if (m_natTraverser == 0)
        return;
    m_natTraverser->ProcessProbeRequest(location);
}

bool NexFacade::IsTraversed(const transport::StationLocation& location) const
{
    if (m_natTraverser == 0)
        return false;
    return m_natTraverser->IsTraversed(location);
}

bool NexFacade::IsTraversed(unsigned int index) const
{
    if (m_natTraverser == 0)
        return false;
    return m_natTraverser->IsTraversed(index);
}

bool NexFacade::PrepareNatTraversal(const transport::StationLocation& location,
                                    const transport::StationConnectionInfo& info,
                                    common::CallContext* context, bool relay, bool passive,
                                    StationIndex stationIndex)
{
    if (m_natTraverser == 0)
        return false;
    return m_natTraverser->PrepareNatTraversal(location, info, context, relay, passive,
                                                 stationIndex == cStationIndexSentinelFE);
}

bool NexFacade::CheckLatestStationLocation(transport::StationLocation& location)
{
    if (m_natTraverser == 0)
        return 0;
    return m_natTraverser->CheckLatestStationLocation(location);
}

void NexFacade::ResetProbeStatus(const transport::StationLocation& location, bool relay)
{
    if (m_natTraverser != 0)
        m_natTraverser->ResetProbeStatus(location, relay);
}

void NexFacade::ReportNatTraversalResult(const transport::StationLocation& location, Result result, bool relay)
{
    if (m_natTraverser != 0)
        m_natTraverser->ReportNatTraversalResult(location, result, relay);
}

void NexFacade::NatServerAddressResolveCompletionCallback(Result, void*)
{
    s_instance->CompletedNatServerAddressResolution();
}

void NexFacade::NatTraverserStartupCompletionCallback(Result, void*)
{
    s_instance->CompleteStartNatSession();
}

void NexFacade::ConvertNexInetAddressToInetAddress(const nex::InetAddress& source, common::InetAddress* destination)
{
    destination->m_address = source.GetAddress();
    destination->m_port = source.GetPortNumber();
}

void NexFacade::ConvertInetAddressToNexInetAddress(const common::InetAddress& source, nex::InetAddress* destination)
{
    destination->SetAddress(source.m_address);
    destination->SetPortNumber(source.m_port);
}

void NexFacade::ConvertNexStationUrlToStationLocation(const nex::StationURL& url, transport::StationLocation* location)
{
    common::InetAddress address;
    ConvertNexInetAddressToInetAddress(*url.GetInetAddress(), &address);
    location->m_stationAddress.SetInetAddress(address);
    location->m_principalId = url.GetPrincipalID();
    location->m_field18 = url.GetConnectionID();
    location->m_field1c = url.GetRVConnectionID();
    location->m_field20 = url.GetURLType();
    location->m_field21 = url.GetStreamID();
    location->m_field22 = url.GetStreamType();
    location->m_field23 = url.GetNATMapping();
    location->m_field24 = url.GetNATFiltering();
    location->m_field25 = url.GetType();
    location->m_field26 = url.GetProbeRequestInitiation();
    if (url.GetUseRelayServer())
    {
        nex::InetAddress relayAddress;
        relayAddress.SetAddress(static_cast<const wchar_t*>(url.GetRelayServerAddress()));
        ConvertNexInetAddressToInetAddress(relayAddress, &address);
        if (address.m_address == 0)
        {
            address.m_address = 0;
            address.m_port = 0;
        }
        else
        {
            address.m_port = url.GetRelayServerPort();
        }
        location->m_inetAddress = address;
    }
}

Result NexFacade::ConvertNexStationUrlToStationConnectionInfo(const nex::qList<nex::StationURL>& urls,
                                                              transport::StationConnectionInfo* info)
{
    if (urls.size() != 1 && urls.size() != 2)
        return cResultInvalidArgument;
    nex::qList<nex::StationURL>::const_iterator url = urls.begin();
    transport::StationLocation* location = &info->m_stationLocation;
    ConvertNexStationUrlToStationLocation(*url, location);
    location->Trace(cTraceStationLocation);
    ++url;
    if (url == urls.end())
        return 0;
    location = &info->m_localStationLocation;
    ConvertNexStationUrlToStationLocation(*url, location);
    location->Trace(cTraceStationLocation);
    return 0;
}

}}}
