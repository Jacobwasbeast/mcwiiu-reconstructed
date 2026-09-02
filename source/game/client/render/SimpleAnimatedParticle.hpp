#ifndef GAME_CLIENT_RENDER_SIMPLEANIMATEDPARTICLE_HPP
#define GAME_CLIENT_RENDER_SIMPLEANIMATEDPARTICLE_HPP

#include "game/client/render/Particle.hpp"

class Level;

class SimpleAnimatedParticle : public Particle
{
public:
    SimpleAnimatedParticle(Level* level,
                           double x,
                           double y,
                           double z,
                           int baseTex,
                           int numTex,
                           float baseGravity);
    virtual ~SimpleAnimatedParticle();

    virtual void tick();
    virtual bool isTransparent();
    virtual int getLightColor(float partialTicks);
    virtual float getBrightness(float partialTicks);

    void setColor(int color);
    void setFadeColor(int color);
    void setBaseAirFriction(float airFriction);

    int baseTex;
    int numTex;
    float baseGravity;
    float baseAirFriction;
    float fadeR;
    float fadeG;
    float fadeB;
    bool hasFade;
};

#endif
