#include "nn/pia/transport/DispatchJob.hpp"

#include "nn/pia/Transport.hpp"

namespace nn { namespace pia { namespace transport {

DispatchJob::~DispatchJob()
{
}

unsigned int DispatchJob::ExecuteCore()
{
    g_piaTransport->dispatch();
    return common::Job::cExecuteResultNextStep;
}

}}}
