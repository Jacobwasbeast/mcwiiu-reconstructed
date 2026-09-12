#include "nn/pia/lan/LanFacade.hpp"

#include "nn/pia/lan/LanProtocol.hpp"
#include "nn/pia/lan/LanSupport.hpp"
#include "nn/pia/HeapManager.hpp"
#include "platform/wiiu/pead.hpp"

#include <new>

namespace nn { namespace pia { namespace lan {

LanFacade* g_lanFacade = 0;

namespace
{
void* g_lanFacadeStorage = 0;
}

LanFacade::LanFacade()
    : m_lanProtocol(0),
      m_networkSetting()
{
    m_networkSetting.m_field04 = 0xffffff00;
}

LanFacade::~LanFacade()
{
    m_lanProtocol = 0;
}

Result LanFacade::Initialize()
{
    return 0;
}

Result LanFacade::Bind(const LanNetworkSetting& setting)
{
    if (setting.m_field00 == 0)
        return 0x10c07;

    m_networkSetting = setting;
    if (m_lanProtocol != 0)
        return m_lanProtocol->Bind(setting);
    return 0;
}

Result LanFacade::CreateInstance()
{
    if (IsInitialized() == 0)
        return 0x10c0c;
    if (IsInSetupMode() == 0)
        return 0x10c08;
    if (g_lanFacade != 0)
        return 0x10c0f;

    pead::Heap* heap = common::HeapManager::GetHeap();
    g_lanFacadeStorage = pead::allocateFromSeadHeap(sizeof(LanFacade), heap, 8);
    if (g_lanFacadeStorage == 0)
        return 0x10c01;

    g_lanFacade = new (g_lanFacadeStorage) LanFacade();
    g_lanFacade->Initialize();
    return 0;
}

LanFacade* LanFacade::GetInstance()
{
    return g_lanFacade;
}

}}}
