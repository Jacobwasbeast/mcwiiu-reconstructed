#include "nn/nex/InterfaceTable.hpp"

#include <nn/ac/ac_cpp.h>

namespace nn
{
namespace nex
{

InterfaceTable::InterfaceTable()
{
    m_uiCount = 0;
    m_pInterfaces = 0;
}

InterfaceTable::~InterfaceTable()
{
    cleanup();
}

void InterfaceTable::cleanup()
{
    m_uiCount = 0;
    if (m_pInterfaces != 0)
    {
        delete[] m_pInterfaces;
        m_pInterfaces = 0;
    }
}

bool InterfaceTable::Query()
{
    uint32_t uiAddress = 0;
    uint32_t uiMask = 0;
    if (nn::ac::GetAssignedAddress(&uiAddress).IsFailure())
    {
        return false;
    }
    if (nn::ac::GetAssignedSubnet(&uiMask).IsFailure())
    {
        return false;
    }
    m_pInterfaces = new InterfaceInfo[1];
    m_pInterfaces[0].SetAddress(uiAddress);
    m_pInterfaces[0].SetMask(uiMask);
    m_pInterfaces[0].SetBroadcastAddress(uiAddress | ~uiMask);
    m_uiCount = 1;
    return true;
}

unsigned int InterfaceTable::GetNbInterface()
{
    return m_uiCount;
}

InterfaceInfo& InterfaceTable::operator[](unsigned int uiIndex)
{
    return m_pInterfaces[uiIndex];
}

}
}
