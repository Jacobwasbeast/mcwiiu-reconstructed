#include "game/client/render/GuiParticle.hpp"

void GuiParticle::preTick()
{
    xo = x;
    extra0o = extra0;
    extra1o = extra1;
    yo = y;
    zo = z;
    alphao = alpha;
}

void GuiParticle::remove()
{
    removed = true;
}

void GuiParticle::tick(GuiParticles*)
{
    ++age;
    x += xd;
    y += yd;
    xd *= gravity;
    yd = yd * gravity + 0.1;

    if (age > lifetime)
    {
        remove();
    }

    double progress = (2.0 - 2.0 * ((double)age / (double)lifetime));
    if (progress > 1.0)
    {
        progress = 1.0;
    }
    alpha = progress * progress * 0.5;
}
