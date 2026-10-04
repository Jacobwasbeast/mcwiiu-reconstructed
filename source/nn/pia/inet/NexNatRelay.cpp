#include "nn/pia/inet/NexNatRelay.hpp"

namespace nn { namespace pia { namespace inet {

void NexNatRelay::RequestProbe(const transport::StationLocation& a1)
{
    state_ += 1;
}

uint64_t NexNatRelay::RelayProbeRequest(const transport::StationLocation& a1, const transport::StationLocation& a2)
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

bool NexNatRelay::IsRequestRelayServerSettingCompleted() const
{
    return state_ != 0;
}

uint64_t NexNatRelay::GetRequestRelayServerSettingResult() const
{
    return static_cast<uint64_t>(state_);
}

uint64_t NexNatRelay::ResolveRelayServerSetting()
{
    return static_cast<uint64_t>(state_);
}

uint32_t NexNatRelay::GetRelayServerSetting() const
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexNatRelay::IsRelayServerAvailable() const
{
    return static_cast<uint32_t>(state_);
}

void NexNatRelay::SetRelayServerAddress(const common::InetAddress& a1)
{
    state_ += 1;
}

int32_t NexNatRelay::GetRelayServerAddress() const
{
    return static_cast<int32_t>(state_);
}

int32_t NexNatRelay::GetCurrentUTCEpochTime() const
{
    return static_cast<int32_t>(state_);
}

uint32_t NexNatRelay::GetGameServerId() const
{
    return static_cast<uint32_t>(state_);
}

void NexNatRelay::SetRelayGatheringId(uint32_t a1)
{
    state_ += 1;
}

uint32_t NexNatRelay::GetRelayGatheringId() const
{
    return static_cast<uint32_t>(state_);
}

void NexNatRelay::SetRelayServerSignatureKey(uint8_t* a1)
{
    state_ += 1;
}

int32_t NexNatRelay::GetRelayServerSignatureKey() const
{
    return static_cast<int32_t>(state_);
}

uint64_t NexNatRelay::UpdateRelayTable()
{
    return static_cast<uint64_t>(state_);
}

uint32_t NexNatRelay::RemoveRelayEntry(const transport::StationLocation& a1)
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

uint64_t NexNatRelay::Pack(void* a1, const void* a2, uint32_t a3, const common::InetAddress& a4, const common::InetAddress& a5)
{
    return static_cast<uint64_t>(state_);
}

uint64_t NexNatRelay::Unpack(void* a1, int32_t* a2, common::InetAddress* a3)
{
    return static_cast<uint64_t>(state_);
}

uint32_t NexNatRelay::GetRelayServerAddress(const common::InetAddress& a1) const
{
    return static_cast<uint32_t>(state_);
}

}}}
