#ifndef GAME_ENTITY_MOB_ANIMAL_HPP
#define GAME_ENTITY_MOB_ANIMAL_HPP

#include <string>
#include <boost/shared_ptr.hpp>
#include "game/entity/mob/AgableMob.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/core/InteractionHand.hpp"

class BlockPos;
class CompoundTag;
class DamageSource;
class EntityTooltipDataHolder;
class ItemInstance;
class Level;
template <class T> class EntityDataAccessor;

class Animal : public AgableMob
{
public:
    Animal(Level* level);
    virtual ~Animal();
    static void staticCtor();
    virtual void setDespawnProtected();
    virtual void defineSynchedData();
    void setInLoveValue(int value);
    virtual void customServerAiStep();
    int getInLoveValue();
    virtual void updateDespawnProtectedState();
    virtual void aiStep();
    virtual bool hurt(const DamageSource* source, float damage);
    virtual double getWalkTargetValue(const BlockPos& pos);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual double getRidingHeight();
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual bool checkSpawnRules();
    virtual int getAmbientSoundInterval();
    virtual bool removeWhenFarAway();
    virtual int getExperienceReward(boost::shared_ptr<Player> player);
    virtual bool isFood(const not_null_ptr<ItemInstance>& itemInstance);
    bool isBreedCoolingDown();
    void usePlayerItem(boost::shared_ptr<Player> player, not_null_ptr<ItemInstance> itemInstance);
    bool AgeUpBaby(const boost::shared_ptr<Player>& player, not_null_ptr<ItemInstance> itemInstance);
    virtual bool mobInteract(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand);
    virtual void setInLove(boost::shared_ptr<Player> player);
    boost::shared_ptr<Player> getLoveCause();
    bool isInLove();
    void resetLove();
    virtual bool canMate(const boost::shared_ptr<Animal>& other);
    virtual void handleEntityEvent(unsigned char event, int data);
    virtual unsigned int GetInteractTooltip(const EntityTooltipDataHolder& holder);
    bool isDespawnProtected() const;
    virtual void dropLeash(bool dropLeash, bool suppressDrops);
    virtual boost::shared_ptr<Entity> changeDimension(int dimension);
protected:
    int mType;
    std::wstring mLoveCause;
    int mLoveCausePresent;
    int mInLove;
    int mLoveCauseId;
    bool mDespawnProtected;
    short mMinX;
    short mMaxX;
    short mMinZ;
    short mMaxZ;
    int mRandomSeed;
    static const EntityDataAccessor<int>* DATA_IN_LOVE;
    static const EntityDataAccessor<int>* DATA_IN_LOVE_TIME;
};

#endif
