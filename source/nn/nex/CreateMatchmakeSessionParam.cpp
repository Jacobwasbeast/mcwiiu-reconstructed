#include "nn/nex/CreateMatchmakeSessionParam.hpp"

namespace nn
{
namespace nex
{

namespace
{

const unsigned short DEFAULT_PARTICIPATION_COUNT = 1;

}

CreateMatchmakeSessionParam::~CreateMatchmakeSessionParam()
{
}

void CreateMatchmakeSessionParam::Reset()
{
    m_MatchmakeSession = MatchmakeSession();
    m_AdditionalParticipants = qList<unsigned int>();
    m_uiGatheringIDForParticipationCheck = 0;
    m_uiCreateMatchmakeSessionOption = 0;
    m_strJoinMessage = String();
    m_uiParticipationCount = DEFAULT_PARTICIPATION_COUNT;
}

}
}
