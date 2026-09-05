#ifndef GAME_CORE_EXPERIENCEORB_HPP
#define GAME_CORE_EXPERIENCEORB_HPP

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/Entity.hpp"

class CompoundTag;
class DamageSource;
class Level;

class ExperienceOrb : public Entity
{
public:
    ExperienceOrb(Level* level, double x, double y, double z, int value);
    ExperienceOrb(Level* level);
    virtual ~ExperienceOrb();

    static int getExperienceValue(int value);
    void _init();
    virtual bool makeStepSound();
    virtual void defineSynchedData();
    virtual int getLightColor();
    virtual void tick();
    virtual bool updateInWaterState();
    virtual void burn(int damage);
    virtual bool hurt(const DamageSource* source, float damage);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    int xpToDurability(int value);
    int durabilityToXp(int value);
    virtual void playerTouch(boost::shared_ptr<Player> player);
    int getValue();
    unsigned char getIcon();
    virtual bool isAttackable();
    virtual bool shouldRender(double distanceSqr);
    static void create(Level* level);
    virtual unsigned long long GetType();

private:
    int mCount;
    int mAge;
    int mPickupDelay;
    int mHealth;
    int mValue;
    boost::shared_ptr<Player> mFollowingPlayer;
    int mFollowingTime;
};

#endif
