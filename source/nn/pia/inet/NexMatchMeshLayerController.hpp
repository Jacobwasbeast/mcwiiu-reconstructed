#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHMESHLAYERCONTROLLER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHMESHLAYERCONTROLLER_HPP

#include "nn/pia/Common.hpp"
#include "nn/pia/session/Session.hpp"

namespace nn { namespace pia { namespace session { class IMatchmakeSession; }}}
namespace nn { namespace nex { class NgsBridgeInterface; }}

namespace nn { namespace pia { namespace inet {

class NexMatchMeshLayerController
{
public:
    NexMatchMeshLayerController();
    virtual ~NexMatchMeshLayerController();
    void CancelStartNatSession();
    unsigned long long CheckConnectedStatus();
    unsigned long long CheckConnectedStatusCore(nn::nex::NgsBridgeInterface* ngsInterface);
    unsigned int CheckSessionId(unsigned int p0);
    void Cleanup();
    void CleanupEventHandler();
    void ClearSessionId(unsigned int p0);
    unsigned int GetMatchmakeSessionTotalParticipants() const;
    unsigned int GetMatchmakeSessionTotalStations() const;
    unsigned int GetMatchmakeSessionTotalStations(unsigned int p0) const;
    unsigned int GetNgsInterface(unsigned char p0);
    unsigned char GetNgsInterfaceIndexBoundNexFacade() const;
    unsigned long long GetSessionId(unsigned int p0) const;
    unsigned int GetSessionIdArraySize() const;
    unsigned long long GetSessionParticipantInfo(unsigned int p0) const;
    bool IsAllStationJoined(unsigned int p0);
    unsigned int IsJoinedUnknownSession(unsigned int p0, unsigned int p1);
    unsigned int IsParticipated(unsigned int p0, unsigned long long p1) const;
    void MeshCleanup();
    unsigned long long MeshStartup(session::IMatchmakeSession* matchmakeSession, bool p1);
    void SetLeaveFlag();
    unsigned long long StartNatSession();
    unsigned long long Startup(const session::Session::StartupSetting& setting);
    void Trace(unsigned long long p0) const;
    unsigned int WaitStartNatSession(Result* result);

private:
    unsigned int m_state;
};

class NexNotificationEventHandler
{
public:
    NexNotificationEventHandler();
    void AllClearSessionId();
    void AllClearIgnoredSessionId();
    void Cleanup();
    unsigned char IsUnregisterGathering();
    unsigned long long SaveSessionId(unsigned long long p0, unsigned long long p1);
    unsigned int CheckSessionId(unsigned long long p0);
    unsigned int IsIgnoredSessionId(unsigned long long p0);
    void AddIgnoredSessionId(unsigned long long p0);
    void ClearSessionId(unsigned long long p0);
    unsigned int GetSessionId(unsigned long long p0);
    int GetSessionParticipantInfo(unsigned long long p0);
    unsigned int GetMatchmakeSessionTotalStations(unsigned long long p0);
    unsigned int GetMatchmakeSessionTotalParticipants(unsigned long long p0);
    unsigned int IsParticipated(unsigned long long p0, unsigned long long p1, unsigned long long p2);
    unsigned long long RemoveIgnoredSessionId(unsigned long long p0);
    void AddParticipantPrincipalId(unsigned long long p0, unsigned long long p1, unsigned long long p2, unsigned long long p3, unsigned long long p4, unsigned long long p5);
    void RemoveParticipantPrincipalId(unsigned long long p0, unsigned long long p1, unsigned long long p2, unsigned long long p3);
    unsigned long long ProcessNotificationEvent(const void* p0);
private:
    unsigned int m_state;
};

}}}

#endif
