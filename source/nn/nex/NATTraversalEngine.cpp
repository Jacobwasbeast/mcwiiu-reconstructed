#include "nn/nex/NATTraversalEngine.hpp"

#include <algorithm>

#include "nn/nex/Core.hpp"
#include "nn/nex/NATRelayInterface.hpp"
#include "nn/nex/Relay.hpp"
#include "nn/nex/RootTransport.hpp"
#include "nn/nex/Scheduler.hpp"

namespace nn
{
namespace nex
{

namespace
{

const unsigned int STATION_URL_TYPE_BIT = 1;
const unsigned long long TRACE_NAT_PROBE = 0x40000;
const NATProperties::MappingProperties NAT_MAPPING_ENDPOINT_DEPENDENT =
    static_cast<NATProperties::MappingProperties>(2);
const int PROBE_INTERVAL = 90000;
const unsigned int PROBE_MAX_COUNT = 12;

int s_iProbeTimeout = 60000;
unsigned int s_uiMaxProbes = 32;

inline bool IsSameStation(const StationURL& rLeft, const StationURL& rRight)
{
    return String::IsEqual(rLeft.GetAddress(), rRight.GetAddress()) &&
        rLeft.GetRVConnectionID() == rRight.GetRVConnectionID();
}

}

NATTraversalEngine::NATTraversalEngine()
    : m_unsupportedNATReferee(), m_pRelay(0), m_pEcho(0), m_pStream(0), m_pTransportAdapter(0),
      m_activeProbes(), m_completedProbes(), m_nextProbeTime(),
      m_pCheckAndRelayJob(0), m_callContext(), m_bSendingDummyPackets(false),
      m_nextDummyPacketTime(), m_uiProbeRequestCount(0), m_probeRequests(),
      m_relayServerAddress(L""), m_uiRelayServerPort(0), m_bUseRelayServer(false)
{
}

NATTraversalEngine::~NATTraversalEngine()
{
}

bool NATTraversalEngine::RegisterEcho(NATEchoInterface* pEcho)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (m_pEcho != 0)
        return false;

    m_pEcho = pEcho;
    return true;
}

bool NATTraversalEngine::UnregisterEcho()
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (m_pEcho == 0)
        return false;

    m_pEcho = 0;
    return true;
}

bool NATTraversalEngine::UnregisterTransportAdapter()
{
    if (m_pTransportAdapter == 0)
        return false;

    m_pTransportAdapter = 0;
    return true;
}

unsigned int NATTraversalEngine::GetNumOfMaxProbes()
{
    NATProperties::MappingProperties mapping =
        RootTransport::GetInstance()->GetNATProperties().GetNATMapping();
    if (mapping == NAT_MAPPING_ENDPOINT_DEPENDENT)
        return 1;

    return s_uiMaxProbes;
}

bool NATTraversalEngine::CanUseRelay(const StationURL& rStationURL, bool bCheckURL) const
{
    if (!RootTransport::GetInstance()->GetRelay()->IsEnabled() ||
        String::IsEqual(m_relayServerAddress, L"") || m_uiRelayServerPort == 0)
        return false;

    if (bCheckURL)
    {
        if (rStationURL.GetUseRelayServer())
            return false;

        if (((rStationURL.GetType() >> STATION_URL_TYPE_BIT) & 1) == 0)
            return false;
    }

    return true;
}

bool NATTraversalEngine::ShouldUseRelay(const StationURL& rStationURL, bool bCheckURL) const
{
    if (CanUseRelay(rStationURL, bCheckURL) && m_bUseRelayServer)
        return true;

    return false;
}

bool NATTraversalEngine::PrepareNATTraversal(StationURL stationURL)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (m_pRelay == 0)
        return false;

    if (stationURL.GetRVConnectionID() == 0)
        return false;

    stationURL.Trace(TRACE_NAT_PROBE);
    stationURL.SetProbeRequestID(m_uiProbeRequestCount++);
    if (ShouldUseRelay(stationURL, false))
    {
        stationURL.Trace(TRACE_NAT_PROBE);
        stationURL.SetUseRelayServer(true);
        stationURL.SetRelayServerAddress(m_relayServerAddress);
        stationURL.SetRelayServerPort(m_uiRelayServerPort);
    }

    qList<StationURL>::iterator it;
    for (it = m_pendingURLs.begin(); it != m_pendingURLs.end(); ++it)
    {
        if (IsSameStation(*it, stationURL))
            break;
    }

    if (it == m_pendingURLs.end() && m_activeProbes.FindProbe(stationURL) == 0 &&
        m_completedProbes.FindProbe(stationURL) == 0)
    {
        m_pendingURLs.push_back(stationURL);
        return true;
    }

    return false;
}

bool NATTraversalEngine::PrepareNATTraversal(const qList<StationURL>& rStationURLs)
{
    {
        ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
        if (m_pRelay == 0)
            return false;
    }

    for (qList<StationURL>::const_iterator it = rStationURLs.begin(); it != rStationURLs.end();
         ++it)
    {
        if (PrepareNATTraversal(*it))
            return true;
    }

    return true;
}

void NATTraversalEngine::ReportNATTraversalResult(unsigned int uiConnectionID, bool bSucceeded,
    unsigned int uiRVConnectionID)
{
    if (m_pRelay != 0)
    {
        NATTraversalResult result = GetNATTraversalResult(uiConnectionID);
        m_pRelay->ReportNATTraversalResultDetail(uiConnectionID, bSucceeded, result,
            uiRVConnectionID);
    }
}

void NATTraversalEngine::ReportNATProperties(NATProperties::MappingProperties mapping,
    NATProperties::FilteringProperties filtering, unsigned int uiPortIncrease)
{
    if (m_pRelay != 0)
        m_pRelay->ReportNATProperties(mapping, filtering, uiPortIncrease);
}

void NATTraversalEngine::TimeoutProbeRequest(const Time& rTime)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    ProbeRequestMap::iterator it = m_probeRequests.begin();
    while (it != m_probeRequests.end())
    {
        if (rTime <= it->second.first)
        {
            ++it;
        }
        else
        {
            m_completedProbes.push_back(
                URLProbe(it->second.second, s_iProbeTimeout, PROBE_INTERVAL, PROBE_MAX_COUNT));
            ProbeRequestMap::iterator expired = it;
            ++it;
            m_probeRequests.erase(expired);
        }
    }
}

void NATTraversalEngine::StopProbes()
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    std::list<unsigned int, MemAllocator<unsigned int> > completedIDs;
    std::list<StationURL, MemAllocator<StationURL> > relayURLs;

    URLProbeList::iterator probe = m_activeProbes.begin();
    while (probe != m_activeProbes.end())
    {
        if (probe->UpdateIsNeeded(Time(Time::GetTime())) && probe->m_iPingTime == -1)
        {
            ++probe;
            continue;
        }

        probe->Trace(TRACE_NAT_PROBE);
        if (probe->m_iPingTime == -1)
        {
            if (!CanUseRelay(probe->m_stationURL, true) ||
                probe->m_stationURL.GetProbeRequestInitiation())
                m_completedProbes.push_back(*probe);
            else
                relayURLs.push_back(probe->m_stationURL);

            bool bUseRelay = false;
            if (RootTransport::GetInstance()->GetNATProperties().GetNATMapping() ==
                NAT_MAPPING_ENDPOINT_DEPENDENT)
            {
                bUseRelay = false;
                if (RootTransport::GetInstance()->GetRelay()->IsEnabled())
                {
                    if (!String::IsEqual(m_relayServerAddress, L"") && m_uiRelayServerPort != 0)
                        bUseRelay = true;
                }
            }

            if (bUseRelay)
            {
                m_bUseRelayServer = true;
                for (qList<StationURL>::iterator url = m_pendingURLs.begin();
                     url != m_pendingURLs.end(); ++url)
                {
                    if (!url->GetUseRelayServer() &&
                        ((url->GetType() >> STATION_URL_TYPE_BIT) & 1) != 0)
                    {
                        url->SetUseRelayServer(true);
                        url->SetRelayServerAddress(m_relayServerAddress);
                        url->SetRelayServerPort(m_uiRelayServerPort);
                    }
                }
            }

            URLProbeList::iterator failed = probe;
            probe = m_activeProbes.erase(failed);
        }
        else
        {
            unsigned int uiConnectionID = probe->m_stationURL.GetRVConnectionID();
            qList<StationURL>::iterator url = m_pendingURLs.begin();
            while (url != m_pendingURLs.end())
            {
                if (url->GetRVConnectionID() == uiConnectionID)
                {
                    url->Trace(TRACE_NAT_PROBE);
                    url = m_pendingURLs.erase(url);
                }
                else
                {
                    ++url;
                }
            }

            completedIDs.push_back(uiConnectionID);
            m_completedProbes.push_back(*probe);
            URLProbeList::iterator succeeded = probe;
            probe = m_activeProbes.erase(succeeded);
        }
    }

    probe = m_activeProbes.begin();
    while (!completedIDs.empty() && probe != m_activeProbes.end())
    {
        if (std::find(completedIDs.begin(), completedIDs.end(),
                probe->m_stationURL.GetRVConnectionID()) != completedIDs.end())
        {
            probe->Trace(TRACE_NAT_PROBE);
            m_completedProbes.push_back(*probe);
            URLProbeList::iterator moved = probe;
            probe = m_activeProbes.erase(moved);
        }
        else
        {
            ++probe;
        }
    }

    probe = m_completedProbes.begin();
    while (probe != m_completedProbes.end())
    {
        if (Time::GetTime() <= probe->m_expirationTime)
        {
            ++probe;
        }
        else
        {
            probe->Trace(TRACE_NAT_PROBE);
            URLProbeList::iterator expired = probe;
            probe = m_completedProbes.erase(expired);
        }
    }

    for (std::list<StationURL, MemAllocator<StationURL> >::iterator url = relayURLs.begin();
         url != relayURLs.end(); ++url)
    {
        url->Trace(TRACE_NAT_PROBE);
        url->SetUseRelayServer(true);
        url->SetRelayServerAddress(m_relayServerAddress);
        url->SetRelayServerPort(m_uiRelayServerPort);
        m_pendingURLs.push_front(*url);
    }
}

bool NATTraversalEngine::Terminate()
{
    if (m_callContext.GetState() == CallContext::CallInProgress)
        m_callContext.Cancel(CallContext::CallCanceled);

    return true;
}

bool NATTraversalEngine::RegisterTransportAdapter(NATTransportAdapter* pTransportAdapter)
{
    if (m_pTransportAdapter != 0)
        return false;

    m_pTransportAdapter = pTransportAdapter;
    return true;
}

void NATTraversalEngine::StartSendDummyPacket()
{
    m_bSendingDummyPackets = true;
    m_nextDummyPacketTime = Time(Time::GetTime());
}

void NATTraversalEngine::StopSendDummyPacket()
{
    m_bSendingDummyPackets = false;
}

int NATTraversalEngine::GetNumOfSendingProbes()
{
    int iCount = 0;
    for (URLProbeList::iterator it = m_activeProbes.begin();
         it != m_activeProbes.end(); ++it)
    {
        if ((it->m_stationURL.GetType() >> STATION_URL_TYPE_BIT) & 1)
            ++iCount;
    }
    return iCount;
}

void NATTraversalEngine::ClearProbeList()
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    m_activeProbes.clear();
    m_completedProbes.clear();
}

int NATTraversalEngine::GetURLPingTime(const StationURL& rStationURL)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    URLProbe* pProbe = m_completedProbes.FindProbe(rStationURL);
    if (pProbe != 0)
        return pProbe->m_iPingTime;

    return -1;
}

int NATTraversalEngine::GetCIDPingTime(unsigned int uiConnectionID)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    URLProbe* pProbe = m_completedProbes.FindProbe(uiConnectionID);
    if (pProbe != 0)
        return pProbe->m_iPingTime;

    return -1;
}

bool NATTraversalEngine::GetUpdatedURL(const StationURL& rStationURL, StationURL* pUpdatedURL)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    URLProbe* pProbe = m_completedProbes.FindProbe(rStationURL);
    if (pProbe != 0 && pProbe->m_stationURL != rStationURL)
    {
        *pUpdatedURL = pProbe->m_stationURL;
        return true;
    }

    return false;
}

bool NATTraversalEngine::GetProbeFailed(const StationURL& rStationURL)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    URLProbe* pProbe = m_completedProbes.FindProbe(rStationURL);
    if (pProbe != 0)
        return pProbe->m_iPingTime == -1;

    if (m_activeProbes.FindProbe(rStationURL) != 0)
        return false;

    for (qList<StationURL>::iterator it = m_pendingURLs.begin(); it != m_pendingURLs.end(); ++it)
    {
        if (IsSameStation(*it, rStationURL))
            return false;
    }

    for (ProbeRequestMap::iterator it = m_probeRequests.begin(); it != m_probeRequests.end(); ++it)
    {
        if (IsSameStation(it->second.second, rStationURL))
            return false;
    }

    return true;
}

NATTraversalResult NATTraversalEngine::GetNATTraversalResult(unsigned int uiConnectionID)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    URLProbe* pProbe = m_completedProbes.FindProbe(uiConnectionID);
    if (pProbe == 0)
        return NAT_TRAVERSAL_UNKNOWN;

    if (pProbe->m_iPingTime != -1)
    {
        if (pProbe->m_stationURL.GetUseRelayServer())
            return NAT_TRAVERSAL_SUCCEEDED_VIA_RELAY;

        return NAT_TRAVERSAL_SUCCEEDED;
    }

    if (pProbe->m_uiProbeCount == 0)
        return NAT_TRAVERSAL_NOT_ATTEMPTED;

    if (pProbe->m_stationURL.GetUseRelayServer())
        return NAT_TRAVERSAL_FAILED_VIA_RELAY;

    return NAT_TRAVERSAL_FAILED;
}

}
}
