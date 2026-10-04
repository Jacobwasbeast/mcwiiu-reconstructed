#include "game/client/render/InkParticle.hpp"
#include "game/world/block/Material.hpp"
#include "game/core/EntityType.hpp"

#include "game/client/render/TextureUVCoordinateSet.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/util/Math.hpp"
#include "game/core/Mth.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"

InkParticle::InkParticle(Level* level, double x, double y, double z,
                         double xa, double ya, double za, arrayWithLength<int> params)
    : Particle(level, x, y, z, xa, ya, za)
{
    float scale = 1.0f;
    mScale = scale;
    if (params.length == 1)
        scale = (float)params.data[0] / 100.0f;

    xd = xd * 0.1 + xa;
    yd = yd * 0.1 + ya;
    zd = zd * 0.1 + za;
    float randomScale = (float)(Math::random() * 0.05);
    rCol = randomScale;
    gCol = randomScale;
    bCol = randomScale;
    float adjustedSize = size * 0.75f * scale;
    size = adjustedSize;
    mStartSize = adjustedSize * 0.5f;
    mEndSize = adjustedSize;
    lifetime = (int)(12.0f * scale / ((float)Math::random() * 0.8f + 0.2f));
}

void InkParticle::render(BufferBuilder* buffer, const boost::shared_ptr<Entity>& camera,
                         float partialTicks, float xa, float ya, float za,
                         float xa2, float za2)
{
    float growth = Mth::clamp(((float)age + partialTicks) / (float)lifetime * 16.0f, 0.0f, 1.0f);
    size = (mEndSize - mStartSize) * growth + mStartSize;
    uvs = TextureUVCoordinateSet::fromOldSystem(8 - (age * 8) / lifetime);
    Particle::render(buffer, camera, partialTicks, xa, ya, za, xa2, za2);
}

void InkParticle::tick()
{
    xo = x;
    yo = y;
    zo = z;
    move(xd, yd, zd);
    if (((LevelSource*)level)->getBlockState(BlockPos(x, y, z))->getMaterial() == Material::AIR)
        yd -= 0.008;
    if (y == yo)
    {
        xd *= 1.1;
        zd *= 1.1;
    }
    xd *= 0.92;
    yd *= 0.92;
    zd *= 0.92;
    if (onGround)
    {
        xd *= 0.7;
        zd *= 0.7;
    }
    if (age++ >= lifetime)
        remove();
}

uint64_t InkParticle::GetType()
{
    return ParticleKind::INK;
}

Particle* InkParticle::Provider::createParticle(ePARTICLE_TYPE type, Level* level,
                                                double x, double y, double z,
                                                double xa, double ya, double za,
                                                arrayWithLength<int> params)
{
    arrayWithLength<int> localParams;
    localParams.data = params.data;
    localParams.length = params.length;
    return new InkParticle(level, x, y, z, xa, ya, za, localParams);
}
