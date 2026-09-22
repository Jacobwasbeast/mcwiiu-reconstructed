#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHBROWSEMATCHMAKEJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHBROWSEMATCHMAKEJOB_HPP

namespace nn { namespace pia { namespace inet {

class NexMatchBrowseMatchmakeJob
{
public:
    NexMatchBrowseMatchmakeJob();
    virtual ~NexMatchBrowseMatchmakeJob();
    unsigned int BrowseMatchmake();
    void Cleanup();
    void StartupImpl(unsigned long long p0, unsigned long long p1);
    unsigned long long StartupImpl(unsigned long long p0, unsigned long long p1, unsigned long long p2);
    void Trace(unsigned long long p0);
    unsigned int WaitBrowseMatchmake();
    unsigned int WaitFindSessionByOwner();
    unsigned int WaitFindSessionByParticipant();
    unsigned int WaitFindSessionBySessionId();
    unsigned int WaitRequestSessionInfo();

private:
    unsigned int m_state;
};

}}}

#endif
