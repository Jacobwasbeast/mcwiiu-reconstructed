#include "game/world/block/DiodeBlock.hpp"
#include "game/core/Direction.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/AbstractProperty.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/client/render/TextureAtlasSprite.hpp"

static const AABB* gDiodeShape = AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.125, 1.0);

DiodeBlock::DiodeBlock(bool on) : HorizontalDirectionalBlock(Material::DECORATION), mOn(on) {}
DiodeBlock::~DiodeBlock() {}

const AABB* DiodeBlock::getShape(const BlockState*, LevelSource*, const BlockPos&) { return gDiodeShape; }
bool DiodeBlock::isCubeShaped(const BlockState*) { return false; }

bool DiodeBlock::mayPlace(Level* level, const BlockPos& pos)
{
    return level->getBlockState(pos.below())->isSolidBlockingCube() && Block::mayPlace(level, pos);
}

bool DiodeBlock::canSurvive(Level* level, const BlockPos& pos, const BlockState*)
{
    return level->getBlockState(pos.below())->isSolidBlockingCube();
}

void DiodeBlock::randomTick(Level*, const BlockPos&, const BlockState*, Random*) {}

void DiodeBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random*)
{
    if (isLocked(level, pos, state)) return;
    bool turnOn = shouldTurnOn(level, pos, state);
    if (!mOn)
    {
        const BlockState* onState = getOnBlock(state);
        level->setBlock(pos, onState, 2, false);
        if (!turnOn)
        {
            level->addToTickNextTick(pos, onState->getBlock(), getTurnOffDelay(state), -1);
        }
    }
    else if (!turnOn)
    {
        level->setBlock(pos, getOffBlock(state), 2, false);
    }
}

bool DiodeBlock::shouldRenderFace(const BlockState*, LevelSource*, const BlockPos&, const Direction* direction)
{
    return direction->getAxis() != Direction::Axis::Y;
}

bool DiodeBlock::isOn(const BlockState*) { return mOn; }

int DiodeBlock::getDirectSignal(const BlockState* state, LevelSource* levelSource, const BlockPos& pos, const Direction* direction)
{
    return state->getSignal(levelSource, pos, direction);
}

int DiodeBlock::getSignal(const BlockState* state, LevelSource* levelSource, const BlockPos& pos, const Direction* direction)
{
    if (isOn(state) && direction == state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY))
        return getOutputSignal(levelSource, pos, state);
    return 0;
}

void DiodeBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block*, const BlockPos&)
{
    if (canSurvive(level, pos, NULL))
    {
        checkTickOnNeighbor(level, pos, state);
        return;
    }
    spawnResources(level, pos, state, 0);
    level->removeBlock(pos);
    for (std::vector<const Direction*>::const_iterator direction = Direction::VALUES.begin(); direction != Direction::VALUES.end(); ++direction)
    {
        level->updateNeighborsAt(pos.relative(*direction), this, false);
    }
}

void DiodeBlock::checkTickOnNeighbor(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (isLocked(level, pos, state) || shouldTurnOn(level, pos, state) == mOn || level->isBlockToBeTickedAt(pos, this)) return;
    int priority = -1;
    if (shouldPrioritize(level, pos, state)) priority = -3;
    else if (mOn) priority = -2;
    level->addToTickNextTick(pos, this, getTurnOnDelay(state), priority);
}

bool DiodeBlock::isLocked(LevelSource*, const BlockPos&, const BlockState*) { return false; }
bool DiodeBlock::shouldTurnOn(Level* level, const BlockPos& pos, const BlockState* state)
{
    return getInputSignal(level, pos, state) > 0;
}

int DiodeBlock::getInputSignal(Level* level, const BlockPos& pos, const BlockState* state)
{
    const Direction* direction = state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY);
    BlockPos inputPos = pos.relative(direction);
    int signal = level->getSignal(inputPos, direction);
    if (signal < 15)
    {
        const BlockState* inputState = level->getBlockState(inputPos);
        if (inputState->getBlock() == Blocks::redstone_wire)
        {
            const AbstractProperty<int>* powerProperty = static_cast<const AbstractProperty<int>*>(inputState->getProperties().back());
            int wireSignal = inputState->getValue(powerProperty);
            if (signal < wireSignal) signal = wireSignal;
        }
        else if (signal < 0)
        {
            signal = 0;
        }
    }
    return signal;
}

int DiodeBlock::getAlternateSignal(LevelSource* levelSource, const BlockPos& pos, const BlockState* state)
{
    const Direction* direction = state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY);
    int first = getAlternateSignalAt(levelSource, pos.relative(direction->getClockWise()), direction->getClockWise());
    int second = getAlternateSignalAt(levelSource, pos.relative(direction->getCounterClockWise()), direction->getCounterClockWise());
    return first > second ? first : second;
}

int DiodeBlock::getAlternateSignalAt(LevelSource* levelSource, const BlockPos& pos, const Direction* direction)
{
    const BlockState* state = levelSource->getBlockState(pos);
    if (!isAlternateInput(state)) return 0;
    if (state->getBlock() == Blocks::redstone_block) return 15;
    if (state->getBlock() == Blocks::redstone_wire)
    {
        const AbstractProperty<int>* powerProperty = static_cast<const AbstractProperty<int>*>(state->getProperties().back());
        return state->getValue(powerProperty);
    }
    return levelSource->getDirectSignal(pos, direction);
}

bool DiodeBlock::isSignalSource(const BlockState*) { return true; }

const BlockState* DiodeBlock::getStateForPlacement(Level*, const BlockPos& pos, const Direction*, float, float, float, int, boost::shared_ptr<LivingEntity> placer)
{
    const Direction* direction = Direction::getDirectionFromLivingEntity(pos, placer);
    TypedBoxed<const Direction*> facing(direction->getOpposite());
    return defaultBlockState()->setBoxedValue(HorizontalDirectionalBlock::FACING_PROPERTY, &facing);
}

void DiodeBlock::setPlacedBy(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<LivingEntity>, not_null_ptr<ItemInstance>)
{
    if (shouldTurnOn(level, pos, state)) level->addToTickNextTick(pos, this, 1);
}

void DiodeBlock::onPlace(Level* level, const BlockPos& pos, const BlockState* state)
{
    updateNeighborsInFront(level, pos, state);
}

void DiodeBlock::updateNeighborsInFront(Level* level, const BlockPos& pos, const BlockState* state)
{
    const Direction* direction = state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY);
    BlockPos front = pos.relative(direction->getOpposite());
    level->neighborChanged(front, this, pos);
    level->updateNeighborsAtExceptFromFacing(front, this, direction);
}

void DiodeBlock::destroy(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (mOn)
    {
        for (std::vector<const Direction*>::const_iterator direction = Direction::VALUES.begin(); direction != Direction::VALUES.end(); ++direction)
        {
            level->updateNeighborsAt(pos.relative(*direction), this, false);
        }
    }
    Block::destroy(level, pos, state);
}

bool DiodeBlock::isSolidRender(const BlockState*) const { return false; }
bool DiodeBlock::isAlternateInput(const BlockState* state) { return state->isSignalSource(); }
int DiodeBlock::getOutputSignal(LevelSource*, const BlockPos&, const BlockState*) { return 15; }

bool DiodeBlock::isDiode(const BlockState* state)
{
    return static_cast<DiodeBlock*>(Blocks::unpowered_repeater)->isSameDiode(state)
        || static_cast<DiodeBlock*>(Blocks::unpowered_comparator)->isSameDiode(state);
}

bool DiodeBlock::isSameDiode(const BlockState* state)
{
    const BlockState* defaultState = defaultBlockState();
    return state->getBlock() == getOnBlock(defaultState)->getBlock() || state->getBlock() == getOffBlock(defaultState)->getBlock();
}

bool DiodeBlock::shouldPrioritize(Level* level, const BlockPos& pos, const BlockState* state)
{
    const Direction* direction = state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY)->getOpposite();
    const BlockState* neighbor = level->getBlockState(pos.relative(direction));
    return isDiode(neighbor) && neighbor->getValue(HorizontalDirectionalBlock::FACING_PROPERTY) != direction;
}

int DiodeBlock::getTurnOffDelay(const BlockState* state) { return getTurnOnDelay(state); }
bool DiodeBlock::isMatching(Block* block) { return isSameDiode(block->defaultBlockState()); }
int DiodeBlock::getRenderLayer() { return 0; }

EBlockFaceShape DiodeBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction* direction)
{
    return direction == Direction::DOWN ? BLOCK_FACE_SOLID : BLOCK_FACE_UNDEFINED;
}

EBlockRenderShape DiodeBlock::getRenderShape(const BlockState*)
{
    return static_cast<EBlockRenderShape>(36);
}

TextureAtlasSprite* DiodeBlock::getTexture(const Direction* direction, const BlockState*)
{
    if (direction == Direction::DOWN)
        return (mOn ? Blocks::redstone_torch : Blocks::unlit_redstone_torch)->getTexture(direction);
    if (direction == Direction::UP) return mIcon;
    return Blocks::stone_slab->getTexture(Direction::UP);
}
