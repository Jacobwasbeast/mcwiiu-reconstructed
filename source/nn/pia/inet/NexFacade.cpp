#include "nn/pia/inet/NexFacade.hpp"

namespace nn { namespace pia { namespace inet {

NexFacade* NexFacade::s_instance = 0;

NexFacade::NexFacade()
    : m_initialized(false), m_started(false), m_natSessionCompleted(false), m_relayServiceEnabled(false),
      m_ignoreNatProbeRequest(false), m_natSessionResult(0), m_gatheringId(0), m_localPrincipalId(0),
      m_localServiceConnectionId(0), m_reservedPort(0), m_randomPortRangeMin(0xc000), m_randomPortRangeMax(0xffff), m_natPropertyMapping(0)
{
}

NexFacade::~NexFacade()
{
    Finalize();
}

NexFacade* NexFacade::CreateInstance()
{
    if (s_instance == 0)
        s_instance = new NexFacade();
    return s_instance;
}

void NexFacade::DestroyInstance()
{
    delete s_instance;
    s_instance = 0;
}

Result NexFacade::Initialize() { m_initialized = true; return 0; }
void NexFacade::Finalize() { Cleanup(); m_initialized = false; }
Result NexFacade::CreateProtocols() { return m_initialized ? 0 : 1; }
Result NexFacade::Bind(const LoginInfo*) { return m_initialized ? 0 : 1; }
void NexFacade::Unbind() { m_started = false; }
Result NexFacade::Startup() { m_started = true; m_natSessionCompleted = false; return 0; }
void NexFacade::Cleanup() { m_started = false; m_natSessionCompleted = false; }
Result NexFacade::StartNatSessionAsync() { m_natSessionCompleted = true; m_natSessionResult = 0; return 0; }
bool NexFacade::IsStartNatSessionCompleted() const { return m_natSessionCompleted; }
Result NexFacade::GetStartNatSessionResult() const { return m_natSessionResult; }
void NexFacade::CancelStartNatSessionAsync() { m_natSessionCompleted = false; }
void NexFacade::StopNatSession() { m_natSessionCompleted = false; }
void NexFacade::UpdateGatheringID(unsigned int gatheringId) { m_gatheringId = gatheringId; }
unsigned int NexFacade::GetNatPropertyMapping() const { return m_natPropertyMapping; }
unsigned long long NexFacade::GetLocalPrincipalId() const { return m_localPrincipalId; }
bool NexFacade::IsRelayServiceEnabled() const { return m_relayServiceEnabled; }
void NexFacade::SetIgnoreNatProbeRequest(bool ignore) { m_ignoreNatProbeRequest = ignore; }
void NexFacade::NotifyLocalServiceConnectionId(unsigned int connectionId) { m_localServiceConnectionId = connectionId; }
unsigned int NexFacade::GetReservedPort() const { return m_reservedPort; }
unsigned int NexFacade::GetLocalServiceConnectionId() const { return m_localServiceConnectionId; }
unsigned int NexFacade::GetRandomPortRangeMin() const { return m_randomPortRangeMin; }
unsigned int NexFacade::GetRandomPortRangeMax() const { return m_randomPortRangeMax; }
bool NexFacade::IsBeginMonitoringDataAvailable() const { return true; }

}}}
