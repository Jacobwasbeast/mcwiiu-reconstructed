#include "game/core/IntegerCondition.hpp"

#include "game/core/CommonMasterGameMode.hpp"
#include "game/core/GameModeOptions.hpp"
#include "game/core/MasterGameMode.hpp"
#include "game/core/XmlConstruct.hpp"
#include "game/core/XmlObject.hpp"
#include "game/minigame/MiniGameDef.hpp"

IntegerCondition::IntegerCondition(const XmlObject& object)
{
    maybe<std::wstring> op = object[L"op"].maybeString();
    if (op.isSet())
        _init(object, parseOperator(op.get()).second);
    else
        _init(object, EOperator_LessThan);
}

IntegerCondition::~IntegerCondition()
{
}

void IntegerCondition::_init(const XmlObject& object, EOperator op)
{
    maybe<int> value = object[L"value"].maybeInt();
    m_threshold = value.isSet() ? value.get() : 0;

    maybe<std::wstring> option = object[L"value"].maybeString();
    if (option.isSet())
    {
        std::pair<bool, eCustomGameModeHostOption> parsed = GameModeOptions::parse(option.get());
        m_option = parsed.first ? static_cast<int>(parsed.second) : -1;
    }
    else
    {
        m_option = -1;
    }

    m_operator = op;
}

std::pair<bool, EOperator> IntegerCondition::parseOperator(std::wstring name)
{
    if (name == L"LessThan")
        return std::make_pair(true, EOperator_LessThan);
    if (name == L"LessThanOrEquals")
        return std::make_pair(true, EOperator_LessThanOrEquals);
    if (name == L"GreaterThan")
        return std::make_pair(true, EOperator_GreaterThan);
    if (name == L"GreaterThanOrEquals")
        return std::make_pair(true, EOperator_GreaterThanOrEquals);
    return std::make_pair(false, EOperator_LessThan);
}

bool IntegerCondition::IsSatisfied(MasterGameMode* gameMode) const
{
    int value = GetValue(gameMode);
    int threshold = static_cast<int>(GetThreshold(gameMode));

    if (m_operator == EOperator_LessThan)
        return value < static_cast<int>(threshold);
    if (m_operator == EOperator_LessThanOrEquals)
        return value <= static_cast<int>(threshold);
    if (m_operator == EOperator_GreaterThan)
        return threshold < value;
    if (m_operator == EOperator_GreaterThanOrEquals)
        return threshold <= value;
    return false;
}

unsigned int IntegerCondition::GetThreshold(MasterGameMode* gameMode) const
{
    if (m_option >= 0)
        return CommonMasterGameMode::GetMiniGame()->HostOption_Time(static_cast<eCustomGameModeHostOption>(m_option));
    return static_cast<unsigned int>(m_threshold);
}

unsigned char IntegerCondition::GetMessage() const
{
    static const unsigned char messages[] = { 0x82, 0x83, 0x84, 0x85 };
    unsigned int op = static_cast<unsigned int>(m_operator);
    if (op > EOperator_GreaterThanOrEquals)
        return 0x82;
    return messages[op];
}

void IntegerCondition::AddIntParams(MasterGameMode* gameMode, std::vector<int>* params) const
{
    params->push_back(GetThreshold(gameMode));
}
