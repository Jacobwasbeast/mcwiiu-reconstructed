#ifndef GAME_CORE_MINECARTSPAWNER_HPP
#define GAME_CORE_MINECARTSPAWNER_HPP

#include "game/core/Minecart.hpp"
#include "game/entity/mob/BaseMobSpawner.hpp"

class BlockState;
class CompoundTag;
class DataFixerUpper;
class Level;

class MinecartSpawner : public Minecart
{
public:
    class MinecartMobSpawner : public BaseMobSpawner
    {
    public:
        explicit MinecartMobSpawner(MinecartSpawner* minecart);
        virtual ~MinecartMobSpawner();
        virtual void broadcastEvent(int event);
        virtual Level* getLevel();
        virtual BlockPos getPos();

    private:
        MinecartSpawner* mMinecart;
    };

    MinecartSpawner(Level* level);
    MinecartSpawner(Level* level, double x, double y, double z);
    virtual ~MinecartSpawner();

    static void create(Level* level);
    static void addDataWalkers(DataFixerUpper* fixer);
    virtual unsigned int getType();
    virtual const BlockState* getDefaultDisplayBlockState();
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual void handleEntityEvent(unsigned char event, int data);
    virtual void tick();
    MinecartMobSpawner* getSpawner();
    virtual unsigned long long GetType();

private:
    MinecartMobSpawner* mSpawner;
};

#endif
