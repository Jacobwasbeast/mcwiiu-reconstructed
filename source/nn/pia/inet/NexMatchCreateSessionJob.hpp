#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHCREATESESSIONJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHCREATESESSIONJOB_HPP

#include "nn/pia/Common.hpp"

namespace nn { namespace pia { namespace session { class CreateSessionSetting; }}}

namespace nn { namespace pia { namespace inet {

class NexMatchCreateSessionJob
{
public:
    NexMatchCreateSessionJob();
    virtual ~NexMatchCreateSessionJob();
    void Cleanup();
    unsigned int CreateMatchmakeSession();
    unsigned int LeaveJoinedMatchmakeSession();
    unsigned int SetNextStepAtFailedCreateMesh(Result result);
    void SetNextStepAtWaitCreateMesh();
    unsigned int StartNatSession();
    Result StartupImpl(const session::CreateSessionSetting* setting);
    void Trace(unsigned long long p0) const;
    unsigned int UnregisterGathering();
    unsigned int WaitCreateMatchmake();
    unsigned int WaitLeaveJoinedMatchmakeSession();
    unsigned int WaitNotification();
    unsigned int WaitStartNatSession();
    unsigned int WaitUnregisterGathering();

private:
    unsigned int m_state;
};

}}}

#endif
