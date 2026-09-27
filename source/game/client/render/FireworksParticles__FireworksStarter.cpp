#include "game/client/render/FireworksParticles__FireworksStarter.hpp"
#include "game/world/Level.hpp"
#include "game/item/DyeItem.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/client/render/FireworksParticles__FireworksOverlayParticle.hpp"
#include "game/core/EntityType.hpp"

#include <cmath>

#include "game/client/render/FireworksParticles__FireworksSparkParticle.hpp"
#include "game/client/render/ParticleEngine.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/Minecraft.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/Random.hpp"

FireworksParticles::FireworksStarter::FireworksStarter(Level* level, double x, double y, double z,
                                   double xa, double ya, double za,
                                   ParticleEngine* particleEngine, CompoundTag* fireworks)
    : Particle(level, x, y, z, xa, ya, za), life(0), particleEngine(particleEngine),
      explosions(NULL), twinkle(false)
{
    lifetime = 8;
    if (fireworks != NULL)
    {
        explosions = fireworks->getList(L"Explosions", Tag::TAG_COMPOUND);
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
    return;
}

void FireworksParticles::FireworksStarter::render(BufferBuilder* buffer, const boost::shared_ptr<Entity>& camera,
                               float partialTicks, float xa, float ya, float za,
                               float xa2, float za2)
{
}

bool FireworksParticles::FireworksStarter::isFarAwayFromCamera()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft != NULL && minecraft->getCameraEntity())
    {
        return !(minecraft->getCameraEntity()->distanceToSqr(x, y, z) < 256.0);
    }
    return true;
}

void FireworksParticles::FireworksStarter::createParticle(double px, double py, double pz,
                                      double xa, double ya, double za,
                                      arrayWithLength<int> colors,
                                      arrayWithLength<int> fadeColors,
                                      bool trail, bool flicker)
{
    FireworksParticles::FireworksSparkParticle* particle =
        new FireworksParticles::FireworksSparkParticle(level, px, py, pz, xa, ya, za, particleEngine);
    particle->setTrail(trail);
    particle->setFlicker(flicker);
    particle->setColor(colors.data[random.nextInt(colors.length)]);
    if (fadeColors.length != 0)
    {
        particle->setFadeColor(fadeColors.data[random.nextInt(fadeColors.length)]);
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
                double dx = random.nextDouble();
                dx = (double)x + (dx - random.nextDouble()) * spread;
                double dy = random.nextDouble();
                dy = (double)y + (dy - random.nextDouble()) * spread;
                double dz = random.nextDouble();
                dz = (double)z + (dz - random.nextDouble()) * spread;
                double distance = std::sqrt(dx * dx + dy * dy + dz * dz);
                double velocity = random.nextGaussian() * 0.05 + distance / scale;
                createParticle(this->x, this->y, this->z,
                               dx / velocity, dy / velocity, dz / velocity,
                               colors, fadeColors, trail, flicker);
                if (x != -radius && x != radius && y != -radius && y != radius)
                    z += radius * 2 - 1;
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
    double firstX = shape.data[0]->data[0];
    double firstY = shape.data[0]->data[1];
    createParticle(x, y, z, firstX * scale, firstY * scale, 0.0,
                   colors, fadeColors, trail, flicker);
    float initialAngle = random.nextFloat() * 3.1415927F;
    double rotationStep = keepShape ? 0.034 : 0.34;
    for (int plane = 0; plane < 3; ++plane)
    {
        double angle = (double)((float)plane * 3.1415927F) * rotationStep + initialAngle;
        double previousX = firstX;
        double previousY = firstY;
        for (unsigned int point = 1; point < shape.length; ++point)
        {
            double nextX = shape.data[point]->data[0];
            double nextY = shape.data[point]->data[1];
            for (double fraction = 0.25; fraction <= 1.0; fraction += 0.25)
            {
                double horizontal = ((nextX - previousX) * fraction + previousX) * scale;
                double vertical = ((nextY - previousY) * fraction + previousY) * scale;
                for (double side = -1.0; side <= 1.0; side += 2.0)
                {
                    createParticle(x, y, z, horizontal * std::cos(angle) * side,
                                   vertical, horizontal * std::sin(angle) * side,
                                   colors, fadeColors, trail, flicker);
                }
            }
            previousX = nextX;
            previousY = nextY;
        }
    }
}

void FireworksParticles::FireworksStarter::createParticleBurst(arrayWithLength<int> colors,
                                           arrayWithLength<int> fadeColors,
                                           bool trail, bool flicker)
{
    double xVelocityBase = random.nextGaussian() * 0.05;
    double zVelocityBase = random.nextGaussian() * 0.05;
    for (int i = 0; i < 70; ++i)
    {
        double xNoise = random.nextGaussian();
        double zNoise = random.nextGaussian();
        double yNoise = random.nextDouble();
        double xVelocity = xd * 0.5 + xNoise * 0.15 + xVelocityBase;
        double yVelocity = yd * 0.5 + yNoise * 0.5;
        double zVelocity = zd * 0.5 + zNoise * 0.15 + zVelocityBase;
        createParticle(x, y, z, xVelocity, yVelocity, zVelocity,
                       colors, fadeColors, trail, flicker);
    }
}

void FireworksParticles::FireworksStarter::tick()
{
    if (life == 0 && explosions != NULL)
    {
        const bool farAway = isFarAwayFromCamera();
        bool large = explosions->size() >= 3;
        for (int i = 0; !large && i < explosions->size(); ++i)
        {
            if (explosions->getCompound(i)->getByte(L"Type") == 1)
                large = true;
        }
        const SoundEvent* blast;
        if (large)
            blast = farAway ? SoundEvents::FIREWORK_LARGE_BLAST_FAR : SoundEvents::FIREWORK_LARGE_BLAST;
        else
            blast = farAway ? SoundEvents::FIREWORK_BLAST_FAR : SoundEvents::FIREWORK_BLAST;
        level->playLocalSound(x, y, z, blast, SoundSource::eSoundSource_Ambient, 20.0f,
                              0.95f + random.nextFloat() * 0.1f, true, 100.0f);
    }

    if (life % 2 == 0 && explosions != NULL && life / 2 < explosions->size())
    {
        CompoundTag* explosion = explosions->getCompound(life / 2);
        const int type = explosion->getByte(L"Type");
        const bool trail = explosion->getBoolean(L"Trail");
        const bool flicker = explosion->getBoolean(L"Flicker");
        arrayWithLength<int> colors = explosion->getIntArray(L"Colors");
        arrayWithLength<int> fadeColors = explosion->getIntArray(L"FadeColors");
        if (colors.length == 0)
        {
            colors = arrayWithLength<int>(1, true);
            colors.data[0] = DyeItem::DYE_COLORS[0];
        }

        if (type == 1)
        {
            createParticleBall(0.5, 4, colors, fadeColors, trail, flicker);
        }
        else if (type == 2)
        {
            static const double STAR_SHAPE[6][2] = {
                { 0.0, 1.0 }, { 0.3455, 0.309 }, { 0.9511, 0.309 },
                { 0.3795918367346939, -0.12653061224489795 }, { 0.6122448979591837, -0.8040816326530612 },
                { 0.0, -0.35918367346938773 } };
            array2DWithLength<double> shape(6, 2);
            for (unsigned int point = 0; point < shape.length; ++point)
            {
                shape.data[point]->data[0] = STAR_SHAPE[point][0];
                shape.data[point]->data[1] = STAR_SHAPE[point][1];
            }
            createParticleShape(0.5, shape, colors, fadeColors, trail, flicker, false);
            for (unsigned int point = 0; point < shape.length; ++point)
                delete[] shape.data[point]->data;
            delete[] shape.data;
        }
        else if (type == 3)
        {
            static const double CREEPER_SHAPE[12][2] = {
                { 0.0, 0.2 }, { 0.2, 0.2 }, { 0.2, 0.6 }, { 0.6, 0.6 }, { 0.6, 0.2 }, { 0.2, 0.2 },
                { 0.2, 0.0 }, { 0.4, 0.0 }, { 0.4, -0.6 }, { 0.2, -0.6 }, { 0.2, -0.4 }, { 0.0, -0.4 } };
            array2DWithLength<double> shape(12, 2);
            for (unsigned int point = 0; point < shape.length; ++point)
            {
                shape.data[point]->data[0] = CREEPER_SHAPE[point][0];
                shape.data[point]->data[1] = CREEPER_SHAPE[point][1];
            }
            createParticleShape(0.5, shape, colors, fadeColors, trail, flicker, true);
            for (unsigned int point = 0; point < shape.length; ++point)
                delete[] shape.data[point]->data;
            delete[] shape.data;
        }
        else if (type == 4)
        {
            createParticleBurst(colors, fadeColors, trail, flicker);
        }
        else
        {
            createParticleBall(0.25, 2, colors, fadeColors, trail, flicker);
        }

        const int colour = colors.data[0];
        FireworksOverlayParticle* overlay = new FireworksOverlayParticle(level, x, y, z);
        overlay->setColor((float)((colour >> 16) & 0xFF) / 255.0f, (float)((colour >> 8) & 0xFF) / 255.0f,
                          (float)(colour & 0xFF) / 255.0f);
        particleEngine->add(overlay);
    }

    ++life;
    if (life > lifetime)
    {
        if (twinkle)
        {
            const bool farAway = isFarAwayFromCamera();
            level->playLocalSound(x, y, z,
                                  farAway ? SoundEvents::FIREWORK_TWINKLE_FAR : SoundEvents::FIREWORK_TWINKLE,
                                  SoundSource::eSoundSource_Ambient, 20.0f, 0.9f + random.nextFloat() * 0.15f,
                                  true, 100.0f);
        }
        remove();
    }
}

int FireworksParticles::FireworksStarter::getParticleTexture()
{
    return 0;
}

uint64_t FireworksParticles::FireworksStarter::GetType()
{
    return ParticleKind::FIREWORKS_STARTER;
}
