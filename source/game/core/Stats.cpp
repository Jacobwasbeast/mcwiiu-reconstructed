#include "game/core/Stats.hpp"

#include "game/core/StatsCounter.hpp"

#include "game/world/block/Block.hpp"

#include "game/item/Item.hpp"

#include "game/core/Achievements.hpp"

#include "game/item/ItemStat.hpp"

#include "game/core/GeneralStat.hpp"

#include <string>

template <class T>
std::wstring _toString(T value);

Stat* Stats::walkOneM = NULL;
Stat* Stats::swimOneM = NULL;
Stat* Stats::fallOneM = NULL;
Stat* Stats::climbOneM = NULL;
Stat* Stats::minecartOneM = NULL;
Stat* Stats::boatOneM = NULL;
Stat* Stats::pigOneM = NULL;
Stat* Stats::portalsUsed = NULL;
Stat* Stats::cowsMilked = NULL;
Stat* Stats::netherLavaCollected = NULL;
Stat* Stats::killsZombie = NULL;
Stat* Stats::killsSkeleton = NULL;
Stat* Stats::killsCreeper = NULL;
Stat* Stats::killsSpider = NULL;
Stat* Stats::killsSpiderJockey = NULL;
Stat* Stats::killsZombiePigman = NULL;
Stat* Stats::killsSlime = NULL;
Stat* Stats::killsGhast = NULL;
Stat* Stats::killsNetherZombiePigman = NULL;
Stat* Stats::mg03Score = NULL;
Stat* Stats::mg03RingCount = NULL;
Stat* Stats::mg03BestTimeScoreChallenge = NULL;
Stat* Stats::mg03BestTime = NULL;
Stat* Stats::befriendsWolf = NULL;
Stat* Stats::totalBlocksMined = NULL;
Stat* Stats::timePlayed = NULL;
Stat* Stats::killsEnderdragon = NULL;
Stat* Stats::completeTheEnd = NULL;

bool Stats::blockStatsBuilt = false;
bool Stats::itemStatsBuilt = false;
bool Stats::craftableStatsBuilt = false;

arrayWithLength<Stat*> Stats::blockMinedStats;
arrayWithLength<Stat*> Stats::itemCollectedStats;
arrayWithLength<Stat*> Stats::itemCraftedStats;
arrayWithLength<Stat*> Stats::blockPlacedStats;
arrayWithLength<Stat*> Stats::biomesVisitedStats;
arrayWithLength<Stat*> Stats::rainbowCollectionStats;
arrayWithLength<Stat*> Stats::armourDyedStats;

void Stats::init()
{
}

void Stats::staticCtor()
{
    Stat* stat;

    stat = new GeneralStat(2000, L"stat.walkOneM", Stat::distanceFormatter);
    stat->setAwardLocallyOnly();
    walkOneM = stat->postConstruct(true);

    stat = new GeneralStat(2001, L"stat.swimOneM", Stat::distanceFormatter);
    stat->setAwardLocallyOnly();
    swimOneM = stat->postConstruct(true);

    stat = new GeneralStat(2002, L"stat.fallOneM", Stat::distanceFormatter);
    stat->setAwardLocallyOnly();
    fallOneM = stat->postConstruct(true);

    stat = new GeneralStat(2003, L"stat.climbOneM", Stat::distanceFormatter);
    stat->setAwardLocallyOnly();
    climbOneM = stat->postConstruct(true);

    stat = new GeneralStat(2004, L"stat.minecartOneM", Stat::distanceFormatter);
    stat->setAwardLocallyOnly();
    minecartOneM = stat->postConstruct(true);

    stat = new GeneralStat(2005, L"stat.boatOneM", Stat::distanceFormatter);
    stat->setAwardLocallyOnly();
    boatOneM = stat->postConstruct(true);

    stat = new GeneralStat(2006, L"stat.pigOneM", Stat::distanceFormatter);
    stat->setAwardLocallyOnly();
    pigOneM = stat->postConstruct(true);

    portalsUsed = (new GeneralStat(2007, L"stat.portalsUsed"))->postConstruct(true);
    cowsMilked = (new GeneralStat(2008, L"stat.cowsMilked"))->postConstruct(true);
    netherLavaCollected = (new GeneralStat(2009, L"stat.netherLavaCollected"))->postConstruct(true);
    killsZombie = (new GeneralStat(2010, L"stat.killsZombie"))->postConstruct(true);
    killsSkeleton = (new GeneralStat(2011, L"stat.killsSkeleton"))->postConstruct(true);
    killsCreeper = (new GeneralStat(2012, L"stat.killsCreeper"))->postConstruct(true);
    killsSpider = (new GeneralStat(2013, L"stat.killsSpider"))->postConstruct(true);
    killsSpiderJockey = (new GeneralStat(2014, L"stat.killsSpiderJockey"))->postConstruct(true);
    killsZombiePigman = (new GeneralStat(2015, L"stat.killsZombiePigman"))->postConstruct(true);
    killsSlime = (new GeneralStat(2016, L"stat.killsSlime"))->postConstruct(true);
    killsGhast = (new GeneralStat(2017, L"stat.killsGhast"))->postConstruct(true);
    killsNetherZombiePigman = (new GeneralStat(2018, L"stat.killsNetherZombiePigman"))->postConstruct(true);

    stat = new GeneralStat(3000, L"stat.MG03Score");
    stat->setAwardLocallyOnly();
    mg03Score = stat->postConstruct(false);

    stat = new GeneralStat(3001, L"stat.MG03RingCount");
    stat->setAwardLocallyOnly();
    mg03RingCount = stat->postConstruct(false);

    stat = new GeneralStat(3002, L"stat.MG03BestTimeScoreChallenge");
    stat->setAwardLocallyOnly();
    mg03BestTimeScoreChallenge = stat->postConstruct(false);

    stat = new GeneralStat(3003, L"stat.MG03BestTime");
    stat->setAwardLocallyOnly();
    mg03BestTime = stat->postConstruct(false);

    befriendsWolf = (new GeneralStat(2019, L"stat.befriendsWolf"))->postConstruct(true);
    totalBlocksMined = (new GeneralStat(2020, L"stat.totalBlocksMined"))->postConstruct(true);

    stat = new GeneralStat(2021, L"stat.timePlayed");
    stat->setAwardLocallyOnly();
    timePlayed = stat->postConstruct(true);

    buildBlockStats();
    Achievements::init();
    Achievements::staticCtor();
    buildAdditionalStats();
}

void Stats::buildBlockStats()
{
    Stat* stat;

    blockMinedStats = arrayWithLength<Stat*>(256, true);

    stat = new ItemStat(0x01000000, L"mineBlock.dirt", Item::getId(Item::byBlock(Block::dirt)));
    blockMinedStats.data[Item::getId(Item::byBlock(Block::dirt))] = stat;
    blockMinedStats.data[Item::getId(Item::byBlock(Block::grass))] = stat;
    blockMinedStats.data[Item::getId(Item::byBlock(Block::farmland))] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01000001, L"mineBlock.stone", Item::getId(Item::byBlock(Block::stone)));
    blockMinedStats.data[Item::getId(Item::byBlock(Block::stone))] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01000002, L"mineBlock.sand", Item::getId(Item::byBlock(Block::sand)));
    blockMinedStats.data[Item::getId(Item::byBlock(Block::sand))] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01000003, L"mineBlock.cobblestone", Item::getId(Item::byBlock(Block::cobblestone)));
    blockMinedStats.data[Item::getId(Item::byBlock(Block::cobblestone))] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01000004, L"mineBlock.gravel", Item::getId(Item::byBlock(Block::gravel)));
    blockMinedStats.data[Item::getId(Item::byBlock(Block::gravel))] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01000005, L"mineBlock.clay", Item::getId(Item::byBlock(Block::clay)));
    blockMinedStats.data[Item::getId(Item::byBlock(Block::clay))] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01000006, L"mineBlock.obsidian", Item::getId(Item::byBlock(Block::obsidian)));
    blockMinedStats.data[Item::getId(Item::byBlock(Block::obsidian))] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01000007, L"mineBlock.coal", Item::getId(Item::byBlock(Block::coalOre)));
    blockMinedStats.data[Item::getId(Item::byBlock(Block::coalOre))] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01000008, L"mineBlock.iron", Item::getId(Item::byBlock(Block::ironOre)));
    blockMinedStats.data[Item::getId(Item::byBlock(Block::ironOre))] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01000009, L"mineBlock.gold", Item::getId(Item::byBlock(Block::goldOre)));
    blockMinedStats.data[Item::getId(Item::byBlock(Block::goldOre))] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x0100000A, L"mineBlock.diamond", Item::getId(Item::byBlock(Block::diamondOre)));
    blockMinedStats.data[Item::getId(Item::byBlock(Block::diamondOre))] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x0100000B, L"mineBlock.redstone", Item::getId(Item::byBlock(Block::redstoneOre)));
    blockMinedStats.data[Item::getId(Item::byBlock(Block::redstoneOre))] = stat;
    blockMinedStats.data[Item::getId(Item::byBlock(Block::litRedstoneOre))] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x0100000C, L"mineBlock.lapisLazuli", Item::getId(Item::byBlock(Block::lapisOre)));
    blockMinedStats.data[Item::getId(Item::byBlock(Block::lapisOre))] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x0100000D, L"mineBlock.netherrack", Item::getId(Item::byBlock(Block::netherrack)));
    blockMinedStats.data[Item::getId(Item::byBlock(Block::netherrack))] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x0100000E, L"mineBlock.soulSand", Item::getId(Item::byBlock(Block::soulSand)));
    blockMinedStats.data[Item::getId(Item::byBlock(Block::soulSand))] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x0100000F, L"mineBlock.glowstone", Item::getId(Item::byBlock(Block::glowstone)));
    blockMinedStats.data[Item::getId(Item::byBlock(Block::glowstone))] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01000010, L"mineBlock.wood", Item::getId(Item::byBlock(Block::log)));
    blockMinedStats.data[Item::getId(Item::byBlock(Block::log))] = stat;
    blockMinedStats.data[Item::getId(Item::byBlock(Block::log2))] = stat;
    stat->postConstruct(true);

    blockStatsBuilt = true;
    buildCraftableStats();
}

void Stats::buildItemStats()
{
    itemStatsBuilt = true;
    buildCraftableStats();
}

void Stats::buildCraftableStats()
{
    if (!blockStatsBuilt || !itemStatsBuilt || craftableStatsBuilt)
    {
        return;
    }

    Stat* stat;

    craftableStatsBuilt = true;

    itemCollectedStats = arrayWithLength<Stat*>(32000, true);

    stat = new ItemStat(0x01010000, L"collectItem.egg", Item::getId(Item::egg));
    itemCollectedStats.data[Item::getId(Item::egg)] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01000012, L"mineBlock.wheat", Item::getId(Item::byBlock(Block::wheat)));
    blockMinedStats.data[Item::getId(Item::byBlock(Block::wheat))] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01000013, L"mineBlock.mushroom1", Item::getId(Item::byBlock(Block::brownMushroom)));
    blockMinedStats.data[Item::getId(Item::byBlock(Block::brownMushroom))] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01000011, L"mineBlock.sugar", Item::getId(Item::byBlock(Block::reeds)));
    blockMinedStats.data[Item::getId(Item::byBlock(Block::reeds))] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01010004, L"collectItem.pumpkin", Item::getId(Item::byBlock(Block::pumpkin)));
    itemCollectedStats.data[Item::getId(Item::byBlock(Block::pumpkin))] = stat;
    itemCollectedStats.data[Item::getId(Item::byBlock(Block::litPumpkin))] = stat;
    stat->postConstruct(true);

    itemCraftedStats = arrayWithLength<Stat*>(16000, true);

    stat = new ItemStat(0x01020000, L"craftItem.plank", Item::getId(Item::byBlock(Block::planks)));
    itemCraftedStats.data[Item::getId(Item::byBlock(Block::planks))] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01020001, L"craftItem.workbench", Item::getId(Item::byBlock(Block::craftingTable)));
    itemCraftedStats.data[Item::getId(Item::byBlock(Block::craftingTable))] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01020002, L"craftItem.stick", Item::getId(Item::stick));
    itemCraftedStats.data[Item::getId(Item::stick)] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01020003, L"craftItem.woodenShovel", Item::getId(Item::woodenShovel));
    itemCraftedStats.data[Item::getId(Item::woodenShovel)] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01020004, L"craftItem.woodenPickAxe", Item::getId(Item::woodenPickAxe));
    itemCraftedStats.data[Item::getId(Item::woodenPickAxe)] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01020005, L"craftItem.stonePickAxe", Item::getId(Item::stonePickAxe));
    itemCraftedStats.data[Item::getId(Item::stonePickAxe)] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01020006, L"craftItem.ironPickAxe", Item::getId(Item::ironPickAxe));
    itemCraftedStats.data[Item::getId(Item::ironPickAxe)] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01020007, L"craftItem.diamondPickAxe", Item::getId(Item::diamondPickAxe));
    itemCraftedStats.data[Item::getId(Item::diamondPickAxe)] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01020008, L"craftItem.goldPickAxe", Item::getId(Item::goldPickAxe));
    itemCraftedStats.data[Item::getId(Item::goldPickAxe)] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01020009, L"craftItem.stoneShovel", Item::getId(Item::stoneShovel));
    itemCraftedStats.data[Item::getId(Item::stoneShovel)] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x0102000A, L"craftItem.ironShovel", Item::getId(Item::ironShovel));
    itemCraftedStats.data[Item::getId(Item::ironShovel)] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x0102000B, L"craftItem.diamondShovel", Item::getId(Item::diamondShovel));
    itemCraftedStats.data[Item::getId(Item::diamondShovel)] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x0102000C, L"craftItem.goldShovel", Item::getId(Item::goldShovel));
    itemCraftedStats.data[Item::getId(Item::goldShovel)] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x0102000D, L"craftItem.woodenAxe", Item::getId(Item::woodenAxe));
    itemCraftedStats.data[Item::getId(Item::woodenAxe)] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x0102000E, L"craftItem.stoneAxe", Item::getId(Item::stoneAxe));
    itemCraftedStats.data[Item::getId(Item::stoneAxe)] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x0102000F, L"craftItem.ironAxe", Item::getId(Item::ironAxe));
    itemCraftedStats.data[Item::getId(Item::ironAxe)] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01020010, L"craftItem.diamondAxe", Item::getId(Item::diamondAxe));
    itemCraftedStats.data[Item::getId(Item::diamondAxe)] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01020011, L"craftItem.goldAxe", Item::getId(Item::goldAxe));
    itemCraftedStats.data[Item::getId(Item::goldAxe)] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01020012, L"craftItem.woodenHoe", Item::getId(Item::woodenHoe));
    itemCraftedStats.data[Item::getId(Item::woodenHoe)] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01020013, L"craftItem.stoneHoe", Item::getId(Item::stoneHoe));
    itemCraftedStats.data[Item::getId(Item::stoneHoe)] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01020014, L"craftItem.ironHoe", Item::getId(Item::ironHoe));
    itemCraftedStats.data[Item::getId(Item::ironHoe)] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01020015, L"craftItem.diamondHoe", Item::getId(Item::diamondHoe));
    itemCraftedStats.data[Item::getId(Item::diamondHoe)] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01020016, L"craftItem.goldHoe", Item::getId(Item::goldHoe));
    itemCraftedStats.data[Item::getId(Item::goldHoe)] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01020017, L"craftItem.glowstone", Item::getId(Item::byBlock(Block::glowstone)));
    itemCraftedStats.data[Item::getId(Item::byBlock(Block::glowstone))] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01020018, L"craftItem.tnt", Item::getId(Item::byBlock(Block::tnt)));
    itemCraftedStats.data[Item::getId(Item::byBlock(Block::tnt))] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01020019, L"craftItem.bowl", Item::getId(Item::bowl));
    itemCraftedStats.data[Item::getId(Item::bowl)] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x0102001A, L"craftItem.bucket", Item::getId(Item::bucket));
    itemCraftedStats.data[Item::getId(Item::bucket)] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x0102001B, L"craftItem.flintAndSteel", Item::getId(Item::flintAndSteel));
    itemCraftedStats.data[Item::getId(Item::flintAndSteel)] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x0102001C, L"craftItem.fishingRod", Item::getId(Item::fishingRod));
    itemCraftedStats.data[Item::getId(Item::fishingRod)] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x0102001D, L"craftItem.clock", Item::getId(Item::clock));
    itemCraftedStats.data[Item::getId(Item::clock)] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x0102001E, L"craftItem.compass", Item::getId(Item::compass));
    itemCraftedStats.data[Item::getId(Item::compass)] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x0102001F, L"craftItem.map", Item::getId(Item::map));
    itemCraftedStats.data[Item::getId(Item::map)] = stat;
    stat->postConstruct(true);

    StatsCounter::setupStatBoards();
}

void Stats::buildAdditionalStats()
{
    ItemStat* itemStat;
    Stat* stat;
    int statId = 0x05010000;

    killsEnderdragon = (new GeneralStat(statId++, L"stat.killsEnderdragon"))->postConstruct(true);
    completeTheEnd = (new GeneralStat(statId++, L"stat.completeTheEnd"))->postConstruct(true);

    itemStat = (new ItemStat(statId++, L"craftItem.flowerPot", Item::getId(Item::flowerPot)))->setExtended();
    itemCraftedStats.data[itemStat->getItemId()] = itemStat;
    itemStat->postConstruct(true);

    itemStat = (new ItemStat(statId++, L"craftItem.sign", Item::getId(Item::sign)))->setExtended();
    itemCraftedStats.data[itemStat->getItemId()] = itemStat;
    itemStat->postConstruct(true);

    itemStat = (new ItemStat(statId++, L"mineBlock.emerald", Item::getId(Item::byBlock(Block::emeraldOre))))->setExtended();
    blockMinedStats.data[itemStat->getItemId()] = itemStat;
    itemStat->postConstruct(true);

    itemStat = (new ItemStat(statId++, L"itemsBought.emerald", Item::getId(Item::emerald)))->setExtended();
    itemCollectedStats.data[itemStat->getItemId()] = itemStat;
    itemStat->postConstruct(true);

    blockPlacedStats = arrayWithLength<Stat*>(1000, true);

    itemStat = (new ItemStat(statId++, L"blockPlaced.flowerPot", Block::getId(Block::flowerPot)))->setExtended();
    blockPlacedStats.data[itemStat->getItemId()] = itemStat;
    itemStat->postConstruct(true);

    itemStat = (new ItemStat(statId++, L"blockPlaced.sign", Block::getId(Block::standingSign)))->setExtended();
    blockPlacedStats.data[itemStat->getItemId()] = itemStat;
    itemStat->postConstruct(true);

    itemStat = (new ItemStat(statId++, L"blockPlaced.wallsign", Block::getId(Block::wallSign)))->setExtended();
    blockPlacedStats.data[itemStat->getItemId()] = itemStat;
    itemStat->postConstruct(true);

    rainbowCollectionStats = arrayWithLength<Stat*>(16, true);

    for (unsigned int wool = 0; wool < 16; wool++)
    {
        stat = new GeneralStat(statId++, L"rainbowCollection." + _toString(wool));
        stat->setWoolStat(true);
        stat->setExtended();
        rainbowCollectionStats.data[wool] = stat;
        stat->postConstruct(true);
    }

    biomesVisitedStats = arrayWithLength<Stat*>(40, true);

    for (unsigned int biome = 0; biome < 40; biome++)
    {
        stat = new GeneralStat(statId++, L"biomesVisited." + _toString(biome));
        stat->setBiomeStat(true);
        stat->setExtended();
        biomesVisitedStats.data[biome] = stat;
        stat->postConstruct(true);
    }

    itemStat = (new ItemStat(statId++, L"itemCrafted.porkchop", Item::getId(Item::porkchop)))->setExtended();
    itemCraftedStats.data[itemStat->getItemId()] = itemStat;
    itemStat->postConstruct(true);

    itemStat = (new ItemStat(statId++, L"itemEaten.porkchop", Item::getId(Item::porkchop)))->setExtended();
    blockPlacedStats.data[itemStat->getItemId()] = itemStat;
    itemStat->postConstruct(true);

    armourDyedStats = arrayWithLength<Stat*>(4, true);

    stat = new GeneralStat(statId++, L"armorDyed.helmet");
    stat->setArmourStat(true);
    stat->setAwardLocallyOnly();
    stat->setExtended();
    armourDyedStats.data[0] = stat;
    stat->postConstruct(true);

    stat = new GeneralStat(statId++, L"armorDyed.chestplate");
    stat->setArmourStat(true);
    stat->setAwardLocallyOnly();
    stat->setExtended();
    armourDyedStats.data[1] = stat;
    stat->postConstruct(true);

    stat = new GeneralStat(statId++, L"armorDyed.leggings");
    stat->setArmourStat(true);
    stat->setAwardLocallyOnly();
    stat->setExtended();
    armourDyedStats.data[2] = stat;
    stat->postConstruct(true);

    stat = new GeneralStat(statId++, L"armorDyed.boots");
    stat->setArmourStat(true);
    stat->setAwardLocallyOnly();
    stat->setExtended();
    armourDyedStats.data[3] = stat;
    stat->postConstruct(true);
}

Stat* Stats::get(int id)
{
    return Stat::statsById->at(id);
}
