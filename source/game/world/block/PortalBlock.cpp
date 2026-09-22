#include "game/world/block/PortalBlock.hpp"

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

static EnumProperty<Direction::Axis*>* gPortalAxis = NULL;
static const AABB* gPortalShapeX = AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 1.0, 0.001);
static const AABB* gPortalShapeZ = AABB::newPermanent(0.0, 0.0, 0.0, 0.001, 1.0, 1.0);

PortalBlock::PortalBlock()
    : HalfTransparentBlock(L"portal", Material::PORTAL, false)
{
    DerivedInit();
    TypedBoxed<Direction::Axis*> axis(Direction::Axis::X);
    registerDefaultState(mStateDefinition->any()->setBoxedValue(gPortalAxis, &axis));
    setTicking(true);
}

PortalBlock::~PortalBlock()
{
}

void PortalBlock::blockStaticCtor()
{
    boost::unordered_set<Direction::Axis*> values;
    values.insert(Direction::Axis::X);
    values.insert(Direction::Axis::Y);
    values.insert(Direction::Axis::Z);
    gPortalAxis = EnumProperty<Direction::Axis*>::create(L"axis", typeid(Direction::Axis*), values,
                                                          Predicates<Direction::Axis*>::alwaysTrue);
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
    filled[0] = 0;
    filled[1] = 0;

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
    Direction::Axis* axis = state->getValue(gPortalAxis);
    if (axis == Direction::Axis::X)
        return gPortalShapeX;
    return gPortalShapeZ;
}

void PortalBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random)
{
    Block::tick(level, pos, state, random);

    if (level->mDimension->isNaturalDimension()
        && level->getGameRules()->getBoolean(3)
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
            std::wstring customName;
            boost::shared_ptr<Mob> mob =
                SpawnEggItem::spawnMobAt(level, EntityIO::getKey(ENTITY_TYPE_PIG_ZOMBIE),
                                         (double)ground.x + 0.5, (double)ground.y + 1.1,
                                         (double)ground.z + 0.5, &error, customName);
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
    if (x.isValid())
    {
        if (create) x.createPortalBlocks();
        return true;
    }
    PortalShape z(level, pos, Direction::Axis::Z);
    if (!z.isValid()) return false;
    if (create) z.createPortalBlocks();
    return true;
}

void PortalBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block, const BlockPos& fromPos)
{
    Direction::Axis* axis = state->getValue(gPortalAxis);
    if (axis == Direction::Axis::X)
    {
        PortalShape shape(level, pos, axis);
        if (!shape.isValid() || shape.mNumPortalBlocks < shape.mWidth * shape.mHeight)
        {
            level->setBlockAndUpdate(pos, Blocks::AIR->defaultBlockState());
        }
    }
    else if (axis == Direction::Axis::Z)
    {
        PortalShape shape(level, pos, axis);
        if (!shape.isValid() || shape.mNumPortalBlocks < shape.mWidth * shape.mHeight)
        {
            level->setBlockAndUpdate(pos, Blocks::AIR->defaultBlockState());
        }
    }
}

bool PortalBlock::shouldRenderFace(const BlockState* state, LevelSource* levelSource, const BlockPos& pos, const Direction* direction)
{
    BlockPos neighbourPos = pos.relative(direction);
    Direction::Axis* axis = NULL;

    if (state->getBlock() == this)
    {
        axis = state->getValue(gPortalAxis);
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
    if (entity->GetType() == 0x8000003ULL)
        return;
    if (entity->isPassenger() || entity->isVehicle() || !entity->canChangeDimensions())
        return;

    if ((entity->GetType() & 0xC800000ULL) == 0xC800000ULL)
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
    Direction::Axis* axis = (data & 3) == 2 ? Direction::Axis::Z : Direction::Axis::X;
    TypedBoxed<Direction::Axis*> value(axis);
    return defaultBlockState()->setBoxedValue(gPortalAxis, &value);
}

int PortalBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return getDataForAxis(state->getValue(gPortalAxis));
}

const BlockState* PortalBlock::rotate(const BlockState* state, Rotation* rotation)
{
    if (rotation->getType() != Rotation::eTYPE_Clockwise90 && rotation->getType() != Rotation::eTYPE_CounterClockwise90)
        return state;
    Direction::Axis* axis = state->getValue(gPortalAxis);
    if (axis == Direction::Axis::X) axis = Direction::Axis::Z;
    else if (axis == Direction::Axis::Z) axis = Direction::Axis::X;
    TypedBoxed<Direction::Axis*> value(axis);
    return state->setBoxedValue(gPortalAxis, &value);
}

bool PortalBlock::canContainLiquid(const BlockState* state) const { return false; }

BlockStateDefinition* PortalBlock::createBlockStateDefinition()
{
    static const Property* properties[1] = { NULL };
    if (properties[0] == NULL) properties[0] = gPortalAxis;
    return new BlockStateDefinition(this, properties, 1);
}

EBlockFaceShape PortalBlock::getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos, const Direction* direction)
{
    return BLOCK_FACE_UNDEFINED;
}

bool PortalBlock::mayPick() { return false; }

PortalBlock::PortalShape::PortalShape(Level* level, const BlockPos& pos, Direction::Axis* axis)
    : mLevel(level), mAxis(axis), mRightDir(axis == Direction::Axis::X ? Direction::NORTH : Direction::EAST),
      mLeftDir(mRightDir->getOpposite()), mNumPortalBlocks(0), mBottomLeft(NULL), mHeight(0), mWidth(0)
{
    BlockPos bottom = pos;
    while (bottom.y > 0 && isEmptyBlock(level->getBlock(bottom.below()))) bottom = bottom.below();
    int distance = getDistanceUntilEdge(bottom, mRightDir);
    if (distance > 0)
    {
        mBottomLeft = new BlockPos(bottom.relative(mRightDir));
        mWidth = getDistanceUntilEdge(*mBottomLeft, mLeftDir);
        if (mWidth >= 2 && mWidth <= 21) calculatePortalHeight();
    }
}

int PortalBlock::PortalShape::getHeight() { return mHeight; }

bool PortalBlock::PortalShape::isEmptyBlock(Block* block)
{
    return block == Blocks::AIR || block == Blocks::structure_void || block->getCraftingMaterial() == Material::AIR;
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
        if (!isEmptyBlock(mLevel->getBlock(at)))
        {
            break;
        }
        if (mLevel->getBlock(at.below()) != Blocks::obsidian)
        {
            break;
        }
        distance = base + 1;
        BlockPos at2 = pos.relative(direction, distance);
        if (!isEmptyBlock(mLevel->getBlock(at2)))
        {
            break;
        }
        if (mLevel->getBlock(at2.below()) != Blocks::obsidian)
        {
            break;
        }
        --steps;
        base += 2;
        distance = base;
    } while (steps != 0);

    if (mLevel->getBlock(pos.relative(direction, distance)) == Blocks::obsidian)
    {
        return distance;
    }
    return 0;
}

void PortalBlock::PortalShape::calculatePortalHeight()
{
    int height = 0;
    for (; height < 21; ++height)
    {
        mHeight = height;
        if (mWidth > 0)
        {
            int width = 0;
            bool rowFailed = false;
            do
            {
                BlockPos at = mBottomLeft->relative(mLeftDir, width).above(mHeight);
                Block* block = mLevel->getBlock(at);
                if (!isEmptyBlock(block))
                {
                    rowFailed = true;
                    break;
                }
                if (block == Blocks::portal)
                {
                    ++mNumPortalBlocks;
                }
                if (width == 0)
                {
                    if (mLevel->getBlock(at.relative(mRightDir)) != Blocks::obsidian)
                    {
                        rowFailed = true;
                        break;
                    }
                }
                else if (width == mWidth - 1)
                {
                    if (mLevel->getBlock(at.relative(mLeftDir)) != Blocks::obsidian)
                    {
                        rowFailed = true;
                        break;
                    }
                }
                ++width;
            } while (width < mWidth);
            if (rowFailed)
            {
                break;
            }
        }
    }
    mHeight = height;

    for (int width = 0; width < mWidth; ++width)
    {
        BlockPos at = mBottomLeft->relative(mLeftDir, width).above(mHeight);
        if (mLevel->getBlock(at) != Blocks::obsidian)
        {
            mBottomLeft = NULL;
            mHeight = 0;
            mWidth = 0;
            return;
        }
    }

    if ((unsigned int)(mHeight - 3) < 19)
    {
        return;
    }

    mBottomLeft = NULL;
    mHeight = 0;
    mWidth = 0;
}

bool PortalBlock::PortalShape::isValid() { return mWidth >= 2 && mWidth <= 21 && mHeight >= 3 && mHeight <= 20; }
int PortalBlock::PortalShape::getWidth() { return mWidth; }

void PortalBlock::PortalShape::createPortalBlocks()
{
    for (int width = 0; width < mWidth; ++width)
        for (int height = 0; height < mHeight; ++height)
        {
            BlockPos at = mBottomLeft->relative(mLeftDir, width).above(height);
            TypedBoxed<Direction::Axis*> axis(mAxis);
            mLevel->setBlock(at, Blocks::portal->defaultBlockState()->setBoxedValue(gPortalAxis, &axis), 2, false);
        }
}
