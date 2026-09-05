#include "game/world/block/TorchBlock.hpp"

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
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TypedBoxed.hpp"

DirectionProperty* TorchBlock::FACING = NULL;

static const AABB* TORCH_SHAPE = AABB::newPermanent(0.25, 0.0, 0.25, 0.75, 0.625, 0.75);
static const AABB* TORCH_SHAPE_UP = AABB::newPermanent(0.25, 0.375, 0.25, 0.75, 1.0, 0.75);
static const AABB* TORCH_SHAPE_NORTH = AABB::newPermanent(0.25, 0.2, 0.75, 0.75, 0.8, 1.0);
static const AABB* TORCH_SHAPE_SOUTH = AABB::newPermanent(0.25, 0.2, 0.0, 0.75, 0.8, 0.25);
static const AABB* TORCH_SHAPE_WEST = AABB::newPermanent(0.75, 0.2, 0.25, 1.0, 0.8, 0.75);
static const AABB* TORCH_SHAPE_EAST = AABB::newPermanent(0.0, 0.2, 0.25, 0.25, 0.8, 0.75);

TorchBlock::TorchBlock()
    : Block(Material::DECORATION)
{
    DerivedInit();
    const BlockState* state = mStateDefinition->any();
    TypedBoxed<const Direction*> facing(Direction::UP);
    registerDefaultState(state->setBoxedValue(FACING, &facing));
    setTicking(true);
}

TorchBlock::~TorchBlock()
{
}

void TorchBlock::blockStaticCtor()
{
    FACING = DirectionProperty::create(L"facing", new Predicates<const Direction*>::FunctionPredicate(DirectionProperty::NOT_DOWN));
}

int TorchBlock::getSpawnResourcesAuxValue(const BlockState*) { return 5; }

const AABB* TorchBlock::getShape(const BlockState* state, LevelSource*, const BlockPos&)
{
    switch (state->getValue(FACING)->get3DDataValue())
    {
    case 2: return TORCH_SHAPE_NORTH;
    case 3: return TORCH_SHAPE_SOUTH;
    case 4: return TORCH_SHAPE_WEST;
    case 5: return TORCH_SHAPE_EAST;
    case 1: return TORCH_SHAPE_UP;
    default: return TORCH_SHAPE;
    }
}

const AABB* TorchBlock::getClipAABB(const BlockState*, LevelSource*, const BlockPos&) { return NULL; }
bool TorchBlock::isSolidRender(const BlockState*) const { return false; }
bool TorchBlock::isCubeShaped(const BlockState*) { return false; }
bool TorchBlock::waterSpreadCausesSpawn() { return true; }
bool TorchBlock::canContainLiquid(const BlockState*) const { return false; }

bool TorchBlock::canStandOn(Level* level, const BlockPos& pos)
{
    const BlockState* state = level->getBlockState(pos);
    Block* block = state->getBlock();
    if (!state->isSolidBlockingBlock())
    {
        return block != NULL && !Block::isExceptionBlockForAttachingPlusPiston(block);
    }
    return block != NULL;
}

bool TorchBlock::canBePlacedOn(Level* level, const BlockPos& pos, const Direction* direction)
{
    BlockPos supportPos = pos.relative(direction->getOpposite());
    const BlockState* supportState = level->getBlockState(supportPos);
    Block* support = supportState->getBlock();
    if (direction == Direction::UP) return canStandOn(level, supportPos);
    if (direction == Direction::DOWN) return false;
    return !Block::isExceptionBlockForAttachingPlusPiston(support) &&
           !supportState->canProvideSupport(direction, BLOCK_SUPPORT_CENTER);
}

bool TorchBlock::mayPlace(Level* level, const BlockPos& pos)
{
    const BlockState* state = level->getBlockState(pos);
    const BlockState* above = level->getBlockState(pos.above());
    if (state->getBlock() == Block::byId(0) && above->getBlock() == Block::byId(0))
        return false;
    for (unsigned int i = 0; i < Direction::VALUES.size(); ++i)
        if (Direction::VALUES[i] != Direction::DOWN && canBePlacedOn(level, pos, Direction::VALUES[i])) return true;
    return false;
}

const BlockState* TorchBlock::getStateForPlacement(Level* level, const BlockPos& pos, const Direction* direction, float, float, float, int, boost::shared_ptr<LivingEntity>)
{
    if (canBePlacedOn(level, pos, direction))
    {
        TypedBoxed<const Direction*> facing(direction);
        return defaultBlockState()->setBoxedValue(FACING, &facing);
    }
    for (std::vector<const Direction*>::const_iterator it = Direction::VALUES.begin(); it != Direction::VALUES.end(); ++it)
    {
        if (canBePlacedOn(level, pos, *it))
        {
            TypedBoxed<const Direction*> facing(*it);
            return defaultBlockState()->setBoxedValue(FACING, &facing);
        }
    }
    return defaultBlockState();
}

void TorchBlock::onPlace(Level* level, const BlockPos& pos, const BlockState* state) { checkCanSurvive(level, pos, state); }

void TorchBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block*, const BlockPos&)
{
    checkCanSurvive(level, pos, state);
}

bool TorchBlock::checkDoPop(Level* level, const BlockPos& pos, const BlockState* state)
{
    const Direction* direction = state->getValue(FACING);
    BlockPos supportPos = pos.relative(direction->getOpposite());
    if (direction->getAxis()->isHorizontal() && level->getBlockState(supportPos)->canProvideSupport(direction, BLOCK_SUPPORT_CENTER))
        return false;
    if (!direction->getAxis()->isVertical() || !canStandOn(level, supportPos)) return false;
    spawnResources(level, pos, state, 0);
    level->removeBlock(pos);
    return true;
}

bool TorchBlock::checkCanSurvive(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (state->getBlock() == this && canBePlacedOn(level, pos, state->getValue(FACING))) return true;
    if (level->getBlockState(pos)->getBlock() == this)
    {
        spawnResources(level, pos, state, 0);
        level->removeBlock(pos);
    }
    return false;
}

void TorchBlock::animateTick(const BlockState* state, Level* level, const BlockPos& pos, Random*)
{
    const Direction* direction = state->getValue(FACING);
    double x = pos.x + 0.5;
    double y = pos.y + 0.7;
    double z = pos.z + 0.5;
    if (direction->getAxis()->isHorizontal())
    {
        const Direction* opposite = direction->getOpposite();
        x += 0.27 * opposite->getNormal().x;
        z += 0.27 * opposite->getNormal().z;
        y += 0.22;
    }
    level->addParticle(PARTICLE_SMOKE, x, y, z, 0.0, 0.0, 0.0, arrayWithLength<int>());
    level->addParticle(PARTICLE_FLAME, x, y, z, 0.0, 0.0, 0.0, arrayWithLength<int>());
}

int TorchBlock::getRenderLayer() { return 0; }

const BlockState* TorchBlock::getBlockState(int data)
{
    const Direction* direction;
    switch (data)
    {
    case 1: direction = Direction::EAST; break;
    case 2: direction = Direction::WEST; break;
    case 3: direction = Direction::SOUTH; break;
    case 4: direction = Direction::NORTH; break;
    default: direction = Direction::UP; break;
    }
    TypedBoxed<const Direction*> facing(direction);
    return defaultBlockState()->setBoxedValue(FACING, &facing);
}

int TorchBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    static const int values[] = { 5, 5, 2, 3, 4, 1 };
    int data = state->getValue(FACING)->get3DDataValue();
    return data >= 0 && data < 6 ? values[data] : 5;
}

const BlockState* TorchBlock::rotate(const BlockState* state, Rotation* rotation)
{
    TypedBoxed<const Direction*> facing(rotation->rotate(state->getValue(FACING)));
    return state->setBoxedValue(FACING, &facing);
}

const BlockState* TorchBlock::mirror(const BlockState* state, Mirror* mirror)
{
    return rotate(state, mirror->getRotation(state->getValue(FACING)));
}

BlockStateDefinition* TorchBlock::createBlockStateDefinition()
{
    const Property* properties[] = { FACING };
    return new BlockStateDefinition(this, properties, 1);
}

EBlockFaceShape TorchBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction*)
{
    return BLOCK_FACE_UNDEFINED;
}

EBlockRenderShape TorchBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_INVISIBLE;
}
