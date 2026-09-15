#include "game/world/block/ShulkerBox_BlockColor__L0__createDefault__11BlockColorsSFv.hpp"

#include "game/client/Minecraft.hpp"
#include "game/core/ColourTable.hpp"
#include "game/world/block/ShulkerBoxBlock.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockPos.hpp"

int ShulkerBox_BlockColor__L0__createDefault__11BlockColorsSFv::getColor(
    const BlockState* state, LevelSource*, const BlockPos*, int)
{
    const ShulkerBoxBlock* shulkerBox = static_cast<const ShulkerBoxBlock*>(state->getBlock());
    return static_cast<int>(Minecraft::GetInstance()->getColourTable()->getColour(
        shulkerBox->getDyeColor()->getShulkerColor()));
}
