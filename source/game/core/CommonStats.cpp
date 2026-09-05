#include "game/core/CommonStats.hpp"

#include <cstring>

#include "game/core/Stats.hpp"
#include "game/item/Item.hpp"
#include "game/item/Items.hpp"

CommonStats::~CommonStats()
{
}

Stat* CommonStats::get_stat(int id)
{
    return Stats::get(id);
}

Stat* CommonStats::get_walkOneM()
{
    return Stats::walkOneM;
}

Stat* CommonStats::get_swimOneM()
{
    return Stats::swimOneM;
}

Stat* CommonStats::get_fallOneM()
{
    return Stats::fallOneM;
}

Stat* CommonStats::get_climbOneM()
{
    return Stats::climbOneM;
}

Stat* CommonStats::get_minecartOneM()
{
    return Stats::minecartOneM;
}

Stat* CommonStats::get_boatOneM()
{
    return Stats::boatOneM;
}

Stat* CommonStats::get_pigOneM()
{
    return Stats::pigOneM;
}

Stat* CommonStats::get_portalsCreated()
{
    return Stats::portalsUsed;
}

Stat* CommonStats::get_cowsMilked()
{
    return Stats::cowsMilked;
}

Stat* CommonStats::get_netherLavaCollected()
{
    return Stats::netherLavaCollected;
}

Stat* CommonStats::get_killsZombie()
{
    return Stats::killsZombie;
}

Stat* CommonStats::get_killsSkeleton()
{
    return Stats::killsSkeleton;
}

Stat* CommonStats::get_killsCreeper()
{
    return Stats::killsCreeper;
}

Stat* CommonStats::get_killsSpider()
{
    return Stats::killsSpider;
}

Stat* CommonStats::get_killsSpiderJockey()
{
    return Stats::killsSpiderJockey;
}

Stat* CommonStats::get_killsZombiePigman()
{
    return Stats::killsZombiePigman;
}

Stat* CommonStats::get_killsSlime()
{
    return Stats::killsSlime;
}

Stat* CommonStats::get_killsGhast()
{
    return Stats::killsGhast;
}

Stat* CommonStats::get_killsNetherZombiePigman()
{
    return Stats::killsNetherZombiePigman;
}

Stat* CommonStats::get_breedEntity(unsigned long long entityType)
{
    if (entityType == 0x2D6A0000ULL)
    {
        return GenericStats::repopulation();
    }
    return NULL;
}

Stat* CommonStats::get_tamedEntity(unsigned long long entityType)
{
    if (entityType == 0x2D6C0001ULL)
    {
        return GenericStats::lionTamer();
    }
    if (entityType == 0x0D6C0002ULL)
    {
        return Stats::befriendsWolf;
    }
    if (entityType == 0x2D690001ULL)
    {
        return GenericStats::leaderOfThePack();
    }
    return NULL;
}

Stat* CommonStats::get_craftedEntity(unsigned long long entityType)
{
    if (entityType == 0x0D580002ULL)
    {
        return GenericStats::bodyGuard();
    }
    if (entityType == 0x5D540047ULL)
    {
        return GenericStats::spawnWither();
    }
    return NULL;
}

Stat* CommonStats::get_shearedEntity(unsigned long long entityType)
{
    if (entityType == 0x2D680002ULL)
    {
        return GenericStats::haveAShearfulDay();
    }
    return NULL;
}

Stat* CommonStats::get_totalBlocksMined()
{
    return Stats::totalBlocksMined;
}

Stat* CommonStats::get_timePlayed()
{
    return Stats::timePlayed;
}

Stat* CommonStats::get_blocksPlaced(int blockId)
{
    return Stats::blockPlacedStats.data[blockId];
}

Stat* CommonStats::get_blocksMined(int blockId)
{
    if (blockId > 255)
    {
        return NULL;
    }
    return Stats::blockMinedStats.data[blockId];
}

Stat* CommonStats::get_itemsCollected(int itemId, int auxValue)
{
    if (itemId == 35)
    {
        return Stats::rainbowCollectionStats.data[auxValue];
    }
    if (itemId != Item::getId(Items::dye))
    {
        return Stats::itemCollectedStats.data[itemId];
    }
    return NULL;
}

Stat* CommonStats::get_itemsCrafted(int itemId)
{
    return Stats::itemCraftedStats.data[itemId];
}

Stat* CommonStats::get_itemsSmelted(int itemId)
{
    return Stats::itemCraftedStats.data[itemId];
}

Stat* CommonStats::get_itemsUsed(int itemId)
{
    if (itemId == 0x140 || itemId == 0x19C)
    {
        return Stats::blockPlacedStats.data[itemId];
    }
    if (itemId == 0x1C1)
    {
        return GenericStats::cheatingDeath();
    }
    return NULL;
}

Stat* CommonStats::get_itemsBought(int itemId)
{
    if (itemId == Item::getId(Items::dye))
    {
        return Stats::itemCollectedStats.data[itemId];
    }
    return NULL;
}

Stat* CommonStats::get_armourDyed(int itemId)
{
    if (itemId < 298 || itemId > 301)
    {
        return NULL;
    }
    return Stats::armourDyedStats.data[itemId - 298];
}

Stat* CommonStats::get_killsEnderdragon()
{
    return Stats::killsEnderdragon;
}

Stat* CommonStats::get_completeTheEnd()
{
    return Stats::completeTheEnd;
}

Stat* CommonStats::get_enteredBiome(int biomeId)
{
    if (biomeId > 40)
    {
        biomeId -= 128;
    }
    if (biomeId > 39 && biomeId < 48)
    {
        return Stats::biomesVisitedStats.data[0];
    }
    return Stats::biomesVisitedStats.data[biomeId];
}

Stat* CommonStats::get_achievement(eAward award)
{
    switch ((int)award)
    {
    case 0: return GenericStats::openInventory();
    case 1: return GenericStats::mineWood();
    case 2: return GenericStats::buildWorkbench();
    case 3: return GenericStats::buildPickaxe();
    case 4: return GenericStats::buildFurnace();
    case 5: return GenericStats::acquireIron();
    case 6: return GenericStats::buildHoe();
    case 7: return GenericStats::makeBread();
    case 8: return GenericStats::bakeCake();
    case 9: return GenericStats::buildBetterPickaxe();
    case 10: return GenericStats::cookFish();
    case 11: return GenericStats::onARail();
    case 12: return GenericStats::buildSword();
    case 13: return GenericStats::killEnemy();
    case 14: return GenericStats::killCow();
    case 15: return GenericStats::flyPig();
    case 16: return GenericStats::leaderOfThePack();
    case 17: return GenericStats::MOARTools();
    case 18: return GenericStats::dispenseWithThis();
    case 19: return GenericStats::InToTheNether();
    case 20: return GenericStats::mine100Blocks();
    case 21: return GenericStats::kill10Creepers();
    case 22: return GenericStats::porkChop();
    case 23: return GenericStats::passingTheTime();
    case 24: return GenericStats::archer();
    case 25: return GenericStats::socialPost();
    case 26: return GenericStats::snipeSkeleton();
    case 27: return GenericStats::diamonds();
    case 28: return NULL;
    case 29: return GenericStats::ghast();
    case 30: return GenericStats::blazeRod();
    case 31: return GenericStats::potion();
    case 32: return GenericStats::theEnd();
    case 33: return GenericStats::winGame();
    case 34: return GenericStats::enchantments();
    case 35: return GenericStats::overkill();
    case 36: return GenericStats::bookcase();
    case 37: return GenericStats::adventuringTime();
    case 38: return GenericStats::repopulation();
    case 39: return GenericStats::diamondsToYou();
    case 40: return GenericStats::theHaggler();
    case 41: return GenericStats::potPlanter();
    case 42: return GenericStats::itsASign();
    case 43: return GenericStats::ironBelly();
    case 44: return GenericStats::haveAShearfulDay();
    case 45: return GenericStats::rainbowCollection();
    case 46: return GenericStats::stayinFrosty();
    case 47: return GenericStats::chestfulOfCobblestone();
    case 48: return GenericStats::renewableEnergy();
    case 49: return GenericStats::musicToMyEars();
    case 50: return GenericStats::bodyGuard();
    case 51: return GenericStats::ironMan();
    case 52: return GenericStats::zombieDoctor();
    case 53: return GenericStats::lionTamer();
    case 54: return GenericStats::spawnWither();
    case 55: return GenericStats::killWither();
    case 56: return GenericStats::fullBeacon();
    case 57: return GenericStats::overpowered();
    case 58: return GenericStats::tieDyeOutfit();
    case 59: return GenericStats::trampoline();
    case 60: return GenericStats::theStudent();
    case 61: return GenericStats::hasBecomeTheMaster();
    case 62: return GenericStats::tisButAScratch();
    case 63: return GenericStats::cupid();
    case 64: return GenericStats::youWontLikeMeWhenImHungry();
    case 65: return GenericStats::mine();
    case 66: return GenericStats::theEndAgain();
    case 67: return GenericStats::youNeedAMint();
    case 68: return GenericStats::superSonic();
    case 69: return GenericStats::drySpell();
    case 70: return GenericStats::freeDiver();
    case 71: return GenericStats::superFuel();
    case 72: return GenericStats::saddleUp();
    case 73: return GenericStats::tasteOfYourOwnMedicine();
    case 74: return GenericStats::beamMeUp();
    case 75: return GenericStats::mapRoom();
    case 76: return GenericStats::camouflage();
    case 77: return GenericStats::backFromTheDead();
    case 78: return GenericStats::equipYourFists();
    case 79: return GenericStats::tenaciousBall();
    case 80: return GenericStats::deadshot();
    case 81: return GenericStats::suckMySnowballs();
    case 82: return GenericStats::mountainTop();
    case 83: return GenericStats::willLikeNoOther();
    case 84: return GenericStats::theDeepEnd();
    case 85: return GenericStats::greatViewFromUpHere();
    case 86: return GenericStats::changeOfSheets();
    case 87: return GenericStats::cheatingDeath();
    case 88: return GenericStats::soIGotThatGoingForMe();
    case 89: return GenericStats::letItGo();
    case 90: return GenericStats::feelingIll();
    case 91: return GenericStats::_1234Pickle();
    case 92: return GenericStats::alternativeFuel();
    case 93: return GenericStats::moskstraumen();
    case 94: return GenericStats::castaway();
    case 95: return GenericStats::sleepWithTheFishes();
    case 96: return GenericStats::echolocation();
    default: return NULL;
    }
}

Stat* CommonStats::get_MG03_Score()
{
    return Stats::mg03Score;
}

Stat* CommonStats::get_MG03_RingCount()
{
    return Stats::mg03RingCount;
}

Stat* CommonStats::get_MG03_BestTimeScoreChallenge()
{
    return Stats::mg03BestTimeScoreChallenge;
}

Stat* CommonStats::get_MG03_BestTime()
{
    return Stats::mg03BestTime;
}

arrayWithLength<unsigned char> CommonStats::makeParam(int value)
{
    arrayWithLength<unsigned char> result(4, true);
    std::memcpy(result.data, &value, 4);
    return result;
}

arrayWithLength<unsigned char> CommonStats::getParam_MG03Score(int value)
{
    return makeParam(value);
}

arrayWithLength<unsigned char> CommonStats::getParam_MG03RingCount(int value)
{
    return makeParam(value);
}

arrayWithLength<unsigned char> CommonStats::getParam_MG03BestTime(unsigned int value)
{
    arrayWithLength<unsigned char> result(4, true);
    std::memcpy(result.data, &value, 4);
    return result;
}

arrayWithLength<unsigned char> CommonStats::getParam_walkOneM(int value)
{
    return makeParam(value);
}

arrayWithLength<unsigned char> CommonStats::getParam_swimOneM(int value)
{
    return makeParam(value);
}

arrayWithLength<unsigned char> CommonStats::getParam_fallOneM(int value)
{
    return makeParam(value);
}

arrayWithLength<unsigned char> CommonStats::getParam_climbOneM(int value)
{
    return makeParam(value);
}

arrayWithLength<unsigned char> CommonStats::getParam_minecartOneM(int value)
{
    return makeParam(value);
}

arrayWithLength<unsigned char> CommonStats::getParam_boatOneM(int value)
{
    return makeParam(value);
}

arrayWithLength<unsigned char> CommonStats::getParam_pigOneM(int value)
{
    return makeParam(value);
}

arrayWithLength<unsigned char> CommonStats::getParam_blocksMined(int blockId, int auxValue, int count)
{
    return makeParam(count);
}

arrayWithLength<unsigned char> CommonStats::getParam_itemsCollected(int itemId, int auxValue, int count)
{
    return makeParam(count);
}

arrayWithLength<unsigned char> CommonStats::getParam_itemsCrafted(int itemId, int auxValue, int count)
{
    return makeParam(count);
}

arrayWithLength<unsigned char> CommonStats::getParam_itemsSmelted(int itemId, int auxValue, int count)
{
    return makeParam(count);
}

arrayWithLength<unsigned char> CommonStats::getParam_itemsBought(int itemId, int auxValue, int count)
{
    return makeParam(count);
}

arrayWithLength<unsigned char> CommonStats::getParam_time(int value)
{
    return makeParam(value);
}

arrayWithLength<unsigned char> CommonStats::getParam_noArgs()
{
    return makeParam(1);
}

unsigned int CommonStats::readParam(arrayWithLength<unsigned char> value)
{
    if (value.length != 4)
    {
        return 1;
    }
    unsigned int result;
    std::memcpy(&result, value.data, 4);
    return result;
}
