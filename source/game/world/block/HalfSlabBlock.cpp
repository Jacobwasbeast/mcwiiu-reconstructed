#include "game/world/block/HalfSlabBlock.hpp"

#include <typeinfo>

#include <boost/unordered_set.hpp>

#include "game/core/Direction.hpp"
#include "game/util/Random.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/TypedBoxed.hpp"

class AABB;
extern Block* gHalfSlabStone;
extern Block* gHalfSlabWood;
extern Block* gHalfSlabStone2;
extern Block* gHalfSlabPurpur;
extern Block* gHalfSlabPrismarine;
extern Block* gDoubleSlabPurpur;
extern Block* gDoubleSlabWood;
extern Block* gDoubleSlabStone2;
extern Block* gDoubleSlabStone;
extern Block* gDoubleSlabPrismarine;

static const AABB* FULL_BLOCK_SHAPE = NULL;
static const AABB* BOTTOM_AABB = NULL;
static const AABB* TOP_AABB = NULL;

AbstractProperty<HalfSlabBlock::Half*>* HalfSlabBlock::TYPE = NULL;
HalfSlabBlock::Half* HalfSlabBlock::TOP = new HalfSlabBlock::Half(L"top");
HalfSlabBlock::Half* HalfSlabBlock::BOTTOM = new HalfSlabBlock::Half(L"bottom");
HalfSlabBlock::Half* HalfSlabBlock::HALVES[HalfSlabBlock::Half::HALF_COUNT] =
{
    HalfSlabBlock::TOP,
    HalfSlabBlock::BOTTOM
};

HalfSlabBlock::Half::Half(const std::wstring& name)
    : mName(name)
{
}

std::wstring HalfSlabBlock::Half::toString() const
{
    return mName;
}

std::wstring HalfSlabBlock::Half::getSerializedName() const
{
    return mName;
}

void HalfSlabBlock::blockStaticCtor()
{
    std::wstring name = L"half";

    boost::unordered_set<Half*> values;
    for (int i = 0; i < Half::HALF_COUNT; ++i)
    {
        values.insert(HALVES[i]);
    }

    TYPE = EnumProperty<Half*>::create(name, typeid(Half), values, NULL);
}

HalfSlabBlock::HalfSlabBlock(Material* material)
    : Block(material)
{
    _init();
}

HalfSlabBlock::HalfSlabBlock(Material* material, const MaterialColor* materialColor)
    : Block(material, materialColor)
{
    _init();
}

HalfSlabBlock::~HalfSlabBlock()
{
}

void HalfSlabBlock::_init()
{
    mBlockType = EBLOCK_TYPE_SLAB;

    setLightBlock(255);
}

void HalfSlabBlock::DerivedInit()
{
    Block::DerivedInit();

    mFullBlock = isFullSize();

    setLightBlock(255);
}

bool HalfSlabBlock::isSilkTouchable()
{
    return false;
}

const AABB* HalfSlabBlock::getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    if (isFullSize())
    {
        return FULL_BLOCK_SHAPE;
    }

    return state->getValue(TYPE) == TOP ? TOP_AABB : BOTTOM_AABB;
}

bool HalfSlabBlock::isTopSolidBlocking(const BlockState* state)
{
    HalfSlabBlock* block = (HalfSlabBlock*)state->getBlock();

    return block->isFullSize() || state->getValue(TYPE) == TOP;
}

EBlockFaceShape HalfSlabBlock::getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos, const Direction* direction)
{
    HalfSlabBlock* block = (HalfSlabBlock*)state->getBlock();

    if (block->isFullSize())
    {
        return BLOCK_FACE_SOLID;
    }

    if (direction == Direction::UP && state->getValue(TYPE) == TOP)
    {
        return BLOCK_FACE_SOLID;
    }

    if (direction == Direction::DOWN && state->getValue(TYPE) == BOTTOM)
    {
        return BLOCK_FACE_SOLID;
    }

    return BLOCK_FACE_UNDEFINED;
}

bool HalfSlabBlock::isSolidRender(const BlockState* state) const
{
    return isFullSize();
}

const BlockState* HalfSlabBlock::getStateForPlacement(Level* level, const BlockPos& pos, const Direction* direction, float hitX, float hitY, float hitZ, int auxValue, boost::shared_ptr<LivingEntity> placer)
{
    const BlockState* state = Block::getStateForPlacement(level, pos, direction, hitX, hitY, hitZ, auxValue, placer);

    TypedBoxed<Half*> bottom(BOTTOM);

    state = state->setBoxedValue(TYPE, &bottom);

    if (!isFullSize() && (direction == Direction::DOWN || (direction != Direction::UP && hitY > 0.5f)))
    {
        TypedBoxed<Half*> top(TOP);

        return state->setBoxedValue(TYPE, &top);
    }

    return state;
}

int HalfSlabBlock::getResourceCount(Random* random)
{
    return isFullSize() ? 2 : 1;
}

bool HalfSlabBlock::isCubeShaped(const BlockState* state)
{
    return isFullSize();
}

bool HalfSlabBlock::isHalfSlab(Block* block)
{
    return block == gHalfSlabStone || block == gHalfSlabWood || block == gHalfSlabStone2
        || block == gHalfSlabPurpur || block == gHalfSlabPrismarine;
}

bool HalfSlabBlock::isHalfSlab(const BlockState* state)
{
    return isHalfSlab(state->getBlock());
}

bool HalfSlabBlock::isHalfOrFullSlab(Block* block)
{
    return isHalfSlab(block) || isFullSlab(block);
}

bool HalfSlabBlock::shouldRenderFace(const BlockState* state, LevelSource* levelSource, const BlockPos& pos, const Direction* direction)
{
    if (isFullSize())
    {
        return Block::shouldRenderFace(state, levelSource, pos, direction);
    }

    if (direction != Direction::UP && direction != Direction::DOWN
        && !Block::shouldRenderFace(state, levelSource, pos, direction))
    {
        return false;
    }

    const BlockState* neighbourState = levelSource->getBlockState(pos.relative(direction));

    bool topSlab = isHalfSlab(state) && state->getValue(TYPE) == TOP;

    if (isHalfSlab(neighbourState) && neighbourState->getValue(TYPE) == TOP)
    {
        if (direction == Direction::DOWN)
        {
            return true;
        }

        if (direction == Direction::UP && Block::shouldRenderFace(state, levelSource, pos, direction))
        {
            return true;
        }

        return !(isHalfSlab(state) && topSlab);
    }

    if (direction != Direction::UP
        && (direction != Direction::DOWN || !Block::shouldRenderFace(state, levelSource, pos, direction)))
    {
        return !(isHalfSlab(neighbourState) && !topSlab);
    }

    return true;
}

bool HalfSlabBlock::isPathfindable(LevelSource* levelSource, const BlockPos& pos)
{
    const BlockState* state = levelSource->getBlockState(pos);

    if (!isFullSize() && state->getValue(TYPE) == BOTTOM)
    {
        return true;
    }

    return Block::isPathfindable(levelSource, pos);
}

bool HalfSlabBlock::isSlabBlock()
{
    return true;
}

bool HalfSlabBlock::isFullSlab(Block* block)
{
    return block == gDoubleSlabPurpur || block == gDoubleSlabWood || block == gDoubleSlabStone2
        || block == gDoubleSlabStone || block == gDoubleSlabPrismarine;
}

Block* HalfSlabBlock::GetDoubleSlabBlockFromSlab(Block* block)
{
    if (block == gHalfSlabStone)
    {
        return gDoubleSlabStone;
    }

    if (block == gHalfSlabWood)
    {
        return gDoubleSlabWood;
    }

    if (block == gHalfSlabStone2)
    {
        return gDoubleSlabStone2;
    }

    if (block == gHalfSlabPurpur)
    {
        return gDoubleSlabPurpur;
    }

    if (block == gHalfSlabPrismarine)
    {
        return gDoubleSlabPrismarine;
    }

    return NULL;
}

bool HalfSlabBlock::liquidCanFlowIntoFromDirection(LevelSource* levelSource, const BlockPos& pos, const Direction* direction) const
{
    if (direction == Direction::UP || direction == Direction::DOWN)
    {
        if (isHalfSlab(levelSource->getBlockState(pos)))
        {
            const Direction* halfFace =
                (levelSource->getBlockData(pos.x, pos.y, pos.z) & 8) != 0 ? Direction::UP : Direction::DOWN;

            if (direction == halfFace)
            {
                return false;
            }
        }
    }

    return true;
}

bool HalfSlabBlock::isBottomSlab(int data)
{
    return (data & 8) == 0;
}
