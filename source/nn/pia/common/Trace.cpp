#include "nn/pia/common/Trace.hpp"
#include "nn/pia/Common.hpp"
#include "nn/pia/HeapManager.hpp"
#include "platform/wiiu/pead.hpp"

#include <string.h>
#include <new>

namespace nn { namespace pia { namespace common {

Trace* Trace::s_instance = 0;
void* Trace::s_storage = 0;

Trace::Trace() : m_level(0), m_enabled(1), m_start(), m_flags(0), m_ready(true)
{
    m_start.SetNow();
}

void Trace::CreateInstance()
{
    if (IsInitialized() == 0)
        return;
    if (IsInSetupMode() == 0)
        return;
    if (s_instance != 0)
        return;
    s_storage = pead::allocateFromSeadHeap(sizeof(Trace), HeapManager::GetHeap(), 8);
    if (s_storage == 0)
        return;
    memset(s_storage, 0, sizeof(Trace));
    s_instance = new (s_storage) Trace();
}

void Trace::DestroyInstance()
{
    s_instance = 0;
    if (s_storage != 0)
    {
        pead::getArraySize(s_storage);
        pead::freeToSeadHeap(s_storage);
        s_storage = 0;
    }
}

}}}
