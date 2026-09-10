#include "game/client/render/BubbleParticle.hpp"

#include "game/core/Mth.hpp"
#include "game/util/Math.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Material.hpp"

BubbleParticle::BubbleParticle(Level* level,
                               double x,
                               double y,
                               double z,
                               double xa,
                               double ya,
                               double za,
                               arrayWithLength<int> params)
    : Particle(level, x, y, z, xa, ya, za)
{
    if (params.length == 4)
    {
        setColor(params.data[0] / 255.0f,
                 params.data[1] / 255.0f,
                 params.data[2] / 255.0f);
        alpha = params.data[3] / 255.0f;
    }

    setMiscTex(32);
    setSize(0.02f, 0.02f);
    size *= random.nextFloat() * 0.6f + 0.2f;
    xd = xa * 0.2 + (Math::random() * 2.0 - 1.0) * 0.02;
    yd = ya * 0.2 + (Math::random() * 2.0 - 1.0) * 0.02;
    zd = za * 0.2 + (Math::random() * 2.0 - 1.0) * 0.02;
    lifetime = (int)(8.0 / (Math::random() * 0.8 + 0.2));

    BlockPos pos(x, y, z);
    if (level->getBlockState(pos)->getMaterial() == Material::BUBBLE_COLUMN)
    {
        lifetime *= 5;
        int blockId = level->getBlockId((int)x, (int)y + 1, (int)z);
        if (blockId == 9 || blockId == 8)
        {
            lifetime *= 3;
        }
    }
}

BubbleParticle::~BubbleParticle()
{
}

void BubbleParticle::tick()
{
    xo = x;
    yo = y;
    zo = z;

    yd += 0.002;
    if (level->getBlockState(BlockPos(x, y, z))->getMaterial() == Material::BUBBLE_COLUMN)
    {
        yd += 0.003;
    }

    move(xd, yd, zd);

    xd *= 0.85;
    yd *= 0.85;
    zd *= 0.85;

    BlockPos pos(x, y, z);
    const BlockState* state = level->getBlockState(pos);
    Block* block = level->getBlock(pos);
    Block* blockBelow = level->getBlock(pos.below());
    bool survives = true;
    if (!block->ParticlesSurviveWithin()
        && (!blockBelow->ParticlesSurviveWithin() || state->getMaterial() != Material::AIR))
    {
        survives = false;
    }

    Material* material = level->getBlockState(BlockPos(x, y, z))->getMaterial();
    if (material != Material::WATER
        && material != Material::BUBBLE_COLUMN
        && !survives)
    {
        lifetime = 0;
    }

    if (lifetime-- <= 0)
    {
        remove();
    }
}

int BubbleParticle::getRawBr()
{
    int brightness = 15;
    BlockPos pos(x, y, z);
    if (level->hasChunkAt(pos))
    {
        brightness = level->getRawBrightness(pos);
    }

    return Mth::floor(Mth::Lerp(7.5f, 15.0f, (float)brightness / 15.0f));
}

float BubbleParticle::getBrightness(float partialTicks)
{
    return level->mDimension->getBrightnessRamp()[getRawBr()];
}

int BubbleParticle::getLightColor(float partialTicks)
{
    BlockPos pos(x, y, z);
    int key = (pos.z & 0xFF) | ((pos.y & 0xFF) << 8) | ((pos.x & 0xFF) << 16);
    if (key == cachedLightPos)
    {
        return cachedLightColor;
    }

    cachedLightPos = key;
    int brightness = getRawBr();
    cachedLightColor = brightness << 4 | brightness << 20;
    return cachedLightColor;
}

uint64_t BubbleParticle::GetType()
{
    return 0x0000000000080203ULL;
}

Particle* BubbleParticle::Provider::createParticle(ePARTICLE_TYPE type,
                                                   Level* level,
                                                   double x,
                                                   double y,
                                                   double z,
                                                   double xa,
                                                   double ya,
                                                   double za,
                                                   arrayWithLength<int> params)
{
    arrayWithLength<int> localParams;
    localParams.data = params.data;
    localParams.length = params.length;
    return new BubbleParticle(level, x, y, z, xa, ya, za, localParams);
}
