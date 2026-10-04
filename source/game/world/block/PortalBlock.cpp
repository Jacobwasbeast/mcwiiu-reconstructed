#include "game/world/block/PortalBlock.hpp"
#include "game/core/EntityType.hpp"

#include <cstring>
#include <typeinfo>
#include <boost/unordered_set.hpp>

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/core/Boxed.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/Direction.hpp"
#include "game/core/GameModeConstraints.hpp"
#include "game/core/GameRules.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Rotation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/EntityIO.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/SpawnEggItem.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockInWorld.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/Property.hpp"
#include "game/world/block/TypedBoxed.hpp"

EnumProperty<Direction::Axis*>* PortalBlock::AXIS = NULL;
static Direction::Axis* gPortalAxes[2];
static const AABB* gPortalShapeX = AABB::newPermanent(0.0, 0.0, 0.375, 1.0, 1.0, 0.625);
static const AABB* gPortalShapeZ = AABB::newPermanent(0.375, 0.0, 0.0, 0.625, 1.0, 1.0);
static const AABB* gPortalShapeY = AABB::newPermanent(0.375, 0.0, 0.375, 0.625, 1.0, 0.625);

PortalBlock::PortalBlock()
    : HalfTransparentBlock(L"portal", Material::PORTAL, false)
{
    DerivedInit();
    TypedBoxed<Direction::Axis*> axis(Direction::Axis::X);
    registerDefaultState(mStateDefinition->any()->setBoxedValue(AXIS, &axis));
    setTicking(true);
}

PortalBlock::~PortalBlock()
{
}

void PortalBlock::blockStaticCtor()
{
    gPortalAxes[0] = Direction::Axis::X;
    gPortalAxes[1] = Direction::Axis::Z;
    std::wstring name(L"axis");
    const Predicate<Direction::Axis*>* filter = Predicates<Direction::Axis*>::alwaysTrue;
    EnumProperty<Direction::Axis*>* axis;
    {
        boost::unordered_set<Direction::Axis*> values;
        values.insert(gPortalAxes[0]);
        values.insert(gPortalAxes[1]);
        axis = EnumProperty<Direction::Axis*>::create(name, typeid(Direction::Axis*), values, filter);
    }
    AXIS = axis;
}

BlockPattern::BlockPatternMatch* PortalBlock::getPortalShape(Level* level, const BlockPos& pos)
{
    Direction::Axis* axis = Direction::Axis::Z;
    PortalShape shape(level, pos, Direction::Axis::X);
    boost::shared_ptr<BlockPattern::BlockLoadingCache> cache(BlockPattern::createLevelCache(level, true));

    if (!shape.isValid())
    {
        axis = Direction::Axis::X;
        shape = PortalShape(level, pos, Direction::Axis::Z);
    }

    if (!shape.isValid())
    {
        return new BlockPattern::BlockPatternMatch(pos, Direction::NORTH, Direction::UP, cache, 1, 1, 1);
    }

    int filled[2];
    std::memset(filled, 0, sizeof(filled));

    const Direction* facing = shape.mRightDir->getCounterClockWise();
    BlockPos topLeft = shape.mBottomLeft->above(shape.getHeight() - 1);

    for (int i = 0; i < 2; ++i)
    {
        Direction::AxisDirection* axisDirection = Direction::AxisDirection::VALUES[i];
        BlockPos frontTopLeft = facing->getAxisDirection() == axisDirection
                                    ? topLeft
                                    : topLeft.relative(shape.mRightDir, shape.getWidth() - 1);
        BlockPattern::BlockPatternMatch match(frontTopLeft, Direction::get(axisDirection, axis),
                                              Direction::UP, cache,
                                              shape.getWidth(), shape.getHeight(), 1);

        for (int x = 0; x < shape.getWidth(); ++x)
        {
            for (int y = 0; y < shape.getHeight(); ++y)
            {
                BlockInWorld* blockInWorld = match.getBlock(x, y, 1);
                if (blockInWorld->getState() != NULL
                    && blockInWorld->getState()->getMaterial() != Material::AIR)
                {
                    ++filled[axisDirection->getOrdinal()];
                }
            }
        }
    }

    Direction::AxisDirection* best = Direction::AxisDirection::POSITIVE;
    for (int i = 0; i < 2; ++i)
    {
        if (filled[Direction::AxisDirection::VALUES[i]->getOrdinal()] < filled[best->getOrdinal()])
            best = Direction::AxisDirection::VALUES[i];
    }

    BlockPos frontTopLeft = facing->getAxisDirection() == best
                                ? topLeft
                                : topLeft.relative(shape.mRightDir, shape.getWidth() - 1);
    return new BlockPattern::BlockPatternMatch(frontTopLeft, Direction::get(best, axis),
                                               Direction::UP, cache,
                                               shape.getWidth(), shape.getHeight(), 1);
}

const AABB* PortalBlock::getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    Direction::Axis* axis = state->getValue(AXIS);
    Direction::Axis::EAxis id = axis->getId();
    if (id == Direction::Axis::eAxis_X)
        return gPortalShapeX;
    if (id == Direction::Axis::eAxis_Z)
        return gPortalShapeZ;
    return gPortalShapeY;
}

void PortalBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random)
{
    Block::tick(level, pos, state, random);

    if (level->mDimension->isNaturalDimension()
        && level->getGameRules()->getBoolean(GameRules::RULE_DOMOBSPAWNING)
        && random->nextInt(2000) < level->getDifficulty()->getId())
    {
        int y = pos.y;
        BlockPos ground = pos;
        while (!level->getBlockState(ground)->isTopSolidBlocking() && ground.y > 0)
        {
            ground = ground.below();
        }

        if (y > 0 && !level->getBlockState(ground.above())->isSolidBlockingCubeAndNotSignalSource())
        {
            int error = 0;
            boost::shared_ptr<Entity> mob =
                SpawnEggItem::spawnMobAt(level, EntityIO::getKey(EntityType::PIG_ZOMBIE),
                                         (double)ground.x + 0.5, (double)ground.y + 1.1,
                                         (double)ground.z + 0.5, &error, std::wstring(L""));
            if (mob)
            {
                mob->mPortalCooldown = mob->getDimensionChangingDelay();
            }
        }
    }
}

const AABB* PortalBlock::getClipAABB(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    return NULL;
}

int PortalBlock::getDataForAxis(Direction::Axis* axis)
{
    if (axis == Direction::Axis::X) return 1;
    if (axis == Direction::Axis::Z) return 2;
    return 0;
}

bool PortalBlock::isCubeShaped(const BlockState* state)
{
    return false;
}

bool PortalBlock::trySpawnPortal(Level* level, const BlockPos& pos, bool create)
{
    PortalShape x(level, pos, Direction::Axis::X);
    if (x.isValid() && x.mNumPortalBlocks == 0)
    {
        if (create) x.createPortalBlocks();
        return true;
    }
    PortalShape z(level, pos, Direction::Axis::Z);
    if (z.isValid() && z.mNumPortalBlocks == 0)
    {
        if (create) z.createPortalBlocks();
        return true;
    }
    return false;
}

void PortalBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block, const BlockPos& fromPos)
{
    Direction::Axis* axis = state->getValue(AXIS);
    if (axis == Direction::Axis::X)
    {
        PortalShape shape(level, pos, Direction::Axis::X);
        if (!shape.isValid() || shape.mNumPortalBlocks < shape.mWidth * shape.mHeight)
        {
            level->setBlockAndUpdate(pos, Blocks::air->defaultBlockState());
        }
    }
    else if (axis == Direction::Axis::Z)
    {
        PortalShape shape(level, pos, Direction::Axis::Z);
        if (!shape.isValid() || shape.mNumPortalBlocks < shape.mWidth * shape.mHeight)
        {
            level->setBlockAndUpdate(pos, Blocks::air->defaultBlockState());
        }
    }
}

bool PortalBlock::shouldRenderFace(const BlockState* state, LevelSource* levelSource, const BlockPos& pos, const Direction* direction)
{
    BlockPos neighbourPos = pos.relative(direction);
    Direction::Axis* axis = NULL;

    if (state->getBlock() == this)
    {
        axis = state->getValue(AXIS);
        if (axis == NULL)
        {
            return false;
        }
        if (axis == Direction::Axis::Z && direction != Direction::EAST && direction != Direction::WEST)
        {
            return false;
        }
        if (axis == Direction::Axis::X && direction != Direction::SOUTH && direction != Direction::NORTH)
        {
            return false;
        }
    }

    bool westOpen = false;
    if (levelSource->getBlockState(neighbourPos.west())->getBlock() == this)
    {
        if (levelSource->getBlockState(neighbourPos.west(2))->getBlock() != this)
        {
            westOpen = true;
        }
    }

    bool eastOpen = false;
    if (levelSource->getBlockState(neighbourPos.east())->getBlock() == this)
    {
        if (levelSource->getBlockState(neighbourPos.east(2))->getBlock() != this)
        {
            eastOpen = true;
        }
    }

    bool northOpen = false;
    if (levelSource->getBlockState(neighbourPos.north())->getBlock() == this)
    {
        if (levelSource->getBlockState(neighbourPos.north(2))->getBlock() != this)
        {
            northOpen = true;
        }
    }

    bool southOpen = false;
    if (levelSource->getBlockState(neighbourPos.south())->getBlock() == this)
    {
        if (levelSource->getBlockState(neighbourPos.south(2))->getBlock() != this)
        {
            southOpen = true;
        }
    }

    bool westEastAllowed = true;
    if (!westOpen && !eastOpen && axis != Direction::Axis::X)
    {
        westEastAllowed = false;
    }
    bool northSouthAllowed = true;
    if (!northOpen && !southOpen && axis != Direction::Axis::Z)
    {
        northSouthAllowed = false;
    }
    if ((!westEastAllowed || (direction != Direction::WEST && direction != Direction::EAST))
        && (!northSouthAllowed || (direction != Direction::NORTH && direction != Direction::SOUTH)))
    {
        return false;
    }
    return true;
}

int PortalBlock::getResourceCount(Random* random) { return 0; }
int PortalBlock::getRenderLayer() { return 1; }

void PortalBlock::entityInside(Level*, const BlockPos& pos, const BlockState*, const boost::shared_ptr<Entity>& entity)
{
    if (entity->GetType() == EntityType::EXPERIENCE_ORB)
        return;
    if (entity->isPassenger() || entity->isVehicle() || !entity->canChangeDimensions())
        return;

    if ((entity->GetType() & EntityType::PLAYER) == EntityType::PLAYER)
    {
        if (Minecraft::GetInstance()->mGameMode == NULL)
            return;
        if (!Minecraft::GetInstance()->mGameMode->getGameModeConstraints()->CanUsePortal())
            return;
    }

    entity->handleInsidePortal(pos);
}

void PortalBlock::animateTick(const BlockState*, Level* level, const BlockPos& pos, Random* random)
{
    if (random->nextInt(100) == 0)
    {
        level->playLocalSound((double)pos.x + 0.5, (double)pos.y + 0.5, (double)pos.z + 0.5,
                              SoundEvents::PORTAL_AMBIENT, SoundSource::eSoundSource_Blocks,
                              0.5f, random->nextFloat() * 0.4f + 0.8f, false, 16.0f);
    }

    for (int i = 0; i < 4; ++i)
    {
        double x = (double)((float)pos.x + random->nextFloat());
        double y = (double)((float)pos.y + random->nextFloat());
        double z = (double)((float)pos.z + random->nextFloat());
        double xa = ((double)random->nextFloat() - 0.5) * 0.5;
        double ya = ((double)random->nextFloat() - 0.5) * 0.5;
        double za = ((double)random->nextFloat() - 0.5) * 0.5;
        int side = random->nextInt(2) * 2 - 1;
        if (level->getBlockState(pos.west())->getBlock() != this
            && level->getBlockState(pos.east())->getBlock() != this)
        {
            x = (double)pos.x + 0.5 + 0.25 * (double)side;
            xa = (double)(random->nextFloat() * 2.0f * (float)side);
        }
        else
        {
            z = (double)pos.z + 0.5 + 0.25 * (double)side;
            za = (double)(random->nextFloat() * 2.0f * (float)side);
        }

        arrayWithLength<int> params;
        level->addParticle(PARTICLE_PORTAL, x, y, z, xa, ya, za, params);
    }
}

not_null_ptr<ItemInstance> PortalBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState*)
{
    return ItemInstance::EMPTY;
}

bool PortalBlock::isWaterBlocking() const { return true; }
bool PortalBlock::waterSpreadCausesSpawn() { return false; }

const BlockState* PortalBlock::getBlockState(int data)
{
    const BlockState* state = defaultBlockState();
    return state->setValue(AXIS, (data & 3) == 2 ? Direction::Axis::Z : Direction::Axis::X);
}

int PortalBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return getDataForAxis(state->getValue(AXIS));
}

const BlockState* PortalBlock::rotate(const BlockState* state, Rotation* rotation)
{
    if (rotation->getType() != Rotation::eTYPE_Clockwise90 && rotation->getType() != Rotation::eTYPE_CounterClockwise90)
        return state;
    Direction::Axis::EAxis id = state->getValue(AXIS)->getId();
    if (id == Direction::Axis::eAxis_X)
    {
        return state->setValue(AXIS, Direction::Axis::Z);
    }
    if (id == Direction::Axis::eAxis_Z)
    {
        return state->setValue(AXIS, Direction::Axis::X);
    }
    return state;
}

bool PortalBlock::canContainLiquid(const BlockState* state) const { return false; }

BlockStateDefinition* PortalBlock::createBlockStateDefinition()
{
    static const Property* properties[1] = { AXIS };
    return new BlockStateDefinition(this, properties, 1);
}

EBlockFaceShape PortalBlock::getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos, const Direction* direction)
{
    return BLOCK_FACE_UNDEFINED;
}

bool PortalBlock::mayPick() { return false; }

PortalBlock::PortalShape::PortalShape(Level* level, const BlockPos& pos, Direction::Axis* axis)
    : mLevel(level), mAxis(axis), mNumPortalBlocks(0), mBottomLeft(NULL), mWidth(0)
{
    if (axis == Direction::Axis::X)
    {
        mLeftDir = Direction::EAST;
        mRightDir = Direction::WEST;
    }
    else
    {
        mLeftDir = Direction::NORTH;
        mRightDir = Direction::SOUTH;
    }
    BlockPos origin = pos;
    BlockPos bottom = origin;
    while (bottom.y > origin.y - 21 && bottom.y > 0
           && isEmptyBlock(level->getBlockState(bottom.below())->getBlock()))
    {
        bottom = bottom.below();
    }
    int offset = getDistanceUntilEdge(bottom, mLeftDir) - 1;
    if (offset >= 0)
    {
        mBottomLeft = new BlockPos(bottom.relative(mLeftDir, offset));
        int width = getDistanceUntilEdge(*mBottomLeft, mRightDir);
        if (width < 2 || width > 21)
        {
            mBottomLeft = NULL;
            mWidth = 0;
        }
        else
        {
            mWidth = width;
        }
    }
    if (mBottomLeft != NULL)
    {
        mHeight = calculatePortalHeight();
    }
}

int PortalBlock::PortalShape::getHeight() { return mHeight; }

bool PortalBlock::PortalShape::isEmptyBlock(Block* block)
{
    return block->mMaterial == Material::AIR || block == Blocks::fire || block == Blocks::portal;
}

int PortalBlock::PortalShape::getDistanceUntilEdge(const BlockPos& pos, const Direction* direction)
{
    int base = 0;
    int distance = 0;
    int steps = 11;
    do
    {
        distance = base;
        BlockPos at = pos.relative(direction, base);
        if (!isEmptyBlock(mLevel->getBlockState(at)->getBlock()))
        {
            break;
        }
        if (mLevel->getBlockState(at.below())->getBlock() != Blocks::obsidian)
        {
            break;
        }
        distance = base + 1;
        at = pos.relative(direction, distance);
        if (!isEmptyBlock(mLevel->getBlockState(at)->getBlock()))
        {
            break;
        }
        if (mLevel->getBlockState(at.below())->getBlock() != Blocks::obsidian)
        {
            break;
        }
        --steps;
        base += 2;
        distance = base;
    } while (steps != 0);

    if (mLevel->getBlockState(pos.relative(direction, distance))->getBlock() == Blocks::obsidian)
    {
        return distance;
    }
    return 0;
}

int PortalBlock::PortalShape::calculatePortalHeight()
{
    bool blocked = false;
    for (mHeight = 0; mHeight < 21; ++mHeight)
    {
        for (int width = 0; width < mWidth; ++width)
        {
            BlockPos at = mBottomLeft->relative(mRightDir, width).above(mHeight);
            Block* block = mLevel->getBlockState(at)->getBlock();
            if (!isEmptyBlock(block))
            {
                blocked = true;
                break;
            }
            if (block == Blocks::portal)
            {
                ++mNumPortalBlocks;
            }
            if (width == 0)
            {
                if (mLevel->getBlockState(at.relative(mLeftDir))->getBlock() != Blocks::obsidian)
                {
                    blocked = true;
                    break;
                }
            }
            else if (width == mWidth - 1)
            {
                if (mLevel->getBlockState(at.relative(mRightDir))->getBlock() != Blocks::obsidian)
                {
                    blocked = true;
                    break;
                }
            }
        }
        if (blocked)
        {
            break;
        }
    }

    for (int width = 0; width < mWidth; ++width)
    {
        BlockPos at = mBottomLeft->relative(mRightDir, width).above(mHeight);
        if (mLevel->getBlockState(at)->getBlock() != Blocks::obsidian)
        {
            mBottomLeft = NULL;
            mHeight = 0;
            mWidth = 0;
            return 0;
        }
    }

    if (mHeight >= 3 && mHeight <= 21)
    {
        return mHeight;
    }

    mBottomLeft = NULL;
    mHeight = 0;
    mWidth = 0;
    return 0;
}

bool PortalBlock::PortalShape::isValid() { return mWidth >= 2 && mWidth <= 21 && mHeight >= 3 && mHeight <= 21; }
int PortalBlock::PortalShape::getWidth() { return mWidth; }

void PortalBlock::PortalShape::createPortalBlocks()
{
    for (int width = 0; width < mWidth; ++width)
    {
        BlockPos column = mBottomLeft->relative(mRightDir, width);
        for (int height = 0; height < mHeight; ++height)
        {
            mLevel->setBlock(column.above(height),
                             Blocks::portal->defaultBlockState()->setValue(AXIS, mAxis),
                             Block::UPDATE_CLIENTS, false);
        }
    }
}
