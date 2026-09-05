#include "game/core/BubbleColumnSegment.hpp"

#include "game/entity/mob/Entity.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/util/Random.hpp"

static Property* gBubbleColumnSegmentDrag = NULL;

BubbleColumnSegment::BubbleColumnSegment(Material* material)
    : Block(material),
      mUnknown(0)
{
    TypedBoxed<bool> drag(true);
    DerivedInit();
    registerDefaultState(defaultBlockState()->setBoxedValue(gBubbleColumnSegmentDrag, &drag));
}

BubbleColumnSegment::~BubbleColumnSegment()
{
}

bool BubbleColumnSegment::isCubeShaped(const BlockState* state)
{
    return false;
}

const AABB* BubbleColumnSegment::getClipAABB(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    return NULL;
}

bool BubbleColumnSegment::isSolidRender(const BlockState* state) const
{
    return false;
}

void BubbleColumnSegment::entityInside(Level* level, const BlockPos& pos, const BlockState* state,
                                       const boost::shared_ptr<Entity>& entity)
{
    const BlockState* aboveState = level->getBlockState(pos.above());
    if (aboveState->getBlock() == Blocks::WATER || aboveState->getBlock() == Blocks::bubble_column_segment)
    {
        entity->onInsideBubbleColumn(getFlowDownward(state));
    }
}

void BubbleColumnSegment::onPlace(Level* level, const BlockPos& pos, const BlockState* state)
{
    level->neighborChanged(pos, this, pos);
}

bool BubbleColumnSegment::getFlowDownward(const BlockState* state)
{
    if (state->getBlock() == Blocks::WATER)
        return false;
    if (state->getBlock() == Blocks::bubble_column_segment)
        return static_cast<const TypedBoxed<bool>*>(state->getBoxedValue(gBubbleColumnSegmentDrag))->getValue();
    return true;
}

bool BubbleColumnSegment::isValidBubbleColumnLocation(Level* level, const BlockPos& pos)
{
    const BlockState* state = level->getBlockState(pos);
    const BlockState* above = level->getBlockState(pos.above());
    if (state->getBlock() == Blocks::WATER && above->getBlock() == Blocks::AIR)
        return true;
    if (state->getBlock() != Blocks::bubble_column_segment)
        return true;
    return level->getBlockState(pos.west())->getBlock() != NULL
        && level->getBlockState(pos.east())->getBlock() != NULL
        && level->getBlockState(pos.north())->getBlock() != NULL
        && level->getBlockState(pos.south())->getBlock() != NULL;
}

bool BubbleColumnSegment::addBubbleColumnSegment(Level* level, const BlockPos& pos)
{
    BlockPos above = pos.above();
    if (level->getBlockState(above)->getBlock() != Blocks::WATER
        && level->getBlockState(above)->getBlock() != Blocks::bubble_column_segment
        && !level->getBlockState(above)->getBlock()->mayReplaceWithPlace(level, above))
        return false;
    if (!isValidBubbleColumnLocation(level, above))
        return false;
    TypedBoxed<bool> drag(getFlowDownward(level->getBlockState(pos)));
    const BlockState* bubble = Blocks::bubble_column_segment->defaultBlockState()->setBoxedValue(gBubbleColumnSegmentDrag, &drag);
    level->setBlock(above, bubble, 2, false);
    level->setExtraBlock(above, Blocks::bubble_column_segment->defaultBlockState(), 2, false);
    return true;
}

void BubbleColumnSegment::tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random)
{
    if (isValidBubbleColumnLocation(level, pos))
        addBubbleColumnSegment(level, pos);
    else
    {
        level->setBlockAndUpdate(pos, Blocks::WATER->defaultBlockState());
        level->neighborChanged(pos, level->getBlock(pos), pos);
    }
}

void BubbleColumnSegment::RectifyBubbleFlow(Level* level, const BlockPos& pos, bool down)
{
    TypedBoxed<bool> drag(down);
    const BlockState* state = Blocks::bubble_column_segment->defaultBlockState()->setBoxedValue(gBubbleColumnSegmentDrag, &drag);
    level->setBlockAndUpdate(pos, state);
}

int BubbleColumnSegment::getTickDelay(Level* level)
{
    return 5;
}

void BubbleColumnSegment::animateTick(const BlockState* state, Level* level, const BlockPos& pos, Random* random)
{
    const BlockPos above = pos.above();
    const BlockState* aboveState = level->getBlockState(above);
    if (aboveState->getBlock() == Blocks::WATER || aboveState->getBlock() == Blocks::bubble_column_segment)
    {
        if (random->nextInt(4) == 0)
            level->addAlwaysVisibleParticle(ePARTICLE_BUBBLE, pos.x + random->nextFloat(), pos.y + 1.0,
                                            pos.z + random->nextFloat(), 0.0, 0.98, 0.0,
                                            arrayWithLength<int>(0, true));
        if (random->nextInt(200) == 0)
            level->addAlwaysVisibleParticle(ePARTICLE_BUBBLE_POP, pos.x, pos.y + 1.0, pos.z,
                                            0.0, 0.0, 0.0, arrayWithLength<int>(0, true));
    }
}

void BubbleColumnSegment::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos,
                                          Block* block, const BlockPos& fromPos)
{
    level->neighborChanged(pos, this, pos);
}

bool BubbleColumnSegment::canSurvive(Level* level, const BlockPos& pos, const BlockState* state)
{
    const BlockState* below = level->getBlockState(pos.below());
    if (below->getBlock() != Blocks::WATER && below->getBlock() != Blocks::bubble_column_segment
        && below->getBlock() != Blocks::soul_sand)
        return false;
    if (isValidBubbleColumnLocation(level, pos))
    {
        if (below->getBlock() == Blocks::WATER)
            RectifyBubbleFlow(level, pos, getFlowDownward(below));
        else if (getFlowDownward(below) != getFlowDownward(state))
        {
            TypedBoxed<bool> drag(below->getBlock() == Blocks::soul_sand);
            level->setBlock(pos, Blocks::bubble_column_segment->defaultBlockState()->setBoxedValue(gBubbleColumnSegmentDrag, &drag), 2, false);
        }
        return true;
    }
    return false;
}

bool BubbleColumnSegment::canInstantlyTick()
{
    return false;
}

bool BubbleColumnSegment::isPathfindable(LevelSource* levelSource, const BlockPos& pos)
{
    return true;
}

bool BubbleColumnSegment::mayPick()
{
    return false;
}

Item* BubbleColumnSegment::getResource(const BlockState* state, Random* random, int lootBonus)
{
    return NULL;
}

int BubbleColumnSegment::getResourceCount(Random* random)
{
    return 0;
}

int BubbleColumnSegment::getRenderLayer()
{
    return 1;
}

EBlockFaceShape BubbleColumnSegment::getBlockFaceShape(LevelSource* levelSource, const BlockState* state,
                                                       const BlockPos& pos, const Direction* direction)
{
    return BLOCK_FACE_UNDEFINED;
}

EBlockRenderShape BubbleColumnSegment::getRenderShape(const BlockState* state)
{
    return static_cast<EBlockRenderShape>(-1);
}

const BlockState* BubbleColumnSegment::getBlockState(int data)
{
    TypedBoxed<bool> drag(data != 0);
    return defaultBlockState()->setBoxedValue(gBubbleColumnSegmentDrag, &drag);
}

int BubbleColumnSegment::convertBlockStateToLegacyData(const BlockState* state)
{
    return getFlowDownward(state) ? 1 : 0;
}

bool BubbleColumnSegment::canContainLiquid(const BlockState* state) const
{
    return true;
}

bool BubbleColumnSegment::mayReplaceWithPlace(LevelSource* levelSource, const BlockPos& pos)
{
    return true;
}

EPistonPushReaction BubbleColumnSegment::getPistonPushReaction(const BlockState* state)
{
    return PISTON_PUSH_DESTROY;
}

BlockStateDefinition* BubbleColumnSegment::createBlockStateDefinition()
{
    if (gBubbleColumnSegmentDrag == NULL)
        gBubbleColumnSegmentDrag = BooleanProperty::create(L"drag");
    const Property* properties[1] = { gBubbleColumnSegmentDrag };
    return new BlockStateDefinition(this, properties, 1);
}
