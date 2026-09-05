#include "game/client/render/BreakingItemParticle.hpp"

#include "game/client/render/TextureAtlasSprite.hpp"
#include "game/client/render/TextureUVCoordinateSet.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/item/Item.hpp"
#include "game/item/Items.hpp"

BreakingItemParticle::BreakingItemParticle(Level* level, double x, double y, double z,
                                           Item* item, int meta)
    : Particle(level, x, y, z, 0.0, 0.0, 0.0)
{
    _init(item, meta);
}

BreakingItemParticle::BreakingItemParticle(Level* level, double x, double y, double z,
                                           double xa, double ya, double za,
                                           Item* item, int meta)
    : Particle(level, x, y, z, 0.0, 0.0, 0.0)
{
    _init(item, meta);
    xd = xd * 0.10000000149011612 + xa;
    yd = yd * 0.10000000149011612 + ya;
    zd = zd * 0.10000000149011612 + za;
}

BreakingItemParticle::~BreakingItemParticle()
{
}

uint32_t BreakingItemParticle::getParticleTexture()
{
    return item->getIconType() != 0 ? 2 : 1;
}

void BreakingItemParticle::render(BufferBuilder* buffer,
                                  const boost::shared_ptr<Entity>& camera,
                                  float partialTicks, float xa, float ya, float za,
                                  float xa2, float za2)
{
    float scale = alpha * 0.10000000149011612f;
    float px = (float)(xo + (x - xo) * partialTicks - Particle::xOff);
    float py = (float)(yo + (y - yo) * partialTicks - Particle::yOff);
    float pz = (float)(zo + (z - zo) * partialTicks - Particle::zOff);
    float x1 = xa * scale;
    float y1 = ya * scale;
    float z1 = za * scale;
    float x2 = xa2 * scale;
    float z2 = za2 * scale;
    float u0 = (float)sprite->getU(size * 0.25f * 0.0625f);
    float u1 = (float)sprite->getU((size + 1.0f) * 0.25f * 0.0625f);
    float v0 = (float)sprite->getV(size * 0.25f * 0.0625f);
    float v1 = (float)sprite->getV((size + 1.0f) * 0.25f * 0.0625f);
    int light = getLightColor(partialTicks);

    buffer->color(rCol, gCol, bCol, alpha);
    buffer->tex2(light);
    buffer->vertexUV(px - x1 - x2, py - y1, pz - z1 - z2, u1, v1);
    buffer->vertexUV(px - x1 + x2, py + y1, pz - z1 + z2, u1, v0);
    buffer->vertexUV(px + x1 + x2, py + y1, pz + z1 + z2, u0, v0);
    buffer->vertexUV(px + x1 - x2, py - y1, pz + z1 - z2, u0, v1);
}

void BreakingItemParticle::_init(Item* item, int meta)
{
    this->item = item;
    setSprite(item->getIcon(meta));
    rCol = 1.0f;
    gCol = 1.0f;
    bCol = 1.0f;
    alpha *= 0.5f;
    lifetime = 4;
    uvs = TextureUVCoordinateSet(size * 0.25f * 0.0625f,
                                 textureIndex * 0.25f * 0.0625f,
                                 size * 0.25f * 0.0625f + 0.015609375f,
                                 textureIndex * 0.25f * 0.0625f + 0.015609375f,
                                 1, 1);
}

long long BreakingItemParticle::GetType()
{
    return 0x0008020200000000LL;
}

Particle* BreakingItemParticle::Provider::createParticle(ePARTICLE_TYPE type, Level* level,
                                                          double x, double y, double z,
                                                          double xa, double ya, double za,
                                                          arrayWithLength<int> params)
{
    int meta = params.length > 1 ? params.data[1] : 0;
    return new BreakingItemParticle(level, x, y, z, xa, ya, za,
                                    Item::byId(params.data[0]), meta);
}

Particle* BreakingItemParticle::SlimeProvider::createParticle(ePARTICLE_TYPE type, Level* level,
                                                               double x, double y, double z,
                                                               double xa, double ya, double za,
                                                               arrayWithLength<int> params)
{
    return new BreakingItemParticle(level, x, y, z, Items::slimeBall, 0);
}

Particle* BreakingItemParticle::SnowballProvider::createParticle(ePARTICLE_TYPE type, Level* level,
                                                                  double x, double y, double z,
                                                                  double xa, double ya, double za,
                                                                  arrayWithLength<int> params)
{
    return new BreakingItemParticle(level, x, y, z, Items::snowball, 0);
}
