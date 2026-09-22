#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXNATRELAY_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXNATRELAY_HPP

#include <cstdint>

namespace nn { namespace pia { namespace inet {
struct InetAddress;
struct StationLocation;

class NexNatRelay
{
public:
    NexNatRelay();
    ~NexNatRelay();
    void RequestProbe(StationLocation* a1);
    uint64_t RelayProbeRequest(StationLocation* a1, StationLocation* a2);
    uint64_t ReportNatTraversalResult(uint32_t a1, bool a2, uint32_t a3);
    void ReportNatProperties(uint32_t a1, uint32_t a2, uint32_t a3);
    void SetLocalCID(uint32_t a1);
    uint64_t RequestRelayServerSettingAsync();
    bool IsRequestRelayServerSettingCompleted();
    uint64_t GetRequestRelayServerSettingResult();
    uint64_t ResolveRelayServerSetting();
    uint32_t GetRelayServerSetting();
    void IsRelayServerAvailable();
    void SetRelayServerAddress(InetAddress* a1);
    int32_t GetRelayServerAddress();
    int32_t GetCurrentUTCEpochTime();
    uint32_t GetGameServerId();
    void SetRelayGatheringId(uint32_t a1);
    uint32_t GetRelayGatheringId();
    void SetRelayServerSignatureKey(uint8_t* a1);
    int32_t GetRelayServerSignatureKey();
    uint64_t UpdateRelayTable();
    uint32_t RemoveRelayEntry(StationLocation* a1);
    void Cleanup();
    void Trace(uint64_t a1);
    uint64_t Pack(uint32_t a1, int32_t a2, uint32_t a3, uint32_t a4, uint32_t a5, uint32_t a6);
    uint64_t Unpack(uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4);
    uint32_t GetRelayServerAddress(InetAddress* a1);

private:
    std::uint64_t state_;
};

}}}

#endif
