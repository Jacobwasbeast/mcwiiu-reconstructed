#include "game/client/render/WaterCurrentDownParticle.hpp"

#include "game/core/Mth.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BlockPos.hpp"

WaterCurrentDownParticle::WaterCurrentDownParticle(Level* level, double x, double y, double z)
    : Particle(level, x, y, z, 0.0, 0.0, 0.0),
      rotation(0.0f)
{
    setMiscTex(32);
    lifetime = (int)(random.nextFloat() * 60.0f) + 30;
    setSize(0.02f, 0.02f);
    size *= random.nextFloat() * 0.6f + 0.2f;
    gravity = 0.002f;
}

WaterCurrentDownParticle::~WaterCurrentDownParticle()
{
}

void WaterCurrentDownParticle::tick()
{
    xo = x;
    yo = y;
    zo = z;

    xd += (double)((float)(0.6f * Mth::cos(rotation)));
    zd += (double)((float)(0.6f * Mth::sin(rotation)));
    xd *= 0.07;
    zd *= 0.07;
    move(xd, yd, zd);

    if (age++ >= lifetime || onGround)
        remove();

    BlockPos pos(x, y, z);
    Block* block = level->getBlockState(pos)->getBlock();
    Block* below = level->getBlockState(pos.below())->getBlock();

    if (below != Blocks::WATER && below != Blocks::FLOWING_WATER && !block->isLiquidBlock())
        remove();

    rotation += 0.08f;
}

int WaterCurrentDownParticle::getRawBr()
{
    int brightness = 15;
    BlockPos pos(x, y, z);

    if (level->hasChunkAt(pos))
        brightness = level->getRawBrightness(pos);

    return Mth::floor(Mth::Lerp(0.98f, 15.0f, (float)brightness / 15.0f));
}

float WaterCurrentDownParticle::getBrightness(float partialTicks)
{
    return level->getBrightness(BlockPos(x, y, z));
}

int WaterCurrentDownParticle::getLightColor(float partialTicks)
{
    BlockPos pos(x, y, z);
    int key = (pos.x & 0xff) | ((pos.y & 0xff) << 8) | ((pos.z & 0xff) << 16);

    if (key == cachedLightPos)
        return cachedLightColor;

    cachedLightPos = key;
    int brightness = getRawBr();
    cachedLightColor = (brightness << 20) | (brightness << 4);
    return cachedLightColor;
}

long long WaterCurrentDownParticle::GetType()
{
    return PARTICLE_TYPE_CURRENT_DOWN;
}

Particle* WaterCurrentDownParticle::Provider::createParticle(ePARTICLE_TYPE type, Level* level,
                                                             double x, double y, double z,
                                                             double xa, double ya, double za,
                                                             arrayWithLength<int> params)
{
    return new WaterCurrentDownParticle(level, x, y, z);
}
