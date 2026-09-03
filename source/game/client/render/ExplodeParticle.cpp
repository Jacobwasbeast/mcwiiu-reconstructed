#include "game/client/render/ExplodeParticle.hpp"

#include "game/core/BufferBuilder.hpp"
#include "game/core/ColourTable.hpp"
#include "game/util/Math.hpp"

#include "game/core/Minecraft.hpp"

ExplodeParticle::ExplodeParticle(Level* level,
                                 double x,
                                 double y,
                                 double z,
                                 double xa,
                                 double ya,
                                 double za)
    : Particle(level, x, y, z, xa, ya, za)
{
    xd = xa + (float)(Math::random() * 2.0 - 1.0) * 0.05f;
    yd = ya + (float)(Math::random() * 2.0 - 1.0) * 0.05f;
    zd = za + (float)(Math::random() * 2.0 - 1.0) * 0.05f;

    unsigned int colour =
        Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Particle_Explode);

    float red = (float)((colour >> 16) & 0xFF) / 255.0f;
    float green = (float)((colour >> 8) & 0xFF) / 255.0f;
    float blue = (float)(colour & 0xFF) / 255.0f;

    float brightness = random.nextFloat() * 0.3f + 0.7f;

    rCol = red * brightness;
    gCol = green * brightness;
    bCol = blue * brightness;

    size = random.nextFloat() * random.nextFloat() * 6.0f + 1.0f;
    lifetime = (int)(16.0f / (random.nextFloat() * 0.8f + 0.2f)) + 2;
}

ExplodeParticle::~ExplodeParticle()
{
}

long long ExplodeParticle::GetType()
{
    return PARTICLE_TYPE_EXPLODE;
}

void ExplodeParticle::tick()
{
    xo = x;
    yo = y;
    zo = z;

    if (age++ >= lifetime)
    {
        remove();
    }

    setMiscTex(7 - age * 8 / lifetime);

    yd += 0.004;

    move(xd, yd, zd);

    xd *= 0.9;
    yd *= 0.9;
    zd *= 0.9;

    if (onGround)
    {
        xd *= 0.7;
        zd *= 0.7;
    }
}

void ExplodeParticle::render(BufferBuilder* buffer,
                             const boost::shared_ptr<Entity>& camera,
                             float partialTicks,
                             float xa,
                             float ya,
                             float za,
                             float xa2,
                             float za2)
{
    float px = (float)(xo + (x - xo) * partialTicks - Particle::xOff);
    float py = (float)(yo + (y - yo) * partialTicks - Particle::yOff);
    float pz = (float)(zo + (z - zo) * partialTicks - Particle::zOff);

    if (px * px + py * py + pz * pz >= 9.0f)
    {
        Particle::render(buffer, camera, partialTicks, xa, ya, za, xa2, za2);
    }
}

Particle* ExplodeParticle::Provider::createParticle(ePARTICLE_TYPE type,
                                                    Level* level,
                                                    double x,
                                                    double y,
                                                    double z,
                                                    double xa,
                                                    double ya,
                                                    double za,
                                                    arrayWithLength<int> params)
{
    return new ExplodeParticle(level, x, y, z, xa, ya, za);
}
