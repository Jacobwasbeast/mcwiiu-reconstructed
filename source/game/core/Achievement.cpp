#include "game/core/Achievement.hpp"

#include "game/item/ItemInstance.hpp"

#include "game/core/I18n.hpp"
#include "game/item/Item.hpp"
#include "game/world/block/Block.hpp"

#include <cstddef>
#include <vector>

static std::vector<Achievement*>* achievements = new std::vector<Achievement*>();

Achievement::Achievement(int id, const std::wstring& name, int displayColumn, int displayRow,
                         Item* item, Achievement* parent)
    : Stat(id + 5242880, L"achievement." + name),
      displayColumn(displayColumn),
      displayRow(displayRow),
      parent(parent),
      description(I18n::get(L"achievement." + name + L".desc")),
      icon(new ItemInstance(item))
{
}

Achievement::Achievement(int id, const std::wstring& name, int displayColumn, int displayRow,
                         Block* block, Achievement* parent)
    : Stat(id + 5242880, L"achievement." + name),
      displayColumn(displayColumn),
      displayRow(displayRow),
      parent(parent),
      description(I18n::get(L"achievement." + name + L".desc")),
      icon(new ItemInstance(block))
{
}

Achievement::~Achievement()
{
}

Achievement* Achievement::postConstruct(bool addToList)
{
    Stat::postConstruct(addToList);

    achievements->push_back(this);

    return this;
}

bool Achievement::isAchievement()
{
    return true;
}

Achievement* Achievement::setExtended()
{
    Stat::setExtended();

    return this;
}

void Achievement::setGolden()
{
    golden = true;
}

void Achievement::setAwardLocallyOnly()
{
    awardLocallyOnly = true;
}

std::wstring Achievement::getDescription()
{
    if (descFormatter != NULL)
    {
        return descFormatter->format(description);
    }

    return description;
}

int Achievement::getAchievementID()
{
    return id - 5242880;
}
