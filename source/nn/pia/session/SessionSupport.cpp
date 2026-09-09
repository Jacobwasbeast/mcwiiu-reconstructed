#include "nn/pia/session/SessionSupport.hpp"

extern void SetModuleId(const char* moduleId);

namespace nn { namespace pia { namespace session {

namespace
{
unsigned char s_initialized = 0;
unsigned char s_duringSetup = 0;
}

unsigned char IsInitialized()
{
    return s_initialized;
}

unsigned char IsDuringSetup()
{
    return s_duringSetup;
}

void EmbedModuleInfo()
{
    SetModuleId("[SDK+Nintendo:PIA_SESSION_5_9_0]");
}

}}}
