#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_JOINT_SESSION_JOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_JOINT_SESSION_JOB_HPP

#include "nn/pia/Common.hpp"
#include "nn/pia/ListBase.hpp"
#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/common/StepSequenceJob.hpp"
#include "nn/pia/session/Session.hpp"
#include "nn/pia/session/SessionProtocol.hpp"

namespace nn { namespace pia { namespace session {

class CreateSessionSetting;
class JoinRandomSessionSetting;
class JoinSessionSetting;

class JointSessionJob : public common::StepSequenceJob
{
public:
    class StationIdNode : public common::ListNode
    {
    public:
        unsigned long long m_stationId;
    };

    JointSessionJob();
    virtual ~JointSessionJob();
    virtual void Trace(unsigned long long value) const;
    virtual unsigned int GetJointSessionStateForDebug() const = 0;
    virtual void InitializeImpl();
    virtual void FinalizeImpl();
    virtual Result StartupJoinRandomJointSessionImpl(
        const JoinRandomSessionSetting* setting) = 0;
    virtual Result StartupCreateJointSessionImpl(const CreateSessionSetting* setting) = 0;
    virtual Result StartupJoinJointSessionImpl(const JoinSessionSetting* setting) = 0;
    virtual Result StartupLeaveJointSessionImpl() = 0;
    virtual Result StartupDestroyJointSessionImpl() = 0;
    virtual Result StartupPassiveProcessImpl(Session::ProcessType processType,
                                             SessionProtocol::MessageType messageType,
                                             const unsigned long long* stationIds,
                                             unsigned short stationIdNum,
                                             unsigned long long senderStationId) = 0;
    virtual Result ReceiveNextSessionIdImpl(Session::ProcessType processType,
                                            unsigned int sessionId,
                                            const unsigned char* applicationData,
                                            unsigned char applicationDataSize,
                                            unsigned long long senderStationId) = 0;
    virtual Result ReceiveLeaveInvitationImpl(Session::ProcessType processType,
                                              const unsigned long long* stationIds,
                                              unsigned short stationIdNum,
                                              unsigned long long senderStationId) = 0;
    virtual Result ReceiveLeaveMeshInvitationImpl(Session::ProcessType processType,
                                                  unsigned long long senderStationId) = 0;
    virtual Result ReceiveCompletionResponseImpl(Session::ProcessType processType,
                                                 unsigned long long senderStationId) = 0;
    virtual void NoticeHostChangeEventImpl(unsigned long long hostStationId);
    virtual void NoticeOwnerChangeEventImpl(unsigned int sessionId,
                                            unsigned long long ownerStationId);
    virtual void NoticeLeaveEventImpl(unsigned int sessionId,
                                      unsigned long long stationId);
    virtual void NoticeUnregisterEventImpl(unsigned int sessionId);
    virtual void CleanupImpl() = 0;

    void Cleanup();
    bool AcceptLeaveStationId(unsigned long long stationId);
    bool IsMigrateSessionToJointSession() const;
    bool IsMigrateJointSessionToSession() const;
    void NoticeStartingEventByProcessType();
    void ClearProcessTypeAndNoticeEndingEvent();
    void ClearProcessTypeAndNoticeFailureEvent();

    common::CallContext* m_callerContext;
    common::CallContext m_callContext;
    Session::ProcessType m_processType;
    unsigned int m_field5c;
    unsigned int m_field60;
    unsigned long long m_leaveStationId;
    common::ListBase m_companionStationIdList;
    common::ListBase m_companionStationIdFreeList;
    int m_stationIdNodeOffset;
    StationIdNode* m_stationIdNodes;
    unsigned int m_stationIdNodeNum;
    StationIdNode* m_stationIdNodeStorage;
    unsigned int m_field98;
    unsigned char m_field9c;
    unsigned char m_field9d;
    bool m_leaveStationIdAccepted;
    unsigned char m_field9f;
    unsigned char m_fielda0;
    bool m_sessionDisconnectDetected;
    unsigned char m_fielda2;
    unsigned char m_fielda3;
    unsigned int m_fielda4;
    unsigned char m_reserveda8[8];
    unsigned int m_fieldb0;
    unsigned int m_fieldb4;
    common::Time m_startTime;
    unsigned int m_fieldc0;
    unsigned int m_fieldc4;
    unsigned char m_fieldc8;
};

}}}

#endif
