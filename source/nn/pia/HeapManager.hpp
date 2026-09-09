#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_HEAPMANAGER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_HEAPMANAGER_HPP

#include <stdint.h>

#include "platform/wiiu/pead.hpp"
#include "nn/pia/ModuleStackChecker.hpp"

namespace nn { namespace pia { namespace common {

class HeapManager
{
public:
    static pead::Heap* GetHeap(ModuleType moduleType);
    static pead::Heap* GetHeap();
    static void SetCurrentHeap(ModuleType moduleType);
    static void ClearCurrentHeap();
    static void Setup(ModuleType moduleType, unsigned int size,
                      const pead::SafeStringBase<char>& name);
    static void Cleanup(ModuleType moduleType);

private:
    static unsigned int convert(ModuleType moduleType);
};

}}}

#endif
