#include "game/client/render/PlayerCloudParticle.hpp"

#include <cmath>

#include "game/core/Mth.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/world/Level.hpp"

PlayerCloudParticle::PlayerCloudParticle(Level* level,
                                         double x,
                                         double y,
                                         double z,
                                         double xa,
                                         double ya,
                                         double za)
    : Particle(level, x, y, z, 0.0, 0.0, 0.0)
{
    xd = xd * 0.1 + xa;
    yd = yd * 0.1 + ya;
    zd = zd * 0.1 + za;
    rCol = 1.0f - random.nextFloat() * 0.3f;
    gCol = rCol;
    bCol = rCol;
    alpha *= 6.0f;
    size = alpha;
    lifetime = (int)std::floor(8.0 / (random.nextDouble() * 0.8 + 0.3) * 2.5);
}

PlayerCloudParticle::~PlayerCloudParticle()
{
}

void PlayerCloudParticle::render(BufferBuilder* buffer,
                                 const boost::shared_ptr<Entity>& camera,
                                 float partialTicks,
                                 float xa,
                                 float ya,
                                 float za,
                                 float xa2,
                                 float za2)
{
    float progress = ((float)age + partialTicks) / (float)lifetime;
    alpha = size * Mth::clamp(progress * 32.0f, 0.0f, 1.0f);
    Particle::render(buffer, camera, partialTicks, xa, ya, za, xa2, za2);
}

void PlayerCloudParticle::tick()
{
    xo = x;
    yo = y;
    zo = z;
    ++age;

    if (age >= lifetime)
    {
        remove();
    }

    setMiscTex(7 - (age << 3) / lifetime);
    move(xd, yd, zd);

    xd *= 0.96;
    yd *= 0.96;
    zd *= 0.96;

    boost::shared_ptr<Entity> nearestPlayer = level->getNearestPlayer(x, y, z, 2.0, false, 2.0);
    if (nearestPlayer)
    {
        double playerY = nearestPlayer->mBoundingBox->mMinY;
        if (playerY < y)
        {
            y = (playerY - y) * 0.5 + y;
            yd = (nearestPlayer->mY - yd) * 0.5 + yd;
            setPos(x, y, z);
        }
    }

    if (onGround)
    {
        xd *= 0.7;
        zd *= 0.7;
    }
}

long long PlayerCloudParticle::GetType()
{
    return 0x0008021800000000LL;
}

Particle* PlayerCloudParticle::Provider::createParticle(ePARTICLE_TYPE type,
                                                         Level* level,
                                                         double x,
                                                         double y,
                                                         double z,
                                                         double xa,
                                                         double ya,
                                                         double za,
                                                         arrayWithLength<int> params)
{
    return new PlayerCloudParticle(level, x, y, z, xa, ya, za);
}
