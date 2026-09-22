#include "nn/pia/inet/NexNatRelay.hpp"

namespace nn { namespace pia { namespace inet {

void NexNatRelay::RequestProbe(StationLocation* a1)
{
    state_ += 1;
}

uint64_t NexNatRelay::RelayProbeRequest(StationLocation* a1, StationLocation* a2)
{
    return static_cast<uint64_t>(state_);
}

uint64_t NexNatRelay::ReportNatTraversalResult(uint32_t a1, bool a2, uint32_t a3)
{
    return static_cast<uint64_t>(state_);
}

void NexNatRelay::ReportNatProperties(uint32_t a1, uint32_t a2, uint32_t a3)
{
    state_ += 1;
}

void NexNatRelay::SetLocalCID(uint32_t a1)
{
    state_ += 1;
}

uint64_t NexNatRelay::RequestRelayServerSettingAsync()
{
    return static_cast<uint64_t>(state_);
}

bool NexNatRelay::IsRequestRelayServerSettingCompleted()
{
    return state_ != 0;
}

uint64_t NexNatRelay::GetRequestRelayServerSettingResult()
{
    return static_cast<uint64_t>(state_);
}

uint64_t NexNatRelay::ResolveRelayServerSetting()
{
    return static_cast<uint64_t>(state_);
}

uint32_t NexNatRelay::GetRelayServerSetting()
{
    return static_cast<uint32_t>(state_);
}

void NexNatRelay::IsRelayServerAvailable()
{
    state_ += 1;
}

void NexNatRelay::SetRelayServerAddress(InetAddress* a1)
{
    state_ += 1;
}

int32_t NexNatRelay::GetRelayServerAddress()
{
    return static_cast<int32_t>(state_);
}

int32_t NexNatRelay::GetCurrentUTCEpochTime()
{
    return static_cast<int32_t>(state_);
}

uint32_t NexNatRelay::GetGameServerId()
{
    return static_cast<uint32_t>(state_);
}

void NexNatRelay::SetRelayGatheringId(uint32_t a1)
{
    state_ += 1;
}

uint32_t NexNatRelay::GetRelayGatheringId()
{
    return static_cast<uint32_t>(state_);
}

void NexNatRelay::SetRelayServerSignatureKey(uint8_t* a1)
{
    state_ += 1;
}

int32_t NexNatRelay::GetRelayServerSignatureKey()
{
    return static_cast<int32_t>(state_);
}

uint64_t NexNatRelay::UpdateRelayTable()
{
    return static_cast<uint64_t>(state_);
}

uint32_t NexNatRelay::RemoveRelayEntry(StationLocation* a1)
{
    return static_cast<uint32_t>(state_);
}

void NexNatRelay::Cleanup()
{
    state_ += 1;
}

void NexNatRelay::Trace(uint64_t a1)
{
    state_ += 1;
}

uint64_t NexNatRelay::Pack(uint32_t a1, int32_t a2, uint32_t a3, uint32_t a4, uint32_t a5, uint32_t a6)
{
    return static_cast<uint64_t>(state_);
}

uint64_t NexNatRelay::Unpack(uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4)
{
    return static_cast<uint64_t>(state_);
}

uint32_t NexNatRelay::GetRelayServerAddress(InetAddress* a1)
{
    return static_cast<uint32_t>(state_);
}

}}}
