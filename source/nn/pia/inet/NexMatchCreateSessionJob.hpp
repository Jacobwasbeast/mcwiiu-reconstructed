#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHCREATESESSIONJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHCREATESESSIONJOB_HPP

namespace nn { namespace pia { namespace inet {

class NexMatchCreateSessionJob
{
public:
    NexMatchCreateSessionJob();
    virtual ~NexMatchCreateSessionJob();
    void Cleanup();
    unsigned int CreateMatchmakeSession();
    unsigned int LeaveJoinedMatchmakeSession();
    unsigned int SetNextStepAtFailedCreateMesh(unsigned long long p0, unsigned long long p1, unsigned long long p2);
    void SetNextStepAtWaitCreateMesh();
    unsigned int StartNatSession();
    unsigned long long StartupImpl(unsigned long long p0, unsigned long long p1);
    void Trace(unsigned long long p0);
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
