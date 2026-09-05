#include "game/world/block/ClayBlock.hpp"

#include "game/core/DyeColor.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Material.hpp"

class SeaGrass
{
public:
    static bool trySpawnSeaGrass(bool checkOnly, Level* level, Random* random,
                                 const BlockPos& pos, const BlockState* state);
};

ClayBlock::ClayBlock()
    : Block(Material::CLAY)
{
    DerivedInit();
}

bool ClayBlock::isValidBonemealTarget(Level*, const BlockPos&, const BlockState*, bool)
{
    return true;
}

bool ClayBlock::isBonemealSuccess(Level* level, Random* random, const BlockPos& pos, const BlockState* state)
{
    return SeaGrass::trySpawnSeaGrass(true, level, random, pos, state);
}

void ClayBlock::performBonemeal(Level* level, Random* random, const BlockPos& pos, const BlockState* state)
{
    SeaGrass::trySpawnSeaGrass(false, level, random, pos, state);
}

int ClayBlock::getResourceCount(Random*)
{
    return 4;
}

Item* ClayBlock::getResource(const BlockState*, Random*, int)
{
    return Items::clayBall;
}

unsigned int ClayBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if (Item::getId(tooltipData.m_pItemInstance->getItem()) == 351)
    {
        BlockPos above = tooltipData.m_blockPos.above();
        if (((Level*)tooltipData.m_pLevel)->isUnderWater(above, 270, 0)
            && tooltipData.m_eInteractionResult != 2
            && DyeColor::byItemData(tooltipData.m_pItemInstance->getAuxValue())->getBlockData() == 0)
        {
            return 0x9F5FDC40u;
        }
    }

    return Block::GetInteractTooltip(tooltipData);
}

ClayBlock::~ClayBlock()
{
}
