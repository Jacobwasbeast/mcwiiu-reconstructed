#include "game/core/CustomizableSourceSettings.hpp"

CustomizableSourceSettings::Builder::Builder()
{
    setDefaults();
}

void CustomizableSourceSettings::Builder::setDefaults()
{
    biomeSize = 684.4119873046875f;
    riverSize = 512;
    dirtCount = 200;
    dirtSize = 200;
    unused10 = 0;
    useCaves = true;
    unused12 = 0;
    useDungeons = true;
    useStrongholds = true;
    useVillages = true;
    unused16 = 0;
    useMineShafts = true;
    unused18 = 0;
    useTemples = true;
    unused1a = 0;
    unused1b = 0;
    dirtMinHeight = 80;
    dirtMaxHeight = 80;
    gravelCount = 0;
    gravelSize = 0;
    gravelMinHeight = 12;
    gravelMaxHeight = 1;
    dioriteCount = 1;
    dioriteSize = 0;
    dioriteMinHeight = 1;
    dioriteMaxHeight = 0;
    useMonuments = true;
    monumentSpacing = 8;
    useRavines = true;
    useWaterLakes = true;
    useLavaLakes = true;
    waterLakeChance = 4;
    useLavaOceans = true;
    lavaLakeChance = 80;
    unused64 = true;
    fixedBiome = -1;
    biomeSizeValue = 4;
    riverSizeValue = 4;
    setting70 = 33;
    setting74 = 10;
    setting78 = 0;
    setting7c = 128;
    setting80 = 33;
    setting84 = 8;
    setting88 = 0;
    setting8c = 128;
    setting90 = 33;
    setting94 = 8;
    setting98 = 0;
    setting9c = 128;
    settinga0 = 33;
    settinga4 = 10;
    graniteCount = 4;
    graniteSize = 4;
    graniteMinHeight = 33;
    graniteMaxHeight = 10;
    andesiteCount = 0;
    andesiteSize = 128;
    andesiteMinHeight = 33;
    andesiteMaxHeight = 8;
    coalCount = 0;
    coalSize = 128;
    coalMinHeight = 33;
    coalMaxHeight = 8;
    ironCount = 0;
    ironSize = 128;
    ironMinHeight = 33;
    ironMaxHeight = 10;
    goldCount = 0;
    goldSize = 128;
    goldMinHeight = 33;
    goldMaxHeight = 10;
    redstoneCount = 0;
    redstoneSize = 128;
    redstoneMinHeight = 33;
    redstoneMaxHeight = 10;
    diamondCount = 0;
    diamondSize = 128;
    diamondMinHeight = 17;
    diamondMaxHeight = 20;
    lapisCount = 0;
    lapisSize = 64;
    lapisMinHeight = 9;
    lapisMaxHeight = 20;
}

CustomizableSourceSettings::Builder* CustomizableSourceSettings::Builder::fromString(SuperflatConfig*)
{
    return new CustomizableSourceSettings::Builder();
}

CustomizableSourceSettings* CustomizableSourceSettings::Builder::build()
{
    return new CustomizableSourceSettings(this);
}

#define COPY_SETTING(name) name = builder->name

CustomizableSourceSettings::CustomizableSourceSettings(Builder* builder)
{
    COPY_SETTING(biomeSize);
    COPY_SETTING(riverSize);
    COPY_SETTING(dirtCount);
    COPY_SETTING(dirtSize);
    COPY_SETTING(unused10);
    COPY_SETTING(useCaves);
    COPY_SETTING(unused12);
    COPY_SETTING(useDungeons);
    COPY_SETTING(useStrongholds);
    COPY_SETTING(useVillages);
    COPY_SETTING(unused16);
    COPY_SETTING(useMineShafts);
    COPY_SETTING(unused18);
    COPY_SETTING(useTemples);
    COPY_SETTING(unused1a);
    COPY_SETTING(unused1b);
    COPY_SETTING(dirtMinHeight);
    COPY_SETTING(dirtMaxHeight);
    COPY_SETTING(gravelCount);
    COPY_SETTING(gravelSize);
    COPY_SETTING(gravelMinHeight);
    COPY_SETTING(gravelMaxHeight);
    COPY_SETTING(dioriteCount);
    COPY_SETTING(dioriteSize);
    COPY_SETTING(dioriteMinHeight);
    COPY_SETTING(dioriteMaxHeight);
    COPY_SETTING(useMonuments);
    COPY_SETTING(monumentSpacing);
    COPY_SETTING(useRavines);
    COPY_SETTING(useWaterLakes);
    COPY_SETTING(useLavaLakes);
    COPY_SETTING(waterLakeChance);
    COPY_SETTING(useLavaOceans);
    COPY_SETTING(lavaLakeChance);
    COPY_SETTING(unused64);
    COPY_SETTING(fixedBiome);
    COPY_SETTING(biomeSizeValue);
    COPY_SETTING(riverSizeValue);
    COPY_SETTING(setting70);
    COPY_SETTING(setting74);
    COPY_SETTING(setting78);
    COPY_SETTING(setting7c);
    COPY_SETTING(setting80);
    COPY_SETTING(setting84);
    COPY_SETTING(setting88);
    COPY_SETTING(setting8c);
    COPY_SETTING(setting90);
    COPY_SETTING(setting94);
    COPY_SETTING(setting98);
    COPY_SETTING(setting9c);
    COPY_SETTING(settinga0);
    COPY_SETTING(settinga4);
    COPY_SETTING(graniteCount);
    COPY_SETTING(graniteSize);
    COPY_SETTING(graniteMinHeight);
    COPY_SETTING(graniteMaxHeight);
    COPY_SETTING(andesiteCount);
    COPY_SETTING(andesiteSize);
    COPY_SETTING(andesiteMinHeight);
    COPY_SETTING(andesiteMaxHeight);
    COPY_SETTING(coalCount);
    COPY_SETTING(coalSize);
    COPY_SETTING(coalMinHeight);
    COPY_SETTING(coalMaxHeight);
    COPY_SETTING(ironCount);
    COPY_SETTING(ironSize);
    COPY_SETTING(ironMinHeight);
    COPY_SETTING(ironMaxHeight);
    COPY_SETTING(goldCount);
    COPY_SETTING(goldSize);
    COPY_SETTING(goldMinHeight);
    COPY_SETTING(goldMaxHeight);
    COPY_SETTING(redstoneCount);
    COPY_SETTING(redstoneSize);
    COPY_SETTING(redstoneMinHeight);
    COPY_SETTING(redstoneMaxHeight);
    COPY_SETTING(diamondCount);
    COPY_SETTING(diamondSize);
    COPY_SETTING(diamondMinHeight);
    COPY_SETTING(diamondMaxHeight);
    COPY_SETTING(lapisCount);
    COPY_SETTING(lapisSize);
    COPY_SETTING(lapisMinHeight);
    COPY_SETTING(lapisMaxHeight);
}

#undef COPY_SETTING
