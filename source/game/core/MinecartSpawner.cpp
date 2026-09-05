#include "game/core/MinecartSpawner.hpp"

#include <cwchar>

#include "game/core/OverwritableWalker.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"

void PIXBeginNamedEvent(float colour, const char* name, ...);
void PIXEndNamedEvent();

void MinecartSpawner::create(Level* level)
{
    new MinecartSpawner(level);
}

MinecartSpawner::MinecartSpawner(Level* level)
    : Minecart(level), mSpawner(new MinecartMobSpawner(this))
{
}

MinecartSpawner::MinecartSpawner(Level* level, double x, double y, double z)
    : Minecart(level, x, y, z), mSpawner(new MinecartMobSpawner(this))
{
}

MinecartSpawner::~MinecartSpawner()
{
    delete mSpawner;
}

void MinecartSpawner::addDataWalkers(DataFixerUpper* fixer)
{
    fixer->addWalker(DataFixTypes::eDataFixTypes_Entity,
                     new OverwritableWalker(0x000000000A800006ULL, L"MobSpawner"));
}

unsigned int MinecartSpawner::getType()
{
    return 0;
}

const BlockState* MinecartSpawner::getDefaultDisplayBlockState()
{
    return Blocks::mob_spawner->defaultBlockState();
}

void MinecartSpawner::readAdditionalSaveData(CompoundTag* tag)
{
    Minecart::readAdditionalSaveData(tag);
    mSpawner->load(tag);
}

void MinecartSpawner::addAdditonalSaveData(CompoundTag* tag)
{
    Minecart::addAdditonalSaveData(tag);
    mSpawner->save(tag);
}

void MinecartSpawner::handleEntityEvent(unsigned char event, int data)
{
    mSpawner->onEventTriggered(data);
}

void MinecartSpawner::tick()
{
    PIXBeginNamedEvent(4.5f, "Minecart spawner tick");
    Minecart::tick();
    mSpawner->tick();
    PIXEndNamedEvent();
}

MinecartSpawner::MinecartMobSpawner* MinecartSpawner::getSpawner()
{
    return mSpawner;
}

unsigned long long MinecartSpawner::GetType()
{
    return 0x0A80000600000000ULL;
}

MinecartSpawner::MinecartMobSpawner::MinecartMobSpawner(MinecartSpawner* minecart)
    : BaseMobSpawner(), mMinecart(minecart)
{
}

MinecartSpawner::MinecartMobSpawner::~MinecartMobSpawner()
{
}

void MinecartSpawner::MinecartMobSpawner::broadcastEvent(int event)
{
    getLevel()->broadcastEntityEvent(mMinecart->shared_from_this(), (unsigned char)event, 0);
}

Level* MinecartSpawner::MinecartMobSpawner::getLevel()
{
    return mMinecart->mLevel;
}

BlockPos MinecartSpawner::MinecartMobSpawner::getPos()
{
    return BlockPos(mMinecart->shared_from_this());
}
