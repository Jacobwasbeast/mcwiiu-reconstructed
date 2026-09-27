#include "game/client/render/FireworksParticles__FireworksOverlayParticle.hpp"
#include "game/core/EntityType.hpp"

#include <cmath>

#include "game/core/BufferBuilder.hpp"

namespace FireworksParticles
{
FireworksOverlayParticle::FireworksOverlayParticle(Level* level, double x, double y, double z)
    : Particle(level, x, y, z)
{
    setAlpha(0.9900000095367432f);
    setLifetime(4);
}

FireworksOverlayParticle::~FireworksOverlayParticle()
{
}

void FireworksOverlayParticle::render(BufferBuilder* buffer,
                                      const boost::shared_ptr<Entity>& camera,
                                      float partialTicks,
                                      float xa,
                                      float ya,
                                      float za,
                                      float xa2,
                                      float za2)
{
    (void)camera;
    float phase = (age + partialTicks - 1.0f) * 0.25f;
    float sine = (float)std::sin(phase * 3.1415927410125732f);
    float scale = 7.099999904632568f * sine;
    setAlpha(0.6000000238418579f - phase * 0.5f);

    float px = (float)((x - xo) * partialTicks + xo - xOff);
    float py = (float)((y - yo) * partialTicks + yo - yOff);
    float pz = (float)((z - zo) * partialTicks + zo - zOff);

    if (3.0f <= px * px + py * py + pz * pz)
    {
        int texture = getParticleTexture();
        buffer->color(rCol, gCol, bCol, getAlpha());
        buffer->tex2(texture);

        float x0 = py - xa * scale;
        float y0 = px - ya * scale;
        float z0 = pz - za * scale;
        float x1 = xa2 * scale;
        float y1 = za2 * scale;

        buffer->vertexUV(x0 - x1, y0, z0 - y1, 0.5f, 0.375f);
        buffer->vertexUV(x0 + x1, y0 + 2.0f * ya * scale, z0 + y1, 0.5f, 0.125f);
        x0 = py + xa * scale;
        z0 = pz + za * scale;
        buffer->vertexUV(x0 + x1, y0 + 2.0f * ya * scale, z0 + y1, 0.25f, 0.125f);
        buffer->vertexUV(x0 - x1, y0, z0 - y1, 0.25f, 0.375f);
    }
}

uint64_t FireworksOverlayParticle::GetType()
{
    return ParticleKind::FIREWORKS_OVERLAY;
}
}
