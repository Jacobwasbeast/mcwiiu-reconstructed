#include "game/client/render/FallingDustParticle.hpp"

#include <cmath>

#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockColors.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"

FallingDustParticle::FallingDustParticle(Level* level,
                                         double x,
                                         double y,
                                         double z,
                                         float red,
                                         float green,
                                         float blue)
    : Particle(level, x, y, z, 0.0, 0.0, 0.0), m_size(0.0f), m_gravity(0.0f)
{
    xd = 0.0;
    yd = 0.0;
    zd = 0.0;
    m_gravity = (random.nextFloat() - 0.5f) * 0.1f;
    rCol = red;
    gCol = green;
    bCol = blue;
    m_size = Particle::size * 0.75f * 0.9f;
    Particle::size = m_size;
    lifetime = (int)std::floor(32.0f / (random.nextFloat() * 0.8f + 0.2f) * 0.9f);
    uo = random.nextFloat() * 6.2831855f;
}

FallingDustParticle::~FallingDustParticle()
{
}

void FallingDustParticle::render(BufferBuilder* buffer,
                                 const boost::shared_ptr<Entity>& camera,
                                 float partialTicks,
                                 float xa,
                                 float ya,
                                 float za,
                                 float xa2,
                                 float za2)
{
    float progress = ((float)age + partialTicks) / (float)lifetime;
    float alphaScale = Mth::clamp(progress * 32.0f, 0.0f, 1.75f);
    Particle::size = m_size * alphaScale;
    Particle::render(buffer, camera, partialTicks, xa, ya, za, xa2, za2);
}

void FallingDustParticle::tick()
{
    int oldAge = age;
    int particleLifetime = lifetime;
    ++age;
    xo = x;
    yo = y;
    zo = z;
    if (oldAge >= particleLifetime)
        remove();

    xd = yd + m_gravity * 3.1415927f * 2.0f;
    if (onGround)
    {
        xd = 0.5f;
        yd = 0.5f;
    }
    setMiscTex(7 - (age << 3) / particleLifetime);
    move(xd, yd, zd);
    yd -= 0.003;
    if (yd < -0.14)
        yd = -0.14;
}

uint64_t FallingDustParticle::GetType()
{
    return PARTICLE_TYPE_FALLING_DUST;
}

Particle* FallingDustParticle::Provider::createParticle(ePARTICLE_TYPE type,
                                                         Level* level,
                                                         double x,
                                                         double y,
                                                         double z,
                                                         double xa,
                                                         double ya,
                                                         double za,
                                                         arrayWithLength<int> params)
{
    if (params.data == 0)
        return 0;

    const BlockState* state = Block::stateByIdWithData(params.data[0]);
    if (state == 0)
        return 0;

    Block* block = state->getBlock();
    BlockPos position(x, y, z);
    int colour = Minecraft::GetInstance()->getBlockColors()->getColor(state, (LevelSource*)level, position);
    if (block->instanceof((EBlockType)11))
        colour = block->getColor(state);
    float red = (float)((colour >> 16) & 0xff) / 255.0f;
    float green = (float)((colour >> 8) & 0xff) / 255.0f;
    float blue = (float)(colour & 0xff) / 255.0f;
    return new FallingDustParticle(level, x, y, z, red, green, blue);
}
