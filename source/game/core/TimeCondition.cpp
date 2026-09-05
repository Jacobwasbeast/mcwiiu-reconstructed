#include "game/core/TimeCondition.hpp"

#include "game/core/CommonMasterGameMode.hpp"
#include "game/core/MasterGameMode.hpp"
#include "game/core/XmlConstruct.hpp"
#include "game/core/XmlObject.hpp"
#include "game/minigame/MiniGameDef.hpp"

TimeCondition::TimeCondition(const XmlObject& object)
    : IntegerCondition(object)
    , m_offset(0)
{
    maybe<std::wstring> op = object[L"op"].maybeString();
    if (!op.isSet())
    {
        m_operator = EOperator_GreaterThanOrEquals;
    }
    else
    {
        std::pair<bool, EOperator> parsed = IntegerCondition::parseOperator(op.get());
        if (!parsed.first)
            m_operator = EOperator_GreaterThanOrEquals;
    }

    maybe<int> offset = object[L"offset"].maybeInt();
    if (offset.isSet())
        m_offset = offset.get();
}

TimeCondition::~TimeCondition()
{
}

void TimeCondition::AddIntParams(MasterGameMode* gameMode, std::vector<int>* params) const
{
    params->push_back(GetValue(gameMode));
    params->push_back(0xe5800c3a);
}

unsigned int TimeCondition::GetThreshold(MasterGameMode* gameMode) const
{
    int threshold;
    if (m_option < 0)
    {
        threshold = m_threshold;
    }
    else
    {
        threshold = static_cast<int>(CommonMasterGameMode::GetMiniGame()->HostOption_Time(
            static_cast<eCustomGameModeHostOption>(m_option)));
    }
    return static_cast<unsigned int>(threshold + m_offset);
}

bool TimeCondition::IsSatisfied(MasterGameMode* gameMode) const
{
    int value;
    if (m_option < 0)
    {
        value = m_threshold;
    }
    else
    {
        value = static_cast<int>(CommonMasterGameMode::GetMiniGame()->HostOption_Time(
            static_cast<eCustomGameModeHostOption>(m_option)));
    }

    if (value == -1)
        return false;
    if (value == -2)
        return true;
    return IntegerCondition::IsSatisfied(gameMode);
}
