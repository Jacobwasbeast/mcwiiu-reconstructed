#ifndef GAME_ENTITY_MOB_TAMABLEANIMAL_HPP
#define GAME_ENTITY_MOB_TAMABLEANIMAL_HPP

#include <string>
#include <boost/shared_ptr.hpp>

#include "game/entity/mob/Animal.hpp"
#include "game/entity/mob/OwnableEntity.hpp"
#include "game/core/InteractionHand.hpp"

class CompoundTag;
class DamageSource;
class Entity;
class Level;
class Player;
class SitGoal;
class Team;
class Vec3;
template <class T> class EntityDataAccessor;

class TamableAnimal : public Animal, public OwnableEntity
{
public:
    TamableAnimal(Level* level);
    virtual ~TamableAnimal();
    virtual std::wstring getOwnerUUID();
    virtual boost::shared_ptr<LivingEntity> getOwner();
    static void staticCtor();
    virtual void fjDerivedCtorCalls();
    virtual void defineSynchedData();
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual bool canBeLeashed(const boost::shared_ptr<Player>& player);
    virtual void spawnTamingParticles(bool success);
    virtual void handleEntityEvent(unsigned char event, int data);
    bool isTame();
    void setTame(bool tame);
    virtual void reassessTameGoals();
    virtual bool removeWhenFarAway();
    bool isSitting();
    void setSitting(bool sitting);
    void setOwnerUUID(const std::wstring& uuid);
    virtual void tame(boost::shared_ptr<Player> player);
    bool isOwnedBy(boost::shared_ptr<LivingEntity> entity);
    SitGoal* getSitGoal();
    virtual bool wantsToAttack(boost::shared_ptr<LivingEntity> owner, boost::shared_ptr<LivingEntity> target);
    virtual Team* getTeam();
    virtual bool isAlliedTo(boost::shared_ptr<Entity> entity);
    virtual void die(const DamageSource* source);
    virtual bool interactAt(const boost::shared_ptr<Player>& player, Vec3* pos,
                            InteractionHand::EInteractionHand hand);

protected:
    SitGoal* mSitGoal;
    static const EntityDataAccessor<unsigned char>* DATA_FLAGS;
    static const EntityDataAccessor<std::wstring>* DATA_OWNER_UUID;
};

#endif
