#include "game/core/GenericStats.hpp"

#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"

GenericStats::~GenericStats()
{
}

Stat* GenericStats::openInventory()
{
    return gStats->get_achievement((eAward)0);
}

Stat* GenericStats::mineWood()
{
    return gStats->get_achievement((eAward)1);
}

Stat* GenericStats::buildWorkbench()
{
    return gStats->get_achievement((eAward)2);
}

Stat* GenericStats::buildPickaxe()
{
    return gStats->get_achievement((eAward)3);
}

Stat* GenericStats::buildFurnace()
{
    return gStats->get_achievement((eAward)4);
}

Stat* GenericStats::acquireIron()
{
    return gStats->get_achievement((eAward)5);
}

Stat* GenericStats::buildHoe()
{
    return gStats->get_achievement((eAward)6);
}

Stat* GenericStats::makeBread()
{
    return gStats->get_achievement((eAward)7);
}

Stat* GenericStats::bakeCake()
{
    return gStats->get_achievement((eAward)8);
}

Stat* GenericStats::buildBetterPickaxe()
{
    return gStats->get_achievement((eAward)9);
}

Stat* GenericStats::cookFish()
{
    return gStats->get_achievement((eAward)10);
}

Stat* GenericStats::onARail()
{
    return gStats->get_achievement((eAward)11);
}

Stat* GenericStats::buildSword()
{
    return gStats->get_achievement((eAward)12);
}

Stat* GenericStats::killEnemy()
{
    return gStats->get_achievement((eAward)13);
}

Stat* GenericStats::killCow()
{
    return gStats->get_achievement((eAward)14);
}

Stat* GenericStats::flyPig()
{
    return gStats->get_achievement((eAward)15);
}

Stat* GenericStats::snipeSkeleton()
{
    return gStats->get_achievement((eAward)26);
}

Stat* GenericStats::diamonds()
{
    return gStats->get_achievement((eAward)27);
}

Stat* GenericStats::ghast()
{
    return gStats->get_achievement((eAward)29);
}

Stat* GenericStats::blazeRod()
{
    return gStats->get_achievement((eAward)30);
}

Stat* GenericStats::potion()
{
    return gStats->get_achievement((eAward)31);
}

Stat* GenericStats::theEnd()
{
    return gStats->get_achievement((eAward)32);
}

Stat* GenericStats::winGame()
{
    return gStats->get_achievement((eAward)33);
}

Stat* GenericStats::enchantments()
{
    return gStats->get_achievement((eAward)34);
}

Stat* GenericStats::overkill()
{
    return gStats->get_achievement((eAward)35);
}

Stat* GenericStats::bookcase()
{
    return gStats->get_achievement((eAward)36);
}

Stat* GenericStats::leaderOfThePack()
{
    return gStats->get_achievement((eAward)16);
}

Stat* GenericStats::MOARTools()
{
    return gStats->get_achievement((eAward)17);
}

Stat* GenericStats::dispenseWithThis()
{
    return gStats->get_achievement((eAward)18);
}

Stat* GenericStats::InToTheNether()
{
    return gStats->get_achievement((eAward)19);
}

Stat* GenericStats::socialPost()
{
    return gStats->get_achievement((eAward)25);
}

Stat* GenericStats::mine100Blocks()
{
    return gStats->get_achievement((eAward)20);
}

Stat* GenericStats::kill10Creepers()
{
    return gStats->get_achievement((eAward)21);
}

Stat* GenericStats::adventuringTime()
{
    return gStats->get_achievement((eAward)37);
}

Stat* GenericStats::repopulation()
{
    return gStats->get_achievement((eAward)38);
}

Stat* GenericStats::diamondsToYou()
{
    return gStats->get_achievement((eAward)39);
}

Stat* GenericStats::porkChop()
{
    return gStats->get_achievement((eAward)22);
}

Stat* GenericStats::passingTheTime()
{
    return gStats->get_achievement((eAward)23);
}

Stat* GenericStats::archer()
{
    return gStats->get_achievement((eAward)24);
}

Stat* GenericStats::theHaggler()
{
    return gStats->get_achievement((eAward)40);
}

Stat* GenericStats::potPlanter()
{
    return gStats->get_achievement((eAward)41);
}

Stat* GenericStats::itsASign()
{
    return gStats->get_achievement((eAward)42);
}

Stat* GenericStats::ironBelly()
{
    return gStats->get_achievement((eAward)43);
}

Stat* GenericStats::haveAShearfulDay()
{
    return gStats->get_achievement((eAward)44);
}

Stat* GenericStats::rainbowCollection()
{
    return gStats->get_achievement((eAward)45);
}

Stat* GenericStats::stayinFrosty()
{
    return gStats->get_achievement((eAward)46);
}

Stat* GenericStats::chestfulOfCobblestone()
{
    return gStats->get_achievement((eAward)47);
}

Stat* GenericStats::renewableEnergy()
{
    return gStats->get_achievement((eAward)48);
}

Stat* GenericStats::musicToMyEars()
{
    return gStats->get_achievement((eAward)49);
}

Stat* GenericStats::bodyGuard()
{
    return gStats->get_achievement((eAward)50);
}

Stat* GenericStats::ironMan()
{
    return gStats->get_achievement((eAward)51);
}

Stat* GenericStats::zombieDoctor()
{
    return gStats->get_achievement((eAward)52);
}

Stat* GenericStats::lionTamer()
{
    return gStats->get_achievement((eAward)53);
}

Stat* GenericStats::spawnWither()
{
    return gStats->get_achievement((eAward)54);
}

Stat* GenericStats::killWither()
{
    return gStats->get_achievement((eAward)55);
}

Stat* GenericStats::fullBeacon()
{
    return gStats->get_achievement((eAward)56);
}

Stat* GenericStats::overpowered()
{
    return gStats->get_achievement((eAward)57);
}

Stat* GenericStats::tieDyeOutfit()
{
    return gStats->get_achievement((eAward)58);
}

Stat* GenericStats::trampoline()
{
    return gStats->get_achievement((eAward)59);
}

Stat* GenericStats::theStudent()
{
    return gStats->get_achievement((eAward)60);
}

Stat* GenericStats::hasBecomeTheMaster()
{
    return gStats->get_achievement((eAward)61);
}

Stat* GenericStats::tisButAScratch()
{
    return gStats->get_achievement((eAward)62);
}

Stat* GenericStats::cupid()
{
    return gStats->get_achievement((eAward)63);
}

Stat* GenericStats::youWontLikeMeWhenImHungry()
{
    return gStats->get_achievement((eAward)64);
}

Stat* GenericStats::mine()
{
    return gStats->get_achievement((eAward)65);
}

Stat* GenericStats::theEndAgain()
{
    return gStats->get_achievement((eAward)66);
}

Stat* GenericStats::youNeedAMint()
{
    return gStats->get_achievement((eAward)67);
}

Stat* GenericStats::superSonic()
{
    return gStats->get_achievement((eAward)68);
}

Stat* GenericStats::drySpell()
{
    return gStats->get_achievement((eAward)69);
}

Stat* GenericStats::freeDiver()
{
    return gStats->get_achievement((eAward)70);
}

Stat* GenericStats::superFuel()
{
    return gStats->get_achievement((eAward)71);
}

Stat* GenericStats::saddleUp()
{
    return gStats->get_achievement((eAward)72);
}

Stat* GenericStats::tasteOfYourOwnMedicine()
{
    return gStats->get_achievement((eAward)73);
}

Stat* GenericStats::beamMeUp()
{
    return gStats->get_achievement((eAward)74);
}

Stat* GenericStats::mapRoom()
{
    return gStats->get_achievement((eAward)75);
}

Stat* GenericStats::camouflage()
{
    return gStats->get_achievement((eAward)76);
}

Stat* GenericStats::backFromTheDead()
{
    return gStats->get_achievement((eAward)77);
}

Stat* GenericStats::equipYourFists()
{
    return gStats->get_achievement((eAward)78);
}

Stat* GenericStats::tenaciousBall()
{
    return gStats->get_achievement((eAward)79);
}

Stat* GenericStats::deadshot()
{
    return gStats->get_achievement((eAward)80);
}

Stat* GenericStats::suckMySnowballs()
{
    return gStats->get_achievement((eAward)81);
}

Stat* GenericStats::mountainTop()
{
    return gStats->get_achievement((eAward)82);
}

Stat* GenericStats::willLikeNoOther()
{
    return gStats->get_achievement((eAward)83);
}

Stat* GenericStats::theDeepEnd()
{
    return gStats->get_achievement((eAward)84);
}

Stat* GenericStats::greatViewFromUpHere()
{
    return gStats->get_achievement((eAward)85);
}

Stat* GenericStats::changeOfSheets()
{
    return gStats->get_achievement((eAward)86);
}

Stat* GenericStats::cheatingDeath()
{
    return gStats->get_achievement((eAward)87);
}

Stat* GenericStats::soIGotThatGoingForMe()
{
    return gStats->get_achievement((eAward)88);
}

Stat* GenericStats::letItGo()
{
    return gStats->get_achievement((eAward)89);
}

Stat* GenericStats::feelingIll()
{
    return gStats->get_achievement((eAward)90);
}

Stat* GenericStats::_1234Pickle()
{
    return gStats->get_achievement((eAward)91);
}

Stat* GenericStats::alternativeFuel()
{
    return gStats->get_achievement((eAward)92);
}

Stat* GenericStats::moskstraumen()
{
    return gStats->get_achievement((eAward)93);
}

Stat* GenericStats::castaway()
{
    return gStats->get_achievement((eAward)94);
}

Stat* GenericStats::sleepWithTheFishes()
{
    return gStats->get_achievement((eAward)95);
}

Stat* GenericStats::echolocation()
{
    return gStats->get_achievement((eAward)96);
}

arrayWithLength<unsigned char> GenericStats::param_MG03Score(int value)
{
    return gStats->getParam_MG03Score(value);
}

arrayWithLength<unsigned char> GenericStats::param_MG03BestTime(unsigned int value)
{
    return gStats->getParam_MG03BestTime(value);
}

arrayWithLength<unsigned char> GenericStats::param_MG03RingCount(int value)
{
    return gStats->getParam_MG03RingCount(value);
}

arrayWithLength<unsigned char> GenericStats::param_walk(int value)
{
    return gStats->getParam_walkOneM(value);
}

arrayWithLength<unsigned char> GenericStats::param_swim(int value)
{
    return gStats->getParam_swimOneM(value);
}

arrayWithLength<unsigned char> GenericStats::param_fall(int value)
{
    return gStats->getParam_fallOneM(value);
}

arrayWithLength<unsigned char> GenericStats::param_climb(int value)
{
    return gStats->getParam_climbOneM(value);
}

arrayWithLength<unsigned char> GenericStats::param_minecart(int value)
{
    return gStats->getParam_minecartOneM(value);
}

arrayWithLength<unsigned char> GenericStats::param_boat(int value)
{
    return gStats->getParam_boatOneM(value);
}

arrayWithLength<unsigned char> GenericStats::param_pig(int value)
{
    return gStats->getParam_pigOneM(value);
}

arrayWithLength<unsigned char> GenericStats::param_cowsMilked()
{
    return gStats->getParam_cowsMilked();
}

arrayWithLength<unsigned char> GenericStats::param_blocksPlaced(const BlockState* state, int count)
{
    return gStats->getParam_blocksPlaced(Block::getId(state->getBlock()), state->getBlock()->convertBlockStateToLegacyData(state), count);
}

arrayWithLength<unsigned char> GenericStats::param_blocksMined(const BlockState* state, int count)
{
    return gStats->getParam_blocksMined(Block::getId(state->getBlock()), state->getBlock()->convertBlockStateToLegacyData(state), count);
}

arrayWithLength<unsigned char> GenericStats::param_itemsCollected(Item* item, int auxValue, int count)
{
    return gStats->getParam_itemsCollected(Item::getId(item), auxValue, count);
}

arrayWithLength<unsigned char> GenericStats::param_itemsCrafted(Item* item, int auxValue, int count)
{
    return gStats->getParam_itemsCrafted(Item::getId(item), auxValue, count);
}

arrayWithLength<unsigned char> GenericStats::param_itemsSmelted(Item* item, int auxValue, int count)
{
    return gStats->getParam_itemsSmelted(Item::getId(item), auxValue, count);
}

arrayWithLength<unsigned char> GenericStats::param_itemsUsed(boost::shared_ptr<Player> player, not_null_ptr<ItemInstance> itemInstance)
{
    if (player && !itemInstance->isEmpty())
    {
        return gStats->getParam_itemsUsed(player, itemInstance);
    }

    return gStats->getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::param_itemsBought(Item* item, int auxValue, int count)
{
    return gStats->getParam_itemsBought(Item::getId(item), auxValue, count);
}

arrayWithLength<unsigned char> GenericStats::param_mobKill(boost::shared_ptr<Player> player, boost::shared_ptr<Mob> mob, const DamageSource* damageSource)
{
    if (player && mob)
    {
        return gStats->getParam_mobKill(player, mob, damageSource);
    }

    return gStats->getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::param_breedEntity(unsigned long long entityType)
{
    return gStats->getParam_breedEntity(entityType);
}

arrayWithLength<unsigned char> GenericStats::param_tamedEntity(unsigned long long entityType)
{
    return gStats->getParam_tamedEntity(entityType);
}

arrayWithLength<unsigned char> GenericStats::param_curedEntity(unsigned long long entityType)
{
    return gStats->getParam_curedEntity(entityType);
}

arrayWithLength<unsigned char> GenericStats::param_craftedEntity(unsigned long long entityType)
{
    return gStats->getParam_craftedEntity(entityType);
}

arrayWithLength<unsigned char> GenericStats::param_shearedEntity(unsigned long long entityType)
{
    return gStats->getParam_shearedEntity(entityType);
}

arrayWithLength<unsigned char> GenericStats::param_time(int value)
{
    return gStats->getParam_time(value);
}

arrayWithLength<unsigned char> GenericStats::param_changedDimension(int fromDimension, int toDimension)
{
    return gStats->getParam_changedDimension(fromDimension, toDimension);
}

arrayWithLength<unsigned char> GenericStats::param_enteredBiome(int value)
{
    return gStats->getParam_enteredBiome(value);
}

arrayWithLength<unsigned char> GenericStats::param_noArgs()
{
    return gStats->getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::param_openInventory()
{
    return gStats->getParam_achievement((eAward)0);
}

arrayWithLength<unsigned char> GenericStats::param_mineWood()
{
    return gStats->getParam_achievement((eAward)1);
}

arrayWithLength<unsigned char> GenericStats::param_buildWorkbench()
{
    return gStats->getParam_achievement((eAward)2);
}

arrayWithLength<unsigned char> GenericStats::param_buildPickaxe()
{
    return gStats->getParam_achievement((eAward)3);
}

arrayWithLength<unsigned char> GenericStats::param_buildFurnace()
{
    return gStats->getParam_achievement((eAward)4);
}

arrayWithLength<unsigned char> GenericStats::param_acquireIron()
{
    return gStats->getParam_achievement((eAward)5);
}

arrayWithLength<unsigned char> GenericStats::param_buildHoe()
{
    return gStats->getParam_achievement((eAward)6);
}

arrayWithLength<unsigned char> GenericStats::param_makeBread()
{
    return gStats->getParam_achievement((eAward)7);
}

arrayWithLength<unsigned char> GenericStats::param_bakeCake()
{
    return gStats->getParam_achievement((eAward)8);
}

arrayWithLength<unsigned char> GenericStats::param_buildBetterPickaxe()
{
    return gStats->getParam_achievement((eAward)9);
}

arrayWithLength<unsigned char> GenericStats::param_cookFish()
{
    return gStats->getParam_achievement((eAward)10);
}

arrayWithLength<unsigned char> GenericStats::param_onARail(int value)
{
    return gStats->getParam_onARail(value);
}

arrayWithLength<unsigned char> GenericStats::param_buildSword()
{
    return gStats->getParam_achievement((eAward)12);
}

arrayWithLength<unsigned char> GenericStats::param_killEnemy()
{
    return gStats->getParam_achievement((eAward)13);
}

arrayWithLength<unsigned char> GenericStats::param_killCow()
{
    return gStats->getParam_achievement((eAward)14);
}

arrayWithLength<unsigned char> GenericStats::param_flyPig()
{
    return gStats->getParam_achievement((eAward)15);
}

arrayWithLength<unsigned char> GenericStats::param_snipeSkeleton()
{
    return gStats->getParam_achievement((eAward)26);
}

arrayWithLength<unsigned char> GenericStats::param_diamonds()
{
    return gStats->getParam_achievement((eAward)27);
}

arrayWithLength<unsigned char> GenericStats::param_ghast()
{
    return gStats->getParam_achievement((eAward)29);
}

arrayWithLength<unsigned char> GenericStats::param_blazeRod()
{
    return gStats->getParam_achievement((eAward)30);
}

arrayWithLength<unsigned char> GenericStats::param_potion()
{
    return gStats->getParam_achievement((eAward)31);
}

arrayWithLength<unsigned char> GenericStats::param_theEnd()
{
    return gStats->getParam_achievement((eAward)32);
}

arrayWithLength<unsigned char> GenericStats::param_winGame()
{
    return gStats->getParam_achievement((eAward)33);
}

arrayWithLength<unsigned char> GenericStats::param_enchantments()
{
    return gStats->getParam_achievement((eAward)34);
}

arrayWithLength<unsigned char> GenericStats::param_overkill(int value)
{
    return gStats->getParam_overkill(value);
}

arrayWithLength<unsigned char> GenericStats::param_bookcase()
{
    return gStats->getParam_achievement((eAward)36);
}

arrayWithLength<unsigned char> GenericStats::param_leaderOfThePack()
{
    return gStats->getParam_achievement((eAward)16);
}

arrayWithLength<unsigned char> GenericStats::param_MOARTools()
{
    return gStats->getParam_achievement((eAward)17);
}

arrayWithLength<unsigned char> GenericStats::param_dispenseWithThis()
{
    return gStats->getParam_achievement((eAward)18);
}

arrayWithLength<unsigned char> GenericStats::param_InToTheNether()
{
    return gStats->getParam_achievement((eAward)19);
}

arrayWithLength<unsigned char> GenericStats::param_socialPost()
{
    return gStats->getParam_achievement((eAward)25);
}

arrayWithLength<unsigned char> GenericStats::param_mine100Blocks()
{
    return gStats->getParam_achievement((eAward)20);
}

arrayWithLength<unsigned char> GenericStats::param_kill10Creepers()
{
    return gStats->getParam_achievement((eAward)21);
}

arrayWithLength<unsigned char> GenericStats::param_adventuringTime()
{
    return gStats->getParam_achievement((eAward)37);
}

arrayWithLength<unsigned char> GenericStats::param_repopulation()
{
    return gStats->getParam_achievement((eAward)38);
}

arrayWithLength<unsigned char> GenericStats::param_porkChop()
{
    return gStats->getParam_achievement((eAward)22);
}

arrayWithLength<unsigned char> GenericStats::param_diamondsToYou()
{
    return gStats->getParam_achievement((eAward)39);
}

arrayWithLength<unsigned char> GenericStats::param_passingTheTime()
{
    return gStats->getParam_achievement((eAward)23);
}

arrayWithLength<unsigned char> GenericStats::param_archer()
{
    return gStats->getParam_achievement((eAward)24);
}

arrayWithLength<unsigned char> GenericStats::param_theHaggler()
{
    return gStats->getParam_achievement((eAward)40);
}

arrayWithLength<unsigned char> GenericStats::param_potPlanter()
{
    return gStats->getParam_achievement((eAward)41);
}

arrayWithLength<unsigned char> GenericStats::param_itsASign()
{
    return gStats->getParam_achievement((eAward)42);
}

arrayWithLength<unsigned char> GenericStats::param_ironBelly()
{
    return gStats->getParam_achievement((eAward)43);
}

arrayWithLength<unsigned char> GenericStats::param_haveAShearfulDay()
{
    return gStats->getParam_achievement((eAward)44);
}

arrayWithLength<unsigned char> GenericStats::param_rainbowCollection()
{
    return gStats->getParam_achievement((eAward)45);
}

arrayWithLength<unsigned char> GenericStats::param_stayinFrosty()
{
    return gStats->getParam_achievement((eAward)46);
}

arrayWithLength<unsigned char> GenericStats::param_chestfulOfCobblestone(int value)
{
    return gStats->getParam_chestfulOfCobblestone(value);
}

arrayWithLength<unsigned char> GenericStats::param_renewableEnergy()
{
    return gStats->getParam_achievement((eAward)48);
}

arrayWithLength<unsigned char> GenericStats::param_musicToMyEars(int value)
{
    return gStats->getParam_musicToMyEars(value);
}

arrayWithLength<unsigned char> GenericStats::param_bodyGuard()
{
    return gStats->getParam_achievement((eAward)50);
}

arrayWithLength<unsigned char> GenericStats::param_ironMan()
{
    return gStats->getParam_achievement((eAward)51);
}

arrayWithLength<unsigned char> GenericStats::param_zombieDoctor()
{
    return gStats->getParam_achievement((eAward)52);
}

arrayWithLength<unsigned char> GenericStats::param_lionTamer()
{
    return gStats->getParam_achievement((eAward)53);
}

arrayWithLength<unsigned char> GenericStats::param_tieDyeOutfit(int value)
{
    return gStats->getParam_tieDyeOutfit(value);
}

arrayWithLength<unsigned char> GenericStats::param_trampoline(int value)
{
    return gStats->getParam_trampoline(value);
}

arrayWithLength<unsigned char> GenericStats::param_spawnWither()
{
    return gStats->getParam_spawnWither();
}

arrayWithLength<unsigned char> GenericStats::param_killWither()
{
    return gStats->getParam_killWither();
}

arrayWithLength<unsigned char> GenericStats::param_fullBeacon()
{
    return gStats->getParam_fullBeacon();
}

arrayWithLength<unsigned char> GenericStats::param_overpowered()
{
    return gStats->getParam_overpowered();
}

arrayWithLength<unsigned char> GenericStats::param_smeltEverything()
{
    return gStats->getParam_smeltEverything();
}

arrayWithLength<unsigned char> GenericStats::param_freightStation()
{
    return gStats->getParam_freightStation();
}

arrayWithLength<unsigned char> GenericStats::param_theStudent()
{
    return gStats->getParam_theStudent();
}

arrayWithLength<unsigned char> GenericStats::param_hasBecomeTheMaster()
{
    return gStats->getParam_hasBecomeTheMaster();
}

arrayWithLength<unsigned char> GenericStats::param_tisButAScratch()
{
    return gStats->getParam_tisButAScratch();
}

arrayWithLength<unsigned char> GenericStats::param_cupid()
{
    return gStats->getParam_cupid();
}

arrayWithLength<unsigned char> GenericStats::param_youWontLikeMeWhenImHungry()
{
    return gStats->getParam_youWontLikeMeWhenImHungry();
}

arrayWithLength<unsigned char> GenericStats::param_mine()
{
    return gStats->getParam_mine();
}

arrayWithLength<unsigned char> GenericStats::param_theEndAgain()
{
    return gStats->getParam_achievement((eAward)66);
}

arrayWithLength<unsigned char> GenericStats::param_youNeedAMint()
{
    return gStats->getParam_achievement((eAward)67);
}

arrayWithLength<unsigned char> GenericStats::param_superSonic(int value)
{
    return gStats->getParam_superSonic(value);
}

arrayWithLength<unsigned char> GenericStats::param_drySpell()
{
    return gStats->getParam_achievement((eAward)69);
}

arrayWithLength<unsigned char> GenericStats::param_freeDiver(int value)
{
    return gStats->getParam_freeDiver(value);
}

arrayWithLength<unsigned char> GenericStats::param_superFuel()
{
    return gStats->getParam_achievement((eAward)71);
}

arrayWithLength<unsigned char> GenericStats::param_saddleUp()
{
    return gStats->getParam_achievement((eAward)72);
}

arrayWithLength<unsigned char> GenericStats::param_tasteOfYourOwnMedicine()
{
    return gStats->getParam_achievement((eAward)73);
}

arrayWithLength<unsigned char> GenericStats::param_beamMeUp(int value)
{
    return gStats->getParam_beamMeUp(value);
}

arrayWithLength<unsigned char> GenericStats::param_mapRoom(int value)
{
    return gStats->getParam_mapRoom(value);
}

arrayWithLength<unsigned char> GenericStats::param_camouflage()
{
    return gStats->getParam_achievement((eAward)76);
}

arrayWithLength<unsigned char> GenericStats::param_backFromTheDead()
{
    return gStats->getParam_achievement((eAward)77);
}

arrayWithLength<unsigned char> GenericStats::param_equipYourFists()
{
    return gStats->getParam_achievement((eAward)78);
}

arrayWithLength<unsigned char> GenericStats::param_tenaciousBall(int value)
{
    return gStats->getParam_tenaciousBall(value);
}

arrayWithLength<unsigned char> GenericStats::param_deadshot()
{
    return gStats->getParam_achievement((eAward)80);
}

arrayWithLength<unsigned char> GenericStats::param_suckMySnowballs(int value)
{
    return gStats->getParam_suckMySnowballs(value);
}

arrayWithLength<unsigned char> GenericStats::param_mountainTop()
{
    return gStats->getParam_achievement((eAward)82);
}

arrayWithLength<unsigned char> GenericStats::param_willLikeNoOther()
{
    return gStats->getParam_achievement((eAward)83);
}

arrayWithLength<unsigned char> GenericStats::param_theDeepEnd()
{
    return gStats->getParam_achievement((eAward)84);
}

arrayWithLength<unsigned char> GenericStats::param_greatViewFromUpHere(int value)
{
    return gStats->getParam_greatViewFromUpHere(value);
}

arrayWithLength<unsigned char> GenericStats::param_soIGotThatGoingForMe(int value)
{
    return gStats->getParam_soIGotThatGoingForMe(value);
}

arrayWithLength<unsigned char> GenericStats::param_letItGo()
{
    return gStats->getParam_achievement((eAward)89);
}

arrayWithLength<unsigned char> GenericStats::param_feelingIll()
{
    return gStats->getParam_achievement((eAward)90);
}

arrayWithLength<unsigned char> GenericStats::param_changeOfSheets()
{
    return gStats->getParam_achievement((eAward)86);
}

arrayWithLength<unsigned char> GenericStats::param_1234Pickle()
{
    return gStats->getParam_achievement((eAward)91);
}

arrayWithLength<unsigned char> GenericStats::param_alternativeFuel()
{
    return gStats->getParam_achievement((eAward)92);
}

arrayWithLength<unsigned char> GenericStats::param_moskstraumen()
{
    return gStats->getParam_achievement((eAward)93);
}

arrayWithLength<unsigned char> GenericStats::param_castaway()
{
    return gStats->getParam_achievement((eAward)94);
}

arrayWithLength<unsigned char> GenericStats::param_sleepWithTheFishes()
{
    return gStats->getParam_achievement((eAward)95);
}

arrayWithLength<unsigned char> GenericStats::param_echolocation()
{
    return gStats->getParam_achievement((eAward)96);
}

Stat* GenericStats::get_achievement(eAward award)
{
    return NULL;
}

Stat* GenericStats::get_walkOneM()
{
    return NULL;
}

Stat* GenericStats::get_swimOneM()
{
    return NULL;
}

Stat* GenericStats::get_fallOneM()
{
    return NULL;
}

Stat* GenericStats::get_climbOneM()
{
    return NULL;
}

Stat* GenericStats::get_minecartOneM()
{
    return NULL;
}

Stat* GenericStats::get_boatOneM()
{
    return NULL;
}

Stat* GenericStats::get_pigOneM()
{
    return NULL;
}

Stat* GenericStats::get_portalsCreated()
{
    return NULL;
}

Stat* GenericStats::get_cowsMilked()
{
    return NULL;
}

Stat* GenericStats::get_netherLavaCollected()
{
    return NULL;
}

Stat* GenericStats::get_killMob()
{
    return NULL;
}

Stat* GenericStats::get_killsZombie()
{
    return NULL;
}

Stat* GenericStats::get_killsSkeleton()
{
    return NULL;
}

Stat* GenericStats::get_killsCreeper()
{
    return NULL;
}

Stat* GenericStats::get_killsSpider()
{
    return NULL;
}

Stat* GenericStats::get_killsSpiderJockey()
{
    return NULL;
}

Stat* GenericStats::get_killsZombiePigman()
{
    return NULL;
}

Stat* GenericStats::get_killsSlime()
{
    return NULL;
}

Stat* GenericStats::get_killsGhast()
{
    return NULL;
}

Stat* GenericStats::get_killsNetherZombiePigman()
{
    return NULL;
}

Stat* GenericStats::get_breedEntity(unsigned long long entityType)
{
    return NULL;
}

Stat* GenericStats::get_tamedEntity(unsigned long long entityType)
{
    return NULL;
}

Stat* GenericStats::get_curedEntity(unsigned long long entityType)
{
    return NULL;
}

Stat* GenericStats::get_craftedEntity(unsigned long long entityType)
{
    return NULL;
}

Stat* GenericStats::get_shearedEntity(unsigned long long entityType)
{
    return NULL;
}

Stat* GenericStats::get_totalBlocksMined()
{
    return NULL;
}

Stat* GenericStats::get_timePlayed()
{
    return NULL;
}

Stat* GenericStats::get_blocksPlaced(int value)
{
    return NULL;
}

Stat* GenericStats::get_blocksMined(int value)
{
    return NULL;
}

Stat* GenericStats::get_itemsCollected(int value, int auxValue)
{
    return NULL;
}

Stat* GenericStats::get_itemsCrafted(int value)
{
    return NULL;
}

Stat* GenericStats::get_itemsSmelted(int value)
{
    return get_itemsCrafted(value);
}

Stat* GenericStats::get_itemsUsed(int value)
{
    return NULL;
}

Stat* GenericStats::get_itemsBought(int value)
{
    return NULL;
}

Stat* GenericStats::get_armourDyed(int value)
{
    return NULL;
}

Stat* GenericStats::get_killsEnderdragon()
{
    return NULL;
}

Stat* GenericStats::get_completeTheEnd()
{
    return NULL;
}

Stat* GenericStats::get_changedDimension(int value, int auxValue)
{
    return NULL;
}

Stat* GenericStats::get_enteredBiome(int value)
{
    return NULL;
}

Stat* GenericStats::get_MG03_Score()
{
    return NULL;
}

Stat* GenericStats::get_MG03_RingCount()
{
    return NULL;
}

Stat* GenericStats::get_MG03_BestTimeScoreChallenge()
{
    return NULL;
}

Stat* GenericStats::get_MG03_BestTime()
{
    return NULL;
}

arrayWithLength<unsigned char> GenericStats::getParam_walkOneM(int value)
{
    return getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::getParam_swimOneM(int value)
{
    return getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::getParam_fallOneM(int value)
{
    return getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::getParam_climbOneM(int value)
{
    return getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::getParam_minecartOneM(int value)
{
    return getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::getParam_boatOneM(int value)
{
    return getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::getParam_pigOneM(int value)
{
    return getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::getParam_cowsMilked()
{
    return getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::getParam_blocksPlaced(int value, int auxValue, int count)
{
    return getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::getParam_blocksMined(int value, int auxValue, int count)
{
    return getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::getParam_itemsCollected(int value, int auxValue, int count)
{
    return getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::getParam_itemsCrafted(int value, int auxValue, int count)
{
    return getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::getParam_itemsSmelted(int value, int auxValue, int count)
{
    return getParam_itemsCrafted(value, auxValue, count);
}

arrayWithLength<unsigned char> GenericStats::getParam_itemsUsed(boost::shared_ptr<Player> player, not_null_ptr<ItemInstance> itemInstance)
{
    return getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::getParam_itemsBought(int value, int auxValue, int count)
{
    return getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::getParam_mobKill(boost::shared_ptr<Player> player, boost::shared_ptr<Mob> mob, const DamageSource* damageSource)
{
    return getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::getParam_breedEntity(unsigned long long entityType)
{
    return getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::getParam_tamedEntity(unsigned long long entityType)
{
    return getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::getParam_curedEntity(unsigned long long entityType)
{
    return getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::getParam_craftedEntity(unsigned long long entityType)
{
    return getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::getParam_shearedEntity(unsigned long long entityType)
{
    return getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::getParam_time(int value)
{
    return getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::getParam_changedDimension(int value, int auxValue)
{
    return getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::getParam_enteredBiome(int value)
{
    return getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::getParam_achievement(eAward award)
{
    return getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::getParam_onARail(int value)
{
    return getParam_achievement((eAward)11);
}

arrayWithLength<unsigned char> GenericStats::getParam_overkill(int value)
{
    return getParam_achievement((eAward)35);
}

arrayWithLength<unsigned char> GenericStats::getParam_openInventory(int value)
{
    return getParam_achievement((eAward)0);
}

arrayWithLength<unsigned char> GenericStats::getParam_chestfulOfCobblestone(int value)
{
    return getParam_achievement((eAward)47);
}

arrayWithLength<unsigned char> GenericStats::getParam_musicToMyEars(int value)
{
    return getParam_achievement((eAward)49);
}

arrayWithLength<unsigned char> GenericStats::getParam_MG03Score(int value)
{
    return getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::getParam_MG03RingCount(int value)
{
    return getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::getParam_MG03BestTime(unsigned int value)
{
    return getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::getParam_tieDyeOutfit(int value)
{
    return getParam_achievement((eAward)58);
}

arrayWithLength<unsigned char> GenericStats::getParam_trampoline(int value)
{
    return getParam_achievement((eAward)59);
}

arrayWithLength<unsigned char> GenericStats::getParam_spawnWither()
{
    return getParam_achievement((eAward)54);
}

arrayWithLength<unsigned char> GenericStats::getParam_killWither()
{
    return getParam_achievement((eAward)55);
}

arrayWithLength<unsigned char> GenericStats::getParam_fullBeacon()
{
    return getParam_achievement((eAward)56);
}

arrayWithLength<unsigned char> GenericStats::getParam_overpowered()
{
    return getParam_achievement((eAward)57);
}

arrayWithLength<unsigned char> GenericStats::getParam_smeltEverything()
{
    return gStats->getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::getParam_camouflage()
{
    return gStats->getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::getParam_freightStation()
{
    return gStats->getParam_noArgs();
}

arrayWithLength<unsigned char> GenericStats::getParam_theStudent()
{
    return getParam_achievement((eAward)60);
}

arrayWithLength<unsigned char> GenericStats::getParam_hasBecomeTheMaster()
{
    return getParam_achievement((eAward)61);
}

arrayWithLength<unsigned char> GenericStats::getParam_tisButAScratch()
{
    return getParam_achievement((eAward)62);
}

arrayWithLength<unsigned char> GenericStats::getParam_cupid()
{
    return getParam_achievement((eAward)63);
}

arrayWithLength<unsigned char> GenericStats::getParam_youWontLikeMeWhenImHungry()
{
    return getParam_achievement((eAward)64);
}

arrayWithLength<unsigned char> GenericStats::getParam_mine()
{
    return getParam_achievement((eAward)65);
}

arrayWithLength<unsigned char> GenericStats::getParam_superSonic(int value)
{
    return getParam_achievement((eAward)68);
}

arrayWithLength<unsigned char> GenericStats::getParam_freeDiver(int value)
{
    return getParam_achievement((eAward)70);
}

arrayWithLength<unsigned char> GenericStats::getParam_beamMeUp(int value)
{
    return getParam_achievement((eAward)74);
}

arrayWithLength<unsigned char> GenericStats::getParam_mapRoom(int value)
{
    return getParam_achievement((eAward)75);
}

arrayWithLength<unsigned char> GenericStats::getParam_tenaciousBall(int value)
{
    return getParam_achievement((eAward)79);
}

arrayWithLength<unsigned char> GenericStats::getParam_suckMySnowballs(int value)
{
    return getParam_achievement((eAward)81);
}

arrayWithLength<unsigned char> GenericStats::getParam_greatViewFromUpHere(int value)
{
    return getParam_achievement((eAward)85);
}

arrayWithLength<unsigned char> GenericStats::getParam_soIGotThatGoingForMe(int value)
{
    return getParam_achievement((eAward)88);
}

arrayWithLength<unsigned char> GenericStats::getParam_noArgs()
{
    return arrayWithLength<unsigned char>();
}

Stat* GenericStats::itemsCollected(not_null_ptr<ItemInstance> itemInstance)
{
    return gStats->get_itemsCollected(Item::VALIDgetId(itemInstance->getItem()), itemInstance->getAuxValue());
}

arrayWithLength<unsigned char> GenericStats::param_itemsCollected(not_null_ptr<ItemInstance> itemInstance)
{
    return param_itemsCollected(itemInstance->getItem(), itemInstance->getAuxValue(), itemInstance->getCount());
}

arrayWithLength<unsigned char> GenericStats::param_itemsBought(not_null_ptr<ItemInstance> itemInstance)
{
    return param_itemsBought(itemInstance->getItem(), itemInstance->getAuxValue(), itemInstance->getCount());
}
