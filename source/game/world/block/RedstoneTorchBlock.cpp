#include "game/world/block/RedstoneTorchBlock.hpp"

#include <deque>
#include <typeinfo>
#include <boost/unordered_map.hpp>

#include "game/client/render/ParticleType.hpp"
#include "game/core/Boxed.hpp"
#include "game/core/Direction.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Property.hpp"
#include "game/world/block/TypedBoxed.hpp"

static boost::unordered_map<Level*, std::deque<RedstoneTorchBlock::Toggle>*> sToggleLists;

static const Direction* getTorchFacing(const BlockState* state)
{
    boost::unordered_map<const Property*, Boxed*>* values = state->getValues();
    for (boost::unordered_map<const Property*, Boxed*>::const_iterator it = values->begin(); it != values->end(); ++it)
        if (it->second != NULL && it->second->isA(typeid(const Direction*)))
            return static_cast<TypedBoxed<const Direction*>*>(it->second)->getValue();
    return Direction::UP;
}

static const BlockState* copyTorchState(const BlockState* source, const BlockState* target)
{
    boost::unordered_map<const Property*, Boxed*>* values = source->getValues();
    const BlockState* result = target;
    for (boost::unordered_map<const Property*, Boxed*>::const_iterator it = values->begin(); it != values->end(); ++it)
        result = result->setBoxedValue(it->first, it->second);
    return result;
}

RedstoneTorchBlock::Toggle::Toggle(const BlockPos& position, long long gameTime)
    : pos(position), time(gameTime)
{
}

RedstoneTorchBlock::RedstoneTorchBlock(bool lit)
    : TorchBlock(), mLit(lit)
{
    ClearDerivedInit();
    DerivedInit();
    setTicking(true);
}

RedstoneTorchBlock::~RedstoneTorchBlock()
{
}

void RedstoneTorchBlock::removeLevelReferences(Level* level)
{
    boost::unordered_map<Level*, std::deque<Toggle>*>::iterator it = sToggleLists.find(level);
    if (it != sToggleLists.end())
    {
        delete it->second;
        sToggleLists.erase(it);
    }
}

bool RedstoneTorchBlock::isToggledTooFrequently(Level* level, const BlockPos& pos, bool addToggle)
{
    std::deque<Toggle>*& toggles = sToggleLists[level];
    if (toggles == NULL)
        toggles = new std::deque<Toggle>();
    long long gameTime = level->getGameTime();
    while (!toggles->empty() && gameTime - toggles->front().time >= 61)
        toggles->pop_front();
    if (addToggle)
        toggles->push_back(Toggle(pos, gameTime));
    int count = 0;
    for (std::deque<Toggle>::const_iterator it = toggles->begin(); it != toggles->end(); ++it)
        if (it->pos.equals(pos) && ++count > 7)
            return true;
    return false;
}

int RedstoneTorchBlock::getTickDelay(Level*)
{
    return 2;
}

void RedstoneTorchBlock::onPlace(Level* level, const BlockPos& pos, const BlockState*)
{
    if (mLit)
        for (std::vector<const Direction*>::const_iterator it = Direction::VALUES.begin(); it != Direction::VALUES.end(); ++it)
            level->updateNeighborsAt(pos.relative(*it), this, false);
}

void RedstoneTorchBlock::onRemove(Level* level, const BlockPos& pos, const BlockState*)
{
    if (mLit)
        for (std::vector<const Direction*>::const_iterator it = Direction::VALUES.begin(); it != Direction::VALUES.end(); ++it)
            level->updateNeighborsAt(pos.relative(*it), this, false);
}

int RedstoneTorchBlock::getSignal(const BlockState* state, LevelSource*, const BlockPos&, const Direction* direction)
{
    return mLit && getTorchFacing(state) != direction ? 15 : 0;
}

bool RedstoneTorchBlock::hasNeighborSignal(Level* level, const BlockPos& pos, const BlockState* state)
{
    const Direction* direction = getTorchFacing(state)->getOpposite();
    return level->hasSignal(pos.relative(direction), direction);
}

void RedstoneTorchBlock::randomTick(Level*, const BlockPos&, const BlockState*, Random*)
{
}

void RedstoneTorchBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random)
{
    bool powered = hasNeighborSignal(level, pos, state);
    if (!mLit && !powered && !isToggledTooFrequently(level, pos, false))
        level->setBlock(pos, copyTorchState(state, Blocks::redstone_torch->defaultBlockState()), 3, false);
    else if (mLit && powered)
    {
        level->setBlock(pos, copyTorchState(state, Blocks::unlit_redstone_torch->defaultBlockState()), 3, false);
        if (isToggledTooFrequently(level, pos, true) && random != NULL)
            for (int i = 0; i < 5; ++i)
                level->addParticle(PARTICLE_RED_DUST, pos.x + random->nextDouble(), pos.y + random->nextDouble(), pos.z + random->nextDouble(), 0.0, 0.0, 0.0, arrayWithLength<int>());
    }
}

void RedstoneTorchBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block*, const BlockPos&)
{
    if (hasNeighborSignal(level, pos, state) == mLit)
        level->setBlock(pos, copyTorchState(state, mLit ? Blocks::unlit_redstone_torch->defaultBlockState() : Blocks::redstone_torch->defaultBlockState()), 3, false);
}

int RedstoneTorchBlock::getDirectSignal(const BlockState* state, LevelSource* levelSource, const BlockPos& pos, const Direction* direction)
{
    return direction == Direction::DOWN ? getSignal(state, levelSource, pos, direction) : 0;
}

Item* RedstoneTorchBlock::getResource(const BlockState*, Random*, int)
{
    return Item::byBlock(Blocks::redstone_torch);
}

bool RedstoneTorchBlock::isSignalSource(const BlockState*)
{
    return true;
}

void RedstoneTorchBlock::animateTick(const BlockState* state, Level* level, const BlockPos& pos, Random* random)
{
    if (!mLit)
        return;
    double x = pos.x + 0.5 + (random->nextDouble() - 0.5) * 0.2;
    double y = pos.y + 0.7 + (random->nextDouble() - 0.5) * 0.2;
    double z = pos.z + 0.5 + (random->nextDouble() - 0.5) * 0.2;
    const Direction* facing = getTorchFacing(state);
    if (facing->getAxis()->isHorizontal())
    {
        Vec3i normal = facing->getOpposite()->getNormal();
        x += 0.27 * normal.x;
        z += 0.27 * normal.z;
        y += 0.22;
    }
    level->addParticle(PARTICLE_RED_DUST, x, y, z, 0.0, 0.0, 0.0, arrayWithLength<int>());
}

not_null_ptr<ItemInstance> RedstoneTorchBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState*)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(Blocks::redstone_torch));
}

bool RedstoneTorchBlock::isMatching(Block* block)
{
    return block == Blocks::unlit_redstone_torch || block == Blocks::redstone_torch;
}
