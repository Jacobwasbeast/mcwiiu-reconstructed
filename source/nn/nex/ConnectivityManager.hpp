#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_CONNECTIVITYMANAGER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_CONNECTIVITYMANAGER_HPP

#include <cstdint>

#include "nn/nex/PseudoSingleton.hpp"

namespace nn
{
namespace nex
{

class CallContext;
class ConnectivityTester;
class NATTransportAdapter;
class StationProbeList;

class ConnectivityManager : public PseudoSingleton
{
public:
    ConnectivityManager();
    virtual ~ConnectivityManager();

    static ConnectivityManager* GetInstance();
    static ConnectivityManager* GetInstanceWithoutNetworkCheck();

    bool RegisterConnectivityTester(ConnectivityTester* connectivityTester);
    bool UnregisterConnectivityTester();
    bool RegisterTransportAdapter(NATTransportAdapter* transportAdapter);
    bool UnregisterTransportAdapter();

    bool StartNATSession();
    bool StartNATSession(CallContext* callContext);
    bool StartNATSessionInternal(CallContext* callContext);
    void OnStartNATSession(CallContext* callContext);
    bool StopNATSession();
    bool IsNATSessionStarted();
    bool CheckUnupdateNATPort();

    bool ProbeStations(CallContext* callContext, StationProbeList* stationProbeList,
                       unsigned int timeout, bool useRelay);
    bool InitiateProbes(StationProbeList& stationProbeList);

    void SetProbeLifetime(int probeLifetime);
    void SetProbeKeepAlive(int probeKeepAlive);

    static void SetRelayPublicAddress();
    static void SetSuspendBossDaemonOnNATSession(bool suspend);
    static bool GetSuspendBossDaemonOnNATSession();

protected:
    bool m_natSessionStarted;
    bool m_natTraversalEnabled;
    unsigned short m_portNumber;
    unsigned short m_reservedPortNumber;
    ConnectivityTester* m_connectivityTester;
    std::uint32_t m_callContextId;
    std::uint32_t m_probeLifetime;
    std::int32_t m_bossDaemonSuspendCount;
    bool m_packetBufferManagerInitialized;
};

}
}

#endif
