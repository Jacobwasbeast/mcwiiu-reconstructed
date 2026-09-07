#include "game/core/ConditionConnective.hpp"

#include "game/core/LastKillTimeCondition.hpp"
#include "game/core/MasterGameMode.hpp"
#include "game/core/PostGameTimeCondition.hpp"
#include "game/core/RoundTimeCondition.hpp"
#include "game/core/ShowdownTimeCondition.hpp"
#include "game/core/TeamsCondition.hpp"
#include "game/core/XmlObject.hpp"
#include "game/world/generation/PercentagePlayersVotedCondition.hpp"
#include "game/world/generation/PlayersCondition.hpp"
#include "game/world/generation/PlayersEnteredVolumeCondition.hpp"
#include "game/world/generation/TimeSincePlayersCondition.hpp"

bool equalsIgnoreCase(const std::wstring& first, const std::wstring& second);

ConditionConnective::ConditionConnective(const XmlObject& object)
    : m_type(parseType(object.mName).second), m_conditions()
{
    const std::vector<XmlChild>& children = object.getChildrenRaw();
    for (std::vector<XmlChild>::const_iterator child = children.begin(); child != children.end(); ++child)
    {
        if (child->m_type == 1)
        {
            GameCondition* condition = parseCondition(*child->xml());
            if (condition != NULL)
                m_conditions.push_back(condition);
        }
    }
}

ConditionConnective::~ConditionConnective()
{
    for (std::vector<GameCondition*>::iterator it = m_conditions.begin(); it != m_conditions.end(); ++it)
        delete *it;
}

std::pair<bool, int> ConditionConnective::parseType(std::wstring name)
{
    if (name == L"Conjunction")
        return std::make_pair(true, 0);
    if (name == L"Disjunction")
        return std::make_pair(true, 1);
    return std::make_pair(false, 0);
}

GameCondition* ConditionConnective::parseCondition(const XmlObject& object)
{
    if (equalsIgnoreCase(object.mName, L"PlayersCondition"))
        return new PlayersCondition(object);
    if (equalsIgnoreCase(object.mName, L"TeamsCondition"))
        return new TeamsCondition(object);
    if (equalsIgnoreCase(object.mName, L"RoundTimeCondition"))
        return new RoundTimeCondition(object);
    if (equalsIgnoreCase(object.mName, L"ShowdownTimeCondition"))
        return new ShowdownTimeCondition(object);
    if (equalsIgnoreCase(object.mName, L"PostGameTimeCondition"))
        return new PostGameTimeCondition(object);
    if (equalsIgnoreCase(object.mName, L"LastKillTimeCondition"))
        return new LastKillTimeCondition(object);
    if (equalsIgnoreCase(object.mName, L"TimeSincePlayersCondition"))
        return new TimeSincePlayersCondition(object);
    if (equalsIgnoreCase(object.mName, L"PlayersVotedPercent"))
        return new PercentagePlayersVotedCondition(object);
    if (equalsIgnoreCase(object.mName, L"Conjunction") || equalsIgnoreCase(object.mName, L"Disjunction"))
        return new ConditionConnective(object);
    if (equalsIgnoreCase(object.mName, L"PlayersEnteredVolumeCondition"))
        return new PlayersEnteredVolumeCondition(object);
    return NULL;
}

GameCondition* ConditionConnective::FirstConditionWithValue(bool value, MasterGameMode* gameMode) const
{
    for (std::vector<GameCondition*>::const_iterator it = m_conditions.begin(); it != m_conditions.end(); ++it)
    {
        if ((*it)->IsSatisfied(gameMode) == value)
            return *it;
    }
    return NULL;
}

bool ConditionConnective::IsSatisfied(MasterGameMode* gameMode) const
{
    if (m_type != 0)
    {
        if (m_type == 1)
            return FirstConditionWithValue(true, gameMode) != NULL;
        return true;
    }
    return FirstConditionWithValue(false, gameMode) == NULL;
}

unsigned char ConditionConnective::GetMessage() const
{
    return 0;
}

void ConditionConnective::AddIntParams(MasterGameMode* gameMode, std::vector<int>* params) const
{
}

bool ConditionConnective::UpdateMessage(MasterGameMode* gameMode, int message) const
{
    for (std::vector<GameCondition*>::const_iterator it = m_conditions.begin(); it != m_conditions.end(); ++it)
    {
        if ((*it)->UpdateMessage(gameMode, message))
            return true;
    }
    return false;
}
