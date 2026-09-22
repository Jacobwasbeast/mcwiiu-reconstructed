#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHGENERATESYSTEMPASSWORDJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHGENERATESYSTEMPASSWORDJOB_HPP

namespace nn { namespace pia { namespace inet {

class NexMatchGenerateSystemPasswordJob
{
public:
    NexMatchGenerateSystemPasswordJob();
    virtual ~NexMatchGenerateSystemPasswordJob();
    void Cleanup();
    unsigned int GenerateMatchmakeSystemPassword();
    unsigned int StartupImpl();
    void Trace(unsigned long long p0);
    unsigned int WaitGenerateMatchmakeSystemPassword();

private:
    unsigned int m_result;
    bool m_started;
};

}}}

#endif
