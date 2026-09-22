#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHMESHLAYERCONTROLLER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHMESHLAYERCONTROLLER_HPP

namespace nn { namespace pia { namespace inet {

class NexMatchMeshLayerController
{
public:
    NexMatchMeshLayerController();
    virtual ~NexMatchMeshLayerController();
    void CancelStartNatSession();
    unsigned long long CheckConnectedStatus();
    unsigned long long CheckConnectedStatusCore(unsigned long long p0, unsigned long long p1);
    unsigned int CheckSessionId(unsigned long long p0);
    void Cleanup();
    void CleanupEventHandler();
    void ClearSessionId(unsigned long long p0);
    unsigned int GetMatchmakeSessionTotalParticipants();
    unsigned int GetMatchmakeSessionTotalStations();
    unsigned int GetMatchmakeSessionTotalStations(unsigned long long p0);
    unsigned int GetNgsInterface(unsigned long long p0);
    unsigned char GetNgsInterfaceIndexBoundNexFacade();
    unsigned long long GetSessionId(unsigned long long p0);
    unsigned int GetSessionIdArraySize();
    unsigned long long GetSessionParticipantInfo(unsigned long long p0);
    bool IsAllStationJoined(unsigned long long p0);
    unsigned int IsJoinedUnknownSession(unsigned long long p0, unsigned long long p1);
    unsigned int IsParticipated(unsigned long long p0, unsigned long long p1);
    void MeshCleanup();
    unsigned long long MeshStartup(unsigned long long p0, unsigned long long p1, unsigned long long p2);
    void SetLeaveFlag();
    unsigned long long StartNatSession();
    unsigned long long Startup(unsigned long long p0, unsigned long long p1);
    void Trace(unsigned long long p0);
    unsigned int WaitStartNatSession(unsigned long long p0);

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
