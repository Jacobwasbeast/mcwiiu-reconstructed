#ifndef GAME_CORE_OCELOT_HPP
#define GAME_CORE_OCELOT_HPP

#include <string>
#include <boost/shared_ptr.hpp>

#include "game/entity/mob/TamableAnimal.hpp"
#include "game/core/InteractionHand.hpp"

class AgableMob;
class CompoundTag;
class DamageSource;
class DataFixerUpper;
class DifficultyInstance;
class Entity;
class EntityTooltipDataHolder;
class Goal;
class ItemInstance;
class Level;
class MobGroupData;
class SoundEvent;

class Ocelot : public TamableAnimal
{
public:
    Ocelot(Level* level);
    virtual ~Ocelot();
    static void create(Level* level);
    void setSittingOnBlock(bool sitting);
    static void staticCtor();
    virtual void registerGoals();
    virtual void defineSynchedData();
    void SetIsTempted(bool tempted);
    virtual void customServerAiStep();
    virtual bool removeWhenFarAway();
    virtual void registerAttributes();
    virtual void causeFallDamage(float fallDistance, float damageMultiplier);
    static void addDataWalkers(DataFixerUpper* fixer);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual const SoundEvent* getAmbientSound();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getDeathSound();
    virtual float getSoundVolume();
    virtual bool doHurtTarget(boost::shared_ptr<Entity> target);
    virtual bool hurt(const DamageSource* source, float damage);
    virtual const void* getDefaultLootTable();
    virtual bool mobInteract(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand);
    virtual boost::shared_ptr<AgableMob> getBreedOffspring(boost::shared_ptr<AgableMob> partner);
    virtual bool isFood(const not_null_ptr<ItemInstance>& item);
    virtual bool canMate(const boost::shared_ptr<Animal>& other);
    int getCatType();
    void setCatType(int type);
    virtual bool checkSpawnRules();
    virtual bool checkSpawnObstruction();
    virtual std::wstring getName();
    virtual MobGroupData* finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data,
                                           const std::wstring& dataString);
    virtual void positionRider(const boost::shared_ptr<Entity>& passenger);
    bool isSittingOnBlock();
    virtual unsigned int GetInteractTooltip(const EntityTooltipDataHolder& holder);
    virtual void reassessTameGoals();
    virtual unsigned long long GetType();

private:
    Goal* mSitOnBlockGoal;
    Goal* mTemptGoal;
    int mCatType;
    bool mIsTempted;
    bool mSittingOnBlock;
};

#endif
