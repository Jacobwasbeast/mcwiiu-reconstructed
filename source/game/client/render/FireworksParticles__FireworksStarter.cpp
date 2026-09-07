#include "game/client/render/FireworksParticles__FireworksStarter.hpp"

#include <cmath>

#include "game/client/render/ParticleEngine.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/util/Random.hpp"

class StarterSparkParticle : public Particle
{
public:
    StarterSparkParticle(Level* level, double x, double y, double z,
                         double xa, double ya, double za)
        : Particle(level, x, y, z, xa, ya, za), trail(false), flicker(false)
    {
        setAlpha(0.9900000095367432f);
        lifetime = random.nextInt(12) + 48;
    }

    virtual uint64_t GetType()
    {
        return 0x8022100000000LL;
    }

    void setColorValue(int color)
    {
        setColor((float)((color >> 16) & 255) / 255.0f,
                 (float)((color >> 8) & 255) / 255.0f,
                 (float)(color & 255) / 255.0f);
    }

    void setTrail(bool value)
    {
        trail = value;
    }

    void setFlicker(bool value)
    {
        flicker = value;
    }

private:
    bool trail;
    bool flicker;
};

FireworksParticles::FireworksStarter::FireworksStarter(Level* level, double x, double y, double z,
                                   double xa, double ya, double za,
                                   ParticleEngine* particleEngine, CompoundTag* fireworks)
    : Particle(level, x, y, z, xa, ya, za), life(0), particleEngine(particleEngine),
      explosions(NULL), twinkle(false)
{
    lifetime = 8;
    if (fireworks != NULL)
    {
        explosions = fireworks->getList(L"Explosions", 10);
        if (explosions != NULL && explosions->size() > 0)
        {
            lifetime = explosions->size() * 2 - 1;
            for (int i = 0; i < explosions->size(); ++i)
            {
                if (explosions->getCompound(i)->getBoolean(L"Flicker"))
                {
                    twinkle = true;
                    lifetime += 15;
                    break;
                }
            }
        }
        else
        {
            explosions = NULL;
        }
    }
}

FireworksParticles::FireworksStarter::~FireworksStarter()
{
}

void FireworksParticles::FireworksStarter::render(BufferBuilder* buffer, const boost::shared_ptr<Entity>& camera,
                               float partialTicks, float xa, float ya, float za,
                               float xa2, float za2)
{
}

bool FireworksParticles::FireworksStarter::isFarAwayFromCamera()
{
    return false;
}

void FireworksParticles::FireworksStarter::createParticle(double px, double py, double pz,
                                      double xa, double ya, double za,
                                      arrayWithLength<int> colors,
                                      arrayWithLength<int> fadeColors,
                                      bool trail, bool flicker)
{
    StarterSparkParticle* particle = new StarterSparkParticle(level, px, py, pz, xa, ya, za);
    particle->setTrail(trail);
    particle->setFlicker(flicker);
    particle->setColorValue(colors.data[random.nextInt(colors.length)]);
    if (fadeColors.length != 0)
    {
        (void)fadeColors.data[random.nextInt(fadeColors.length)];
    }
    particleEngine->add(particle);
}

void FireworksParticles::FireworksStarter::createParticleBall(double scale, int radius,
                                          arrayWithLength<int> colors,
                                          arrayWithLength<int> fadeColors,
                                          bool trail, bool flicker)
{
    const double spread = 0.5;
    for (int x = -radius; x <= radius; ++x)
    {
        for (int y = -radius; y <= radius; ++y)
        {
            for (int z = -radius; z <= radius; ++z)
            {
                if (x * x + y * y + z * z <= radius * radius)
                {
                    double dx = (double)x + random.nextDouble() * spread - random.nextDouble() * spread;
                    double dy = (double)y + random.nextDouble() * spread - random.nextDouble() * spread;
                    double dz = (double)z + random.nextDouble() * spread - random.nextDouble() * spread;
                    double distance = std::sqrt(dx * dx + dy * dy + dz * dz);
                    double velocity = random.nextGaussian() * 0.05 + distance / scale;
                    createParticle(this->x, this->y, this->z,
                                   dx / velocity, dy / velocity, dz / velocity,
                                   colors, fadeColors, trail, flicker);
                }
            }
        }
    }
}

void FireworksParticles::FireworksStarter::createParticleShape(double scale,
                                           array2DWithLength<double> shape,
                                           arrayWithLength<int> colors,
                                           arrayWithLength<int> fadeColors,
                                           bool trail, bool flicker, bool keepShape)
{
    if (shape.length == 0)
    {
        return;
    }
    for (unsigned int i = 0; i < shape.length; ++i)
    {
        if (shape.data[i] == NULL || shape.data[i]->length < 2)
        {
            continue;
        }
        double x0 = shape.data[i]->data[0];
        double y0 = shape.data[i]->data[1];
        for (unsigned int j = 1; j < shape.data[i]->length; ++j)
        {
            double x1 = shape.data[i]->data[j * 2];
            double y1 = shape.data[i]->data[j * 2 + 1];
            double dx = (x1 - x0) * scale;
            double dy = (y1 - y0) * scale;
            double distance = std::sqrt(dx * dx + dy * dy);
            int steps = (int)(distance / 0.25) + 1;
            for (int k = 0; k < steps; ++k)
            {
                double t = (double)k / (double)steps;
                double angle = ((double)i + random.nextFloat() * 0.25) * 0.5;
                double velocity = keepShape ? 1.0 : 0.95;
                createParticle(this->x, this->y, this->z,
                               (x0 + (x1 - x0) * t) * velocity * std::cos(angle),
                               (y0 + (y1 - y0) * t) * velocity,
                               (x0 + (x1 - x0) * t) * velocity * std::sin(angle),
                               colors, fadeColors, trail, flicker);
            }
            x0 = x1;
            y0 = y1;
        }
    }
}

void FireworksParticles::FireworksStarter::createParticleBurst(arrayWithLength<int> colors,
                                           arrayWithLength<int> fadeColors,
                                           bool trail, bool flicker)
{
    for (int i = 0; i < 14; ++i)
    {
        double xVelocity = random.nextGaussian() * 0.05;
        double yVelocity = random.nextGaussian() * 0.05;
        double zVelocity = random.nextGaussian() * 0.05;
        createParticle(x, y, z, xVelocity, yVelocity, zVelocity,
                       colors, fadeColors, trail, flicker);
    }
}

void FireworksParticles::FireworksStarter::tick()
{
    if (life == 0 && explosions != NULL)
    {
        for (int i = 0; i < explosions->size(); ++i)
        {
            CompoundTag* explosion = explosions->getCompound(i);
            arrayWithLength<int> colors = explosion->getIntArray(L"Colors");
            arrayWithLength<int> fadeColors = explosion->getIntArray(L"FadeColors");
            int type = explosion->getByte(L"Type");
            bool trail = explosion->getBoolean(L"Trail");
            bool flicker = explosion->getBoolean(L"Flicker");
            if (type == 1)
            {
                createParticleBall(0.5, 4, colors, fadeColors, trail, flicker);
            }
            else if (type == 2)
            {
                createParticleBurst(colors, fadeColors, trail, flicker);
            }
            else
            {
                createParticleBall(0.5, 2, colors, fadeColors, trail, flicker);
            }
            delete[] colors.data;
            delete[] fadeColors.data;
        }
    }
    ++life;
    if (life > lifetime)
    {
        remove();
    }
}

int FireworksParticles::FireworksStarter::getParticleTexture()
{
    return 0;
}

uint64_t FireworksParticles::FireworksStarter::GetType()
{
    return 0x8022000000000LL;
}
