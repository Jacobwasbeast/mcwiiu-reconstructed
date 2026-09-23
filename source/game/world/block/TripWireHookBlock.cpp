#include "game/world/block/TripWireHookBlock.hpp"

#include "game/client/audio/SoundEvents.hpp"
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
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/HorizontalDirectionalBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"

DirectionProperty* TripWireHookBlock::FACING = NULL;
AbstractProperty<bool>* TripWireHookBlock::POWERED = NULL;
AbstractProperty<bool>* TripWireHookBlock::ATTACHED = NULL;
AbstractProperty<bool>* TripWireHookBlock::DISARMED = NULL;
const AABB* TripWireHookBlock::AABB_DOWN = AABB::newPermanent(0.0, 0.0, 0.375, 1.0, 0.125, 0.625);
const AABB* TripWireHookBlock::AABB_UP = AABB::newPermanent(0.0, 0.875, 0.375, 1.0, 1.0, 0.625);
const AABB* TripWireHookBlock::AABB_NORTH = AABB::newPermanent(0.3125, 0.0, 0.625, 0.6875, 0.625, 1.0);
const AABB* TripWireHookBlock::AABB_SOUTH = AABB::newPermanent(0.3125, 0.0, 0.0, 0.6875, 0.625, 0.375);
const AABB* TripWireHookBlock::AABB_WEST = AABB::newPermanent(0.625, 0.0, 0.3125, 1.0, 0.625, 0.6875);
const AABB* TripWireHookBlock::AABB_EAST = AABB::newPermanent(0.0, 0.0, 0.3125, 0.375, 0.625, 0.6875);

TripWireHookBlock::TripWireHookBlock() : Block(Material::DECORATION)
{
    DerivedInit();
    const BlockState* state = mStateDefinition->any();
    TypedBoxed<const Direction*> facing(Direction::NORTH);
    state = state->setBoxedValue(FACING, &facing);
    TypedBoxed<bool> falseValue(false);
    state = state->setBoxedValue(POWERED, &falseValue);
    state = state->setBoxedValue(ATTACHED, &falseValue);
    registerDefaultState(state->setBoxedValue(DISARMED, &falseValue));
    setTicking(true);
}

TripWireHookBlock::~TripWireHookBlock() {}

void TripWireHookBlock::blockStaticCtor()
{
    FACING = HorizontalDirectionalBlock::FACING_PROPERTY;
    POWERED = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"powered"));
    ATTACHED = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"attached"));
    DISARMED = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"disarmed"));
}

const AABB* TripWireHookBlock::getShape(const BlockState* state, LevelSource*, const BlockPos&)
{
    switch (state->getValue(FACING)->get3DDataValue())
    {
    case 2: return AABB_NORTH;
    case 3: return AABB_SOUTH;
    case 4: return AABB_WEST;
    default: return AABB_EAST;
    }
}

const AABB* TripWireHookBlock::getClipAABB(const BlockState*, LevelSource*, const BlockPos&) { return NULL; }
bool TripWireHookBlock::isSolidRender(const BlockState*) const { return false; }
bool TripWireHookBlock::isCubeShaped(const BlockState*) { return false; }

bool TripWireHookBlock::mayPlace(Level* level, const BlockPos& pos, const Direction* direction)
{
    BlockPos supportPos = pos.relative(direction->getOpposite());
    Block* support = level->getBlockState(supportPos)->getBlock();
    bool exception = Block::isExceptionBlockForAttaching(support);
    bool pistonException = Block::isExceptionForPistonAttaching(support);
    return direction->getAxis()->isHorizontal() && !exception && !pistonException &&
        (level->getBlockState(supportPos)->canProvideSupport(direction, BLOCK_SUPPORT_CENTER) ||
         !support->isSolidBlockingCube(level->getBlockState(supportPos)));
}

bool TripWireHookBlock::mayPlace(Level* level, const BlockPos& pos)
{
    for (std::vector<const Direction*>::const_iterator it = Direction::VALUES.begin(); it != Direction::VALUES.end(); ++it)
        if (mayPlace(level, pos, *it)) return true;
    return false;
}

const BlockState* TripWireHookBlock::getStateForPlacement(Level*, const BlockPos&, const Direction* direction, float, float, float, int, boost::shared_ptr<LivingEntity>)
{
    const BlockState* state = defaultBlockState();
    TypedBoxed<bool> falseValue(false);
    state = state->setBoxedValue(POWERED, &falseValue);
    state = state->setBoxedValue(ATTACHED, &falseValue);
    if (direction->getAxis()->isHorizontal())
    {
        TypedBoxed<const Direction*> facing(direction);
        state = state->setBoxedValue(FACING, &facing);
    }
    return state;
}

void TripWireHookBlock::notifyNeighbors(Level* level, const BlockPos& pos, const Direction* direction)
{
    level->updateNeighborsAt(pos, this, false);
    level->updateNeighborsAt(pos.relative(direction->getOpposite()), this, false);
}

void TripWireHookBlock::playSound(Level* level, const BlockPos& pos, bool attached, bool powered, bool oldAttached, bool oldPowered)
{
    if (!powered)
    {
        if (oldPowered)
        {
            level->playSound(boost::shared_ptr<Player>(), pos, SoundEvents::TRIPWIRE_CLICK_OFF, SoundSource::eSoundSource_Blocks, 0.4f, 0.5f, 16.0f);
            return;
        }
    }
    else if (!oldPowered)
    {
        level->playSound(boost::shared_ptr<Player>(), pos, SoundEvents::TRIPWIRE_CLICK_ON, SoundSource::eSoundSource_Blocks, 0.4f, 0.6f, 16.0f);
        return;
    }

    if (!attached)
    {
        if (oldAttached)
        {
            level->playSound(boost::shared_ptr<Player>(), pos, SoundEvents::TRIPWIRE_DETACH, SoundSource::eSoundSource_Blocks, 0.4f, 1.2f / (level->mRandom->nextFloat() * 0.2f + 0.9f), 16.0f);
            return;
        }
    }
    else if (!oldAttached)
    {
        level->playSound(boost::shared_ptr<Player>(), pos, SoundEvents::TRIPWIRE_ATTACH, SoundSource::eSoundSource_Blocks, 0.4f, 0.7f, 16.0f);
        return;
    }
}

const BlockState* TripWireHookBlock::calculateState(Level* level, const BlockPos& pos, const BlockState* state, bool attached, bool powered, int distance, const BlockState* replacement)
{
    const Direction* facing = state->getValue(FACING);
    const Direction* opposite = facing->getOpposite();
    bool connected = false;
    bool wirePowered = false;
    bool wireDisarmed = false;
    const BlockState* result = replacement ? replacement : defaultBlockState();
    for (int i = 1; i <= 41; ++i)
    {
        BlockPos check = pos.relative(facing, i);
        const BlockState* other = level->getBlockState(check);
        Block* block = other->getBlock();
        if (block == this)
        {
            if (other->getValue(FACING) == opposite)
            {
                connected = i > 1;
                distance = i;
                break;
            }
        }
        else if (block != this && block->isSolidBlockingCube(other)) break;
        wireDisarmed = wireDisarmed || other->getValue(DISARMED);
        wirePowered = wirePowered || (!other->getValue(DISARMED) && other->getValue(POWERED));
    }
    bool newAttached = connected && attached;
    TypedBoxed<bool> attachedValue(newAttached);
    TypedBoxed<bool> poweredValue(wirePowered || powered);
    TypedBoxed<const Direction*> facingValue(facing);
    result = result->setBoxedValue(FACING, &facingValue);
    result = result->setBoxedValue(ATTACHED, &attachedValue);
    result = result->setBoxedValue(POWERED, &poweredValue);
    if (result->hasProperty(DISARMED))
    {
        TypedBoxed<bool> disarmedValue(wireDisarmed);
        result = result->setBoxedValue(DISARMED, &disarmedValue);
    }
    if (distance > 0)
    {
        BlockPos otherPos = pos.relative(facing, distance);
        const BlockState* other = level->getBlockState(otherPos);
        if (other->getBlock() == this)
        {
            TypedBoxed<const Direction*> otherFacing(opposite);
            const BlockState* otherState = other->setBoxedValue(FACING, &otherFacing)
                ->setBoxedValue(ATTACHED, &attachedValue)
                ->setBoxedValue(POWERED, &poweredValue);
            level->setBlock(otherPos, otherState, 3, false);
            notifyNeighbors(level, otherPos, opposite);
            playSound(level, otherPos, newAttached, poweredValue.getValue(),
                      other->getValue(ATTACHED), other->getValue(POWERED));
        }
    }
    playSound(level, pos, newAttached, poweredValue.getValue(),
              state->getValue(ATTACHED), state->getValue(POWERED));
    if (!attached)
    {
        level->setBlock(pos, result, 3, false);
        notifyNeighbors(level, pos, facing);
    }
    return result;
}

void TripWireHookBlock::setPlacedBy(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<LivingEntity>, not_null_ptr<ItemInstance>)
{
    calculateState(level, pos, state, false, false, 0, NULL);
}

bool TripWireHookBlock::checkCanSurvive(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (mayPlace(level, pos, state->getValue(FACING))) return true;
    spawnResources(level, pos, state, 0);
    level->removeBlock(pos);
    return false;
}

void TripWireHookBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block, const BlockPos& fromPos)
{
    if (fromPos != pos && checkCanSurvive(level, pos, state) && !mayPlace(level, pos, state->getValue(FACING)))
    {
        spawnResources(level, pos, state, 0);
        level->removeBlock(pos);
    }
}

void TripWireHookBlock::randomTick(Level*, const BlockPos&, const BlockState*, Random*) {}
void TripWireHookBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random*)
{
    calculateState(level, pos, state, state->getValue(ATTACHED), state->getValue(POWERED), -1, NULL);
}

void TripWireHookBlock::onRemove(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (state->getValue(ATTACHED) || state->getValue(POWERED))
    {
        calculateState(level, pos, state, true, false, -1, NULL);
        if (state->getValue(POWERED))
        {
            level->updateNeighborsAt(pos, this, false);
            level->updateNeighborsAt(pos.relative(state->getValue(FACING)->getOpposite()), this, false);
        }
    }
    Block::onRemove(level, pos, state);
}

int TripWireHookBlock::getSignal(const BlockState* state, LevelSource*, const BlockPos&, const Direction*) { return state->getValue(POWERED) ? 15 : 0; }
int TripWireHookBlock::getDirectSignal(const BlockState* state, LevelSource*, const BlockPos&, const Direction* direction) { return state->getValue(POWERED) && direction == state->getValue(FACING) ? 15 : 0; }
bool TripWireHookBlock::isSignalSource(const BlockState*) { return true; }
int TripWireHookBlock::getRenderLayer() { return 0; }

const BlockState* TripWireHookBlock::getBlockState(int data)
{
    TypedBoxed<const Direction*> facing(Direction::from2DDataValue(data & 3));
    TypedBoxed<bool> powered((data & 8) != 0);
    TypedBoxed<bool> attached((data & 4) != 0);
    return defaultBlockState()->setBoxedValue(FACING, &facing)->setBoxedValue(POWERED, &powered)->setBoxedValue(ATTACHED, &attached);
}

int TripWireHookBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    int data = state->getValue(FACING)->get2DDataValue();
    if (state->getValue(POWERED)) data |= 8;
    if (state->getValue(ATTACHED)) data |= 4;
    return data;
}

const BlockState* TripWireHookBlock::rotate(const BlockState* state, Rotation* rotation)
{
    TypedBoxed<const Direction*> facing(rotation->rotate(state->getValue(FACING)));
    return state->setBoxedValue(FACING, &facing);
}

const BlockState* TripWireHookBlock::mirror(const BlockState* state, Mirror* mirror)
{
    return rotate(state, mirror->getRotation(state->getValue(FACING)));
}

BlockStateDefinition* TripWireHookBlock::createBlockStateDefinition()
{
    const Property* properties[] = { FACING, POWERED, ATTACHED, DISARMED };
    return new BlockStateDefinition(this, properties, 4);
}

EBlockFaceShape TripWireHookBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction*) { return BLOCK_FACE_UNDEFINED; }
EBlockRenderShape TripWireHookBlock::getRenderShape(const BlockState*) { return static_cast<EBlockRenderShape>(29); }
