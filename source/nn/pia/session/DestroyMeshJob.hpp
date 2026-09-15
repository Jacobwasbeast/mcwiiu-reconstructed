#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_DESTROY_MESH_JOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_DESTROY_MESH_JOB_HPP
#include "nn/pia.h"
#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/common/StepSequenceJob.hpp"
namespace nn { namespace pia { namespace session {
class DestroyMeshJob : public common::StepSequenceJob {
public:
    DestroyMeshJob(); virtual ~DestroyMeshJob();
    Result Startup(common::CallContext*, bool); unsigned int AssociateSystemWith(common::CallContext*);
    unsigned int SendDestroyMesh(); unsigned int WaitDestroyResponse(); void CleanupStatus();
    unsigned int CleanupMesh(); void ReceiveDestroyResponse(StationIndex); void Cleanup();
    virtual void Trace(unsigned long long) const;
    common::CallContext* m_callerContext; common::CallContext m_callContext;
    unsigned int m_step; unsigned int m_timeout; bool m_started; bool m_systemDestroy;
    unsigned char m_stationResponses[32];
};
}}}
#endif
