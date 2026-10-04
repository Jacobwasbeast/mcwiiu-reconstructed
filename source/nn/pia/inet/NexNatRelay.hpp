#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXNATRELAY_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXNATRELAY_HPP

#include <stdint.h>

namespace nn { namespace pia { namespace common { class InetAddress; }}}
namespace nn { namespace pia { namespace transport { class StationLocation; }}}

namespace nn { namespace pia { namespace inet {

class NexNatRelay
{
public:
    NexNatRelay();
    ~NexNatRelay();
    void RequestProbe(const transport::StationLocation& a1);
    uint64_t RelayProbeRequest(const transport::StationLocation& a1, const transport::StationLocation& a2);
    uint64_t ReportNatTraversalResult(uint32_t a1, bool a2, uint32_t a3);
    void ReportNatProperties(uint32_t a1, uint32_t a2, uint32_t a3);
    void SetLocalCID(uint32_t a1);
    uint64_t RequestRelayServerSettingAsync();
    bool IsRequestRelayServerSettingCompleted() const;
    uint64_t GetRequestRelayServerSettingResult() const;
    uint64_t ResolveRelayServerSetting();
    uint32_t GetRelayServerSetting() const;
    uint32_t IsRelayServerAvailable() const;
    void SetRelayServerAddress(const common::InetAddress& a1);
    int32_t GetRelayServerAddress() const;
    int32_t GetCurrentUTCEpochTime() const;
    uint32_t GetGameServerId() const;
    void SetRelayGatheringId(uint32_t a1);
    uint32_t GetRelayGatheringId() const;
    void SetRelayServerSignatureKey(uint8_t* a1);
    int32_t GetRelayServerSignatureKey() const;
    uint64_t UpdateRelayTable();
    uint32_t RemoveRelayEntry(const transport::StationLocation& a1);
    void Cleanup();
    void Trace(uint64_t a1);
    uint64_t Pack(void* a1, const void* a2, uint32_t a3, const common::InetAddress& a4, const common::InetAddress& a5);
    uint64_t Unpack(void* a1, int32_t* a2, common::InetAddress* a3);
    uint32_t GetRelayServerAddress(const common::InetAddress& a1) const;

private:
    uint64_t state_;
};

}}}

#endif
