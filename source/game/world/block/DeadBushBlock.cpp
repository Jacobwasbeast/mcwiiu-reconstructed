#include "game/world/block/DeadBushBlock.hpp"

#include "game/core/GenericStats.hpp"
#include "game/core/Stats.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/generation/Player.hpp"

DeadBushBlock::DeadBushBlock()
    : Bush(Material::PLANT)
{
    DerivedInit();
}

DeadBushBlock::~DeadBushBlock()
{
}

const AABB* DeadBushBlock::getShape(const BlockState*, LevelSource*, const BlockPos&)
{
    return NULL;
}

const MaterialColor* DeadBushBlock::getMapColor(const BlockState*, LevelSource*, const BlockPos&)
{
    return MaterialColor::PLANT;
}

bool DeadBushBlock::mayPlaceOn(Level*, const BlockPos&, const BlockState* state)
{
    Block* block = state->getBlock();
    return block == Blocks::grass || block == Blocks::dirt || block == Blocks::farmland || block == Blocks::sand;
}

bool DeadBushBlock::mayReplaceWithPlace(LevelSource*, const BlockPos&)
{
    return true;
}

Item* DeadBushBlock::getResource(const BlockState*, Random*, int)
{
    return NULL;
}

void DeadBushBlock::playerDestroy(Level* level, boost::shared_ptr<Player> player, const BlockPos& pos,
                                  const BlockState* state, boost::shared_ptr<BlockEntity> blockEntity,
                                  not_null_ptr<ItemInstance> tool)
{
    if (!level->mIsClientSide && tool->getItem() == Items::shears)
    {
        player->awardStat(Stats::blockMinedStats.data[Block::getId(this)],
                          GenericStats::param_blocksMined(state, 1));
        Block::popResource(level, pos, not_null_ptr<ItemInstance>(new ItemInstance(Item::byBlock(this), 1, 0)));
        return;
    }

    Block::playerDestroy(level, player, pos, state, blockEntity, tool);
}
