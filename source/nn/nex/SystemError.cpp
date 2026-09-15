#include "nn/nex/SystemError.hpp"

#include "nn/nex/GlobalVariables.hpp"

namespace nn
{
namespace nex
{

SystemError::~SystemError()
{
}

void SystemError::SignalError(wchar_t*, unsigned int, unsigned int errorCode,
                              unsigned int errorDetail)
{
    GlobalVariables::GetInstancePtr()->GetErrorCode().GetRef() = errorCode;
    GlobalVariables::GetInstancePtr()->GetErrorDetail().GetRef() = errorDetail;
}

void SystemError::EraseErrorElements(unsigned int threadId)
{
    GlobalVariables* globals = GlobalVariables::GetInstancePtr();
    globals->GetErrorCode().ClearValue(threadId);
    globals->GetErrorDetail().ClearValue(threadId);
}

}
}
