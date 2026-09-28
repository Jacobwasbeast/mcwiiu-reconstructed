#include "game/world/block/BlockDegradeRoutine.hpp"
#include "game/world/block/BlockId.hpp"

#include <typeinfo>
#include <vector>

#include "game/core/CMinecraftApp.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/minigame/DegradationRuleDefinition.hpp"
#include "game/minigame/DegradationSequenceRuleDefinition.hpp"
#include "game/minigame/NamedAreaRuleDefinition.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/LevelRuleset.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Iterator.hpp"

BlockDegradeRoutine::BlockDegradeRoutine(ServerLevel* level, DegradationSequenceRuleDefinition* definition)
    : mLevel(level),
      mDefinition(definition),
      mNextActionTime(0),
      mAction(&BlockDegradeRoutine::Action_Wait)
{
    std::vector<GameRuleDefinition*> children;
    mDefinition->getChildren(&children);

    for (std::vector<GameRuleDefinition*>::iterator it = children.begin(); it != children.end(); ++it)
    {
        DegradationRuleDefinition* rule = dynamic_cast<DegradationRuleDefinition*>(*it);
        const std::wstring& targetLayer = rule->getTargetLayer();
        LayerCache* cache = getCacheFor(targetLayer);

        Info info;
        info.rule = rule;
        info.cache = cache;
        mActions.push_back(info);
    }
}

void BlockDegradeRoutine::setNextAction(Action action, long long nextActionTime)
{
    mAction = action;
    mNextActionTime = nextActionTime;
}

void BlockDegradeRoutine::Action_Wait(long long time, long long ticks)
{
    if (mActions.empty())
        return;

    LayerCache* cache = mActions.front().cache;
    AABB* bounds = AABB::newTemp(0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    bounds->set(cache->getCurrentBounds());
    bounds->mMaxY += 2.0;

    std::vector<boost::shared_ptr<Entity> >* entities = mLevel->getEntitiesOfClass(typeid(Player), bounds);
    unsigned int entityCount = entities->size();
    delete entities;

    if (entityCount != 0)
    {
        setNextAction(&BlockDegradeRoutine::Action_Wait, ticks + 400);
        return;
    }

    LayerCache* currentCache = mActions.front().cache;
    DegradationRuleDefinition* rule = mActions.front().rule;
    mActions.pop_front();

    {
        std::vector<BlockPos> positions;
        Iterator<BlockPos*>* it = currentCache->iterator();
        BlockPos* pos = it->next();
        while (it->hasNext())
        {
            if (currentCache->inLayer(*pos))
                positions.push_back(*pos);
            it->next();
        }
        mBlocks = arrayWithLength<BlockPos>(positions);
        delete it;
    }
    delete currentCache;
    rule->ClearCaches();

    mProgress = 11;
    setNextAction(&BlockDegradeRoutine::Action_Break, ticks + 500);
}

void BlockDegradeRoutine::Action_Break(long long time, long long ticks)
{
    if (mProgress < 10)
    {
        Random random(time);
        for (unsigned int i = 0; i < mBlocks.length; ++i)
        {
            int progress = Mth::clamp((int)mProgress + random.nextInt(5) - 1, 0, 9);
            mLevel->destroyBlockProgress(-1 - (int)i, mBlocks.data[i], progress);
        }

        ++mProgress;
        if (mProgress > 10)
            mProgress = 10;

        setNextAction(&BlockDegradeRoutine::Action_Break, ticks + 400);
        return;
    }

    for (unsigned int i = 0; i < mBlocks.length; ++i)
    {
        BlockPos& pos = mBlocks.data[i];
        mLevel->destroyBlockProgress(-1 - (int)i, pos, -1);
        mLevel->setBlockAndData(pos.x, pos.y, pos.z, BlockId::AIR, 0, Block::UPDATE_CLIENTS);
    }

    mProgress = 1;
    setNextAction(&BlockDegradeRoutine::Action_Wait, ticks + 100);
}

BlockDegradeRoutine::LayerCache* BlockDegradeRoutine::getCacheFor(const std::wstring& name) const
{
    LevelRuleset* ruleset = g_pApp->getGameRuleDefinitions();
    if (ruleset == NULL)
        return NULL;

    NamedAreaRuleDefinition* area = ruleset->getNamedAreaRule(name);
    if (area == NULL)
        return NULL;

    return new LayerCache(mLevel, area);
}

BlockDegradeRoutine::LayerCache::LayerCache(ServerLevel* level, NamedAreaRuleDefinition* rule)
    : mLevel(level), mRule(rule), mCurrentBounds(rule->getArea())
{
}

BlockDegradeRoutine::LayerCache::~LayerCache()
{
}

AABB* BlockDegradeRoutine::LayerCache::getCurrentBounds() const
{
    return mCurrentBounds;
}

Iterator<BlockPos*>* BlockDegradeRoutine::LayerCache::iterator() const
{
    BlockPos from(mCurrentBounds->mMinX, mCurrentBounds->mMinY, mCurrentBounds->mMinZ);
    BlockPos to(mCurrentBounds->mMaxX, mCurrentBounds->mMaxY, mCurrentBounds->mMaxZ);
    return BlockPos::between(from, to);
}

void BlockDegradeRoutine::LayerCache::setBroken(const BlockPos* positions, unsigned int count)
{
    for (unsigned int i = 0; i < count; ++i)
        mBroken.insert(positions[i]);
}

bool BlockDegradeRoutine::LayerCache::isBroken(const BlockPos& pos)
{
    if (mBroken.find(pos) != mBroken.end())
        return true;

    Block* block = mLevel->getBlock(pos);
    bool broken = block == NULL;
    if (broken)
        setBroken(&pos, 1);
    return broken;
}

bool BlockDegradeRoutine::LayerCache::inLayer(const BlockPos& pos)
{
    if (isBroken(pos))
        return false;

    Block* block = mLevel->getBlock(pos);
    return mRule->containsBlock(Block::getId(block), pos);
}
