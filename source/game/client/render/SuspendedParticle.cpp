#include "game/client/render/SuspendedParticle.hpp"

#include "game/core/ColourTable.hpp"
#include "game/core/Minecraft.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"

SuspendedParticle::SuspendedParticle(Level* level, double x, double y, double z,
                                     double xa, double ya, double za)
    : Particle(level, x, y, z, 0.0, 0.0, 0.0)
{
    int colour = Minecraft::GetInstance()->getColourTable()->getColour((eMinecraftColour)250);
    rCol = (float)((colour >> 16) & 0xff) / 255.0f;
    gCol = (float)((colour >> 8) & 0xff) / 255.0f;
    bCol = (float)(colour & 0xff) / 255.0f;
    setMiscTex(0);
    setSize(0.01f, 0.01f);
    float velocityScale = random.nextFloat() * 0.6f + 0.2f;
    xd = xa * velocityScale;
    yd = ya * velocityScale;
    zd = za * velocityScale;
    lifetime = (int)(16.0f / (random.nextDouble() * 0.8 + 0.2));
}

SuspendedParticle::~SuspendedParticle()
{
}

void SuspendedParticle::tick()
{
    xo = x;
    yo = y;
    zo = z;
    move(xd, yd, zd);
    BlockPos position(x, y, z);
    const BlockState* state = ((LevelSource*)level)->getBlockState(position);
    if (state == 0 || state->getBlock()->getRenderShape(state) != BLOCK_RENDER_MODEL)
        remove();
    if (--lifetime <= 0)
        remove();
}

uint64_t SuspendedParticle::GetType()
{
    return 0x0000000000080219ULL;
}

Particle* SuspendedParticle::Provider::createParticle(ePARTICLE_TYPE type,
                                                       Level* level,
                                                       double x,
                                                       double y,
                                                       double z,
                                                       double xa,
                                                       double ya,
                                                       double za,
                                                       arrayWithLength<int> params)
{
    return new SuspendedParticle(level, x, y, z, xa, ya, za);
}
