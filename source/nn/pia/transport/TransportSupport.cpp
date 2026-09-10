#include "nn/pia/transport/TransportSupport.hpp"

extern void SetModuleId(const char* moduleId);

namespace nn { namespace pia { namespace transport {

void EmbedModuleInfo()
{
    SetModuleId("[SDK+Nintendo:PIA_TRANSPORT_5_9_0]");
}

}}}
