#include "game/minigame/MiniGameMedals.hpp"

#include <cfloat>
#include <cstddef>
#include <vector>

std::vector<const GameMedal*> gAllMiniGameMedals;
std::vector<const GameMedal*> gBattleMedals;
std::vector<const GameMedal*> gTumbleMedals;
std::vector<const GameMedal*> gGlideMedals;
std::vector<const GameMedal*> gBuildMedals;

int gBattleMedalCount;
int gTumbleMedalCount;
int gGlideMedalCount;
int gBuildMedalCount;

void MiniGameMedals::staticCtor()
{
    int nextMedalId = 0;

    MedalAdder battleMedals(&nextMedalId, &gBattleMedals, &gAllMiniGameMedals);

    battleMedals.addMedal(new MostKillsMedal(0x373F0289, 1.0f, 125, GameMedal::OPERATOR_AT_LEAST, true, false));
    battleMedals.addMedal(new LeastKillsMedal(0xE148AA03, 15.0f, 25, GameMedal::OPERATOR_AT_LEAST, true, false));
    battleMedals.addMedal(new SniperMedal(0xFA877DCF, 15.0f, 625));
    battleMedals.addMedal(new AmphibiousMedal(0x5BA330E9, 1.0f, 3125, GameMedal::OPERATOR_AT_LEAST, true, false));
    battleMedals.addMedal((new FallKillMedal(0x3990407B, 1.0f, 3125))->SetCanMultiAward(true));
    battleMedals.addMedal(new WeaponMedal(0x46558545, 10.0f, 125, (Item*)NULL));
    battleMedals.addMedal(new DamageDealtMedal(0x02F32BA3, 20.0f, 125, GameMedal::OPERATOR_AT_LEAST, true, false));
    battleMedals.addMedal(new DamageDealtMedal(0x0CF03A49, 10.0f, 25, GameMedal::OPERATOR_AT_MOST, true, false));
    battleMedals.addMedal(new WeaponMedal(0xAAF401B3, 10.0f, 125, 1));
    battleMedals.addMedal(new WeaponMedal(0x9FDE2D4C, 10.0f, 125, Items::bow));

    int explosiveWeaponIds[] = { 10, 8, 7, 9 };
    battleMedals.addMedal(new WeaponMedal(0x86DFA3A0, 3.0f, 625, new std::vector<int>(explosiveWeaponIds, explosiveWeaponIds + 4)));

    Scoreboard::EDamageSource directDamageSources[] = { Scoreboard::DAMAGE_SOURCE_ARROW, Scoreboard::DAMAGE_SOURCE_MELEE };
    battleMedals.addMedal(new DamageSourceMedal(0x118D7CB1, 10.0f, 625, new std::vector<Scoreboard::EDamageSource>(directDamageSources, directDamageSources + 2)));

    battleMedals.addMedal(new DamageSourceMedal(0x9B18B91B, 5.0f, 125, Scoreboard::DAMAGE_SOURCE_EXPLOSION));
    battleMedals.addMedal(new DamageSourceMedal(0xA847BB24, 5.0f, 125, Scoreboard::DAMAGE_SOURCE_FIRE));
    battleMedals.addMedal(new DamageSourceMedal(0xEDE3A3FA, 10.0f, 25, Scoreboard::DAMAGE_SOURCE_FALL));
    battleMedals.addMedal(new TankMedal(0x3F27FDC6, 40.0f, 625));
    battleMedals.addMedal(new FoodMedal(0x69158380, 2.0f, 125, (Item*)NULL));

    Item* fishFoods[] = { Items::cookedFish, Items::fish };
    battleMedals.addMedal(new FoodMedal(0x481026AB, 2.0f, 125, new std::vector<Item*>(fishFoods, fishFoods + 2)));

    battleMedals.addMedal(new PotionMedal(0xE04899EF, 2.0f, 125, GameMedal::OPERATOR_AT_LEAST, true, false));
    battleMedals.addMedal(new FoodMedal(0x777DF067, 2.0f, 125, Items::bread));

    Item* meatFoods[] =
    {
        Items::cookedBeef,
        Items::beef,
        Items::cookedPorkchop,
        Items::porkchop,
        Items::cookedChicken,
        Items::chicken
    };
    battleMedals.addMedal(new FoodMedal(0x410B9DD2, 2.0f, 125, new std::vector<Item*>(meatFoods, meatFoods + 6)));

    battleMedals.addMedal(new HoarderMedal(0xBB274286, 5.0f, 125, GameMedal::OPERATOR_AT_LEAST, true, false));
    battleMedals.addMedal(new TreasureHunterMedal(0x483C96F0, 3.0f, 125, GameMedal::OPERATOR_AT_LEAST, true, false));
    battleMedals.addMedal(new SplashPotionMedal(0x02C0E252, 1.0f, 125, GameMedal::OPERATOR_AT_LEAST, true, false));
    battleMedals.addMedal(new SneakingMedal(0x8A6A9582, 2.0f, 25, GameMedal::OPERATOR_AT_LEAST, true, false));
    battleMedals.addMedal(new UnderwaterMedal(0xE268EF55, 10.0f, 25, GameMedal::OPERATOR_AT_LEAST, true, false));
    battleMedals.addMedal(new DistanceMedal(0x15F4E5E1, 100.0f, 25, GameMedal::OPERATOR_AT_LEAST, true, false));
    battleMedals.addMedal(new ClimbingMedal(0xF14B91A1, 10.0f, 25, GameMedal::OPERATOR_AT_LEAST, true, false));
    battleMedals.addMedal(new DistanceMedal(0x8AF1D99B, 100.0f, 25, GameMedal::OPERATOR_AT_MOST, true, false));
    battleMedals.addMedal(new LifeSpanMedal(0xBF5D3CB3, 120.0f, 25, GameMedal::OPERATOR_AT_MOST));
    battleMedals.addMedal((new ConsolationMedal(0xB377B243, 0.0f, 1, GameMedal::OPERATOR_AT_LEAST))->SetCanMultiAward(true));
    battleMedals.addMedal((new ConsolationMedal(0x41704308, 0.0f, 1, GameMedal::OPERATOR_AT_LEAST))->SetCanMultiAward(true));
    battleMedals.addMedal((new ConsolationMedal(0x6772BD71, 0.0f, 1, GameMedal::OPERATOR_AT_LEAST))->SetCanMultiAward(true));
    battleMedals.addMedal((new ConsolationMedal(0x257F217E, 0.0f, 1, GameMedal::OPERATOR_AT_LEAST))->SetCanMultiAward(true));
    battleMedals.addMedal((new ConsolationMedal(0x4B819BE7, 0.0f, 1, GameMedal::OPERATOR_AT_LEAST))->SetCanMultiAward(true));

    MedalAdder tumbleMedals(&nextMedalId, &gTumbleMedals, &gAllMiniGameMedals);

    tumbleMedals.addMedal(new BlockDestroyedMedal(0x2BB44FEE, 0.0f, 125, GameMedal::OPERATOR_AT_LEAST));
    tumbleMedals.addMedal(new BlockDestroyedMedal(0xDF79BDD4, 100.0f, 125, GameMedal::OPERATOR_AT_MOST));
    tumbleMedals.addMedal(new ProjectileDirectHitMedal(0x70D4F647, 5.0f, 125, GameMedal::OPERATOR_AT_LEAST, true, false));
    tumbleMedals.addMedal(new HitsFromProjectileMedal(0xDBB3DF9C, 25.0f, 125, GameMedal::OPERATOR_AT_MOST, true, false));
    tumbleMedals.addMedal((new WinnerWithoutBlockDestructionMedal(0x8A2B0AC2, 1.0f, 625, GameMedal::OPERATOR_AT_LEAST, true, false))->SetCanMultiAward(true));
    tumbleMedals.addMedal(new PodiumPlaceMostBlocksDestroyedMedal(PodiumPlaceMostBlocksDestroyedMedal::PODIUM_FIRST, 0x03AD6CD5, 1.0f, 625, GameMedal::OPERATOR_AT_LEAST));
    tumbleMedals.addMedal(new PodiumPlaceMostBlocksDestroyedMedal(PodiumPlaceMostBlocksDestroyedMedal::PODIUM_SECOND, 0xDA364E83, 1.0f, 625, GameMedal::OPERATOR_AT_LEAST));
    tumbleMedals.addMedal(new PodiumPlaceMostBlocksDestroyedMedal(PodiumPlaceMostBlocksDestroyedMedal::PODIUM_THIRD, 0x2493FAEE, 1.0f, 625, GameMedal::OPERATOR_AT_LEAST));
    tumbleMedals.addMedal(new JumpsMedal(0xDE128049, 5.0f, 125, GameMedal::OPERATOR_AT_LEAST, true, false));
    tumbleMedals.addMedal((new FirstDeathWithoutBlockDestructionMedal(0xB64A8AB1, 1.0f, 125, GameMedal::OPERATOR_AT_LEAST, true, false))->SetCanMultiAward(true));
    tumbleMedals.addMedal(new LongWayDownMedal(0x57745C04, 1.0f, 125, GameMedal::OPERATOR_AT_LEAST, true, false));
    tumbleMedals.addMedal(new TimeSpentOnLayerMedal(TimeSpentOnLayerMedal::LAYER_MEDAL_TOP, 0x75F9EAFC, 20.0f, 625, GameMedal::OPERATOR_AT_LEAST));
    tumbleMedals.addMedal(new TimeSpentOnLayerMedal(TimeSpentOnLayerMedal::LAYER_MEDAL_BOTTOM, 0xFDF26520, 20.0f, 625, GameMedal::OPERATOR_AT_LEAST));
    tumbleMedals.addMedal(new LifeSpanMedal(0x415F73D7, 30.0f, 25, GameMedal::OPERATOR_AT_LEAST));
    tumbleMedals.addMedal(new LifeSpanMedal(0xD01D07DF, 300.0f, 125, GameMedal::OPERATOR_AT_MOST));
    tumbleMedals.addMedal(new BlocksDestroyedFromBelowMedal(0x18F9F129, 5.0f, 3125, GameMedal::OPERATOR_AT_LEAST, true, false));
    tumbleMedals.addMedal((new WinningOnTopLayerMedal(0x8C688A64, 1.0f, 625, GameMedal::OPERATOR_AT_LEAST, true, false))->SetCanMultiAward(true));
    tumbleMedals.addMedal(new MissedSnowballsMedal(0xD5E3FB2E, 20.0f, 625, GameMedal::OPERATOR_AT_LEAST, true, false));
    tumbleMedals.addMedal(new FirstToDieMedal(0xE75F245D, 0.9f, 125));
    tumbleMedals.addMedal(new ImprovementMedal(0x3179D63E, 1.0f, 3125));
    tumbleMedals.addMedal(new FirstToBottomMedal(0xD3DA7ABA, 1.0f, 125, GameMedal::OPERATOR_AT_LEAST, true, false));
    tumbleMedals.addMedal(new TimeInLayerCentreMedal(0xC507906D, 10.0f, 25, GameMedal::OPERATOR_AT_LEAST, true, false));
    tumbleMedals.addMedal(new TimeInLayerCentreMedal(0xCC2971F7, 150.0f, 25, GameMedal::OPERATOR_AT_MOST, true, false));
    tumbleMedals.addMedal(new FirstToBottomAndWonMedal(0xBD402445, 1.0f, 3125, GameMedal::OPERATOR_AT_LEAST, true, false));
    tumbleMedals.addMedal((new ConsolationMedal(0xD97A2CAC, 0.0f, 1, GameMedal::OPERATOR_AT_LEAST))->SetCanMultiAward(true));
    tumbleMedals.addMedal((new ConsolationMedal(0xFF7CA715, 0.0f, 1, GameMedal::OPERATOR_AT_LEAST))->SetCanMultiAward(true));
    tumbleMedals.addMedal(new DistanceMedal(0x15F4E5E1, 25.0f, 25, GameMedal::OPERATOR_AT_LEAST, true, false));
    tumbleMedals.addMedal(new DistanceMedal(0x8AF1D99B, 100.0f, 25, GameMedal::OPERATOR_AT_MOST, true, false));
    tumbleMedals.addMedal(new SneakingMedal(0x8A6A9582, 2.0f, 25, GameMedal::OPERATOR_AT_LEAST, true, false));

    MedalAdder glideMedals(&nextMedalId, &gGlideMedals, &gAllMiniGameMedals);

    glideMedals.addMedal((new MultiConditionMedal(0xFDC49843, 25, new TotalDeathsCondition(1.0f, GameMedal::OPERATOR_AT_LEAST, true)))
        ->addExtraCondition(new PlayerCountCondition(2.0f, GameMedal::OPERATOR_AT_LEAST)));
    glideMedals.addMedal((new MultiConditionMedal(0x79CA8ED4, 625, new TotalDeathsCondition(1.0f, GameMedal::OPERATOR_AT_LEAST, true)))
        ->addExtraCondition(new WinnerCondition())
        ->addExtraCondition(new PlayerCountCondition(2.0f, GameMedal::OPERATOR_AT_LEAST)));
    glideMedals.addMedal(((new MultiConditionMedal(0x98BCFDC3, 625, new TotalPointsCondition(1.0f, GameMedal::OPERATOR_AT_LEAST, true)))
        ->addExtraCondition(new VariantCondition(MINIGAME_VARIANT_SCORE_ATTACK))
        ->addExtraCondition(new PlayerCountCondition(2.0f, GameMedal::OPERATOR_AT_LEAST)))
        ->SetCanMultiAward(true));
    glideMedals.addMedal((new MultiConditionMedal(0xEAA09456, 625, new HasSpawnedAtAllCheckpointsCondition()))->SetCanMultiAward(true));
    glideMedals.addMedal((new MultiConditionMedal(0x48AAC91F, 125, new TotalDamageTakenCondition(10.0f, GameMedal::OPERATOR_AT_MOST, true)))
        ->addExtraCondition(new PlayerCountCondition(2.0f, GameMedal::OPERATOR_AT_LEAST)));
    glideMedals.addMedal((new MultiConditionMedal(0x01B69514, 25, new TotalThermalsEnteredCondition(0.0f, GameMedal::OPERATOR_AT_MOST, true)))
        ->addExtraCondition(new PlayerCountCondition(2.0f, GameMedal::OPERATOR_AT_LEAST)));
    glideMedals.addMedal((new MultiConditionMedal(0xC6338D98, 625, new TotalDamageTakenCondition(10.0f, GameMedal::OPERATOR_AT_MOST, true)))
        ->addExtraCondition(new WinnerCondition())
        ->addExtraCondition(new PlayerCountCondition(2.0f, GameMedal::OPERATOR_AT_LEAST)));
    glideMedals.addMedal((new MultiConditionMedal(0xE4CE5068, 3125, new FinishCourseWithDamageTakenCondition(0.0f)))->SetCanMultiAward(true));
    glideMedals.addMedal((new MultiConditionMedal(0x5C9FCE59, 625, new FinishCourseWithDamageTakenCondition(1.0f)))->SetCanMultiAward(true));
    glideMedals.addMedal((new MultiConditionMedal(0xB87442AA, 625, new FinishCourseWithDamageTakenCondition(2.0f)))->SetCanMultiAward(true));

    RingsMedal::eRingMedalAmount allSmallRingAmounts[] =
    {
        RingsMedal::eRingAmountAll,
        RingsMedal::eRingAmountAll,
        RingsMedal::eRingAmountSome
    };
    glideMedals.addMedal((new RingsMedal(0xDC31EA7F, 125, allSmallRingAmounts, GameMedal::OPERATOR_AT_LEAST))->SetCanMultiAward(true));

    RingsMedal::eRingMedalAmount allMediumRingAmounts[] =
    {
        RingsMedal::eRingAmountAll,
        RingsMedal::eRingAmountSome,
        RingsMedal::eRingAmountAll
    };
    glideMedals.addMedal((new RingsMedal(0xCC9401CF, 625, allMediumRingAmounts, GameMedal::OPERATOR_AT_LEAST))->SetCanMultiAward(true));

    RingsMedal::eRingMedalAmount allLargeRingAmounts[] =
    {
        RingsMedal::eRingAmountSome,
        RingsMedal::eRingAmountAll,
        RingsMedal::eRingAmountAll
    };
    glideMedals.addMedal((new RingsMedal(0x3B1C2825, 3125, allLargeRingAmounts, GameMedal::OPERATOR_AT_LEAST))->SetCanMultiAward(true));

    RingsMedal::eRingMedalAmount mostLargeRingAmounts[] =
    {
        RingsMedal::eRingAmountNone,
        RingsMedal::eRingAmountNone,
        RingsMedal::eRingAmountMost
    };
    glideMedals.addMedal((new RingsMedal(0xDA6B1EBC, 625, mostLargeRingAmounts, GameMedal::OPERATOR_AT_LEAST))->SetCanMultiAward(true));

    RingsMedal::eRingMedalAmount noRingAmounts[] =
    {
        RingsMedal::eRingAmountNone,
        RingsMedal::eRingAmountNone,
        RingsMedal::eRingAmountNone
    };
    glideMedals.addMedal((new RingsMedal(0x1B041C00, 125, noRingAmounts, GameMedal::OPERATOR_AT_LEAST))->SetCanMultiAward(true));

    glideMedals.addMedal(((new MultiConditionMedal(0x6B80D29C, 625, new ReachEndTimeCondition(FLT_MAX, true, GameMedal::OPERATOR_AT_MOST)))
        ->addExtraCondition(new PlayerCountCondition(2.0f, GameMedal::OPERATOR_AT_LEAST)))
        ->SetCanMultiAward(true));
    glideMedals.addMedal((new MultiConditionMedal(0x0932AB28, 125, new TopSpeedCondition(0.0f, GameMedal::OPERATOR_AT_LEAST)))
        ->addExtraCondition(new PlayerCountCondition(2.0f, GameMedal::OPERATOR_AT_LEAST)));
    glideMedals.addMedal(((new MultiConditionMedal(0x47708614, 25, new ImprovedCondition(1.0f, GameMedal::OPERATOR_AT_LEAST)))
        ->addExtraCondition(new FinishedLastRoundCondition()))
        ->SetCanMultiAward(true));
    glideMedals.addMedal((new MultiConditionMedal(0x6EDA42F4, 25, new TimeSpentCondition(1.0f, eTimeSpentBoosting, GameMedal::OPERATOR_AT_LEAST, true)))
        ->addExtraCondition(new PlayerCountCondition(2.0f, GameMedal::OPERATOR_AT_LEAST)));
    glideMedals.addMedal((new MultiConditionMedal(0xC9E72AE0, 25, new TimeSpentCondition(1000.0f, eTimeSpentGliding, GameMedal::OPERATOR_AT_LEAST, true)))
        ->addExtraCondition(new PlayerCountCondition(2.0f, GameMedal::OPERATOR_AT_LEAST)));
    glideMedals.addMedal((new MultiConditionMedal(0x920E2221, 25, new TimeSpentCondition(1.0f, eTimeSpentInThermal, GameMedal::OPERATOR_AT_LEAST, true)))
        ->addExtraCondition(new PlayerCountCondition(2.0f, GameMedal::OPERATOR_AT_LEAST)));
    glideMedals.addMedal((new MultiConditionMedal(0xC8D305C9, 25, new TimeSpentCondition(1.0f, eTimeSpentCloseToScenery, GameMedal::OPERATOR_AT_LEAST, true)))
        ->addExtraCondition(new PlayerCountCondition(2.0f, GameMedal::OPERATOR_AT_LEAST)));
    glideMedals.addMedal((new MultiConditionMedal(0x89F55709, 3125, new LastInFirstRoundCondition()))
        ->addExtraCondition(new WinnerCondition())
        ->addExtraCondition(new PlayerCountCondition(2.0f, GameMedal::OPERATOR_AT_LEAST)));
    glideMedals.addMedal(((new MultiConditionMedal(0x18A608DA, 625, new FastestTimeAndHightestScoreInARoundCondition()))
        ->addExtraCondition(new PlayerCountCondition(2.0f, GameMedal::OPERATOR_AT_LEAST))
        ->addExtraCondition(new VariantCondition(MINIGAME_VARIANT_SCORE_ATTACK)))
        ->SetCanMultiAward(true));
    glideMedals.addMedal(((new MultiConditionMedal(0xE27A7A72, 3125, new WonByAmountCondition(1000.0f, GameMedal::OPERATOR_AT_MOST)))
        ->addExtraCondition(new PlayerCountCondition(2.0f, GameMedal::OPERATOR_AT_LEAST)))
        ->SetCanMultiAward(true));
    glideMedals.addMedal(((new MultiConditionMedal(0xC9D3B088, 625, new WonByAmountCondition(10000.0f, GameMedal::OPERATOR_AT_LEAST)))
        ->addExtraCondition(new PlayerCountCondition(2.0f, GameMedal::OPERATOR_AT_LEAST)))
        ->SetCanMultiAward(true));
    glideMedals.addMedal((new MultiConditionMedal(0x13F33157, 125, new TimeSpentCondition(1000.0f, eTimeSpentGliding, GameMedal::OPERATOR_AT_LEAST, true)))
        ->addExtraCondition(new PlayerCountCondition(2.0f, GameMedal::OPERATOR_AT_LEAST))
        ->addExtraCondition(new WinnerCondition()));
    glideMedals.addMedal((new MultiConditionMedal(0xADD38DE6, 125, new TimeSpentCondition(15.0f, eTimeSpentInFirstPlace, GameMedal::OPERATOR_AT_LEAST, true)))
        ->addExtraCondition(new PlayerCountCondition(2.0f, GameMedal::OPERATOR_AT_LEAST))
        ->addExtraCondition(new NotWinnerCondition()));
    glideMedals.addMedal((new MultiConditionMedal(0xAB7835CB, 625, new TimeSpentCondition(15.0f, eTimeSpentInLastPlace, GameMedal::OPERATOR_AT_LEAST, true)))
        ->addExtraCondition(new PlayerCountCondition(2.0f, GameMedal::OPERATOR_AT_LEAST))
        ->addExtraCondition(new WinnerCondition()));
    glideMedals.addMedal((new MultiConditionMedal(0x8FDDCE64, 25, new NotFastestTimeButWonAnywayCondition()))
        ->addExtraCondition(new VariantCondition(MINIGAME_VARIANT_SCORE_ATTACK)));
    glideMedals.addMedal((new ConsolationMedal(0x45C19F25, 0.0f, 1, GameMedal::OPERATOR_AT_LEAST))->SetCanMultiAward(true));
    glideMedals.addMedal((new ConsolationMedal(0x6BC4198E, 0.0f, 1, GameMedal::OPERATOR_AT_LEAST))->SetCanMultiAward(true));
    glideMedals.addMedal((new ConsolationMedal(0x91C693F7, 0.0f, 1, GameMedal::OPERATOR_AT_LEAST))->SetCanMultiAward(true));
    glideMedals.addMedal((new ConsolationMedal(0x87B53B18, 0.0f, 1, GameMedal::OPERATOR_AT_LEAST))->SetCanMultiAward(true));

    MedalAdder buildMedals(&nextMedalId, &gBuildMedals, &gAllMiniGameMedals);

    buildMedals.addMedal(new BlockPlacedOverTime(0x777DF067, 20.0f, 125, BlockPlacedOverTime::START_DELAY, GameMedal::OPERATOR_AT_LEAST));
    buildMedals.addMedal(new BlockPlacedOverTime(0x777DF067, 10.0f, 125, BlockPlacedOverTime::IN_FIRST_10SECS, GameMedal::OPERATOR_AT_LEAST));
    buildMedals.addMedal(new BlockPlacedOverTime(0x777DF067, 20.0f, 125, BlockPlacedOverTime::TIME_LEFTOVER, GameMedal::OPERATOR_AT_LEAST));
    buildMedals.addMedal(new BlockPlacedOverTime(0x777DF067, 10.0f, 125, BlockPlacedOverTime::IN_LAST_10SECS, GameMedal::OPERATOR_AT_LEAST));
    buildMedals.addMedal(new BlockPlacedMedal(0x777DF067, 10.0f, 625, GameMedal::OPERATOR_AT_MOST));
    buildMedals.addMedal(new BlockPlacedMedal(0x777DF067, 100.0f, 125, GameMedal::OPERATOR_AT_LEAST));

    gBattleMedalCount = gBattleMedals.size();
    gTumbleMedalCount = gTumbleMedals.size();
    gGlideMedalCount = gGlideMedals.size();
    gBuildMedalCount = gBuildMedals.size();
}

const GameMedal* MiniGameMedals::getMedalById(int medalId)
{
    if (medalId >= 0 && (unsigned int)medalId < gAllMiniGameMedals.size())
    {
        return gAllMiniGameMedals.at(medalId);
    }

    return NULL;
}
