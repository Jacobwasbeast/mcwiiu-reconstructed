#include "nn/pia/transport/TransportSupport.hpp"

extern void SetModuleId(const char* moduleId);

namespace nn { namespace pia { namespace transport {

namespace
{
unsigned char s_initialized = 0;
unsigned char s_inSetupMode = 0;
}

unsigned char IsInitialized()
{
    return s_initialized;
}

unsigned char IsInSetupMode()
{
    return s_inSetupMode;
}

void EmbedModuleInfo()
{
    SetModuleId("[SDK+Nintendo:PIA_TRANSPORT_5_9_0]");
}

}}}
