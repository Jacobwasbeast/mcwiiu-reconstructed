#include "game/client/render/FireworksParticles__Provider.hpp"

#include "game/client/render/SimpleAnimatedParticle.hpp"
#include "game/core/Minecraft.hpp"

class FireworksParticles::FireworksSparkParticle : public SimpleAnimatedParticle
{
public:
    FireworksSparkParticle(Level* level, double x, double y, double z,
                           double xa, double ya, double za,
                           ParticleEngine* particleEngine);
    virtual ~FireworksSparkParticle();
    void setTrail(bool trail);
    void setFlicker(bool flicker);
    virtual void render(BufferBuilder* buffer,
                        const boost::shared_ptr<Entity>& camera,
                        float partialTicks,
                        float xa,
                        float ya,
                        float za,
                        float xa2,
                        float za2);
    virtual void tick();
    virtual uint64_t GetType();

private:
    bool trail;
    bool flicker;
    ParticleEngine* particleEngine;
};

Particle* FireworksParticles::Provider::createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z, double xa, double ya, double za, arrayWithLength<int> params)
{
    Particle* particle = new FireworksParticles::FireworksSparkParticle(
        level, x, y, z, xa, ya, za, Minecraft::GetInstance()->mParticleEngine);

    if (params.length == 4)
    {
        particle->setColor(params.data[0] / 255.0f, params.data[1] / 255.0f, params.data[2] / 255.0f);
        particle->setAlpha(params.data[3] / 255.0f);

        delete[] params.data;
    }

    return particle;
}
