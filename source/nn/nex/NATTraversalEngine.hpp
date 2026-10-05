#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_NATTRAVERSALENGINE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_NATTRAVERSALENGINE_HPP

#include <functional>
#include <map>
#include <utility>

#include "nn/nex/CallContext.hpp"
#include "nn/nex/MemAllocator.hpp"
#include "nn/nex/NATProperties.hpp"
#include "nn/nex/NATTraversalResult.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/StationURL.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/Time.hpp"
#include "nn/nex/URLProbeList.hpp"
#include "nn/nex/UnsupportedNATReferee.hpp"
#include "nn/nex/qList.hpp"

namespace nn
{
namespace nex
{

class ConnectivityTester;
class Job;
class NATEchoInterface;
class NATRelayInterface;
class NATTransportAdapter;
class NATTraversalStream;

class NATTraversalEngine : public RootObject
{
public:
    NATTraversalEngine();
    virtual ~NATTraversalEngine();

    virtual bool RegisterRelay(NATRelayInterface* pRelay);
    virtual bool UnregisterRelay();
    virtual bool RegisterEcho(NATEchoInterface* pEcho);
    virtual bool UnregisterEcho();
    virtual bool RegisterConnectivityTester(ConnectivityTester* pConnectivityTester);
    virtual bool UnregisterConnectivityTester();

    bool Terminate();
    bool RegisterTransportAdapter(NATTransportAdapter* pTransportAdapter);
    bool UnregisterTransportAdapter();
    unsigned int GetNumOfMaxProbes();
    bool CanUseRelay(const StationURL& rStationURL, bool bCheckURL) const;
    bool ShouldUseRelay(const StationURL& rStationURL, bool bCheckURL) const;
    bool PrepareNATTraversal(StationURL stationURL);
    bool PrepareNATTraversal(const qList<StationURL>& rStationURLs);
    void ReportNATTraversalResult(unsigned int uiConnectionID, bool bSucceeded,
        unsigned int uiRVConnectionID);
    void ReportNATProperties(NATProperties::MappingProperties mapping,
        NATProperties::FilteringProperties filtering, unsigned int uiPortIncrease);
    void TimeoutProbeRequest(const Time& rTime);
    void StopProbes();
    void StartSendDummyPacket();
    void StopSendDummyPacket();
    int GetNumOfSendingProbes();
    void ClearProbeList();
    int GetURLPingTime(const StationURL& rStationURL);
    int GetCIDPingTime(unsigned int uiConnectionID);
    bool GetUpdatedURL(const StationURL& rStationURL, StationURL* pUpdatedURL);
    bool GetProbeFailed(const StationURL& rStationURL);
    NATTraversalResult GetNATTraversalResult(unsigned int uiConnectionID);

private:
    friend class Relay;

    typedef std::map<unsigned int, std::pair<Time, StationURL>, std::less<unsigned int>,
        MemAllocator<std::pair<const unsigned int, std::pair<Time, StationURL> > > >
        ProbeRequestMap;

    UnsupportedNATReferee m_unsupportedNATReferee;
    NATRelayInterface* m_pRelay;
    NATEchoInterface* m_pEcho;
    NATTraversalStream* m_pStream;
    unsigned int m_uiConnectionID;
    NATTransportAdapter* m_pTransportAdapter;
    qList<StationURL> m_pendingURLs;
    URLProbeList m_activeProbes;
    URLProbeList m_completedProbes;
    Time m_nextProbeTime;
    Job* m_pCheckAndRelayJob;
    CallContext m_callContext;
    bool m_bSendingDummyPackets;
    Time m_nextDummyPacketTime;
    unsigned int m_uiProbeRequestCount;
    ProbeRequestMap m_probeRequests;
    String m_relayServerAddress;
    unsigned short m_uiRelayServerPort;
    bool m_bUseRelayServer;
};

}
}

#endif
