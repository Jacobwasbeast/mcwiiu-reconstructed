#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_UPDATEMATCHMAKESESSIONPARAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_UPDATEMATCHMAKESESSIONPARAM_HPP

#include "nn/nex/_DDL_UpdateMatchmakeSessionParam.hpp"

namespace nn
{
namespace nex
{

class UpdateMatchmakeSessionParam : public _DDL_UpdateMatchmakeSessionParam
{
public:
    enum
    {
        ATTRIBUTE_COUNT = 6,
        MAX_PROGRESS_SCORE = 100
    };

    virtual ~UpdateMatchmakeSessionParam();

    void Reset()
    {
        m_uiValue2 = 0;
        m_uiValue1 = 0;
        unsigned int aAttributes[ATTRIBUTE_COUNT] = {0};
        m_Attributes.assign(aAttributes, aAttributes + ATTRIBUTE_COUNT);
        m_bOpenParticipation = true;
        m_ApplicationBuffer = qVector<unsigned char>();
        m_uiProgressScore = MAX_PROGRESS_SCORE;
        m_MatchmakeParam = MatchmakeParam();
        m_strValue1 = String();
        m_StartedTime = DateTime::s_never;
        m_uiValue3 = 0;
        m_strValue2 = String();
        m_uiValue5 = 0;
        m_uiValue4 = 0;
    }
};

}
}

#endif
