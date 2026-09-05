#include "game/core/ConditionConnective.hpp"

#include "game/core/MasterGameMode.hpp"
#include "game/core/XmlObject.hpp"

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
