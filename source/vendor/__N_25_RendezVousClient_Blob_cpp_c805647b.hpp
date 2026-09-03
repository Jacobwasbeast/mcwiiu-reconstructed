#ifndef SOURCE_VENDOR_N_25_RENDEZVOUSCLIENT_BLOB_CPP_C805647B_HPP
#define SOURCE_VENDOR_N_25_RENDEZVOUSCLIENT_BLOB_CPP_C805647B_HPP

#include <stdint.h>

namespace nn
{
namespace nex
{
class CallContext;
class UserContext;
}
}

class __N_25_RendezVousClient_Blob_cpp_c805647b
{
public:
    uint64_t m_identity;
    uint32_t m_contextState;
    uint32_t m_operationState;
    uint32_t m_callbackState;
    uint64_t m_schedulerState;
    void* m_threadState;
    uint64_t m_completionState;
    uint64_t CallOperationOnCompletion(nn::nex::CallContext*, const nn::nex::UserContext*);
};

#endif
