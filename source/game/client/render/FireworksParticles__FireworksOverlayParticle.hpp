#ifndef GAME_CLIENT_RENDER_FIREWORKSPARTICLES_FIREWORKSOVERLAYPARTICLE_HPP
#define GAME_CLIENT_RENDER_FIREWORKSPARTICLES_FIREWORKSOVERLAYPARTICLE_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/Particle.hpp"

class BufferBuilder;
class Entity;

namespace FireworksParticles
{
class FireworksOverlayParticle : public Particle
{
public:
    FireworksOverlayParticle(Level* level, double x, double y, double z);
    virtual ~FireworksOverlayParticle();
    virtual void render(BufferBuilder* buffer,
                        const boost::shared_ptr<Entity>& camera,
                        float partialTicks,
                        float xa,
                        float ya,
                        float za,
                        float xa2,
                        float za2);
    virtual long long GetType();
};
}

#endif
