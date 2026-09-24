#ifndef GAME_CLIENT_RENDER_SPELLPARTICLE_HPP
#define GAME_CLIENT_RENDER_SPELLPARTICLE_HPP

#include "game/client/render/Particle.hpp"
#include "game/client/render/ParticleProvider.hpp"

class SpellParticle : public Particle
{
public:
    SpellParticle(Level* level, double x, double y, double z,
                  double xa, double ya, double za);
    virtual uint64_t GetType();
    virtual ~SpellParticle();
    virtual void tick();
    virtual bool isTransparent();
    void setBaseTex(int texture);

    class AmbientMobProvider : public ParticleProvider
    {
    public:
        virtual Particle* createParticle(ePARTICLE_TYPE type, Level* level,
                                          double x, double y, double z,
                                          double xa, double ya, double za,
                                          arrayWithLength<int> params);
    };

    class InstantProvider : public ParticleProvider
    {
    public:
        virtual Particle* createParticle(ePARTICLE_TYPE type, Level* level,
                                          double x, double y, double z,
                                          double xa, double ya, double za,
                                          arrayWithLength<int> params);
    };

    class MobProvider : public ParticleProvider
    {
    public:
        virtual Particle* createParticle(ePARTICLE_TYPE type, Level* level,
                                          double x, double y, double z,
                                          double xa, double ya, double za,
                                          arrayWithLength<int> params);
    };

    class Provider : public ParticleProvider
    {
    public:
        virtual Particle* createParticle(ePARTICLE_TYPE type, Level* level,
                                          double x, double y, double z,
                                          double xa, double ya, double za,
                                          arrayWithLength<int> params);
    };

    class WinningInstantProvider : public ParticleProvider
    {
    public:
        virtual Particle* createParticle(ePARTICLE_TYPE type, Level* level,
                                          double x, double y, double z,
                                          double xa, double ya, double za,
                                          arrayWithLength<int> params);
    };

    class WitchProvider : public ParticleProvider
    {
    public:
        virtual Particle* createParticle(ePARTICLE_TYPE type, Level* level,
                                          double x, double y, double z,
                                          double xa, double ya, double za,
                                          arrayWithLength<int> params);
    };

private:
    struct InitialVelocity
    {
        double x;
        double y;
        double z;
    };

    static InitialVelocity makeInitialVelocity();
    SpellParticle(Level* level, double x, double y, double z,
                  double xa, double ya, double za, InitialVelocity velocity);

    int baseTex;
};

#endif
