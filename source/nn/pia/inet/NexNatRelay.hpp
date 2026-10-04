#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXNATRELAY_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXNATRELAY_HPP

#include "nn/pia/Common.hpp"

namespace nn { namespace nex { class RelaySignatureKey; }}
namespace nn { namespace pia { namespace common { class InetAddress; }}}
namespace nn { namespace pia { namespace transport { class StationLocation; }}}

namespace nn { namespace pia { namespace inet {

class NexNatRelay
{
public:
    NexNatRelay();
    virtual ~NexNatRelay();
    virtual void RequestProbe(const transport::StationLocation& location);
    virtual void RelayProbeRequest(const transport::StationLocation& source,
                                   const transport::StationLocation& destination);
    virtual void ReportNatTraversalResult(unsigned int connectionId, bool success, unsigned int detail);
    virtual void ReportNatProperties(unsigned int mapping, unsigned int filtering, unsigned int portIncrement);
    virtual void SetLocalCID(unsigned int connectionId);
    virtual void RequestRelayServerSettingAsync();
    virtual bool IsRequestRelayServerSettingCompleted() const;
    virtual Result GetRequestRelayServerSettingResult() const;
    virtual void ResolveRelayServerSetting();
    virtual const nex::RelaySignatureKey* GetRelayServerSetting() const;
    virtual unsigned int IsRelayServerAvailable() const;
    virtual void SetRelayServerAddress(const common::InetAddress& address);
    virtual const common::InetAddress& GetRelayServerAddress() const;
    virtual unsigned int GetCurrentUTCEpochTime() const;
    virtual unsigned int GetGameServerId() const;
    virtual void SetRelayGatheringId(unsigned int gatheringId);
    virtual unsigned int GetRelayGatheringId() const;
    virtual void SetRelayServerSignatureKey(unsigned char* key);
    virtual const unsigned char* GetRelayServerSignatureKey() const;
    virtual void UpdateRelayTable();
    virtual bool RemoveRelayEntry(const transport::StationLocation& location);
    virtual void Cleanup();
    virtual void Trace(unsigned long long value) const;
    virtual Result Pack(void* destination, const void* payload, unsigned int payloadSize,
                        const common::InetAddress& source, const common::InetAddress& target);
    virtual Result Unpack(void* packet, int* size, common::InetAddress* source);
    const common::InetAddress& GetRelayServerAddress(const common::InetAddress& address) const;
};

}}}

#endif
