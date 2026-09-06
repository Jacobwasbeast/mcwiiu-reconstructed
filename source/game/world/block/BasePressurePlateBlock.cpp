#include "game/world/block/BasePressurePlateBlock.hpp"

#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/FenceBlock.hpp"

static const AABB* PRESSED_AABB = AABB::newPermanent(0.0625, 0.0, 0.0625, 0.9375, 0.03125, 0.9375);
static const AABB* UNPRESSED_AABB = AABB::newPermanent(0.0625, 0.0, 0.0625, 0.9375, 0.0625, 0.9375);
static const AABB* CLIP_AABB = AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.0625, 1.0);

BasePressurePlateBlock::BasePressurePlateBlock(const std::wstring& name, Material* material)
    : Block(material), mTextureName(name)
{
    setTicking(true);
}

BasePressurePlateBlock::BasePressurePlateBlock(const std::wstring& name, Material* material, const MaterialColor* materialColor)
    : Block(material, materialColor), mTextureName(name)
{
    setTicking(true);
}

BasePressurePlateBlock::~BasePressurePlateBlock()
{
}

const AABB* BasePressurePlateBlock::getShape(const BlockState* state, LevelSource*, const BlockPos&)
{
    if (getSignalForState(state) > 0)
        return PRESSED_AABB;
    return UNPRESSED_AABB;
}

int BasePressurePlateBlock::getTickDelay(Level*)
{
    return 20;
}

const AABB* BasePressurePlateBlock::getClipAABB(const BlockState*, LevelSource*, const BlockPos&)
{
    return CLIP_AABB;
}

bool BasePressurePlateBlock::isSolidRender(const BlockState*) const
{
    return false;
}

bool BasePressurePlateBlock::blocksLight()
{
    return false;
}

bool BasePressurePlateBlock::isCubeShaped(const BlockState*)
{
    return false;
}

bool BasePressurePlateBlock::isPathfindable(LevelSource*, const BlockPos&)
{
    return true;
}

bool BasePressurePlateBlock::isPossibleToRespawnInThis()
{
    return true;
}

bool BasePressurePlateBlock::canPlaceOn(Level* level, const BlockPos& pos)
{
    const BlockState* state = level->getBlockState(pos);
    if (state->isSolidBlockingBlock())
        return true;
    return dynamic_cast<FenceBlock*>(state->getBlock()) != NULL;
}

bool BasePressurePlateBlock::mayPlace(Level* level, const BlockPos& pos)
{
    return canPlaceOn(level, pos.below());
}

void BasePressurePlateBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block, const BlockPos& fromPos)
{
    BlockPos below = pos.below();
    Block* supportingBlock = level->getBlockState(below)->getBlock();
    if (!canPlaceOn(level, below) && !Block::isExceptionForPistonAttaching(supportingBlock))
    {
        spawnResources(level, pos, state, 0);
        level->removeBlock(pos);
    }
}

void BasePressurePlateBlock::randomTick(Level*, const BlockPos&, const BlockState*, Random*)
{
}

void BasePressurePlateBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random*)
{
    int signal = getSignalForState(state);
    if (!level->mIsClientSide && signal > 0)
        checkPressed(level, pos, state, signal);
}

void BasePressurePlateBlock::entityInside(Level* level, const BlockPos& pos, const BlockState* state, const boost::shared_ptr<Entity>& entity)
{
    int signal = getSignalForState(state);
    if (!level->mIsClientSide && signal == 0)
        checkPressed(level, pos, state, signal);
}

void BasePressurePlateBlock::checkPressed(Level* level, const BlockPos& pos, const BlockState* state, int signal)
{
    int sensedSignal = getSignalStrength(level, pos);
    bool wasPressed = signal > 0;
    bool isPressed = sensedSignal > 0;
    if (signal != sensedSignal)
    {
        const BlockState* newState = setSignalForState(state, sensedSignal);
        level->setBlock(pos, newState, 2, false);
        updateNeighbours(level, pos);
        level->setBlocksDirty(pos, pos);
    }
    if (isPressed)
    {
        if (!wasPressed)
            playOnSound(level, pos);
        level->addToTickNextTick(pos, this, getTickDelay(level));
    }
    else if (wasPressed)
    {
        playOffSound(level, pos);
    }
}

const AABB* BasePressurePlateBlock::getSensitiveAABB(const BlockPos& pos)
{
    return AABB::newTemp(pos.x + 0.125, pos.y, pos.z + 0.125,
                         pos.x + 0.875, pos.y + 0.25, pos.z + 0.875);
}

void BasePressurePlateBlock::onRemove(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (getSignalForState(state) > 0)
        updateNeighbours(level, pos);
    Block::onRemove(level, pos, state);
}

void BasePressurePlateBlock::updateNeighbours(Level* level, const BlockPos& pos)
{
    level->updateNeighborsAt(pos, this, false);
    level->updateNeighborsAt(pos.below(), this, false);
}

int BasePressurePlateBlock::getSignal(const BlockState* state, LevelSource*, const BlockPos&, const Direction*)
{
    return getSignalForState(state);
}

int BasePressurePlateBlock::getDirectSignal(const BlockState* state, LevelSource*, const BlockPos&, const Direction* direction)
{
    if (direction == Direction::UP)
        return getSignalForState(state);
    return 0;
}

bool BasePressurePlateBlock::isSignalSource(const BlockState*)
{
    return true;
}

EPistonPushReaction BasePressurePlateBlock::getPistonPushReaction(const BlockState*)
{
    return PISTON_PUSH_DESTROY;
}

EBlockFaceShape BasePressurePlateBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction*)
{
    return BLOCK_FACE_UNDEFINED;
}

void BasePressurePlateBlock::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(mTextureName);
}
