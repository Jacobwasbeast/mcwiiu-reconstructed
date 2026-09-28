#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_SYSTEMSETTING_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_SYSTEMSETTING_HPP

#include <stdint.h>
#include "nn/nex/RootObject.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/qChain.hpp"
#include <list>
#include "nn/nex/qList.hpp"

namespace nn { namespace nex {

class SystemSetting : public RootObject
{
public:
    typedef qChain<SystemSetting*, DefaultChainPolicy<SystemSetting*> > SettingList;

    SystemSetting(const wchar_t* name, unsigned int value);
    ~SystemSetting();
    static SettingList* GetList();

    friend class DefaultChainPolicy<SystemSetting*>;

    SystemSetting* m_pNext;
    SystemSetting* m_pPrevious;
    const wchar_t* m_name;
    unsigned int m_value;
    bool m_state;
    uint32_t mVTable;

    bool CheckIfMutable() const;
    void SetValue(unsigned int value);
    static SystemSetting* Find(const wchar_t* name);
    static qList<String> GetAllNames();
};

}}

#endif
