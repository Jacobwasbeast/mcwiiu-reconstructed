#ifndef GAME_CLIENT_RENDER_FIREWORKSPARTICLES__PROVIDER_HPP
#define GAME_CLIENT_RENDER_FIREWORKSPARTICLES__PROVIDER_HPP

#include "game/client/render/ParticleProvider.hpp"

class FireworksParticles
{
public:
    class FireworksSparkParticle;

    class Provider : public ParticleProvider
    {
    public:
        virtual Particle* createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z, double xa, double ya, double za, arrayWithLength<int> params);
    };
};

#endif
