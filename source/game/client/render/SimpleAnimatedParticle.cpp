#include "game/client/render/SimpleAnimatedParticle.hpp"

SimpleAnimatedParticle::SimpleAnimatedParticle(Level* level,
                                               double x,
                                               double y,
                                               double z,
                                               int baseTex,
                                               int numTex,
                                               float baseGravity)
    : Particle(level, x, y, z)
{
    this->baseTex = baseTex;
    this->numTex = numTex;
    this->baseGravity = baseGravity;
    this->baseAirFriction = 0.91f;
    this->fadeR = 0.0f;
    this->fadeG = 0.0f;
    this->fadeB = 0.0f;
    this->hasFade = false;

    setMiscTex(baseTex);
}

SimpleAnimatedParticle::~SimpleAnimatedParticle()
{
}

void SimpleAnimatedParticle::setColor(int color)
{
    Particle::setColor((float)((color >> 16) & 0xFF) / 255.0f,
                       (float)((color >> 8) & 0xFF) / 255.0f,
                       (float)(color & 0xFF) / 255.0f);
}

void SimpleAnimatedParticle::setFadeColor(int color)
{
    fadeR = (float)((color >> 16) & 0xFF) / 255.0f;
    fadeG = (float)((color >> 8) & 0xFF) / 255.0f;
    fadeB = (float)(color & 0xFF) / 255.0f;
    hasFade = true;
}

bool SimpleAnimatedParticle::isTransparent()
{
    return true;
}

void SimpleAnimatedParticle::tick()
{
    xo = x;
    yo = y;
    zo = z;

    if (age++ >= lifetime)
    {
        remove();
    }

    if (age > lifetime / 2)
    {
        setAlpha(1.0f - ((float)age - (float)(lifetime / 2)) / (float)lifetime);

        if (hasFade)
        {
            rCol += (fadeR - rCol) * 0.2f;
            gCol += (fadeG - gCol) * 0.2f;
            bCol += (fadeB - bCol) * 0.2f;
        }
    }

    setMiscTex(baseTex + ((numTex - 1) - age * numTex / lifetime));

    yd += (double)baseGravity;

    move(xd, yd, zd);

    xd *= (double)baseAirFriction;
    yd *= (double)baseAirFriction;
    zd *= (double)baseAirFriction;

    if (onGround)
    {
        xd *= 0.7f;
        zd *= 0.7f;
    }
}

int SimpleAnimatedParticle::getLightColor(float partialTicks)
{
    return 0xF000F0;
}

float SimpleAnimatedParticle::getBrightness(float partialTicks)
{
    return 1.0f;
}

void SimpleAnimatedParticle::setBaseAirFriction(float airFriction)
{
    baseAirFriction = airFriction;
}
