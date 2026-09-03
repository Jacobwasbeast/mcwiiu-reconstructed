#ifndef GAME_CLIENT_RENDER_EXPLODEPARTICLE_HPP
#define GAME_CLIENT_RENDER_EXPLODEPARTICLE_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/Particle.hpp"
#include "game/client/render/ParticleProvider.hpp"

class BufferBuilder;
class Entity;
class Level;

const long long PARTICLE_TYPE_EXPLODE = 0x0000000000080204LL;

class ExplodeParticle : public Particle
{
public:
    class Provider : public ParticleProvider
    {
    public:
        virtual Particle* createParticle(ePARTICLE_TYPE type,
                                         Level* level,
                                         double x,
                                         double y,
                                         double z,
                                         double xa,
                                         double ya,
                                         double za,
                                         arrayWithLength<int> params);
    };

    ExplodeParticle(Level* level, double x, double y, double z, double xa, double ya, double za);

    virtual long long GetType();
    virtual ~ExplodeParticle();
    virtual void tick();
    virtual void render(BufferBuilder* buffer,
                        const boost::shared_ptr<Entity>& camera,
                        float partialTicks,
                        float xa,
                        float ya,
                        float za,
                        float xa2,
                        float za2);
};

#endif
