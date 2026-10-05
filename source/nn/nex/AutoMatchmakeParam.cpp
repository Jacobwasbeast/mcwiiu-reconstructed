#include "nn/nex/AutoMatchmakeParam.hpp"

namespace nn
{
namespace nex
{

namespace
{

const unsigned short DEFAULT_PARTICIPATION_COUNT = 1;

}

AutoMatchmakeParam::~AutoMatchmakeParam()
{
}

void AutoMatchmakeParam::Reset()
{
    m_MatchmakeSession = MatchmakeSession();
    m_AdditionalParticipants = qList<unsigned int>();
    m_uiGatheringIDForParticipationCheck = 0;
    m_uiAutoMatchmakeOption = 0;
    m_strJoinMessage = String();
    m_SearchCriteria = qList<MatchmakeSessionSearchCriteria>();
    m_TargetGatheringIDs = qList<unsigned int>();
    m_uiParticipationCount = DEFAULT_PARTICIPATION_COUNT;
}

}
}
