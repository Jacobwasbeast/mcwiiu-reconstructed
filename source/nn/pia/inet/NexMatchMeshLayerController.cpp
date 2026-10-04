#include "nn/pia/inet/NexMatchMeshLayerController.hpp"

namespace nn { namespace pia { namespace inet {

NexMatchMeshLayerController::NexMatchMeshLayerController() : m_state(0)
{}

NexMatchMeshLayerController::~NexMatchMeshLayerController()
{}

void NexMatchMeshLayerController::CancelStartNatSession()
{
    m_state = 0;

}
unsigned long long NexMatchMeshLayerController::CheckConnectedStatus()
{
    return m_state;

}
unsigned long long NexMatchMeshLayerController::CheckConnectedStatusCore(nn::nex::NgsBridgeInterface* ngsInterface)
{
    return m_state;

}
unsigned int NexMatchMeshLayerController::CheckSessionId(unsigned int p0)
{
    return m_state;

}
void NexMatchMeshLayerController::Cleanup()
{
    m_state = 0;

}
void NexMatchMeshLayerController::CleanupEventHandler()
{
    m_state = 0;

}
void NexMatchMeshLayerController::ClearSessionId(unsigned int p0)
{
    m_state = 0;

}
unsigned int NexMatchMeshLayerController::GetMatchmakeSessionTotalParticipants() const
{
    return m_state;

}
unsigned int NexMatchMeshLayerController::GetMatchmakeSessionTotalStations() const
{
    return m_state;

}
unsigned int NexMatchMeshLayerController::GetMatchmakeSessionTotalStations(unsigned int p0) const
{
    return m_state;

}
unsigned int NexMatchMeshLayerController::GetNgsInterface(unsigned char p0)
{
    return m_state;

}
unsigned char NexMatchMeshLayerController::GetNgsInterfaceIndexBoundNexFacade() const
{
    return m_state;

}
unsigned long long NexMatchMeshLayerController::GetSessionId(unsigned int p0) const
{
    return m_state;

}
unsigned int NexMatchMeshLayerController::GetSessionIdArraySize() const
{
    return m_state;

}
unsigned long long NexMatchMeshLayerController::GetSessionParticipantInfo(unsigned int p0) const
{
    return m_state;

}
bool NexMatchMeshLayerController::IsAllStationJoined(unsigned int p0)
{
    return m_state != 0;

}
unsigned int NexMatchMeshLayerController::IsJoinedUnknownSession(unsigned int p0, unsigned int p1)
{
    return m_state;

}
unsigned int NexMatchMeshLayerController::IsParticipated(unsigned int p0, unsigned long long p1) const
{
    return m_state;

}
void NexMatchMeshLayerController::MeshCleanup()
{
    m_state = 0;

}
unsigned long long NexMatchMeshLayerController::MeshStartup(session::IMatchmakeSession* matchmakeSession, bool p1)
{
    return m_state;

}
void NexMatchMeshLayerController::SetLeaveFlag()
{
    m_state = 0;

}
unsigned long long NexMatchMeshLayerController::StartNatSession()
{
    return m_state;

}
unsigned long long NexMatchMeshLayerController::Startup(const session::Session::StartupSetting& setting)
{
    return m_state;

}
void NexMatchMeshLayerController::Trace(unsigned long long p0) const
{
}
unsigned int NexMatchMeshLayerController::WaitStartNatSession(Result* result)
{
    return m_state;

}

}}}

namespace nn { namespace pia { namespace inet {
NexNotificationEventHandler::NexNotificationEventHandler() : m_state(0) {}
void NexNotificationEventHandler::AllClearSessionId() { m_state = 0; }
void NexNotificationEventHandler::AllClearIgnoredSessionId() { m_state = 0; }
void NexNotificationEventHandler::Cleanup() { m_state = 0; }
unsigned char NexNotificationEventHandler::IsUnregisterGathering() { return static_cast<unsigned char>(m_state != 0); }
unsigned long long NexNotificationEventHandler::SaveSessionId(unsigned long long, unsigned long long) { return m_state; }
unsigned int NexNotificationEventHandler::CheckSessionId(unsigned long long) { return m_state; }
unsigned int NexNotificationEventHandler::IsIgnoredSessionId(unsigned long long) { return m_state; }
void NexNotificationEventHandler::AddIgnoredSessionId(unsigned long long) { m_state = 0; }
void NexNotificationEventHandler::ClearSessionId(unsigned long long) { m_state = 0; }
unsigned int NexNotificationEventHandler::GetSessionId(unsigned long long) { return m_state; }
int NexNotificationEventHandler::GetSessionParticipantInfo(unsigned long long) { return static_cast<int>(m_state); }
unsigned int NexNotificationEventHandler::GetMatchmakeSessionTotalStations(unsigned long long) { return m_state; }
unsigned int NexNotificationEventHandler::GetMatchmakeSessionTotalParticipants(unsigned long long) { return m_state; }
unsigned int NexNotificationEventHandler::IsParticipated(unsigned long long, unsigned long long, unsigned long long) { return m_state; }
unsigned long long NexNotificationEventHandler::RemoveIgnoredSessionId(unsigned long long) { return m_state; }
void NexNotificationEventHandler::AddParticipantPrincipalId(unsigned long long, unsigned long long, unsigned long long, unsigned long long, unsigned long long, unsigned long long) { m_state = 0; }
void NexNotificationEventHandler::RemoveParticipantPrincipalId(unsigned long long, unsigned long long, unsigned long long, unsigned long long) { m_state = 0; }
unsigned long long NexNotificationEventHandler::ProcessNotificationEvent(const void*) { return m_state; }
}}}
