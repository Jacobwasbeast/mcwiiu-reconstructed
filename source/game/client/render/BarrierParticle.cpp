#include "game/client/render/BarrierParticle.hpp"

#include "game/client/render/TextureAtlasSprite.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/item/Item.hpp"
#include "game/world/block/Blocks.hpp"

BarrierParticle::BarrierParticle(Level* level, double x, double y, double z, Item* item)
    : Particle(level, x, y, z, 0.0, 0.0, 0.0)
{
    setSprite(item->getIcon(0));
    xd = 0.0;
    yd = 0.0;
    zd = 0.0;
    size = 1.0f;
    gravity = 90.0f;
    lifetime = 80;
}

BarrierParticle::~BarrierParticle()
{
}

int BarrierParticle::getParticleTexture()
{
    return 1;
}

void BarrierParticle::render(BufferBuilder* buffer,
                             const boost::shared_ptr<Entity>& camera,
                             float partialTicks,
                             float xa,
                             float ya,
                             float za,
                             float xa2,
                             float za2)
{
    float scale = 0.5f;
    float px = (float)(xo + (x - xo) * partialTicks - Particle::xOff);
    float py = (float)(yo + (y - yo) * partialTicks - Particle::yOff);
    float pz = (float)(zo + (z - zo) * partialTicks - Particle::zOff);
    float x1 = xa * scale;
    float y1 = ya * scale;
    float z1 = za * scale;
    float x2 = xa2 * scale;
    float z2 = za2 * scale;
    float u0 = (float)sprite->getU0(false);
    float u1 = (float)sprite->getU1(false);
    float v0 = (float)sprite->getV0(false);
    float v1 = (float)sprite->getV1(false);
    int light = getLightColor(partialTicks);

    buffer->color(rCol, gCol, bCol, alpha);
    buffer->tex2(light);
    buffer->vertexUV(px - x1 - x2, py - y1, pz - z1 - z2, u1, v1);
    buffer->vertexUV(px - x1 + x2, py + y1, pz - z1 + z2, u1, v0);
    buffer->vertexUV(px + x1 + x2, py + y1, pz + z1 + z2, u0, v0);
    buffer->vertexUV(px + x1 - x2, py - y1, pz + z1 - z2, u0, v1);
}

uint64_t BarrierParticle::GetType()
{
    return 0x0000000000080223ULL;
}

Particle* BarrierParticle::Provider::createParticle(ePARTICLE_TYPE type,
                                                    Level* level,
                                                    double x,
                                                    double y,
                                                    double z,
                                                    double xa,
                                                    double ya,
                                                    double za,
                                                    arrayWithLength<int> params)
{
    return new BarrierParticle(level, x, y, z, Item::byBlock(Blocks::BARRIER));
}
