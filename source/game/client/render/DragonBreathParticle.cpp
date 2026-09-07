#include "game/client/render/DragonBreathParticle.hpp"

#include "game/core/ColourTable.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/util/Math.hpp"

DragonBreathParticle::DragonBreathParticle(Level* level, double x, double y, double z,
                                           double xa, double ya, double za)
    : Particle(level, x, y, z, 0.0, 0.0, 0.0)
{
    init(level, x, y, z, xa, ya, za, 1.0f);
}

DragonBreathParticle::~DragonBreathParticle()
{
}

void DragonBreathParticle::init(Level* level, double x, double y, double z,
                                double xa, double ya, double za, float scale)
{
    xd = xa;
    yd = ya;
    zd = za;

    unsigned int start = Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Particle_DragonBreathMin);
    unsigned int end = Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Particle_DragonBreathMax);
    double startRed = ((double)((start >> 16) & 0xff)) / 255.0;
    double startGreen = ((double)((start >> 8) & 0xff)) / 255.0;
    double startBlue = ((double)(start & 0xff)) / 255.0;
    double endRed = ((double)((end >> 16) & 0xff)) / 255.0;
    double endGreen = ((double)((end >> 8) & 0xff)) / 255.0;
    double endBlue = ((double)(end & 0xff)) / 255.0;

    rCol = (float)((endRed - startRed) * Math::random() + startRed);
    gCol = (float)((endGreen - startGreen) * Math::random() + startGreen);
    bCol = (float)((endBlue - startBlue) * Math::random() + startBlue);
    baseAlpha = alpha * 0.75f * scale;
    alpha = baseAlpha;
    lifetime = (int)((double)((float)(20.0 / (random.nextDouble() * 0.8 + 0.2))) * scale);
    hasHitGround = false;
}

void DragonBreathParticle::render(BufferBuilder* buffer,
                                  const boost::shared_ptr<Entity>& camera,
                                  float partialTicks,
                                  float xa,
                                  float ya,
                                  float za,
                                  float xa2,
                                  float za2)
{
    float brightness = Mth::clamp(((float)age + partialTicks) / (float)lifetime * 32.0f,
                                  0.0f, 1.0f);
    alpha = baseAlpha * brightness;
    Particle::render(buffer, camera, partialTicks, xa, ya, za, xa2, za2);
}

void DragonBreathParticle::tick()
{
    xo = x;
    yo = y;
    zo = z;

    int oldAge = age;
    ++age;
    if (lifetime <= oldAge)
    {
        remove();
        return;
    }

    setMiscTex((age * 3) / lifetime + 5);

    if (!onGround)
    {
        if (hasHitGround)
            yd += 0.002;
        move(xd, yd, zd);
    }
    else
    {
        yd = 0.002;
        hasHitGround = true;
        move(xd, yd, zd);
    }

    if (y != yo)
    {
        xd *= 1.1;
        zd *= 1.1;
    }
    xd *= 0.9599999785423279;
    yd *= 0.9599999785423279;
    zd *= 0.9599999785423279;
    if (hasHitGround)
        yd *= 0.9599999785423279;
}

int DragonBreathParticle::getParticleTexture()
{
    return 4;
}

float DragonBreathParticle::getBrightness(float partialTicks)
{
    float progress = (((float)age + partialTicks) / (float)lifetime) * 32.0f;
    float value = Mth::clamp(progress, 0.0f, 1.0f);
    return 0.5f / value + 0.5f;
}

uint64_t DragonBreathParticle::GetType()
{
    return 0x0008021e00000000LL;
}

Particle* DragonBreathParticle::Provider::createParticle(ePARTICLE_TYPE type, Level* level,
                                                          double x, double y, double z,
                                                          double xa, double ya, double za,
                                                          arrayWithLength<int> params)
{
    return new DragonBreathParticle(level, x, y, z, xa, ya, za);
}
