#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_CREATEMATCHMAKESESSIONPARAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_CREATEMATCHMAKESESSIONPARAM_HPP

#include "nn/nex/_DDL_CreateMatchmakeSessionParam.hpp"

namespace nn
{
namespace nex
{

class CreateMatchmakeSessionParam : public _DDL_CreateMatchmakeSessionParam
{
public:
    enum
    {
        DEFAULT_PARTICIPATION_COUNT = 1
    };

    virtual ~CreateMatchmakeSessionParam();

    void Reset()
    {
        m_MatchmakeSession = MatchmakeSession();
        m_AdditionalParticipants = qList<unsigned int>();
        m_uiGatheringIDForParticipationCheck = 0;
        m_uiCreateMatchmakeSessionOption = 0;
        m_strJoinMessage = String();
        m_uiParticipationCount = DEFAULT_PARTICIPATION_COUNT;
    }
};

}
}

#endif
