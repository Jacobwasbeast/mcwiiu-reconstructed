#ifndef GAME_CLIENT_RENDER_ITEMPICKUPPARTICLE_HPP
#define GAME_CLIENT_RENDER_ITEMPICKUPPARTICLE_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/Particle.hpp"

class BufferBuilder;
class Entity;
class EntityRenderDispatcher;
class Level;

const long long PARTICLE_TYPE_ITEM_PICKUP = 0x0000000000080225LL;

class ItemPickupParticle : public Particle
{
public:
    ItemPickupParticle(Level* level,
                       boost::shared_ptr<Entity> itemEntity,
                       boost::shared_ptr<Entity> target,
                       float yOffs);

    virtual long long GetType();
    virtual ~ItemPickupParticle();
    virtual void tick();
    virtual void render(BufferBuilder* buffer,
                        const boost::shared_ptr<Entity>& camera,
                        float partialTicks,
                        float xa,
                        float ya,
                        float za,
                        float xa2,
                        float za2);
    virtual int getParticleTexture();

    boost::shared_ptr<Entity> itemEntity;
    boost::shared_ptr<Entity> target;
    float yOffs;
    EntityRenderDispatcher* entityRenderDispatcher;
};

#endif
