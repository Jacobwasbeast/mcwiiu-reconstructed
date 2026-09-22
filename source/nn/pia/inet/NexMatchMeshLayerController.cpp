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
unsigned long long NexMatchMeshLayerController::CheckConnectedStatusCore(unsigned long long p0, unsigned long long p1)
{
    return m_state;

}
unsigned int NexMatchMeshLayerController::CheckSessionId(unsigned long long p0)
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
void NexMatchMeshLayerController::ClearSessionId(unsigned long long p0)
{
    m_state = 0;

}
unsigned int NexMatchMeshLayerController::GetMatchmakeSessionTotalParticipants()
{
    return m_state;

}
unsigned int NexMatchMeshLayerController::GetMatchmakeSessionTotalStations()
{
    return m_state;

}
unsigned int NexMatchMeshLayerController::GetMatchmakeSessionTotalStations(unsigned long long p0)
{
    return m_state;

}
unsigned int NexMatchMeshLayerController::GetNgsInterface(unsigned long long p0)
{
    return m_state;

}
unsigned char NexMatchMeshLayerController::GetNgsInterfaceIndexBoundNexFacade()
{
    return m_state;

}
unsigned long long NexMatchMeshLayerController::GetSessionId(unsigned long long p0)
{
    return m_state;

}
unsigned int NexMatchMeshLayerController::GetSessionIdArraySize()
{
    return m_state;

}
unsigned long long NexMatchMeshLayerController::GetSessionParticipantInfo(unsigned long long p0)
{
    return m_state;

}
bool NexMatchMeshLayerController::IsAllStationJoined(unsigned long long p0)
{
    return m_state != 0;

}
unsigned int NexMatchMeshLayerController::IsJoinedUnknownSession(unsigned long long p0, unsigned long long p1)
{
    return m_state;

}
unsigned int NexMatchMeshLayerController::IsParticipated(unsigned long long p0, unsigned long long p1)
{
    return m_state;

}
void NexMatchMeshLayerController::MeshCleanup()
{
    m_state = 0;

}
unsigned long long NexMatchMeshLayerController::MeshStartup(unsigned long long p0, unsigned long long p1, unsigned long long p2)
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
unsigned long long NexMatchMeshLayerController::Startup(unsigned long long p0, unsigned long long p1)
{
    return m_state;

}
void NexMatchMeshLayerController::Trace(unsigned long long p0)
{
    m_state = 0;

}
unsigned int NexMatchMeshLayerController::WaitStartNatSession(unsigned long long p0)
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
