#ifndef GAME_ENTITY_MOB_AGABLEWATERANIMAL_HPP
#define GAME_ENTITY_MOB_AGABLEWATERANIMAL_HPP

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/AgableMob.hpp"

class Level;
class Player;
class WaterBoundPathNavigation;

class AgableWaterAnimal : public AgableMob
{
public:
    AgableWaterAnimal(Level* level);
    virtual ~AgableWaterAnimal();

    virtual bool isWaterMob();
    virtual bool checkSpawnRules();
    virtual bool checkSpawnObstruction();
    virtual unsigned int getAmbientSoundInterval();
    virtual int getExperienceReward(boost::shared_ptr<Player> player);
    virtual WaterBoundPathNavigation* createNavigation(Level* level);
    virtual void aiStep();
    virtual bool isPushedByWater();
    virtual bool breathesAir();
};

#endif
