#include "game/world/block/FlowerPot_BlockColor__L0__createDefault__11BlockColorsSFv.hpp"

#include <boost/shared_ptr.hpp>

#include "game/world/LevelSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/FlowerPotBlockEntity.hpp"

FlowerPot_BlockColor__L0__createDefault__11BlockColorsSFv::FlowerPot_BlockColor__L0__createDefault__11BlockColorsSFv(BlockColors* blockColors)
    : mBlockColors(blockColors)
{
}

int FlowerPot_BlockColor__L0__createDefault__11BlockColorsSFv::getColor(
    const BlockState* state, LevelSource* levelSource, const BlockPos* pos, int tintIndex)
{
    if (levelSource != NULL && pos != NULL)
    {
        boost::shared_ptr<FlowerPotBlockEntity> flowerPot =
            boost::dynamic_pointer_cast<FlowerPotBlockEntity>(levelSource->getBlockEntity(*pos));
        if (flowerPot)
        {
            return mBlockColors->getColor(Block::byItem(flowerPot->getPlantItem())->defaultBlockState(),
                                          levelSource, pos, tintIndex);
        }
    }
    return -1;
}
