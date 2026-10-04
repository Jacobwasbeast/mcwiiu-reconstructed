#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXNATRELAYINTERFACE_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXNATRELAYINTERFACE_HPP

#include <stdint.h>

namespace nn { namespace nex {
class CallContext;
class InetAddress;
class NATTraversalRelayClient;
class NATTraversalResult;
class ProtocolCallContext;
class RelaySignatureKey;
class StationURL;
template <typename T> class qList;
}}

namespace nn { namespace pia { namespace inet {
class NatRelayInterface;

class NexNatRelayInterface
{
public:
    NexNatRelayInterface();
    ~NexNatRelayInterface();
    void RegisterRelayClient(nn::nex::NATTraversalRelayClient* a1);
    void UnregisterRelayClient();
    void RelayProbeRequest(const nn::nex::qList<nn::nex::StationURL>& a1, const nn::nex::StationURL& a2);
    uint64_t RequestProbe(const nn::nex::StationURL& a1);
    void ReportNATTraversalResult(const uint32_t& a1, const bool& a2, const uint32_t& a3);
    void ReportNATProperties(uint32_t a1, uint32_t a2, uint32_t a3);
    void UpdateConnectionState(uint32_t a1, const nn::nex::StationURL& a2);
    uint32_t GetRelaySignatureKey(nn::nex::CallContext* a1, nn::nex::RelaySignatureKey* a2);
    void AssociateRelay(NatRelayInterface* a1, nn::nex::ProtocolCallContext* a2);
    void RelayProbeRequest(const nn::nex::qList<nn::nex::StationURL>& a1);
    void CheckCurrentPublicPort(nn::nex::CallContext* a1, const nn::nex::InetAddress& a2, bool a3);
    void DetectSimplifiedNATProperties(nn::nex::CallContext* a1, const nn::nex::InetAddress& a2);
    void ReportNATTraversalResultDetail(const uint32_t& a1, const bool& a2, const nn::nex::NATTraversalResult& a3, uint32_t& a4);

private:
    uint64_t state_;
};

}}}

#endif
