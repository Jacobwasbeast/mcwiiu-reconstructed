#include "game/minigame/CompleteAllRuleDefinition.hpp"

#include "game/core/GameRule.hpp"
#include "game/core/StringConversion.hpp"
#include "game/core/StringUtils.hpp"
#include "game/item/ItemInstance.hpp"

std::wstring CompleteAllRuleDefinition::generateDescriptionString(const std::wstring& description, void* value, int count)
{
    std::wstring text = description;
    const int* values = static_cast<const int*>(value);

    text = replaceAll(text, L"___progress___", _toString(values[1]));
    text = replaceAll(text, L"___goal___", _toString(values[0]));
    return text;
}

void CompleteAllRuleDefinition::getChildren(std::vector<GameRuleDefinition*>* children)
{
    CompoundGameRuleDefinition::getChildren(children);
}

void CompleteAllRuleDefinition::updateStatus(GameRule* rule)
{
    if (rule == NULL)
    {
        return;
    }
    std::vector<GameRuleDefinition*> children;
    CompoundGameRuleDefinition::getChildren(&children);
    bool complete = true;
    for (std::vector<GameRuleDefinition*>::const_iterator it = children.begin(); it != children.end(); ++it)
    {
        if (*it == NULL || !(*it)->getComplete(rule))
        {
            complete = false;
            break;
        }
    }
    setComplete(rule, complete);
}

bool CompleteAllRuleDefinition::onUseBlock(GameRule* rule, int blockId, int x, int y, int z)
{
    bool result = CompoundGameRuleDefinition::onUseBlock(rule, blockId, x, y, z);
    if (result != 0)
    {
        updateStatus(rule);
    }
    return result;
}

bool CompleteAllRuleDefinition::onCollectItem(GameRule* rule, not_null_ptr<ItemInstance> item)
{
    int result = CompoundGameRuleDefinition::onCollectItem(rule, item);
    if (result != 0)
    {
        updateStatus(rule);
    }
    return result;
}

ConsoleGameRules::EGameRuleType CompleteAllRuleDefinition::getActionType()
{
    return static_cast<ConsoleGameRules::EGameRuleType>(27);
}

CompleteAllRuleDefinition::~CompleteAllRuleDefinition()
{
}
