#include "game/client/render/FireworksParticles__Provider.hpp"

class Minecraft;

Particle* FireworksParticles::Provider::createParticle(ePARTICLE_TYPE type, Level* level, double x, double y, double z, double xa, double ya, double za, arrayWithLength<int> params)
{
    Particle* particle = new FireworksSparkParticle(level, x, y, z, xa, ya, za, Minecraft::GetInstance()->m_pParticleEngine);

    if (params.length == 4)
    {
        particle->setColor(params.data[0] / 255.0f, params.data[1] / 255.0f, params.data[2] / 255.0f);
        particle->setAlpha(params.data[3] / 255.0f);

        delete[] params.data;
    }

    return particle;
}
