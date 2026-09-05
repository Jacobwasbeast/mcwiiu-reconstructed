#include "game/world/block/SoulSandBlock.hpp"

#include "game/core/BubbleColumnSegment.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"

SoulSandBlock::SoulSandBlock()
    : Block(Material::SAND, MaterialColor::SAND)
{
    DerivedInit();
    setTicking(true);
}

SoulSandBlock::~SoulSandBlock()
{
}

const AABB* SoulSandBlock::getClipAABB(const BlockState*, LevelSource*, const BlockPos&)
{
    return NULL;
}

void SoulSandBlock::entityInside(Level*, const BlockPos&, const BlockState*,
                                 const boost::shared_ptr<Entity>& entity)
{
    entity->mXd *= 0.4;
    entity->mZd *= 0.4;
}

void SoulSandBlock::tick(Level* level, const BlockPos& pos, const BlockState*, Random*)
{
    const BlockPos above = pos.above();
    if (level->getBlock(above.above()) == Blocks::AIR && level->isEmptyWaterBlock(above))
    {
        const BlockState* liquidState = level->getLiquidBlockState(above);
        if (liquidState != NULL && liquidState->getBlock()->convertBlockStateToLegacyData(liquidState) != 0)
            return;
        if (!level->isEmptyWaterBlock(above))
            return;
    }
    BubbleColumnSegment* bubbleColumnSegment = static_cast<BubbleColumnSegment*>(Blocks::bubble_column_segment);
    bubbleColumnSegment->addBubbleColumnSegment(level, pos);
}

void SoulSandBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos,
                                    Block*, const BlockPos& fromPos)
{
    if (pos.above() == fromPos && level->isEmptyWaterBlock(pos.above()))
        level->addToTickNextTick(pos, this, level->mRandom->nextInt(2, 3));
}

void SoulSandBlock::onPlace(Level* level, const BlockPos& pos, const BlockState*)
{
    const BlockPos above = pos.above();
    if (level->getBlock(above.above()) == Blocks::AIR && level->isEmptyWaterBlock(above))
    {
        const BlockState* liquidState = level->getLiquidBlockState(above);
        if (liquidState != NULL && liquidState->getBlock()->convertBlockStateToLegacyData(liquidState) != 0)
            return;
        if (!level->isEmptyWaterBlock(above))
            return;
    }
    BubbleColumnSegment* bubbleColumnSegment = static_cast<BubbleColumnSegment*>(Blocks::bubble_column_segment);
    bubbleColumnSegment->addBubbleColumnSegment(level, pos);
}
