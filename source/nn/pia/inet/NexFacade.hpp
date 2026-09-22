#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEX_FACADE_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEX_FACADE_HPP

#include "nn/pia.h"

namespace nn { namespace pia { namespace inet {

class NexFacade
{
public:
    struct LoginInfo;
    NexFacade();
    virtual ~NexFacade();
    static NexFacade* CreateInstance();
    static void DestroyInstance();
    Result Initialize();
    void Finalize();
    Result CreateProtocols();
    Result Bind(const LoginInfo* loginInfo);
    void Unbind();
    Result Startup();
    void Cleanup();
    Result StartNatSessionAsync();
    bool IsStartNatSessionCompleted() const;
    Result GetStartNatSessionResult() const;
    void CancelStartNatSessionAsync();
    void StopNatSession();
    void UpdateGatheringID(unsigned int gatheringId);
    unsigned int GetNatPropertyMapping() const;
    unsigned long long GetLocalPrincipalId() const;
    bool IsRelayServiceEnabled() const;
    void SetIgnoreNatProbeRequest(bool ignore);
    void NotifyLocalServiceConnectionId(unsigned int connectionId);
    unsigned int GetReservedPort() const;
    unsigned int GetLocalServiceConnectionId() const;
    unsigned int GetRandomPortRangeMin() const;
    unsigned int GetRandomPortRangeMax() const;
    bool IsBeginMonitoringDataAvailable() const;

private:
    friend class InetProcessHostMigrationJob;
    static NexFacade* s_instance;
    bool m_initialized;
    bool m_started;
    bool m_natSessionCompleted;
    bool m_relayServiceEnabled;
    bool m_ignoreNatProbeRequest;
    Result m_natSessionResult;
    unsigned int m_gatheringId;
    unsigned long long m_localPrincipalId;
    unsigned int m_localServiceConnectionId;
    unsigned int m_reservedPort;
    unsigned int m_randomPortRangeMin;
    unsigned int m_randomPortRangeMax;
    unsigned int m_natPropertyMapping;
};

}}}

#endif
