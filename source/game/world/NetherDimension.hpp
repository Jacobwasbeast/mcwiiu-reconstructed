#ifndef GAME_WORLD_NETHERDIMENSION_HPP
#define GAME_WORLD_NETHERDIMENSION_HPP

#include "game/world/Dimension.hpp"
#include "game/world/WorldBorder.hpp"

class NetherDimension : public Dimension
{
public:
    virtual ~NetherDimension();
    virtual void init();
    virtual Vec3* getFogColor(float celestialAngle, float timeOfDay);
    virtual void updateLightRamp();
    virtual LevelSource* createRandomLevelGenerator() const;
    virtual bool isNaturalDimension();
    virtual bool isValidSpawn(int x, int z) const;
    virtual double getTimeOfDay(long long time, float partialTick) const;
    virtual bool mayRespawn() const;
    virtual bool isFoggyAt(int x, int z);
    virtual WorldBorder* createWorldBorder(Level* level);
    virtual const DimensionType* getType() const;
    virtual int getXZSize();
    virtual AABB* getPlayerConstraints(const BlockPos& pos);
    virtual int getBlockLightColour();

    class NetherDimensionWorldBorder : public WorldBorder
    {
    public:
        explicit NetherDimensionWorldBorder(Level* level);
        virtual ~NetherDimensionWorldBorder();
        virtual double getCenterX();
        virtual double getCenterZ();
    };
};

#endif
