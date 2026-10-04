#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHBROWSEMATCHMAKEJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHBROWSEMATCHMAKEJOB_HPP

#include "nn/pia/Common.hpp"

namespace nn { namespace pia { namespace session { class IMatchmakeSession; class SessionSearchCriteria; }}}

namespace nn { namespace pia { namespace inet {

class NexMatchBrowseMatchmakeJob
{
public:
    NexMatchBrowseMatchmakeJob();
    virtual ~NexMatchBrowseMatchmakeJob();
    unsigned int BrowseMatchmake();
    void Cleanup();
    Result StartupImpl(session::IMatchmakeSession* matchmakeSession, const session::SessionSearchCriteria* searchCriteria);
    Result StartupImpl(session::IMatchmakeSession* matchmakeSession, unsigned int sessionId);
    void Trace(unsigned long long p0) const;
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
