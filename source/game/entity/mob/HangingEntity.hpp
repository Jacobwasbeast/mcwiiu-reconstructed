#ifndef GAME_ENTITY_MOB_HANGINGENTITY_HPP
#define GAME_ENTITY_MOB_HANGINGENTITY_HPP

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/Entity.hpp"
#include "game/world/block/BlockPos.hpp"

class CompoundTag;
class DamageSource;
class Direction;
class LightningBolt;
class Mirror;
class Rotation;
class ItemInstance;
class ItemEntity;

template <class T> class not_null_ptr;

class HangingEntity : public Entity
{
public:
    class HangingEntitySelector;
    HangingEntity(Level* level);
    HangingEntity(Level* level, const BlockPos& pos);
    virtual ~HangingEntity();

    void _init(Level* level);
    virtual void defineSynchedData();
    void setDirection(const Direction* direction);
    double offs(int value);
    void recalculateBoundingBox();
    virtual void tick();
    virtual bool survives();
    virtual bool isPickable();
    virtual bool skipAttackInteraction(boost::shared_ptr<Entity> entity);
    virtual const Direction* getDirection();
    virtual bool hurt(const DamageSource* source, float damage);
    virtual void move(MoverType type, double x, double y, double z, bool pushedByPiston);
    virtual void push(double x, double y, double z);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual void readAdditionalSaveData(CompoundTag* tag) = 0;
    virtual boost::shared_ptr<ItemEntity> spawnAtLocation(not_null_ptr<ItemInstance> itemInstance, float yOffset);
    virtual bool repositionEntityAfterLoad();
    virtual void setPos(double x, double y, double z);
    BlockPos getPos();
    virtual float rotate(Rotation* rotation);
    virtual float mirror(Mirror* mirror);
    virtual void thunderHit(const LightningBolt* lightningBolt);
    virtual bool shouldRender(double distanceSqr);
    virtual bool shouldRenderAtSqrDistance(double distanceSqr);
    virtual unsigned long long GetType();

    virtual int getWidth() = 0;
    virtual int getHeight() = 0;
    virtual void dropItem(boost::shared_ptr<Entity> breaker) = 0;

protected:
    int mTickCounter;
    BlockPos mPos;
    const Direction* mDirection;
};

class HangingEntity::HangingEntitySelector : public Predicate<boost::shared_ptr<Entity> >
{
public:
    virtual bool apply(boost::shared_ptr<Entity> entity) const;
    virtual ~HangingEntitySelector();
};

#endif
