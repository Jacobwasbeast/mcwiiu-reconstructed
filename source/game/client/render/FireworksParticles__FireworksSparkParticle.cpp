#include "game/client/render/FireworksParticles__FireworksSparkParticle.hpp"

#include "game/client/render/ParticleEngine.hpp"
#include "game/core/BufferBuilder.hpp"

FireworksParticles::FireworksSparkParticle::FireworksSparkParticle(
    Level* level, double x, double y, double z, double xa, double ya,
    double za, ParticleEngine* particleEngine)
    : SimpleAnimatedParticle(level, x, y, z, 160, 8, -0.004f),
      trail(false), flicker(false), particleEngine(particleEngine)
{
    xd = xa;
    yd = ya;
    zd = za;
    size *= 0.75f;
    setAlpha(0.9900000095367432f);
    lifetime = random.nextInt(12) + 48;
}

FireworksParticles::FireworksSparkParticle::~FireworksSparkParticle()
{
    return;
}

void FireworksParticles::FireworksSparkParticle::setTrail(bool trail)
{
    this->trail = trail;
}

void FireworksParticles::FireworksSparkParticle::setFlicker(bool flicker)
{
    this->flicker = flicker;
}

void FireworksParticles::FireworksSparkParticle::render(
    BufferBuilder* buffer, const boost::shared_ptr<Entity>& camera,
    float partialTicks, float xa, float ya, float za, float xa2, float za2)
{
    if (!flicker || age < lifetime / 3 || ((age + lifetime) / 3 & 1) == 0)
    {
        float px = (float)((x - xo) * partialTicks + xo);
        float py = (float)((y - yo) * partialTicks + yo);
        float pz = (float)((z - zo) * partialTicks + zo);

        if (2.0f <= px * px + py * py + pz * pz)
        {
            Particle::render(buffer, camera, partialTicks, xa, ya, za, xa2, za2);
        }
    }
}

void FireworksParticles::FireworksSparkParticle::tick()
{
    SimpleAnimatedParticle::tick();

    if (trail && age < lifetime / 2 && (age + lifetime) % 10 == 0)
    {
        FireworksParticles::FireworksSparkParticle* particle =
            new FireworksParticles::FireworksSparkParticle(
            level, x, y, z, 0.0, 0.0, 0.0, particleEngine);
        particle->Particle::setColor(rCol, gCol, bCol);
        particle->age = particle->lifetime / 2;
        particle->setFlicker(flicker);
        particleEngine->add(particle);
    }
}

uint64_t FireworksParticles::FireworksSparkParticle::GetType()
{
    return 0x0000000000080221ULL;
}
