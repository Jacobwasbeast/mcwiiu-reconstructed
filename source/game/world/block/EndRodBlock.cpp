#include "game/world/block/EndRodBlock.hpp"

#include "game/client/render/ParticleType.hpp"
#include "game/core/Direction.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"

static const AABB* END_ROD_SHAPE_X = AABB::newPermanent(0.0, 0.25, 0.25, 1.0, 0.75, 0.75);
static const AABB* END_ROD_SHAPE_Y = AABB::newPermanent(0.25, 0.0, 0.25, 0.75, 1.0, 0.75);
static const AABB* END_ROD_SHAPE_Z = AABB::newPermanent(0.25, 0.25, 0.0, 0.75, 0.75, 1.0);

static const AABB* END_ROD_ROD_DOWN = AABB::newPermanent(0.25, 0.25, 0.25, 0.75, 1.0, 0.75);
static const AABB* END_ROD_ROD_UP = AABB::newPermanent(0.25, 0.0, 0.25, 0.75, 0.75, 0.75);
static const AABB* END_ROD_ROD_NORTH = AABB::newPermanent(0.25, 0.25, 0.25, 0.75, 0.75, 1.0);
static const AABB* END_ROD_ROD_SOUTH = AABB::newPermanent(0.25, 0.25, 0.0, 0.75, 0.75, 0.75);
static const AABB* END_ROD_ROD_WEST = AABB::newPermanent(0.25, 0.25, 0.25, 1.0, 0.75, 0.75);
static const AABB* END_ROD_ROD_EAST = AABB::newPermanent(0.0, 0.25, 0.25, 0.75, 0.75, 0.75);

static const AABB* END_ROD_BASE_DOWN = AABB::newPermanent(0.1875, 0.1875, 0.1875, 0.8125, 1.0, 0.8125);
static const AABB* END_ROD_BASE_UP = AABB::newPermanent(0.1875, 0.0, 0.1875, 0.8125, 0.8125, 0.8125);
static const AABB* END_ROD_BASE_NORTH = AABB::newPermanent(0.1875, 0.1875, 0.1875, 0.8125, 0.8125, 1.0);
static const AABB* END_ROD_BASE_SOUTH = AABB::newPermanent(0.1875, 0.1875, 0.0, 0.8125, 0.8125, 0.8125);
static const AABB* END_ROD_BASE_WEST = AABB::newPermanent(0.1875, 0.1875, 0.1875, 1.0, 0.8125, 0.8125);
static const AABB* END_ROD_BASE_EAST = AABB::newPermanent(0.0, 0.1875, 0.1875, 0.8125, 0.8125, 0.8125);

EndRodBlock::EndRodBlock()
    : DirectionalBlock(Material::DECORATION)
{
    DerivedInit();
}

EndRodBlock::~EndRodBlock()
{
}

void EndRodBlock::DerivedInit()
{
    Block::DerivedInit();
    TypedBoxed<const Direction*> facing(Direction::DOWN);
    registerDefaultState(mStateDefinition->any()->setBoxedValue(DirectionalBlock::FACING_PROPERTY, &facing));
}

const BlockState* EndRodBlock::rotate(const BlockState* state, Rotation* rotation)
{
    TypedBoxed<const Direction*> facing(rotation->rotate(state->getValue(DirectionalBlock::FACING_PROPERTY)));
    return state->setBoxedValue(DirectionalBlock::FACING_PROPERTY, &facing);
}

const BlockState* EndRodBlock::mirror(const BlockState* state, Mirror* mirror)
{
    TypedBoxed<const Direction*> facing(mirror->mirror(state->getValue(DirectionalBlock::FACING_PROPERTY)));
    return state->setBoxedValue(DirectionalBlock::FACING_PROPERTY, &facing);
}

const AABB* EndRodBlock::getShape(const BlockState* state, LevelSource*, const BlockPos&)
{
    switch (state->getValue(DirectionalBlock::FACING_PROPERTY)->getAxis()->getId())
    {
    case Direction::Axis::eAxis_Y: return END_ROD_SHAPE_Y;
    case Direction::Axis::eAxis_Z: return END_ROD_SHAPE_Z;
    default: return END_ROD_SHAPE_X;
    }
}

bool EndRodBlock::isSolidRender(const BlockState*) const { return false; }
bool EndRodBlock::isCubeShaped(const BlockState*) { return false; }
bool EndRodBlock::mayPlace(Level*, const BlockPos&) { return true; }

const BlockState* EndRodBlock::getStateForPlacement(Level* level, const BlockPos& pos, const Direction* direction,
                                                     float, float, float, int, boost::shared_ptr<LivingEntity>)
{
    const BlockState* neighbor = level->getBlockState(pos.relative(direction->getOpposite()));
    if (neighbor->getBlock() == this && neighbor->getValue(DirectionalBlock::FACING_PROPERTY) == direction)
    {
        TypedBoxed<const Direction*> facing(direction->getOpposite());
        return defaultBlockState()->setBoxedValue(DirectionalBlock::FACING_PROPERTY, &facing);
    }
    TypedBoxed<const Direction*> facing(direction);
    return defaultBlockState()->setBoxedValue(DirectionalBlock::FACING_PROPERTY, &facing);
}

void EndRodBlock::animateTick(const BlockState* state, Level* level, const BlockPos& pos, Random* random)
{
    const Direction* direction = state->getValue(DirectionalBlock::FACING_PROPERTY);
    double spread = 0.4 - (random->nextFloat() + random->nextFloat()) * 0.4;
    double x = static_cast<double>(pos.x) + 0.55 - static_cast<double>(random->nextFloat()) * 0.1 + direction->getNormal().x * spread;
    double y = static_cast<double>(pos.y) + 0.55 - static_cast<double>(random->nextFloat()) * 0.1 + direction->getNormal().y * spread;
    double z = static_cast<double>(pos.z) + 0.55 - static_cast<double>(random->nextFloat()) * 0.1 + direction->getNormal().z * spread;
    if (random->nextInt(5) == 0)
    {
        double velocityX = random->nextGaussian() * 0.005;
        double velocityY = random->nextGaussian() * 0.005;
        double velocityZ = random->nextGaussian() * 0.005;
        level->addParticle(PARTICLE_END_ROD, x, y, z, velocityX, velocityY, velocityZ, arrayWithLength<int>());
    }
}

int EndRodBlock::getRenderLayer() { return 0; }

const BlockState* EndRodBlock::getBlockState(int data)
{
    TypedBoxed<const Direction*> facing(Direction::from3DDataValue(data));
    return defaultBlockState()->setBoxedValue(DirectionalBlock::FACING_PROPERTY, &facing);
}

int EndRodBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(DirectionalBlock::FACING_PROPERTY)->get3DDataValue();
}

EPistonPushReaction EndRodBlock::getPistonPushReaction(const BlockState*) { return static_cast<EPistonPushReaction>(0); }
EBlockFaceShape EndRodBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction*)
{
    return static_cast<EBlockFaceShape>(8);
}
EBlockRenderShape EndRodBlock::getRenderShape(const BlockState*) { return static_cast<EBlockRenderShape>(44); }

const AABB* EndRodBlock::getShapeRod(const BlockState* state)
{
    switch (state->getValue(DirectionalBlock::FACING_PROPERTY)->get3DDataValue())
    {
    case 0: return END_ROD_ROD_DOWN;
    case 1: return END_ROD_ROD_UP;
    case 2: return END_ROD_ROD_NORTH;
    case 3: return END_ROD_ROD_SOUTH;
    case 4: return END_ROD_ROD_WEST;
    default: return END_ROD_ROD_EAST;
    }
}

const AABB* EndRodBlock::getShapeBase(const BlockState* state)
{
    switch (state->getValue(DirectionalBlock::FACING_PROPERTY)->get3DDataValue())
    {
    case 0: return END_ROD_BASE_DOWN;
    case 1: return END_ROD_BASE_UP;
    case 2: return END_ROD_BASE_NORTH;
    case 3: return END_ROD_BASE_SOUTH;
    case 4: return END_ROD_BASE_WEST;
    default: return END_ROD_BASE_EAST;
    }
}

BlockStateDefinition* EndRodBlock::createBlockStateDefinition()
{
    const Property* properties[] = { DirectionalBlock::FACING_PROPERTY };
    return new BlockStateDefinition(this, properties, 1);
}

int EndRodBlock::getSpawnResourcesAuxValue(const BlockState* state)
{
    return state->getValue(DirectionalBlock::FACING_PROPERTY)->get3DDataValue();
}
