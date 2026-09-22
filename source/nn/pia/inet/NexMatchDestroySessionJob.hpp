#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHDESTROYSESSIONJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHDESTROYSESSIONJOB_HPP

namespace nn { namespace pia { namespace inet {

class NexMatchDestroySessionJob
{
public:
    NexMatchDestroySessionJob();
    virtual ~NexMatchDestroySessionJob();
    void Cleanup();
    void SetNextStepAtMeshCleanup();
    void SetNextStepAtWaitDestroyMesh();
    unsigned int StartupImpl();
    void Trace(unsigned long long p0);
    unsigned int UnregisterBufferMatchmakeSession();
    unsigned int UnregisterCurrentMatchmakeSession();
    unsigned int WaitUnregisterBufferMatchmakeSession();
    unsigned int WaitUnregisterCurrentMatchmakeSession();

private:
    unsigned int m_state;
};

}}}

#endif
