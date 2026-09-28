#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXNATRELAYINTERFACE_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXNATRELAYINTERFACE_HPP

#include <stdint.h>

namespace nn { namespace pia { namespace inet {
struct CallContext;
struct InetAddress;
struct NATTraversalRelayClient;
struct NatRelayInterface;
struct ProtocolCallContext;
struct StationURL;
struct qList_nn_nex_StationURL;

class NexNatRelayInterface
{
public:
    NexNatRelayInterface();
    ~NexNatRelayInterface();
    void RegisterRelayClient(NATTraversalRelayClient* a1);
    void UnregisterRelayClient();
    void RelayProbeRequest(qList_nn_nex_StationURL* a1, StationURL* a2);
    uint64_t RequestProbe(StationURL* a1);
    void ReportNATTraversalResult(uint32_t* a1, bool* a2, uint32_t* a3);
    void ReportNATProperties(uint32_t a1, uint32_t a2, uint32_t a3);
    void UpdateConnectionState(uint32_t a1, StationURL* a2);
    uint32_t GetRelaySignatureKey(CallContext* a1, int32_t a2);
    void AssociateRelay(NatRelayInterface* a1, ProtocolCallContext* a2);
    void RelayProbeRequest(qList_nn_nex_StationURL* a1);
    void CheckCurrentPublicPort(CallContext* a1, InetAddress* a2, bool a3);
    void DetectSimplifiedNATProperties(CallContext* a1, InetAddress* a2);
    void ReportNATTraversalResultDetail(uint32_t* a1, bool* a2, uint32_t a3, uint32_t* a4);

private:
    uint64_t state_;
};

}}}

#endif
