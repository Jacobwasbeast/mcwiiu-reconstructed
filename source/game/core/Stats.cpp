#include "game/core/Stats.hpp"

#include "game/core/StatsCounter.hpp"

#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"

#include "game/item/Item.hpp"
#include "game/item/Items.hpp"

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
    walkOneM = (new GeneralStat(2000, L"stat.walkOneM", Stat::distanceFormatter))->setAwardLocallyOnly()->postConstruct(true);

    swimOneM = (new GeneralStat(2001, L"stat.swimOneM", Stat::distanceFormatter))->setAwardLocallyOnly()->postConstruct(true);

    fallOneM = (new GeneralStat(2002, L"stat.fallOneM", Stat::distanceFormatter))->setAwardLocallyOnly()->postConstruct(true);

    climbOneM = (new GeneralStat(2003, L"stat.climbOneM", Stat::distanceFormatter))->setAwardLocallyOnly()->postConstruct(true);

    minecartOneM = (new GeneralStat(2004, L"stat.minecartOneM", Stat::distanceFormatter))->setAwardLocallyOnly()->postConstruct(true);

    boatOneM = (new GeneralStat(2005, L"stat.boatOneM", Stat::distanceFormatter))->setAwardLocallyOnly()->postConstruct(true);

    pigOneM = (new GeneralStat(2006, L"stat.pigOneM", Stat::distanceFormatter))->setAwardLocallyOnly()->postConstruct(true);

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

    mg03Score = (new GeneralStat(3000, L"stat.MG03Score"))->setAwardLocallyOnly()->postConstruct(false);

    mg03RingCount = (new GeneralStat(3001, L"stat.MG03RingCount"))->setAwardLocallyOnly()->postConstruct(false);

    mg03BestTimeScoreChallenge = (new GeneralStat(3002, L"stat.MG03BestTimeScoreChallenge"))->setAwardLocallyOnly()->postConstruct(false);

    mg03BestTime = (new GeneralStat(3003, L"stat.MG03BestTime"))->setAwardLocallyOnly()->postConstruct(false);

    befriendsWolf = (new GeneralStat(2019, L"stat.befriendsWolf"))->postConstruct(true);
    totalBlocksMined = (new GeneralStat(2020, L"stat.totalBlocksMined"))->postConstruct(true);

    timePlayed = (new GeneralStat(2021, L"stat.timePlayed"))->setAwardLocallyOnly()->postConstruct(true);

    buildBlockStats();
    Achievements::init();
    Achievements::staticCtor();
    buildAdditionalStats();
}

void Stats::buildBlockStats()
{
    Stat* stat;

    blockMinedStats = arrayWithLength<Stat*>(256, true);

    stat = new ItemStat(0x01000000, L"mineBlock.dirt", Item::getId(Item::byBlock(Blocks::dirt)));
    blockMinedStats.data[Item::getId(Item::byBlock(Blocks::dirt))] = stat;
    blockMinedStats.data[Item::getId(Item::byBlock(Blocks::grass))] = stat;
    blockMinedStats.data[Item::getId(Item::byBlock(Blocks::farmland))] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01000001, L"mineBlock.stone", Item::getId(Item::byBlock(Blocks::cobblestone)));
    blockMinedStats.data[Item::getId(Item::byBlock(Blocks::cobblestone))] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01000002, L"mineBlock.sand", Item::getId(Item::byBlock(Blocks::sand)));
    blockMinedStats.data[Item::getId(Item::byBlock(Blocks::sand))] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01000003, L"mineBlock.cobblestone", Item::getId(Item::byBlock(Blocks::stone)));
    blockMinedStats.data[Item::getId(Item::byBlock(Blocks::stone))] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01000004, L"mineBlock.gravel", Item::getId(Item::byBlock(Blocks::gravel)));
    blockMinedStats.data[Item::getId(Item::byBlock(Blocks::gravel))] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01000005, L"mineBlock.clay", Item::getId(Item::byBlock(Blocks::clay)));
    blockMinedStats.data[Item::getId(Item::byBlock(Blocks::clay))] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01000006, L"mineBlock.obsidian", Item::getId(Item::byBlock(Blocks::obsidian)));
    blockMinedStats.data[Item::getId(Item::byBlock(Blocks::obsidian))] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01000007, L"mineBlock.coal", Item::getId(Item::byBlock(Blocks::coal_ore)));
    blockMinedStats.data[Item::getId(Item::byBlock(Blocks::coal_ore))] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01000008, L"mineBlock.iron", Item::getId(Item::byBlock(Blocks::iron_ore)));
    blockMinedStats.data[Item::getId(Item::byBlock(Blocks::iron_ore))] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01000009, L"mineBlock.gold", Item::getId(Item::byBlock(Blocks::gold_ore)));
    blockMinedStats.data[Item::getId(Item::byBlock(Blocks::gold_ore))] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x0100000A, L"mineBlock.diamond", Item::getId(Item::byBlock(Blocks::diamond_ore)));
    blockMinedStats.data[Item::getId(Item::byBlock(Blocks::diamond_ore))] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x0100000B, L"mineBlock.redstone", Item::getId(Item::byBlock(Blocks::redstone_ore)));
    blockMinedStats.data[Item::getId(Item::byBlock(Blocks::redstone_ore))] = stat;
    blockMinedStats.data[Item::getId(Item::byBlock(Blocks::lit_redstone_ore))] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x0100000C, L"mineBlock.lapisLazuli", Item::getId(Item::byBlock(Blocks::lapis_ore)));
    blockMinedStats.data[Item::getId(Item::byBlock(Blocks::lapis_ore))] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x0100000D, L"mineBlock.netherrack", Item::getId(Item::byBlock(Blocks::netherrack)));
    blockMinedStats.data[Item::getId(Item::byBlock(Blocks::netherrack))] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x0100000E, L"mineBlock.soulSand", Item::getId(Item::byBlock(Blocks::soul_sand)));
    blockMinedStats.data[Item::getId(Item::byBlock(Blocks::soul_sand))] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x0100000F, L"mineBlock.glowstone", Item::getId(Item::byBlock(Blocks::glowstone)));
    blockMinedStats.data[Item::getId(Item::byBlock(Blocks::glowstone))] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01000010, L"mineBlock.wood", Item::getId(Item::byBlock(Blocks::log)));
    blockMinedStats.data[Item::getId(Item::byBlock(Blocks::log))] = stat;
    blockMinedStats.data[Item::getId(Item::byBlock(Blocks::log2))] = stat;
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

    stat = new ItemStat(0x01010000, L"collectItem.egg", Item::getId(Items::egg));
    itemCollectedStats.data[Item::getId(Items::egg)] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01000012, L"mineBlock.wheat", Item::getId(Item::byBlock(Blocks::wheat)));
    blockMinedStats.data[Item::getId(Item::byBlock(Blocks::wheat))] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01000013, L"mineBlock.mushroom1", Item::getId(Item::byBlock(Blocks::brown_mushroom)));
    blockMinedStats.data[Item::getId(Item::byBlock(Blocks::brown_mushroom))] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01000011, L"mineBlock.sugar", Item::getId(Item::byBlock(Blocks::reeds)));
    blockMinedStats.data[Item::getId(Item::byBlock(Blocks::reeds))] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01010004, L"collectItem.pumpkin", Item::getId(Item::byBlock(Blocks::pumpkin)));
    itemCollectedStats.data[Item::getId(Item::byBlock(Blocks::pumpkin))] = stat;
    itemCollectedStats.data[Item::getId(Item::byBlock(Blocks::lit_pumpkin))] = stat;
    stat->postConstruct(true);

    itemCraftedStats = arrayWithLength<Stat*>(16000, true);

    stat = new ItemStat(0x01020000, L"craftItem.plank", Item::getId(Item::byBlock(Blocks::planks)));
    itemCraftedStats.data[Item::getId(Item::byBlock(Blocks::planks))] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01020001, L"craftItem.workbench", Item::getId(Item::byBlock(Blocks::crafting_table)));
    itemCraftedStats.data[Item::getId(Item::byBlock(Blocks::crafting_table))] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01020002, L"craftItem.stick", Item::getId(Items::stick));
    itemCraftedStats.data[Item::getId(Items::stick)] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01020003, L"craftItem.woodenShovel", Item::getId(Items::woodenShovel));
    itemCraftedStats.data[Item::getId(Items::woodenShovel)] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01020004, L"craftItem.woodenPickAxe", Item::getId(Items::woodenPickaxe));
    itemCraftedStats.data[Item::getId(Items::woodenPickaxe)] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01020005, L"craftItem.stonePickAxe", Item::getId(Items::stonePickaxe));
    itemCraftedStats.data[Item::getId(Items::stonePickaxe)] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01020006, L"craftItem.ironPickAxe", Item::getId(Items::ironPickaxe));
    itemCraftedStats.data[Item::getId(Items::ironPickaxe)] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01020007, L"craftItem.diamondPickAxe", Item::getId(Items::diamondPickaxe));
    itemCraftedStats.data[Item::getId(Items::diamondPickaxe)] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01020008, L"craftItem.goldPickAxe", Item::getId(Items::goldenPickaxe));
    itemCraftedStats.data[Item::getId(Items::goldenPickaxe)] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01020009, L"craftItem.stoneShovel", Item::getId(Items::stoneShovel));
    itemCraftedStats.data[Item::getId(Items::stoneShovel)] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x0102000A, L"craftItem.ironShovel", Item::getId(Items::ironShovel));
    itemCraftedStats.data[Item::getId(Items::ironShovel)] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x0102000B, L"craftItem.diamondShovel", Item::getId(Items::diamondShovel));
    itemCraftedStats.data[Item::getId(Items::diamondShovel)] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x0102000C, L"craftItem.goldShovel", Item::getId(Items::goldenShovel));
    itemCraftedStats.data[Item::getId(Items::goldenShovel)] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x0102000D, L"craftItem.woodenAxe", Item::getId(Items::woodenAxe));
    itemCraftedStats.data[Item::getId(Items::woodenAxe)] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x0102000E, L"craftItem.stoneAxe", Item::getId(Items::stoneAxe));
    itemCraftedStats.data[Item::getId(Items::stoneAxe)] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x0102000F, L"craftItem.ironAxe", Item::getId(Items::ironAxe));
    itemCraftedStats.data[Item::getId(Items::ironAxe)] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01020010, L"craftItem.diamondAxe", Item::getId(Items::diamondAxe));
    itemCraftedStats.data[Item::getId(Items::diamondAxe)] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01020011, L"craftItem.goldAxe", Item::getId(Items::goldenAxe));
    itemCraftedStats.data[Item::getId(Items::goldenAxe)] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01020012, L"craftItem.woodenHoe", Item::getId(Items::woodenHoe));
    itemCraftedStats.data[Item::getId(Items::woodenHoe)] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01020013, L"craftItem.stoneHoe", Item::getId(Items::stoneHoe));
    itemCraftedStats.data[Item::getId(Items::stoneHoe)] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01020014, L"craftItem.ironHoe", Item::getId(Items::ironHoe));
    itemCraftedStats.data[Item::getId(Items::ironHoe)] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01020015, L"craftItem.diamondHoe", Item::getId(Items::diamondHoe));
    itemCraftedStats.data[Item::getId(Items::diamondHoe)] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01020016, L"craftItem.goldHoe", Item::getId(Items::goldenHoe));
    itemCraftedStats.data[Item::getId(Items::goldenHoe)] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01020017, L"craftItem.glowstone", Item::getId(Item::byBlock(Blocks::glowstone)));
    itemCraftedStats.data[Item::getId(Item::byBlock(Blocks::glowstone))] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01020018, L"craftItem.tnt", Item::getId(Item::byBlock(Blocks::tnt)));
    itemCraftedStats.data[Item::getId(Item::byBlock(Blocks::tnt))] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x01020019, L"craftItem.bowl", Item::getId(Items::bowl));
    itemCraftedStats.data[Item::getId(Items::bowl)] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x0102001A, L"craftItem.bucket", Item::getId(Items::bucket));
    itemCraftedStats.data[Item::getId(Items::bucket)] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x0102001B, L"craftItem.flintAndSteel", Item::getId(Items::flintAndSteel));
    itemCraftedStats.data[Item::getId(Items::flintAndSteel)] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x0102001C, L"craftItem.fishingRod", Item::getId(Items::fishingRod));
    itemCraftedStats.data[Item::getId(Items::fishingRod)] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x0102001D, L"craftItem.clock", Item::getId(Items::clock));
    itemCraftedStats.data[Item::getId(Items::clock)] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x0102001E, L"craftItem.compass", Item::getId(Items::compass));
    itemCraftedStats.data[Item::getId(Items::compass)] = stat;
    stat->postConstruct(true);

    stat = new ItemStat(0x0102001F, L"craftItem.map", Item::getId(Items::map));
    itemCraftedStats.data[Item::getId(Items::map)] = stat;
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

    itemStat = (new ItemStat(statId++, L"craftItem.flowerPot", Item::getId(Items::flowerPot)))->setExtended();
    itemCraftedStats.data[itemStat->getItemId()] = itemStat;
    itemStat->postConstruct(true);

    itemStat = (new ItemStat(statId++, L"craftItem.sign", Item::getId(Items::sign)))->setExtended();
    itemCraftedStats.data[itemStat->getItemId()] = itemStat;
    itemStat->postConstruct(true);

    itemStat = (new ItemStat(statId++, L"mineBlock.emerald", Item::getId(Item::byBlock(Blocks::emerald_ore))))->setExtended();
    blockMinedStats.data[itemStat->getItemId()] = itemStat;
    itemStat->postConstruct(true);

    itemStat = (new ItemStat(statId++, L"itemsBought.emerald", Item::getId(Items::emerald)))->setExtended();
    itemCollectedStats.data[itemStat->getItemId()] = itemStat;
    itemStat->postConstruct(true);

    blockPlacedStats = arrayWithLength<Stat*>(1000, true);

    itemStat = (new ItemStat(statId++, L"blockPlaced.flowerPot", Block::getId(Blocks::flower_pot)))->setExtended();
    blockPlacedStats.data[itemStat->getItemId()] = itemStat;
    itemStat->postConstruct(true);

    itemStat = (new ItemStat(statId++, L"blockPlaced.sign", Block::getId(Blocks::standing_sign)))->setExtended();
    blockPlacedStats.data[itemStat->getItemId()] = itemStat;
    itemStat->postConstruct(true);

    itemStat = (new ItemStat(statId++, L"blockPlaced.wallsign", Block::getId(Blocks::wall_sign)))->setExtended();
    blockPlacedStats.data[itemStat->getItemId()] = itemStat;
    itemStat->postConstruct(true);

    rainbowCollectionStats = arrayWithLength<Stat*>(16, true);

    for (unsigned int wool = 0; wool < 16; wool++)
    {
        stat = (new GeneralStat(statId++, L"rainbowCollection." + _toString(wool)))->setWoolStat(true)->setExtended();
        rainbowCollectionStats.data[wool] = stat;
        stat->postConstruct(true);
    }

    biomesVisitedStats = arrayWithLength<Stat*>(40, true);

    for (unsigned int biome = 0; biome < 40; biome++)
    {
        stat = (new GeneralStat(statId++, L"biomesVisited." + _toString(biome)))->setBiomeStat(true)->setExtended();
        biomesVisitedStats.data[biome] = stat;
        stat->postConstruct(true);
    }

    itemStat = (new ItemStat(statId++, L"itemCrafted.porkchop", Item::getId(Items::cookedPorkchop)))->setExtended();
    itemCraftedStats.data[itemStat->getItemId()] = itemStat;
    itemStat->postConstruct(true);

    itemStat = (new ItemStat(statId++, L"itemEaten.porkchop", Item::getId(Items::cookedPorkchop)))->setExtended();
    blockPlacedStats.data[itemStat->getItemId()] = itemStat;
    itemStat->postConstruct(true);

    armourDyedStats = arrayWithLength<Stat*>(4, true);

    stat = (new GeneralStat(statId++, L"armorDyed.helmet"))->setArmourStat(true)->setAwardLocallyOnly()->setExtended();
    armourDyedStats.data[0] = stat;
    stat->postConstruct(true);

    stat = (new GeneralStat(statId++, L"armorDyed.chestplate"))->setArmourStat(true)->setAwardLocallyOnly()->setExtended();
    armourDyedStats.data[1] = stat;
    stat->postConstruct(true);

    stat = (new GeneralStat(statId++, L"armorDyed.leggings"))->setArmourStat(true)->setAwardLocallyOnly()->setExtended();
    armourDyedStats.data[2] = stat;
    stat->postConstruct(true);

    stat = (new GeneralStat(statId++, L"armorDyed.boots"))->setArmourStat(true)->setAwardLocallyOnly()->setExtended();
    armourDyedStats.data[3] = stat;
    stat->postConstruct(true);
}

Stat* Stats::get(int id)
{
    return Stat::statsById->at(id);
}
