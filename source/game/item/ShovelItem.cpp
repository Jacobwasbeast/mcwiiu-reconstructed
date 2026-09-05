#include "game/item/ShovelItem.hpp"

#include "game/item/ItemInstance.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/core/Direction.hpp"

static arrayWithLength<Block*> BLOCKS(12, false);

void ShovelItem::staticCtor()
{
    BLOCKS.data[0] = Blocks::snow_layer;
    BLOCKS.data[1] = Blocks::snow;
    BLOCKS.data[2] = Blocks::sand;
    BLOCKS.data[3] = Blocks::dirt;
    BLOCKS.data[4] = Blocks::grass;
    BLOCKS.data[5] = Blocks::gravel;
    BLOCKS.data[6] = Blocks::clay;
    BLOCKS.data[7] = Blocks::farmland;
    BLOCKS.data[8] = Blocks::soul_sand;
    BLOCKS.data[9] = Blocks::mycelium;
    BLOCKS.data[10] = Blocks::grass_path;
    BLOCKS.data[11] = Blocks::concrete_powder;
}

ShovelItem::ShovelItem(const Item::Tier* tier)
    : DiggerItem(1.0f, -3.0f, tier, BLOCKS)
{
}

ShovelItem::~ShovelItem()
{
}

bool ShovelItem::canDestroySpecial(const BlockState* state)
{
    Block* block = state->getBlock();
    return block == Blocks::snow || block == Blocks::snow_layer;
}

InteractionResult::EInteractionResult ShovelItem::useOn(boost::shared_ptr<Player> player,
                                                          Level* level,
                                                          const BlockPos& pos,
                                                          InteractionHand::EInteractionHand hand,
                                                          const Direction* direction,
                                                          float hitX,
                                                          float hitY,
                                                          float hitZ,
                                                          bool simulate)
{
    not_null_ptr<ItemInstance> itemInstance = ((LivingEntity*)(void*)player.get())->getItemInHand(hand);
    BlockPos targetPos = pos.relative(direction);
    const BlockState* targetState = level->getBlockState(targetPos);
    bool blockUsed =
        targetState->getBlock()->use(level, targetPos, targetState, player, hand, direction,
                                     hitX, hitY, hitZ, simulate);
    if (!blockUsed)
    {
        return InteractionResult::eInteractionResult_Fail;
    }

    if (direction != Direction::DOWN &&
        level->getBlockState(pos.above())->getMaterial() == Material::AIR &&
        level->getBlockState(pos)->getBlock() == Blocks::grass_path)
    {
        if (!simulate)
        {
            level->setBlock(pos, Blocks::dirt->defaultBlockState(), 11, false);
            if (!level->mIsClientSide)
            {
                itemInstance->hurtAndBreak(1, boost::shared_ptr<LivingEntity>(player, (LivingEntity*)(void*)player.get()));
            }
        }
        return InteractionResult::eInteractionResult_Success;
    }

    return InteractionResult::eInteractionResult_Pass;
}
