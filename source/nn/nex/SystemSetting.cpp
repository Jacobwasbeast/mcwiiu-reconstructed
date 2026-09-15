#include "nn/nex/SystemSetting.hpp"

#include <algorithm>

namespace nn { namespace nex {

SystemSetting::SettingList* SystemSetting::GetList()
{
    static SettingList settings;
    return &settings;
}

SystemSetting::SystemSetting(const wchar_t* name, unsigned int value)
    : m_pNext(0), m_pPrevious(0), m_name(name), m_value(value), m_state(false)
{
    mVTable = 0x104AC970u;
    GetList()->push_back(this);
}

SystemSetting::~SystemSetting()
{
    SettingList::iterator position = std::find(GetList()->begin(), GetList()->end(), this);
    GetList()->erase(position);
}

}}
