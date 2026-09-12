#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_UPDATE_SESSION_SETTING_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_UPDATE_SESSION_SETTING_HPP

#include "nn/pia.h"

namespace nn { namespace pia { namespace session {

class UpdateSessionSetting
{
public:
    enum ModificationFlag
    {
        cModificationFlag_ApplicationData = 1
    };

    UpdateSessionSetting();
    virtual ~UpdateSessionSetting();

    virtual void Reset();
    virtual void Trace(unsigned long long value) const;

    unsigned int GetModificationFlag() const;
    bool IsModificationFlagSet(ModificationFlag flag) const;
    bool IsSetApplicationData() const;

    unsigned int m_modificationFlag;
};

}}}

#endif
