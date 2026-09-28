#include "game/client/render/SimpleAnimatedParticle.hpp"

SimpleAnimatedParticle::SimpleAnimatedParticle(Level* level, double x, double y,
                                               double z, int texture,
                                               int frameCount, float verticalSpeed)
    : Particle(level, x, y, z), texture(texture), frameCount(frameCount),
      verticalSpeed(verticalSpeed), baseAirFriction(0.91f), fadeRed(0.0f),
      fadeGreen(0.0f), fadeBlue(0.0f), fading(false)
{
    setMiscTex(texture);
}

void SimpleAnimatedParticle::setColor(int color)
{
    Particle::setColor((float)((color >> 16) & 0xff) / 255.0f,
             (float)((color >> 8) & 0xff) / 255.0f,
             (float)(color & 0xff) / 255.0f);
}

void SimpleAnimatedParticle::setFadeColor(int color)
{
    fadeRed = (float)((color >> 16) & 0xff) / 255.0f;
    fadeGreen = (float)((color >> 8) & 0xff) / 255.0f;
    fadeBlue = (float)(color & 0xff) / 255.0f;
    fading = true;
}

bool SimpleAnimatedParticle::isTransparent()
{
    return true;
}

void SimpleAnimatedParticle::tick()
{
    int oldAge = age;
    int oldLifetime = lifetime;
    ++age;
    xo = x;
    yo = y;
    zo = z;

    if (oldLifetime <= oldAge)
    {
        remove();
        oldLifetime = lifetime;
    }

    int fadeStart = oldLifetime / 2;
    int currentAge = age;
    if (fadeStart < currentAge)
    {
        setAlpha(1.0f - (float)(currentAge - fadeStart) / (float)oldLifetime);
        if (fading)
        {
            rCol += (fadeRed - rCol) * 0.2f;
            gCol += (fadeGreen - gCol) * 0.2f;
            bCol += (fadeBlue - bCol) * 0.2f;
        }
    }

    setMiscTex(texture + (frameCount - 1 - age * frameCount / oldLifetime));
    yd += (double)verticalSpeed;
    move(xd, yd, zd);
    xd *= (double)baseAirFriction;
    yd *= (double)baseAirFriction;
    zd *= (double)baseAirFriction;
    if (onGround)
    {
        xd *= 0.699999988079071;
        zd *= 0.699999988079071;
    }
}

int SimpleAnimatedParticle::getLightColor(float partialTicks)
{
    return 0xf000f0;
}

float SimpleAnimatedParticle::getBrightness(float partialTicks)
{
    return 1.0f;
}

void SimpleAnimatedParticle::setBaseAirFriction(float friction)
{
    baseAirFriction = friction;
}
