#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_TRANSPORT_SUPPORT_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_TRANSPORT_SUPPORT_HPP

#include "nn/pia.h"

namespace nn { namespace pia { namespace transport {

unsigned char IsInitialized();
unsigned char IsInSetupMode();
void EmbedModuleInfo();

}}}

#endif
