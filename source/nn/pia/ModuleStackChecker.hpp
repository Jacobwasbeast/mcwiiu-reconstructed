#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_MODULESTACKCHECKER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_MODULESTACKCHECKER_HPP

namespace nn { namespace pia {


enum ModuleType
{
    cModuleType_Invalid = 0
};

class ModuleStackChecker
{
public:
    static bool isInvalid(ModuleType moduleType);
    static bool isInclude(ModuleType moduleType);
    static unsigned int getOrder(ModuleType moduleType);
    static unsigned int getTop();
    static int Register(ModuleType moduleType);
    static int Unregister(ModuleType moduleType);
};

}}

#endif
