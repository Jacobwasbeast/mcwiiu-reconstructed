#include "game/minigame/FullTutorial.hpp"

#include "game/core/AreaConstraint.hpp"
#include "game/core/AreaHint.hpp"
#include "game/core/AreaTask.hpp"
#include "game/core/ChangeStateConstraint.hpp"
#include "game/core/ChoiceTask.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/ControllerTask.hpp"
#include "game/core/GameModeConstraints.hpp"
#include "game/core/GameType.hpp"
#include "game/core/InfoTask.hpp"
#include "game/core/PickupTask.hpp"
#include "game/core/ProcedureCompoundTask.hpp"
#include "game/core/ProgressFlagTask.hpp"
#include "game/core/StateChangeTask.hpp"
#include "game/item/CompleteUsingItemTask.hpp"
#include "game/item/CraftTask.hpp"
#include "game/item/Item.hpp"
#include "game/item/Items.hpp"
#include "game/item/PopulateContainerRuleDefinition.hpp"
#include "game/item/UseItemTask.hpp"
#include "game/minigame/FullTutorialActiveTask.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/ui/UICraftingTask.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelRuleset.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/UseBlockTask.hpp"
#include "game/util/Vec3.hpp"

FullTutorial::FullTutorial(int playerId, GameModeConstraints* constraints, bool enabled)
    : Tutorial(playerId, constraints, true, false), mEnabledForPlayer(enabled),
      mTutorialStarted(false)
{
    int i;
    for (i = 0; i < 44; ++i)
    {
        mStateCompleted[i] = 0;
    }
    overridesGameHostOptions = true;
    addMessage(0x857669aa, false, 3);

    if (!mEnabledForPlayer && getCompleted(0x183) == 0)
    {
        addTask(static_cast<eTutorial_State>(0),
                new InfoTask(this, 0x989a3f76, 0x3f9463f, true, 0,
                             static_cast<ETelemetryChallenges>(0)));
    }
    else
    {
        addTask(static_cast<eTutorial_State>(0),
                new ChoiceTask(this, 0x989a3f76, 0xdf59fc3f, true, 0, 1,
                               static_cast<eTutorial_CompletionAction>(3),
                               static_cast<ETelemetryChallenges>(1)));
    }

    int controllerMappingsA[4] = { 30, 31, 28, 29 };
    int controllerCompletionIdsA[4] = { 10, 9, 6, 5 };
    int controllerMappingsB[4] = { 24, 25, 26, 27 };

    addTask(static_cast<eTutorial_State>(0),
            new ControllerTask(this, 0x68129f42, false, false, controllerMappingsA, 4,
                                controllerCompletionIdsA, 4, controllerMappingsB, 4));
    addTask(static_cast<eTutorial_State>(0),
            new ControllerTask(this, 0xd6b36624, false, false, controllerMappingsB, 4,
                                controllerCompletionIdsA, 4, controllerMappingsA, 4));
    addTask(static_cast<eTutorial_State>(0),
            new InfoTask(this, 0x24614e1f, 0x3f9463f, true, 0,
                         static_cast<ETelemetryChallenges>(0)));

    int mappingSingle32[1] = { 32 };
    addTask(static_cast<eTutorial_State>(0),
            new ControllerTask(this, 0xe21ad07f, false, true, mappingSingle32, 1, NULL, 0, NULL,
                                0));
    int mappingSingle34[1] = { 34 };
    addTask(static_cast<eTutorial_State>(0),
            new ControllerTask(this, 0xfa88ce3a, false, true, mappingSingle34, 1, NULL, 0, NULL,
                                0));

    Item* logPickupItems[8] = {
        Item::byBlock(Blocks::log), Item::byBlock(Blocks::log2),
        Item::byBlock(Blocks::stripped_log_dark_oak), Item::byBlock(Blocks::stripped_log_acacia),
        Item::byBlock(Blocks::stripped_log_birch), Item::byBlock(Blocks::stripped_log_jungle),
        Item::byBlock(Blocks::stripped_log_oak), Item::byBlock(Blocks::stripped_log_spruce)
    };
    addTask(static_cast<eTutorial_State>(0),
            new PickupTask(logPickupItems, 8, 4, -1, this, 0xc76706b5, true, NULL, false, true,
                            true, NULL));

    int mappingsPair1[2] = { 35, 36 };
    int completionPair1[2] = { 2, 1 };
    addTask(static_cast<eTutorial_State>(0),
            new ControllerTask(this, 0xd50ac0c8, false, false, mappingsPair1, 2, completionPair1,
                                2, NULL, 0));
    int mappingSingle37[1] = { 37 };
    addTask(static_cast<eTutorial_State>(0),
            new ControllerTask(this, 0x69b79ddf, false, false, mappingSingle37, 1, NULL, 0, NULL,
                                0));

    addTask(static_cast<eTutorial_State>(0),
            new StateChangeTask(static_cast<eTutorial_State>(1), this, -1, false, NULL, false,
                                 true, true));

    addTask(static_cast<eTutorial_State>(0),
            new InfoTask(this, 0x67613737, 0x3f9463f, true, 0,
                         static_cast<ETelemetryChallenges>(0)));
    addTask(static_cast<eTutorial_State>(0),
            new InfoTask(this, 0xd756c6a6, 0x3f9463f, true, 0,
                         static_cast<ETelemetryChallenges>(0)));
    addTask(static_cast<eTutorial_State>(0),
            new InfoTask(this, 0x642c1bb4, 0x3f9463f, true, 0,
                         static_cast<ETelemetryChallenges>(0)));

    Item* completeUsingFoodItems[15] = {
        Items::mushroomStew, Items::apple,       Items::bread,        Items::porkchop,
        Items::cookedPorkchop, Items::goldenApple, Items::fish,        Items::cookedFish,
        Items::cookie,       Items::cookedBeef,  Items::beef,          Items::cookedChicken,
        Items::chicken,      Items::melon,        Items::rottenFlesh
    };
    addTask(static_cast<eTutorial_State>(0),
            new CompleteUsingItemTask(this, 0x355eb7d9, completeUsingFoodItems, 15, true));

    int mappingSingle40[1] = { 40 };
    addTask(static_cast<eTutorial_State>(0),
            new ControllerTask(this, 0xb5fcd4e7, false, false, mappingSingle40, 1, NULL, 0, NULL,
                                0));

    addTask(static_cast<eTutorial_State>(0),
            new ProgressFlagTask(this, reinterpret_cast<char*>(&mTutorialStarted), 1,
                                  ProgressFlagTask::eProgressFlagAction_Set));

    addTask(static_cast<eTutorial_State>(0),
            new CraftTask(Item::byBlock(Blocks::planks), -1, 1, this, 0x4d449d7d, true, NULL,
                           false, true, true));
    addTask(static_cast<eTutorial_State>(0),
            new CraftTask(Item::byBlock(Blocks::crafting_table), -1, 1, this, 0x2e35e925, true,
                           NULL, false, true, true));
    addTask(static_cast<eTutorial_State>(0),
            new InfoTask(this, 0xeae5d55e, 0x3f9463f, true, 0,
                         static_cast<ETelemetryChallenges>(0)));
    addTask(static_cast<eTutorial_State>(0),
            new UseItemTask(Item::byBlock(Blocks::crafting_table), this, 0xe42098c2, true, NULL,
                             false, true, true));
    addTask(static_cast<eTutorial_State>(0),
            new InfoTask(this, 0x88ac7bfd, 0x3f9463f, true, 0,
                         static_cast<ETelemetryChallenges>(0)));
    addTask(static_cast<eTutorial_State>(0),
            new InfoTask(this, 0x79452ef8, 0x3f9463f, true, 0,
                         static_cast<ETelemetryChallenges>(0)));
    addTask(static_cast<eTutorial_State>(0),
            new InfoTask(this, 0x441c0545, 0x3f9463f, true, 0,
                         static_cast<ETelemetryChallenges>(0)));
    addTask(static_cast<eTutorial_State>(0),
            new ChoiceTask(this, 0x28c71b69, 0x791088b0, true, 0, 1,
                           static_cast<eTutorial_CompletionAction>(3),
                           static_cast<ETelemetryChallenges>(2)));
    addTask(static_cast<eTutorial_State>(0),
            new UseBlockTask(Blocks::crafting_table, this, 0x7de96d85, false, NULL, false, true,
                              true));

    addTask(static_cast<eTutorial_State>(0),
            new ProgressFlagTask(this, reinterpret_cast<char*>(&mTutorialStarted), 2,
                                  ProgressFlagTask::eProgressFlagAction_Set));

    addTask(static_cast<eTutorial_State>(0),
            new CraftTask(Items::stick, -1, 1, this, 0x56bee93b, true, NULL, false, true, true));

    Item* shovelSetItems[5] = { Items::woodenShovel, Items::stoneShovel, Items::ironShovel,
                                 Items::goldenHorseArmor, Items::diamondShovel };
    int shovelSetAux[5] = { -1, -1, -1, -1, -1 };
    addTask(static_cast<eTutorial_State>(0),
            new CraftTask(shovelSetItems, shovelSetAux, 5, 1, this, 0xf9cc6772, true, NULL, false,
                           true, true));

    Item* axeSetItems[5] = { Items::woodenAxe, Items::stoneAxe, Items::ironAxe, Items::goldenAxe,
                              Items::diamondAxe };
    int axeSetAux[5] = { -1, -1, -1, -1, -1 };
    addTask(static_cast<eTutorial_State>(0),
            new CraftTask(axeSetItems, axeSetAux, 5, 1, this, 0xfac53756, true, NULL, false, true,
                           true));

    Item* pickaxeSetItems[5] = { Items::woodenPickaxe, Items::stonePickaxe, Items::ironPickaxe,
                                  Items::goldenPickaxe, Items::diamondPickaxe };
    int pickaxeSetAux[5] = { -1, -1, -1, -1, -1 };
    addTask(static_cast<eTutorial_State>(0),
            new CraftTask(pickaxeSetItems, pickaxeSetAux, 5, 1, this, 0x19cdd9a4, true, NULL,
                           false, true, true));

    Item* cobblestonePickupItems[1] = { Item::byBlock(Blocks::cobblestone) };
    addTask(static_cast<eTutorial_State>(0),
            new PickupTask(cobblestonePickupItems, 1, 8, -1, this, 0x8b96afee, true, NULL, false,
                            true, true, NULL));

    addTask(static_cast<eTutorial_State>(0),
            new ProgressFlagTask(this, reinterpret_cast<char*>(&mTutorialStarted), 4,
                                  ProgressFlagTask::eProgressFlagAction_Set));

    addTask(static_cast<eTutorial_State>(0),
            new CraftTask(Item::byBlock(Blocks::furnace), -1, 1, this, 0xe7cad766, true, NULL,
                           false, true, true));
    addTask(static_cast<eTutorial_State>(0),
            new UseBlockTask(Blocks::furnace, this, 0xf1d4d67e, false, NULL, false, true, true));

    addTask(static_cast<eTutorial_State>(0),
            new ProgressFlagTask(this, reinterpret_cast<char*>(&mTutorialStarted), 8,
                                  ProgressFlagTask::eProgressFlagAction_Set));

    addTask(static_cast<eTutorial_State>(0),
            new StateChangeTask(static_cast<eTutorial_State>(4), this, -1, false, NULL, false,
                                 true, true));

    addTask(static_cast<eTutorial_State>(0),
            new CraftTask(Items::coal, -1, 1, this, 0xf485454f, true, NULL, false, true, true));
    addTask(static_cast<eTutorial_State>(0),
            new CraftTask(Item::byBlock(Blocks::glass), -1, 1, this, 0x5f29384c, true, NULL,
                           false, true, true));

    Item* doorSetItems[6] = { Items::woodenDoor, Items::acaciaDoor, Items::birchDoor,
                               Items::darkOakDoor, Items::jungleDoor, Items::spruceDoor };
    int doorSetAux[6] = { -1, -1, -1, -1, -1, -1 };
    addTask(static_cast<eTutorial_State>(0),
            new CraftTask(doorSetItems, doorSetAux, 6, 1, this, 0x74135c27, true, NULL, false,
                           true, true));
    addTask(static_cast<eTutorial_State>(0),
            new UseItemTask(doorSetItems, 6, this, 0xf39f40bf, false, NULL, false, true, true));

    addTask(static_cast<eTutorial_State>(0),
            new CraftTask(Item::byBlock(Blocks::torch), -1, 1, this, 0xd15ac, true, NULL, false,
                           true, true));

    if (gApp.getGameRuleDefinitions() != NULL)
    {
        AABB* areaTutorial = gApp.getGameRuleDefinitions()->getNamedArea(L"tutorialArea");
        if (areaTutorial != NULL)
        {
            std::vector<TutorialConstraint*>* tutorialAreaConstraints =
                new std::vector<TutorialConstraint*>();
            tutorialAreaConstraints->push_back(new AreaConstraint(
                0x53717f71, areaTutorial->mMinX, areaTutorial->mMinY, areaTutorial->mMinZ,
                areaTutorial->mMaxX, areaTutorial->mMaxY, areaTutorial->mMaxZ, true, true, false,
                2));
            addTask(static_cast<eTutorial_State>(0),
                    new AreaTask(static_cast<eTutorial_State>(0), this, tutorialAreaConstraints,
                                 -1, AreaTask::CompletionStateOnStateChange));
        }
    }

    ProcedureCompoundTask* compound0 = new ProcedureCompoundTask(this);
    compound0->AddTask(new InfoTask(this, 0x857669aa, 0x3f9463f, true, 0,
                                     static_cast<ETelemetryChallenges>(3)));
    compound0->AddTask(new InfoTask(this, 0x59983743, 0x3f9463f, true, 0,
                                     static_cast<ETelemetryChallenges>(0)));
    compound0->AddTask(new InfoTask(this, 0xacd522e4, 0x3f9463f, true, 0,
                                     static_cast<ETelemetryChallenges>(0)));
    addTask(static_cast<eTutorial_State>(0), compound0);

    addTask(static_cast<eTutorial_State>(1),
            new FullTutorialActiveTask(this, static_cast<eTutorial_CompletionAction>(1)));
    addTask(static_cast<eTutorial_State>(1),
            new InfoTask(this, 0x3f434d73, -1, false, 1, static_cast<ETelemetryChallenges>(0)));

    addTask(static_cast<eTutorial_State>(2),
            new FullTutorialActiveTask(this, static_cast<eTutorial_CompletionAction>(1)));
    addTask(static_cast<eTutorial_State>(2),
            new ProgressFlagTask(this, reinterpret_cast<char*>(&mTutorialStarted), 1,
                                  ProgressFlagTask::eProgressFlagAction_Check));
    addTask(static_cast<eTutorial_State>(2),
            new FullTutorialActiveTask(this, static_cast<eTutorial_CompletionAction>(1)));
    addTask(static_cast<eTutorial_State>(2),
            new CraftTask(Item::byBlock(Blocks::planks), -1, 1, this, 0x61c2f830, true, NULL,
                           false, true, true));

    ProcedureCompoundTask* compound1 = new ProcedureCompoundTask(this);
    compound1->AddTask(new UICraftingTask(this, 0xe4451189, Recipe::eGroupType_Structures, false,
                                           NULL, false, true, true));
    compound1->AddTask(new UICraftingTask(this, 0xc8a27304, Item::byBlock(Blocks::crafting_table),
                                           false, NULL, false, true, true));
    compound1->AddTask(new CraftTask(Item::byBlock(Blocks::crafting_table), -1, 1, this,
                                      0x2e35e925, true, NULL, false, true, true));
    addTask(static_cast<eTutorial_State>(2), compound1);

    addTask(static_cast<eTutorial_State>(2),
            new InfoTask(this, 0x12eeee51, -1, false, 1, static_cast<ETelemetryChallenges>(0)));

    addTask(static_cast<eTutorial_State>(3),
            new FullTutorialActiveTask(this, static_cast<eTutorial_CompletionAction>(1)));
    addTask(static_cast<eTutorial_State>(3),
            new ProgressFlagTask(this, reinterpret_cast<char*>(&mTutorialStarted), 2,
                                  ProgressFlagTask::eProgressFlagAction_Check));
    addTask(static_cast<eTutorial_State>(3),
            new CraftTask(Items::stick, -1, 1, this, 0x56bee93b, true, NULL, false, true, true));

    ProcedureCompoundTask* compound2 = new ProcedureCompoundTask(this);
    compound2->AddTask(new UICraftingTask(this, 0xb2a0ad4c, Recipe::eGroupType_Tools, true, NULL,
                                           false, true, true));
    compound2->AddTask(new UICraftingTask(this, 0x76c32bbb, Items::woodenShovel, false, NULL,
                                           false, true, true));
    compound2->AddTask(new CraftTask(shovelSetItems, shovelSetAux, 5, 1, this, 0xf9cc6772, true,
                                      NULL, false, true, true));
    addTask(static_cast<eTutorial_State>(3), compound2);

    addTask(static_cast<eTutorial_State>(3),
            new CraftTask(axeSetItems, axeSetAux, 5, 1, this, 0xfac53756, true, NULL, false, true,
                           true));
    addTask(static_cast<eTutorial_State>(3),
            new CraftTask(pickaxeSetItems, pickaxeSetAux, 5, 1, this, 0x19cdd9a4, true, NULL,
                           false, true, true));
    addTask(static_cast<eTutorial_State>(3),
            new InfoTask(this, 0x63c8c01a, -1, false, 1, static_cast<ETelemetryChallenges>(0)));
    addTask(static_cast<eTutorial_State>(3),
            new ProgressFlagTask(this, reinterpret_cast<char*>(&mTutorialStarted), 4,
                                  ProgressFlagTask::eProgressFlagAction_Check));
    addTask(static_cast<eTutorial_State>(3),
            new CraftTask(Item::byBlock(Blocks::furnace), -1, 1, this, 0x7513d4c3, true, NULL,
                           false, true, true));
    addTask(static_cast<eTutorial_State>(3),
            new InfoTask(this, 0x5fce503f, -1, false, 1, static_cast<ETelemetryChallenges>(0)));
    addTask(static_cast<eTutorial_State>(3),
            new CraftTask(doorSetItems, doorSetAux, 6, 1, this, 0x74135c27, true, NULL, false,
                           true, true));
    addTask(static_cast<eTutorial_State>(3),
            new CraftTask(Item::byBlock(Blocks::torch), -1, 1, this, 0xd15ac, true, NULL, false,
                           true, true));

    addTask(static_cast<eTutorial_State>(0x25),
            new FullTutorialActiveTask(this, static_cast<eTutorial_CompletionAction>(1)));
    addTask(static_cast<eTutorial_State>(0x25),
            new ProgressFlagTask(this, reinterpret_cast<char*>(&mTutorialStarted), 2,
                                  ProgressFlagTask::eProgressFlagAction_Check));
    addTask(static_cast<eTutorial_State>(0x25),
            new CraftTask(Items::stick, -1, 1, this, 0x56bee93b, true, NULL, false, true, true));
    addTask(static_cast<eTutorial_State>(0x25),
            new CraftTask(shovelSetItems, shovelSetAux, 5, 1, this, 0xf9cc6772, true, NULL, false,
                           true, true));
    addTask(static_cast<eTutorial_State>(0x25),
            new CraftTask(axeSetItems, axeSetAux, 5, 1, this, 0xfac53756, true, NULL, false, true,
                           true));
    addTask(static_cast<eTutorial_State>(0x25),
            new CraftTask(pickaxeSetItems, pickaxeSetAux, 5, 1, this, 0x19cdd9a4, true, NULL,
                           false, true, true));
    addTask(static_cast<eTutorial_State>(0x25),
            new InfoTask(this, 0x63c8c01a, -1, false, 1, static_cast<ETelemetryChallenges>(0)));
    addTask(static_cast<eTutorial_State>(0x25),
            new ProgressFlagTask(this, reinterpret_cast<char*>(&mTutorialStarted), 4,
                                  ProgressFlagTask::eProgressFlagAction_Check));
    addTask(static_cast<eTutorial_State>(0x25),
            new CraftTask(Item::byBlock(Blocks::furnace), -1, 1, this, 0x7513d4c3, true, NULL,
                           false, true, true));
    addTask(static_cast<eTutorial_State>(0x25),
            new InfoTask(this, 0x5fce503f, -1, false, 1, static_cast<ETelemetryChallenges>(0)));
    addTask(static_cast<eTutorial_State>(0x25),
            new CraftTask(doorSetItems, doorSetAux, 6, 1, this, 0x74135c27, true, NULL, false,
                           true, true));
    addTask(static_cast<eTutorial_State>(0x25),
            new CraftTask(Item::byBlock(Blocks::torch), -1, 1, this, 0xd15ac, true, NULL, false,
                           true, true));

    addTask(static_cast<eTutorial_State>(4),
            new FullTutorialActiveTask(this, static_cast<eTutorial_CompletionAction>(1)));
    addTask(static_cast<eTutorial_State>(4),
            new ProgressFlagTask(this, reinterpret_cast<char*>(&mTutorialStarted), 8,
                                  ProgressFlagTask::eProgressFlagAction_Check));
    addTask(static_cast<eTutorial_State>(4),
            new CraftTask(Items::coal, -1, 1, this, 0xb9ad8818, true, NULL, false, true, true));
    addTask(static_cast<eTutorial_State>(4),
            new InfoTask(this, 0x4c729fc4, 0x3f9463f, true, 0,
                         static_cast<ETelemetryChallenges>(0)));
    addTask(static_cast<eTutorial_State>(4),
            new CraftTask(Item::byBlock(Blocks::glass), -1, 1, this, 0xa9ca4693, true, NULL,
                           false, true, true));

    if (gApp.getGameRuleDefinitions() != NULL)
    {
        AABB* areaMinecart = gApp.getGameRuleDefinitions()->getNamedArea(L"minecartArea");
        if (areaMinecart != NULL)
        {
            addHint(static_cast<eTutorial_State>(0),
                    new AreaHint(static_cast<eTutorial_Hint>(0x2c), this,
                                 static_cast<eTutorial_State>(0),
                                 static_cast<eTutorial_State>(5), 0x7842d528, areaMinecart->mMinX,
                                 areaMinecart->mMinY, areaMinecart->mMinZ, areaMinecart->mMaxX,
                                 areaMinecart->mMaxY, areaMinecart->mMaxZ, true, true));
        }
    }

    if (gApp.getGameRuleDefinitions() != NULL)
    {
        AABB* areaBoat = gApp.getGameRuleDefinitions()->getNamedArea(L"boatArea");
        if (areaBoat != NULL)
        {
            addHint(static_cast<eTutorial_State>(0),
                    new AreaHint(static_cast<eTutorial_Hint>(0x2c), this,
                                 static_cast<eTutorial_State>(0),
                                 static_cast<eTutorial_State>(6), 0xb11a0f6b, areaBoat->mMinX,
                                 areaBoat->mMinY, areaBoat->mMinZ, areaBoat->mMaxX,
                                 areaBoat->mMaxY, areaBoat->mMaxZ, true, true));
        }
    }

    if (gApp.getGameRuleDefinitions() != NULL)
    {
        AABB* areaFishing = gApp.getGameRuleDefinitions()->getNamedArea(L"fishingArea");
        if (areaFishing != NULL)
        {
            addHint(static_cast<eTutorial_State>(0),
                    new AreaHint(static_cast<eTutorial_Hint>(0x2c), this,
                                 static_cast<eTutorial_State>(0),
                                 static_cast<eTutorial_State>(7), 0x7f2df94f, areaFishing->mMinX,
                                 areaFishing->mMinY, areaFishing->mMinZ, areaFishing->mMaxX,
                                 areaFishing->mMaxY, areaFishing->mMaxZ, true, true));
        }
    }

    if (gApp.getGameRuleDefinitions() != NULL)
    {
        AABB* areaPistonBridge =
            gApp.getGameRuleDefinitions()->getNamedArea(L"pistonBridgeArea");
        if (areaPistonBridge != NULL)
        {
            addHint(static_cast<eTutorial_State>(0),
                    new AreaHint(static_cast<eTutorial_Hint>(0x2c), this,
                                 static_cast<eTutorial_State>(0),
                                 static_cast<eTutorial_State>(-1), 0xf258c0ad,
                                 areaPistonBridge->mMinX, areaPistonBridge->mMinY,
                                 areaPistonBridge->mMinZ, areaPistonBridge->mMaxX,
                                 areaPistonBridge->mMaxY, areaPistonBridge->mMaxZ, true, true));
        }
    }

    if (gApp.getGameRuleDefinitions() != NULL)
    {
        AABB* areaEmeraldElytra =
            gApp.getGameRuleDefinitions()->getNamedArea(L"emeraldElytraArea");
        AABB* areaGoldElytra = gApp.getGameRuleDefinitions()->getNamedArea(L"goldElytraArea");
        AABB* areaDiamondElytra =
            gApp.getGameRuleDefinitions()->getNamedArea(L"diamondElytraArea");

        if (areaEmeraldElytra != NULL)
        {
            addHint(static_cast<eTutorial_State>(0),
                    new AreaHint(static_cast<eTutorial_Hint>(0x2c), this,
                                 static_cast<eTutorial_State>(0),
                                 static_cast<eTutorial_State>(-1), 0x6fcb60c3,
                                 areaEmeraldElytra->mMinX, areaEmeraldElytra->mMinY,
                                 areaEmeraldElytra->mMinZ, areaEmeraldElytra->mMaxX,
                                 areaEmeraldElytra->mMaxY, areaEmeraldElytra->mMaxZ, true, true));
        }
        if (areaGoldElytra != NULL)
        {
            addHint(static_cast<eTutorial_State>(0),
                    new AreaHint(static_cast<eTutorial_Hint>(0x2c), this,
                                 static_cast<eTutorial_State>(0),
                                 static_cast<eTutorial_State>(-1), 0x6fcb60c3,
                                 areaGoldElytra->mMinX, areaGoldElytra->mMinY,
                                 areaGoldElytra->mMinZ, areaGoldElytra->mMaxX,
                                 areaGoldElytra->mMaxY, areaGoldElytra->mMaxZ, true, true));
        }
        if (areaDiamondElytra != NULL)
        {
            addHint(static_cast<eTutorial_State>(0),
                    new AreaHint(static_cast<eTutorial_Hint>(0x2c), this,
                                 static_cast<eTutorial_State>(0),
                                 static_cast<eTutorial_State>(-1), 0x6fcb60c3,
                                 areaDiamondElytra->mMinX, areaDiamondElytra->mMinY,
                                 areaDiamondElytra->mMinZ, areaDiamondElytra->mMaxX,
                                 areaDiamondElytra->mMaxY, areaDiamondElytra->mMaxZ, true, true));
        }
    }

    if (gApp.getGameRuleDefinitions() != NULL)
    {
        AABB* areaCreative = gApp.getGameRuleDefinitions()->getNamedArea(L"creativeArea");
        if (areaCreative != NULL)
        {
            eTutorial_State creativeExitStates[1] = { static_cast<eTutorial_State>(0) };
            gameModeConstraints->AddGlobalConstraint(new ChangeStateConstraint(
                this, static_cast<eTutorial_State>(0x10), creativeExitStates, 1,
                areaCreative->mMinX, areaCreative->mMinY, areaCreative->mMinZ,
                areaCreative->mMaxX, areaCreative->mMaxY, areaCreative->mMaxZ, true, true,
                GameType::CREATIVE));

            addTask(static_cast<eTutorial_State>(0x10),
                    new ChoiceTask(this, 0x4a0984b8, 0xd1ebba23, true, 0, 1,
                                   static_cast<eTutorial_CompletionAction>(3),
                                   static_cast<ETelemetryChallenges>(0xe)));
            addTask(static_cast<eTutorial_State>(0x10),
                    new InfoTask(this, 0xc93a06c2, 0x3f9463f, true, 0,
                                 static_cast<ETelemetryChallenges>(0)));
            addTask(static_cast<eTutorial_State>(0x10),
                    new InfoTask(this, 0x6064ebd6, 0x3f9463f, true, 0,
                                 static_cast<ETelemetryChallenges>(0)));

            int creativeMapping[1] = { 40 };
            addTask(static_cast<eTutorial_State>(0x10),
                    new ControllerTask(this, 0xbeac2f00, false, false, creativeMapping, 1, NULL,
                                        0, NULL, 0));

            addTask(static_cast<eTutorial_State>(0x10),
                    new StateChangeTask(static_cast<eTutorial_State>(0xe), this, -1, false, NULL,
                                         false, true, true));

            ProcedureCompoundTask* compound3 = new ProcedureCompoundTask(this);

            AABB* areaCreativeExit =
                gApp.getGameRuleDefinitions()->getNamedArea(L"creativeExitArea");
            if (areaCreativeExit != NULL)
            {
                std::vector<TutorialConstraint*>* creativeExitConstraints =
                    new std::vector<TutorialConstraint*>();
                creativeExitConstraints->push_back(new AreaConstraint(
                    -1, areaCreativeExit->mMinX, areaCreativeExit->mMinY,
                    areaCreativeExit->mMinZ, areaCreativeExit->mMaxX, areaCreativeExit->mMaxY,
                    areaCreativeExit->mMaxZ, true, false, false, 2));
                compound3->AddTask(new AreaTask(static_cast<eTutorial_State>(0x10), this,
                                                 creativeExitConstraints, 0xe4341f71,
                                                 AreaTask::CompletionStateOnConstraints));
            }

            std::vector<TutorialConstraint*>* creativeAreaConstraints =
                new std::vector<TutorialConstraint*>();
            creativeAreaConstraints->push_back(new AreaConstraint(
                0x53717f71, areaCreative->mMinX, areaCreative->mMinY, areaCreative->mMinZ,
                areaCreative->mMaxX, areaCreative->mMaxY, areaCreative->mMaxZ, true, true, false,
                2));
            compound3->AddTask(new AreaTask(static_cast<eTutorial_State>(0x10), this,
                                             creativeAreaConstraints, -1,
                                             AreaTask::CompletionStateOnStateChange));

            compound3->AddTask(new InfoTask(this, 0xa77b2ab0, 0x3f9463f, true, 0,
                                             static_cast<ETelemetryChallenges>(0)));

            addTask(static_cast<eTutorial_State>(0x10), compound3);
        }
    }

    AddTutorialArea(static_cast<eTutorial_State>(0xc));
    AddTutorialArea(static_cast<eTutorial_State>(0xd));
    AddTutorialArea(static_cast<eTutorial_State>(0x11));
    AddTutorialArea(static_cast<eTutorial_State>(0x13));
    AddTutorialArea(static_cast<eTutorial_State>(0x1a));
    AddTutorialArea(static_cast<eTutorial_State>(0x18));
    AddTutorialArea(static_cast<eTutorial_State>(0x23));
    AddTutorialArea(static_cast<eTutorial_State>(0x21));
    AddTutorialArea(static_cast<eTutorial_State>(0x1f));
    AddTutorialArea(static_cast<eTutorial_State>(0x1c));
    AddTutorialArea(static_cast<eTutorial_State>(0x15));
    AddTutorialArea(static_cast<eTutorial_State>(0x16));
    AddTutorialArea(static_cast<eTutorial_State>(0x17));
    AddTutorialArea(static_cast<eTutorial_State>(0x26));
    AddTutorialArea(static_cast<eTutorial_State>(0x27));
    AddTutorialArea(static_cast<eTutorial_State>(0x28));
    AddTutorialArea(static_cast<eTutorial_State>(0x2a));
    AddTutorialArea(static_cast<eTutorial_State>(0x2b));
}

FullTutorial::~FullTutorial()
{
}

void FullTutorial::AddTutorialArea(eTutorial_State state)
{
    if (gameModeConstraints == NULL || state == eTutorial_State_None)
    {
        return;
    }
    if (gameModeConstraints->HasGlobalConstraint((int)state))
    {
        return;
    }
}

void FullTutorial::PopulateChest(eTutorial_State state)
{
    LevelRuleset* ruleset = gApp.getGameRuleDefinitions();
    if (ruleset == NULL)
    {
        return;
    }

    GameRuleDefinition* definition = ruleset->getRuleByType(
        static_cast<ConsoleGameRules::EGameRuleType>(30));
    if (definition == NULL)
    {
        return;
    }

    ServerLevel* level = gMinecraftServer->getLevel(0);

    std::vector<GameRuleDefinition*> children;
    definition->getChildren(&children, static_cast<ConsoleGameRules::EGameRuleType>(32));

    for (std::vector<GameRuleDefinition*>::iterator it = children.begin();
         it != children.end(); ++it)
    {
        PopulateContainerRuleDefinition* container =
            static_cast<PopulateContainerRuleDefinition*>(*it);

        int x;
        int y;
        int z;
        container->GetPos(x, y, z);

        if (state == static_cast<eTutorial_State>(-2) ||
            gameModeConstraints->InConstraintArea(Vec3::newTemp(x, y, z),
                                                  static_cast<int>(state)))
        {
            container->evaluateRule(level, ruleset,
                                    *MiniGameDef::GetCustomGameModeById(eMiniGameId_Any, true));
        }
    }
}

void FullTutorial::tick()
{
    std::vector<eTutorial_State>::iterator state = completedStates.begin();
    while (state != completedStates.end())
    {
        if (gameModeConstraints->NotInConstraintArea(playerId, static_cast<int>(*state)))
        {
            mStateCompleted[static_cast<int>(*state)] = 0;
            AddTutorialArea(*state);
            PopulateChest(*state);
            state = completedStates.erase(state);
        }
        else
        {
            ++state;
        }
    }
    Tutorial::tick();
}

unsigned char FullTutorial::isStateCompleted(eTutorial_State state)
{
    return mStateCompleted[static_cast<int>(state)];
}

void FullTutorial::setStateCompleted(eTutorial_State state)
{
    mStateCompleted[static_cast<int>(state)] = 1;
    Tutorial::setStateCompleted(state);
}
