#ifndef GAME_CORE_LIGHTNINGBOLT_HPP
#define GAME_CORE_LIGHTNINGBOLT_HPP

#include <vector>

#include "game/client/audio/SoundSource.hpp"
#include "game/entity/mob/Entity.hpp"

class CompoundTag;
class Level;

const unsigned long long ENTITY_TYPE_LIGHTNING_BOLT = 0x000000000B000001ULL;

class LightningBolt : public Entity
{
public:
    LightningBolt(Level* level, double x, double y, double z, bool visualOnly);

    virtual unsigned long long GetType();
    virtual ~LightningBolt();
    virtual void defineSynchedData();
    virtual void tick();
    virtual bool shouldRender(double distanceSqr);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual SoundSource::ESoundSource getSoundSource();

    int mLife;
    std::vector<int> mHitEntities;
    long long mSeed;
    int mFlashes;
    bool mVisualOnly;
};

#endif
