#include "nn/pia/inet/NexNatRelayInterface.hpp"

namespace nn { namespace pia { namespace inet {

void NexNatRelayInterface::RegisterRelayClient(nn::nex::NATTraversalRelayClient* a1)
{
    state_ += 1;
}

void NexNatRelayInterface::UnregisterRelayClient()
{
    state_ += 1;
}

void NexNatRelayInterface::RelayProbeRequest(const nn::nex::qList<nn::nex::StationURL>& a1, const nn::nex::StationURL& a2)
{
    state_ += 1;
}

uint64_t NexNatRelayInterface::RequestProbe(const nn::nex::StationURL& a1)
{
    return static_cast<uint64_t>(state_);
}

void NexNatRelayInterface::ReportNATTraversalResult(const uint32_t& a1, const bool& a2, const uint32_t& a3)
{
    state_ += 1;
}

void NexNatRelayInterface::ReportNATProperties(uint32_t a1, uint32_t a2, uint32_t a3)
{
    state_ += 1;
}

void NexNatRelayInterface::UpdateConnectionState(uint32_t a1, const nn::nex::StationURL& a2)
{
    state_ += 1;
}

uint32_t NexNatRelayInterface::GetRelaySignatureKey(nn::nex::CallContext* a1, nn::nex::RelaySignatureKey* a2)
{
    return static_cast<uint32_t>(state_);
}

void NexNatRelayInterface::AssociateRelay(NatRelayInterface* a1, nn::nex::ProtocolCallContext* a2)
{
    state_ += 1;
}

void NexNatRelayInterface::RelayProbeRequest(const nn::nex::qList<nn::nex::StationURL>& a1)
{
    state_ += 1;
}

void NexNatRelayInterface::CheckCurrentPublicPort(nn::nex::CallContext* a1, const nn::nex::InetAddress& a2, bool a3)
{
    state_ += 1;
}

void NexNatRelayInterface::DetectSimplifiedNATProperties(nn::nex::CallContext* a1, const nn::nex::InetAddress& a2)
{
    state_ += 1;
}

void NexNatRelayInterface::ReportNATTraversalResultDetail(const uint32_t& a1, const bool& a2, const nn::nex::NATTraversalResult& a3, uint32_t& a4)
{
    state_ += 1;
}

}}}
