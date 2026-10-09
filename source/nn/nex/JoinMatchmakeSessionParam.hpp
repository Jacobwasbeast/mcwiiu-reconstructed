#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_JOINMATCHMAKESESSIONPARAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_JOINMATCHMAKESESSIONPARAM_HPP

#include "nn/nex/_DDL_JoinMatchmakeSessionParam.hpp"

namespace nn
{
namespace nex
{

class JoinMatchmakeSessionParam : public _DDL_JoinMatchmakeSessionParam
{
public:
    enum
    {
        DEFAULT_PARTICIPATION_COUNT = 1
    };

    virtual ~JoinMatchmakeSessionParam();

    void Reset()
    {
        m_uiGatheringID = 0;
        m_AdditionalParticipants = qList<unsigned int>();
        m_uiJoinMatchmakeSessionOption = 0;
        m_uiGatheringIDForParticipationCheck = 0;
        m_uiJoinMatchmakeSessionBehavior = 0;
        m_strUserPassword = String(L"");
        m_strSystemPassword = String(L"");
        m_strJoinMessage = String();
        m_uiExtraParticipants = 0;
        m_uiParticipationCount = DEFAULT_PARTICIPATION_COUNT;
    }
};

}
}

#endif
