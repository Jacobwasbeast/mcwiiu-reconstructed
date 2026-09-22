#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHCLEARSYSTEMPASSWORDJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHCLEARSYSTEMPASSWORDJOB_HPP

namespace nn { namespace pia { namespace inet {

class NexMatchClearSystemPasswordJob
{
public:
    NexMatchClearSystemPasswordJob();
    virtual ~NexMatchClearSystemPasswordJob();
    void Cleanup();
    unsigned int ClearMatchmakeSystemPassword();
    unsigned int StartupImpl();
    void Trace(unsigned long long p0);
    unsigned int WaitClearMatchmakeSystemPassword();

private:
    unsigned int m_result;
    bool m_started;
};

}}}

#endif
