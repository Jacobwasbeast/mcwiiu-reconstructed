#ifndef GAME_CORE_VINDICATIONILLAGER_HPP
#define GAME_CORE_VINDICATIONILLAGER_HPP

#include <string>
#include <typeinfo>
#include <boost/shared_ptr.hpp>

#include "game/core/AbstractIllager.hpp"
#include "game/entity/ai/NearestAttackableTargetGoal.hpp"
#include "game/world/block/Predicate.hpp"

class CompoundTag;
class DamageSource;
class DifficultyInstance;
class DataFixerUpper;
class Level;
class MobGroupData;
class SoundEvent;
class Entity;

class VindicationIllager : public AbstractIllager
{
public:
    class LIVING_ENTITY_SELECTOR : public Predicate<boost::shared_ptr<Entity> >
    {
    public:
        virtual bool apply(boost::shared_ptr<Entity> entity) const;
        virtual ~LIVING_ENTITY_SELECTOR();
    };

    class VindicatorJohnnyAttackGoal : public NearestAttackableTargetGoal
    {
    public:
        explicit VindicatorJohnnyAttackGoal(VindicationIllager* illager);
        virtual ~VindicatorJohnnyAttackGoal();
        virtual bool canUse();
    };

    explicit VindicationIllager(Level* level);
    virtual ~VindicationIllager();

    static void create(Level* level);
    static void addDataWalkers(DataFixerUpper* fixer);

    virtual void registerGoals();
    virtual void registerAttributes();
    virtual void defineSynchedData();
    virtual EMobType getMobType();
    virtual const void* getDefaultLootTable();
    virtual const void* getDefaultGearTable();
    bool isAgressive();
    void setAggressive(bool aggressive);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual int getArmPose();
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual MobGroupData* finalizeMobSpawn(const DifficultyInstance& difficulty,
                                           MobGroupData* data,
                                           const std::wstring& dataString);
    virtual void customServerAiStep();
    virtual bool isAlliedTo(boost::shared_ptr<Entity> entity);
    virtual void setCustomName(const std::wstring& name, const std::wstring& ownerUUID,
                               bool namedByRestrictedPlayer);
    virtual const SoundEvent* getAmbientSound();
    virtual const SoundEvent* getDeathSound();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual unsigned long long GetType();

private:
    bool mJohnny;
};

#endif
