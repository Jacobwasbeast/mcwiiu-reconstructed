#include "game/world/biome/TheEndBiome.hpp"

#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/biome/TheEndBiomeDecorator.hpp"
#include "game/world/block/Blocks.hpp"

TheEndBiome::TheEndBiome(EBiomeIDs biomeId, BiomeProperties* properties)
    : Biome(biomeId, eBiomeType_TheEnd, properties)
{
    mTopBlock.blockId = 3;
    mTopBlock.blockData = Blocks::end_stone->convertBlockStateToLegacyData(Blocks::end_stone->defaultBlockState());
    mFillerBlock.blockId = 3;
    mFillerBlock.blockData = Blocks::end_stone->convertBlockStateToLegacyData(Blocks::end_stone->defaultBlockState());

    mMonsters.push_back(new MobSpawnerData(10, 0x5D540043ULL, 4, 4));

    mDecorator = new TheEndBiomeDecorator();
}

TheEndBiome::~TheEndBiome()
{
}

unsigned int TheEndBiome::getSkyColor(float temperature)
{
    return Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Sky_Sky);
}

Vec3* TheEndBiome::getFogColor(float celestialAngle, float partialTicks) const
{
    float brightness = Mth::cos(celestialAngle * 3.1415927F * 2.0F) * 2.0F + 0.5F;
    brightness = Mth::clamp(brightness, 0.0F, 1.0F);

    unsigned int fogColour = Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_End_Fog_Colour);

    float red = (float)(fogColour >> 16 & 0xFF) / 255.0F;
    float green = (float)(fogColour >> 8 & 0xFF) / 255.0F;
    float blue = (float)(fogColour & 0xFF) / 255.0F;
    float scale = brightness * 2.0F + 0.15F;

    return Vec3::newTemp(red * scale, green * scale, blue * scale);
}

Biome::EBiomeBaseClass TheEndBiome::getBaseClass()
{
    return eBiomeBaseClass_TheEnd;
}

bool TheEndBiome::isNatural()
{
    return false;
}
