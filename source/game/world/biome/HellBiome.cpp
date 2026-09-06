#include "game/world/biome/HellBiome.hpp"

#include "game/core/ColourTable.hpp"
#include "game/core/Minecraft.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/biome/BiomeDecorator.hpp"
#include "game/world/biome/HellBiomeDecorator.hpp"

HellBiome::HellBiome(EBiomeIDs biomeId, BiomeProperties* properties)
    : Biome(biomeId, eBiomeType_Hell, properties)
{
    mMonsters.push_back(new MobSpawnerData(100, 0x5D0000C1ULL, 4, 4));
    mMonsters.push_back(new MobSpawnerData(200, 0x5D550041ULL, 4, 4));
    mMonsters.push_back(new MobSpawnerData(1, 0x5D540043ULL, 4, 4));
    mMonsters.push_back(new MobSpawnerData(1, 0x5D540442ULL, 4, 4));
    mCreatures.push_back(new MobSpawnerData(2, 0x5D108001ULL, 4, 4));

    mDecorator = (BiomeDecorator*)new HellBiomeDecorator();
}

HellBiome::~HellBiome()
{
}

Vec3* HellBiome::getFogColor(float celestialAngle, float partialTicks) const
{
    unsigned int colour = Minecraft::GetInstance()->getColourTable()->getColour(
        eMinecraftColour_Under_Water_Fog_Colour);
    return Vec3::newTemp((double)(((colour >> 16) & 0xFF) * 0.00390625F),
                         (double)(((colour >> 8) & 0xFF) * 0.00390625F),
                         (double)((colour & 0xFF) * 0.00390625F));
}

bool HellBiome::isFoggy()
{
    return true;
}

bool HellBiome::isNatural()
{
    return false;
}

unsigned int HellBiome::getSkyColor(float temperature)
{
    return 0;
}

Biome::EBiomeBaseClass HellBiome::getBaseClass()
{
    return eBiomeBaseClass_Hell;
}
