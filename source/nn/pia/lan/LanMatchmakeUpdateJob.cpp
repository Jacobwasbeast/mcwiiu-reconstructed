#include "nn/pia/lan/LanMatchmakeUpdateJob.hpp"

namespace nn { namespace pia { namespace lan {

LanMatchmakeUpdateJob::LanMatchmakeUpdateJob(void* a1)
{
}

LanMatchmakeUpdateJob::~LanMatchmakeUpdateJob()
{
}

uint64_t LanMatchmakeUpdateJob::Startup(uint32_t a1, uint32_t a2)
{
    return 0;
}

uint32_t LanMatchmakeUpdateJob::Cleanup()
{
    return 0;
}

void LanMatchmakeUpdateJob::NotifyDisconnectToMeshLayerController()
{
}

uint64_t LanMatchmakeUpdateJob::SendBrowseReplyMessage(uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5, uint32_t a6, int32_t a7, uint8_t a8)
{
    return 0;
}

uint64_t LanMatchmakeUpdateJob::SendBrowseMessage(uint32_t a1, uint32_t a2)
{
    return 0;
}

uint32_t LanMatchmakeUpdateJob::Update()
{
    return 0;
}

uint32_t LanMatchmakeUpdateJob::RequestBrowseMessage(uint8_t a1)
{
    return 0;
}

bool LanMatchmakeUpdateJob::IsBrowseMessageCompleted()
{
    return 0;
}

void LanMatchmakeUpdateJob::Trace(uint64_t a1)
{
}

}}}
