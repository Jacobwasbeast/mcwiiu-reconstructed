#include "nn/pia/inet/NexNatRelayInterface.hpp"

namespace nn { namespace pia { namespace inet {

void NexNatRelayInterface::RegisterRelayClient(NATTraversalRelayClient* a1)
{
    state_ += 1;
}

void NexNatRelayInterface::UnregisterRelayClient()
{
    state_ += 1;
}

void NexNatRelayInterface::RelayProbeRequest(qList_nn_nex_StationURL* a1, StationURL* a2)
{
    state_ += 1;
}

uint64_t NexNatRelayInterface::RequestProbe(StationURL* a1)
{
    return static_cast<uint64_t>(state_);
}

void NexNatRelayInterface::ReportNATTraversalResult(uint32_t* a1, bool* a2, uint32_t* a3)
{
    state_ += 1;
}

void NexNatRelayInterface::ReportNATProperties(uint32_t a1, uint32_t a2, uint32_t a3)
{
    state_ += 1;
}

void NexNatRelayInterface::UpdateConnectionState(uint32_t a1, StationURL* a2)
{
    state_ += 1;
}

uint32_t NexNatRelayInterface::GetRelaySignatureKey(CallContext* a1, int32_t a2)
{
    return static_cast<uint32_t>(state_);
}

void NexNatRelayInterface::AssociateRelay(NatRelayInterface* a1, ProtocolCallContext* a2)
{
    state_ += 1;
}

void NexNatRelayInterface::RelayProbeRequest(qList_nn_nex_StationURL* a1)
{
    state_ += 1;
}

void NexNatRelayInterface::CheckCurrentPublicPort(CallContext* a1, InetAddress* a2, bool a3)
{
    state_ += 1;
}

void NexNatRelayInterface::DetectSimplifiedNATProperties(CallContext* a1, InetAddress* a2)
{
    state_ += 1;
}

void NexNatRelayInterface::ReportNATTraversalResultDetail(uint32_t* a1, bool* a2, uint32_t a3, uint32_t* a4)
{
    state_ += 1;
}

}}}
