#ifndef NN_PIA_SESSION_PROCESSDESTROYMESHJOB_HPP
#define NN_PIA_SESSION_PROCESSDESTROYMESHJOB_HPP

#include "nn/pia/common/StepSequenceJob.hpp"

namespace nn { namespace pia { namespace session {

class ProcessDestroyMeshJob : public common::StepSequenceJob
{
public:
    ProcessDestroyMeshJob();
    virtual ~ProcessDestroyMeshJob();

    bool Startup();
    ExecuteResult SendDestroyResponse();
    ExecuteResult CleanupMesh();
    void Cleanup();
    virtual void Trace(unsigned long long value) const;

    unsigned int m_endTick;
    unsigned int m_endTickHigh;
    unsigned int m_timeout;
    unsigned char m_started;
};

}}}

#endif
