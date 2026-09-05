#include "game/client/render/WaterDropParticle.hpp"

#include <cmath>

#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/LiquidBlock.hpp"
#include "game/world/block/Material.hpp"

WaterDropParticle::WaterDropParticle(Level* level, double x, double y, double z)
    : Particle(level, x, y, z, 1000.0, 1000.0, 1000.0)
{
    xd *= 0.3;
    yd = random.nextDouble() * 0.2 + 0.1;
    zd *= 0.3;
    rCol = 1.0f;
    gCol = 1.0f;
    bCol = 1.0f;
    setMiscTex(19);
    setSize(0.01f, 0.01f);
    gravity = 0.06f;
    lifetime = (int)(8.0 / (random.nextDouble() * 0.8 + 0.2));
}

WaterDropParticle::~WaterDropParticle()
{
}

void WaterDropParticle::tick()
{
    xo = x;
    yo = y;
    zo = z;
    yd -= gravity;
    move(xd, yd, zd);
    xd *= 0.98;
    yd *= 0.98;
    zd *= 0.98;
    --lifetime;
    if (lifetime < 1)
        remove();

    if (onGround)
    {
        if (random.nextDouble() < 0.5)
            remove();
        xd *= 0.7;
        zd *= 0.7;
    }

    BlockPos position(x, y, z);
    const BlockState* state = level->getBlockState(position);
    if ((state->getMaterial()->isLiquid() || state->getMaterial()->isSolid()) &&
        state->getBlock()->isLiquidBlock())
    {
        double height = LiquidBlock::getHeightFromData(
            state->getBlock()->convertBlockStateToLegacyData(state));
        double surface = std::floor(y) + 1.0 - height;
        if (y < surface)
            remove();
    }
}

long long WaterDropParticle::GetType()
{
    return 0x0008021200000000LL;
}

Particle* WaterDropParticle::Provider::createParticle(ePARTICLE_TYPE type, Level* level,
                                                      double x, double y, double z,
                                                      double xa, double ya, double za,
                                                      arrayWithLength<int> params)
{
    return new WaterDropParticle(level, x, y, z);
}
