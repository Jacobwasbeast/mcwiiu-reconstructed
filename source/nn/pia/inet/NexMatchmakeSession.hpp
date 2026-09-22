#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHMAKESESSION_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHMAKESESSION_HPP

namespace nn { namespace pia { namespace inet {

class NexMatchmakeSession
{
public:
    NexMatchmakeSession();
    virtual ~NexMatchmakeSession();
    unsigned long long BrowseMatchmakeSession(unsigned long long p0, unsigned long long p1);
    void Cleanup();
    unsigned long long ClearSystemPassword(unsigned long long p0, unsigned long long p1, unsigned long long p2);
    unsigned long long CloseParticipate(unsigned long long p0, unsigned long long p1, unsigned long long p2);
    unsigned long long ConvertGatheringIdToPersistentGatheringCode(unsigned long long p0);
    static unsigned int ConvertMatchmakeSystemTypeToSessionType(unsigned long long p0);
    unsigned int ConvertPersistentGatheringCodeToGatheringId(unsigned long long p0, unsigned long long p1, unsigned long long p2);
    static unsigned int ConvertSelectionMethodToMatchmakeSelectionMethod(unsigned long long p0);
    static unsigned int ConvertSessionTypeToMatchmakeSystemType(unsigned long long p0);
    void CopyFoundPersistentGatheringToBrowsedCommunityInfoList(unsigned long long p0, unsigned long long p1);
    void CopyMatchmakeSessionToSessionInfo(unsigned long long p0, unsigned long long p1);
    unsigned long long CreateCommunity(unsigned long long p0, unsigned long long p1);
    unsigned long long CreateMatchmakeSession(unsigned long long p0, unsigned long long p1);
    unsigned long long CreateMatchmakeSessionWithTeam(unsigned long long p0, unsigned long long p1, unsigned long long p2, unsigned long long p3, unsigned long long p4);
    unsigned long long Finalize();
    unsigned long long FindCommunityByGatheringId(unsigned long long p0, unsigned long long p1);
    unsigned long long FindCommunityByOwner(unsigned long long p0, unsigned long long p1, unsigned long long p2, unsigned long long p3, unsigned long long p4, unsigned long long p5, unsigned long long p6);
    unsigned long long FindCommunityByParticipant(unsigned long long p0, unsigned long long p1, unsigned long long p2, unsigned long long p3, unsigned long long p4, unsigned long long p5, unsigned long long p6);
    unsigned long long FindOfficialCommunity(unsigned long long p0, unsigned long long p1, unsigned long long p2, unsigned long long p3, unsigned long long p4);
    unsigned long long FindSessionByOwner(unsigned long long p0, unsigned long long p1, unsigned long long p2);
    unsigned long long FindSessionByParticipant(unsigned long long p0, unsigned long long p1, unsigned long long p2);
    unsigned long long FindSessionBySessionId(unsigned long long p0, unsigned long long p1, unsigned long long p2);
    unsigned long long GenerateSystemPassword(unsigned long long p0, unsigned long long p1, unsigned long long p2);
    unsigned int GetBrowsedCommunityResult();
    unsigned int GetBrowseMatchmakeSessionResult();
    unsigned int GetRandomMatchmakeScore();
    int GetSessionInfo();
    unsigned long long GetStationConnectionInfo(unsigned long long p0, unsigned long long p1, unsigned long long p2);
    unsigned long long Initialize();
    unsigned char IsMatchmakeHost();
    unsigned char IsProcessingJoinMatchmakeSession();
    unsigned char IsRandomMatchmakeScoreAvailable();
    unsigned int IsUpdatableSelectionPriority(unsigned long long p0);
    unsigned long long JoinCommunity(unsigned long long p0, unsigned long long p1);
    unsigned long long JoinMatchmakeSession(unsigned long long p0, unsigned long long p1, unsigned long long p2);
    unsigned long long JoinMatchmakeSessionWithTeam(unsigned long long p0, unsigned long long p1, unsigned long long p2, unsigned long long p3, unsigned long long p4, unsigned long long p5);
    unsigned long long LeaveMatchmakeSession(unsigned long long p0, unsigned long long p1, unsigned long long p2);
    unsigned long long MigrateMatchmakeSessionOwner(unsigned long long p0, unsigned long long p1, unsigned long long p2, unsigned long long p3, unsigned long long p4);
    unsigned long long ModifyAttribute(unsigned long long p0, unsigned long long p1, unsigned long long p2, unsigned long long p3, unsigned long long p4);
    unsigned long long OpenParticipate(unsigned long long p0, unsigned long long p1, unsigned long long p2);
    unsigned long long RandomMatchmakeSession(unsigned long long p0, unsigned long long p1);
    unsigned long long RandomMatchmakeSessionWithTeam(unsigned long long p0, unsigned long long p1, unsigned long long p2, unsigned long long p3, unsigned long long p4);
    unsigned long long ReflectCommunityIdList(unsigned long long p0, unsigned long long p1);
    unsigned long long ReflectCreateCommunitySetting(unsigned long long p0, unsigned long long p1);
    void ReflectCreateSetting(unsigned long long p0, unsigned long long p1);
    unsigned long long ReflectJoinCommunitySetting(unsigned long long p0, unsigned long long p1);
    unsigned long long ReflectJoinRandomSetting(unsigned long long p0, unsigned long long p1, unsigned long long p2);
    unsigned long long ReflectJoinSetting(unsigned long long p0, unsigned long long p1);
    unsigned long long ReflectSettingToMatchmakeSession(unsigned long long p0, unsigned long long p1, unsigned long long p2);
    unsigned long long ReflectUpdateCommunitySetting(unsigned long long p0, unsigned long long p1);
    unsigned long long ReflectUpdateSetting(unsigned long long p0, unsigned long long p1, unsigned long long p2);
    void RegisterFindCommunityResult();
    void RegisterFindSessionResult(unsigned long long p0);
    unsigned long long RegisterSearchCriteria(unsigned long long p0, unsigned long long p1, unsigned long long p2);
    unsigned long long RequestSessionInfo(unsigned long long p0, unsigned long long p1, unsigned long long p2);
    unsigned long long RequestSessionInfoWithoutPermission(unsigned long long p0, unsigned long long p1, unsigned long long p2);
    void SetMatchmakeCurrentParticipantNum(unsigned long long p0);
    void SetMatchmakeOwnerUniqueId(unsigned long long p0, unsigned long long p1, unsigned long long p2);
    unsigned long long SetMatchmakeSessionKey(unsigned long long p0, unsigned long long p1);
    unsigned long long SetMatchmakeSessionKey(unsigned long long p0, unsigned long long p1, unsigned long long p2);
    void SetTemporaryOwnerChangeEvent(unsigned long long p0, unsigned long long p1, unsigned long long p2);
    unsigned long long UnregisterMatchmakeSession(unsigned long long p0, unsigned long long p1, unsigned long long p2);
    void UnregisterSearchCriteria();
    unsigned long long UpdateApplicationBuffer(unsigned long long p0, unsigned long long p1, unsigned long long p2, unsigned long long p3, unsigned long long p4);
    unsigned long long UpdateAttributes(unsigned long long p0, unsigned long long p1, unsigned long long p2, unsigned long long p3);
    unsigned long long UpdateCommunity(unsigned long long p0, unsigned long long p1);
    unsigned long long UpdateMatchmakeSessionHost(unsigned long long p0, unsigned long long p1);
    unsigned long long UpdateSelectionPriority(unsigned long long p0, unsigned long long p1, unsigned long long p2);
    unsigned long long UpdateSessionSetting(unsigned long long p0, unsigned long long p1, unsigned long long p2);
    unsigned int WaitBrowseMatchmakeSession();
    unsigned int WaitClearSystemPassword();
    unsigned int WaitCloseParticipate();
    unsigned int WaitCreateCommunity();
    unsigned int WaitCreateMatchmakeSession(unsigned long long p0);
    unsigned int WaitCreateMatchmakeSessionWithTeam(unsigned long long p0);
    unsigned int WaitFindCommunityByGatheringId();
    unsigned int WaitFindCommunityByOwner();
    unsigned int WaitFindCommunityByParticipant();
    unsigned int WaitFindOfficialCommunity();
    unsigned int WaitFindSessionByOwner();
    unsigned int WaitFindSessionByParticipant();
    unsigned int WaitFindSessionBySessionId();
    unsigned int WaitGenerateSystemPassword();
    unsigned int WaitGetStationConnectionInfo(unsigned long long p0);
    unsigned int WaitJoinCommunity();
    unsigned int WaitJoinMatchmakeSession();
    unsigned int WaitJoinMatchmakeSessionWithTeam();
    unsigned int WaitLeaveMatchmakeSession();
    unsigned int WaitLeaveMatchmakeSessionCompanion();
    unsigned int WaitMigrateMatchmakeSessionOwner();
    unsigned int WaitModifyAttribute();
    unsigned int WaitOpenParticipate();
    unsigned int WaitRandomMatchmakeSession(unsigned long long p0, unsigned long long p1);
    unsigned int WaitRandomMatchmakeSessionWithTeam(unsigned long long p0, unsigned long long p1);
    unsigned int WaitRequestSessionInfo();
    unsigned int WaitRequestSessionInfoWithoutPermission();
    unsigned int WaitUnregisterMatchmakeSession();
    unsigned int WaitUpdateApplicationBuffer();
    unsigned int WaitUpdateAttributes();
    unsigned int WaitUpdateCommunity();
    unsigned int WaitUpdateSessionSetting();

private:
    unsigned int m_state;
};

}}}

#endif
