#include "game/client/render/DripParticle.hpp"

#include <cmath>

#include "game/core/ColourTable.hpp"
#include "game/core/Minecraft.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Material.hpp"

DripParticle::DripParticle(Level* level, double x, double y, double z, Material* material)
    : Particle(level, x, y, z, 0.0, 0.0, 0.0)
    , material(material)
    , delay(40)
{
    unsigned int colour = Minecraft::GetInstance()->getColourTable()->getColour(
        material == Material::WATER ? (eMinecraftColour)244 : (eMinecraftColour)245);
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
        unsigned int colour = Minecraft::GetInstance()->getColourTable()->getColour((eMinecraftColour)244);
        rCol = (float)((double)((colour >> 16) & 255) / 255.0);
        gCol = (float)((double)((colour >> 8) & 255) / 255.0);
        bCol = (float)((double)(colour & 255) / 255.0);
    }
    else
    {
        unsigned int start = Minecraft::GetInstance()->getColourTable()->getColour((eMinecraftColour)245);
        unsigned int end = Minecraft::GetInstance()->getColourTable()->getColour((eMinecraftColour)246);
        float fraction = (float)(40 - delay) / 40.0f;
        rCol = (float)(((double)((start >> 16) & 255) -
                        ((double)((start >> 16) & 255) - (double)((end >> 16) & 255)) * fraction) / 255.0);
        gCol = (float)(((double)((start >> 8) & 255) -
                        ((double)((start >> 8) & 255) - (double)((end >> 8) & 255)) * fraction) / 255.0);
        bCol = (float)(((double)(start & 255) -
                        ((double)(start & 255) - (double)(end & 255)) * fraction) / 255.0);
    }

    --delay;
    if (delay < 1)
    {
        yd -= gravity;
        if (delay < 0)
            remove();
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

    --lifetime;
    if (lifetime < 1)
        remove();

    if (onGround)
    {
        remove();
        if (material == Material::WATER)
        {
            level->addParticle(ePARTICLE_SPLASH, false, x, y, z, 0.0, 0.0, 0.0,
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
    if ((state->getMaterial()->isLiquid() || state->getMaterial()->isSolid()) &&
        state->getBlock()->isLiquidBlock())
    {
        int data = state->getBlock()->convertBlockStateToLegacyData(state);
        float height = data >= 8 ? 0.0f : (float)(data + 1) / 9.0f;
        double surface = std::floor(y) + 1.0 - (double)height;
        if (y < surface)
            remove();
    }
}

long long DripParticle::GetType()
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
