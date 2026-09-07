#include "game/client/render/DripParticle.hpp"

#include <cmath>

#include "game/core/ColourTable.hpp"
#include "game/core/Minecraft.hpp"
#include "game/world/block/AbstractProperty.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/LiquidBlock.hpp"
#include "game/world/block/Material.hpp"

DripParticle::DripParticle(Level* level, double x, double y, double z, Material* material)
    : Particle(level, x, y, z, 0.0, 0.0, 0.0)
    , material(material)
    , delay(40)
{
    unsigned int colour = Minecraft::GetInstance()->getColourTable()->getColour(
        material == Material::WATER ? eMinecraftColour_Particle_DripWater
                                    : eMinecraftColour_Particle_DripLavaStart);
    rCol = (float)((double)((colour >> 16) & 255) / 255.0);
    gCol = (float)((double)((colour >> 8) & 255) / 255.0);
    bCol = (float)((double)(colour & 255) / 255.0);
    setMiscTex(113);
    setSize(0.01f, 0.01f);
    gravity = 0.06f;
    lifetime = (int)(64.0 / (random.nextDouble() * 0.8 + 0.2));
    xd = 0.0;
    yd = 0.0;
    zd = 0.0;
}

DripParticle::~DripParticle()
{
}

int DripParticle::getLightColor(float partialTicks)
{
    if (material != Material::WATER)
        return 0xf000f0;
    return Particle::getLightColor(partialTicks);
}

float DripParticle::getBrightness(float partialTicks)
{
    if (material == Material::WATER)
        return Particle::getBrightness(partialTicks);
    return 10.0f;
}

void DripParticle::tick()
{
    xo = x;
    yo = y;
    zo = z;

    if (material == Material::WATER)
    {
        unsigned int colour = Minecraft::GetInstance()->getColourTable()->getColour(
            eMinecraftColour_Particle_DripWater);
        rCol = (float)((double)((colour >> 16) & 255) / 255.0);
        gCol = (float)((double)((colour >> 8) & 255) / 255.0);
        bCol = (float)((double)(colour & 255) / 255.0);
    }
    else
    {
        unsigned int start = Minecraft::GetInstance()->getColourTable()->getColour(
            eMinecraftColour_Particle_DripLavaStart);
        unsigned int end = Minecraft::GetInstance()->getColourTable()->getColour(
            eMinecraftColour_Particle_DripLavaEnd);
        float fraction = (float)(40 - delay) / 40.0f;
        rCol = (float)(((double)((start >> 16) & 255) -
                        ((double)((start >> 16) & 255) - (double)((end >> 16) & 255)) * fraction) / 255.0);
        gCol = (float)(((double)((start >> 8) & 255) -
                        ((double)((start >> 8) & 255) - (double)((end >> 8) & 255)) * fraction) / 255.0);
        bCol = (float)(((double)(start & 255) -
                        ((double)(start & 255) - (double)(end & 255)) * fraction) / 255.0);
    }

    int currentDelay = delay--;
    yd -= gravity;
    if (currentDelay < 1)
    {
        setMiscTex(112);
        move(xd, yd, zd);
        xd *= 0.98;
        yd *= 0.98;
        zd *= 0.98;
    }
    else
    {
        xd *= 0.02;
        yd *= 0.02;
        zd *= 0.02;
        setMiscTex(113);
        move(xd, yd, zd);
        xd *= 0.98;
        yd *= 0.98;
        zd *= 0.98;
    }

    int currentLifetime = lifetime--;
    if (currentLifetime < 1)
        remove();

    if (onGround)
    {
        if (material == Material::WATER)
        {
            remove();
            level->addParticle(PARTICLE_SPLASH, x, y, z, 0.0, 0.0, 0.0,
                               arrayWithLength<int>());
            xd *= 0.7;
            zd *= 0.7;
        }
        else
        {
            setMiscTex(114);
            xd *= 0.7;
            zd *= 0.7;
        }
    }

    BlockPos position(x, y, z);
    const BlockState* state = level->getBlockState(position);
    if (state->getMaterial()->isLiquid() || state->getMaterial()->isSolid())
    {
        double height = 0.0;
        if (state->getBlock()->isLiquidBlock())
        {
            int data = state->getValue(LiquidBlock::LEVEL);
            height = LiquidBlock::getHeightFromData(data);
        }
        double surface = std::floor(y) + 1.0 - height;
        if (y < surface)
            remove();
    }
}

uint64_t DripParticle::GetType()
{
    return 0x0008021b00000000LL;
}

Particle* DripParticle::LavaProvider::createParticle(ePARTICLE_TYPE type, Level* level,
                                                     double x, double y, double z,
                                                     double xa, double ya, double za,
                                                     arrayWithLength<int> params)
{
    return new DripParticle(level, x, y, z, Material::LAVA);
}

Particle* DripParticle::WaterProvider::createParticle(ePARTICLE_TYPE type, Level* level,
                                                      double x, double y, double z,
                                                      double xa, double ya, double za,
                                                      arrayWithLength<int> params)
{
    return new DripParticle(level, x, y, z, Material::WATER);
}
