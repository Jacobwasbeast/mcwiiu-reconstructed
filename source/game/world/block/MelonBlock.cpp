#include "game/world/block/MelonBlock.hpp"

#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/item/Items.hpp"
#include "game/util/Random.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"

MelonBlock::MelonBlock()
    : Block(Material::VEGETABLE, MaterialColor::COLOR_LIGHT_GREEN)
{
    DerivedInit();
}

MelonBlock::~MelonBlock()
{
}

int MelonBlock::getResourceCount(Random* random)
{
    return random->nextInt(5) + 3;
}

Item* MelonBlock::getResource(const BlockState* state, Random* random, int lootBonus)
{
    return Items::melon;
}

int MelonBlock::getResourceCountForLootBonus(int lootBonus, Random* random)
{
    int count = getResourceCount(random) + random->nextInt(lootBonus + 1);

    if (count > 9)
    {
        count = 9;
    }

    return count;
}

void MelonBlock::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(getIconName() + L"_side");
    mTopIcon = iconRegister->registerIcon(getIconName() + L"_top");
}

TextureAtlasSprite* MelonBlock::getTexture(const Direction* direction, const BlockState* state)
{
    if (direction == Direction::UP || direction == Direction::DOWN)
    {
        return mTopIcon;
    }

    return mIcon;
}
