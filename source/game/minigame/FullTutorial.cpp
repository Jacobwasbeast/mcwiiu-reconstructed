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

namespace TutorialIds {
static const unsigned int AdditionalPlanksCraft = 0x61c2f830;
static const unsigned int AxeSetCraft = 0xfac53756;
static const unsigned int BoatArea = 0xb11a0f6b;
static const unsigned int ChallengeIntroInfo = 0x3f434d73;
static const unsigned int CoalCraft = 0xf485454f;
static const unsigned int CoalCraftCreative = 0xb9ad8818;
static const unsigned int CobblestonePickup = 0x8b96afee;
static const unsigned int ControllerAction = 0xe21ad07f;
static const unsigned int ControllerActionSecondary = 0xfa88ce3a;
static const unsigned int ControllerInventory = 0xb5fcd4e7;
static const unsigned int ControllerJump = 0x69b79ddf;
static const unsigned int ControllerLook = 0xd50ac0c8;
static const unsigned int ControllerLookMove = 0xd6b36624;
static const unsigned int ControllerMove = 0x68129f42;
static const unsigned int CraftingBasicsInfo = 0xeae5d55e;
static const unsigned int CraftingChoice = 0x28c71b69;
static const unsigned int CraftingChoiceFollowup = 0x791088b0;
static const unsigned int CraftingInfo = 0x59983743;
static const unsigned int CraftingInfoAdvanced = 0x79452ef8;
static const unsigned int CraftingTableCraft = 0x2e35e925;
static const unsigned int CraftingTableInfo = 0x441c0545;
static const unsigned int CraftingTableInfoAdvanced = 0xacd522e4;
static const unsigned int CraftingTableUi = 0xc8a27304;
static const unsigned int CraftingTableUse = 0x7de96d85;
static const unsigned int CraftingTableUseInfo = 0x88ac7bfd;
static const unsigned int CraftingTableUseItem = 0xe42098c2;
static const unsigned int CreativeController = 0xbeac2f00;
static const unsigned int CreativeExitArea = 0xe4341f71;
static const unsigned int CreativeExitInfo = 0xa77b2ab0;
static const unsigned int CreativeInfo = 0x6064ebd6;
static const unsigned int CreativeInventoryInfo = 0xc93a06c2;
static const unsigned int CreativeModeChoice = 0x4a0984b8;
static const unsigned int CreativeModeChoiceFollowup = 0xd1ebba23;
static const unsigned int DefaultMessageGroup = 0x3f9463f;
static const unsigned int DoorSetCraft = 0x74135c27;
static const unsigned int DoorUse = 0xf39f40bf;
static const unsigned int ElytraArea = 0x6fcb60c3;
static const unsigned int FishingArea = 0x7f2df94f;
static const unsigned int FoodUse = 0x355eb7d9;
static const unsigned int FurnaceCraftCreative = 0x7513d4c3;
static const unsigned int FurnaceCraft = 0xe7cad766;
static const unsigned int FurnaceUse = 0xf1d4d67e;
static const unsigned int GlassCraft = 0x5f29384c;
static const unsigned int GlassCraftCreative = 0xa9ca4693;
static const unsigned int InventoryInfo = 0x642c1bb4;
static const unsigned int LogPickup = 0xc76706b5;
static const unsigned int MinecartArea = 0x7842d528;
static const unsigned int MovementForwardInfo = 0xd756c6a6;
static const unsigned int MovementInfo = 0x24614e1f;
static const unsigned int MovementLookInfo = 0x67613737;
static const unsigned int PickaxeSetCraft = 0x19cdd9a4;
static const unsigned int PistonBridgeArea = 0xf258c0ad;
static const unsigned int PlanksCraft = 0x4d449d7d;
static const unsigned int ShovelSetCraft = 0xf9cc6772;
static const unsigned int SmeltingBasicsInfo = 0x5fce503f;
static const unsigned int SmeltingInfo = 0x4c729fc4;
static const unsigned int StickCraft = 0x56bee93b;
static const unsigned int StructuresUi = 0xe4451189;
static const unsigned int SurvivalBasicsInfo = 0x12eeee51;
static const unsigned int ToolBasicsInfo = 0x63c8c01a;
static const unsigned int ToolsUi = 0xb2a0ad4c;
static const unsigned int TorchCraft = 0xd15ac;
static const unsigned int TutorialArea = 0x53717f71;
static const unsigned int TutorialStartChoice = 0xdf59fc3f;
static const unsigned int TutorialStartTask = 0x989a3f76;
static const unsigned int WelcomeMessage = 0x857669aa;
static const unsigned int WoodenShovelUi = 0x76c32bbb;
}

namespace TutorialState {
static const eTutorial_State Initial = static_cast<eTutorial_State>(0);
static const eTutorial_State Controller = static_cast<eTutorial_State>(1);
static const eTutorial_State Crafting = static_cast<eTutorial_State>(2);
static const eTutorial_State Tools = static_cast<eTutorial_State>(3);
static const eTutorial_State Furnace = static_cast<eTutorial_State>(4);
static const eTutorial_State Minecart = static_cast<eTutorial_State>(5);
static const eTutorial_State Boat = static_cast<eTutorial_State>(6);
static const eTutorial_State Fishing = static_cast<eTutorial_State>(7);
static const eTutorial_State Nether = static_cast<eTutorial_State>(0xc);
static const eTutorial_State End = static_cast<eTutorial_State>(0xd);
static const eTutorial_State Creative = static_cast<eTutorial_State>(0x10);
static const eTutorial_State AreaState11 = static_cast<eTutorial_State>(0x11);
static const eTutorial_State AreaState13 = static_cast<eTutorial_State>(0x13);
static const eTutorial_State AreaState15 = static_cast<eTutorial_State>(0x15);
static const eTutorial_State AreaState16 = static_cast<eTutorial_State>(0x16);
static const eTutorial_State AreaState17 = static_cast<eTutorial_State>(0x17);
static const eTutorial_State AreaState18 = static_cast<eTutorial_State>(0x18);
static const eTutorial_State AreaState1A = static_cast<eTutorial_State>(0x1a);
static const eTutorial_State AreaState1C = static_cast<eTutorial_State>(0x1c);
static const eTutorial_State AreaState1F = static_cast<eTutorial_State>(0x1f);
static const eTutorial_State AreaState21 = static_cast<eTutorial_State>(0x21);
static const eTutorial_State AreaState23 = static_cast<eTutorial_State>(0x23);
static const eTutorial_State SurvivalCrafting = static_cast<eTutorial_State>(0x25);
static const eTutorial_State AreaState26 = static_cast<eTutorial_State>(0x26);
static const eTutorial_State AreaState27 = static_cast<eTutorial_State>(0x27);
static const eTutorial_State AreaState28 = static_cast<eTutorial_State>(0x28);
static const eTutorial_State AreaState2A = static_cast<eTutorial_State>(0x2a);
static const eTutorial_State AreaState2B = static_cast<eTutorial_State>(0x2b);
static const eTutorial_State CreativeCompletion = static_cast<eTutorial_State>(0xe);
static const eTutorial_State Invalid = static_cast<eTutorial_State>(-1);
static const eTutorial_State Completed = static_cast<eTutorial_State>(-2);
}

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
    addMessage(TutorialIds::WelcomeMessage, false, 3);

    if (!mEnabledForPlayer && getCompleted(0x183) == 0)
    {
        addTask(TutorialState::Initial,
                new InfoTask(this, TutorialIds::TutorialStartTask, TutorialIds::DefaultMessageGroup, true, 0,
                             static_cast<ETelemetryChallenges>(0)));
    }
    else
    {
        addTask(TutorialState::Initial,
                new ChoiceTask(this, TutorialIds::TutorialStartTask, TutorialIds::TutorialStartChoice, true, 0, 1,
                               static_cast<eTutorial_CompletionAction>(3),
                               static_cast<ETelemetryChallenges>(1)));
    }

    int controllerMappingsA[4] = { 30, 31, 28, 29 };
    int controllerCompletionIdsA[4] = { 10, 9, 6, 5 };
    int controllerMappingsB[4] = { 24, 25, 26, 27 };

    addTask(TutorialState::Initial,
            new ControllerTask(this, TutorialIds::ControllerMove, false, false, controllerMappingsA, 4,
                                controllerCompletionIdsA, 4, controllerMappingsB, 4));
    addTask(TutorialState::Initial,
            new ControllerTask(this, TutorialIds::ControllerLookMove, false, false, controllerMappingsB, 4,
                                controllerCompletionIdsA, 4, controllerMappingsA, 4));
    addTask(TutorialState::Initial,
            new InfoTask(this, TutorialIds::MovementInfo, TutorialIds::DefaultMessageGroup, true, 0,
                         static_cast<ETelemetryChallenges>(0)));

    int mappingSingle32[1] = { 32 };
    addTask(TutorialState::Initial,
            new ControllerTask(this, TutorialIds::ControllerAction, false, true, mappingSingle32, 1, NULL, 0, NULL,
                                0));
    int mappingSingle34[1] = { 34 };
    addTask(TutorialState::Initial,
            new ControllerTask(this, TutorialIds::ControllerActionSecondary, false, true, mappingSingle34, 1, NULL, 0, NULL,
                                0));

    Item* logPickupItems[8] = {
        Item::byBlock(Blocks::log), Item::byBlock(Blocks::log2),
        Item::byBlock(Blocks::stripped_log_dark_oak), Item::byBlock(Blocks::stripped_log_acacia),
        Item::byBlock(Blocks::stripped_log_birch), Item::byBlock(Blocks::stripped_log_jungle),
        Item::byBlock(Blocks::stripped_log_oak), Item::byBlock(Blocks::stripped_log_spruce)
    };
    addTask(TutorialState::Initial,
            new PickupTask(logPickupItems, 8, 4, -1, this, TutorialIds::LogPickup, true, NULL, false, true,
                            true, NULL));

    int mappingsPair1[2] = { 35, 36 };
    int completionPair1[2] = { 2, 1 };
    addTask(TutorialState::Initial,
            new ControllerTask(this, TutorialIds::ControllerLook, false, false, mappingsPair1, 2, completionPair1,
                                2, NULL, 0));
    int mappingSingle37[1] = { 37 };
    addTask(TutorialState::Initial,
            new ControllerTask(this, TutorialIds::ControllerJump, false, false, mappingSingle37, 1, NULL, 0, NULL,
                                0));

    addTask(TutorialState::Initial,
            new StateChangeTask(TutorialState::Controller, this, -1, false, NULL, false,
                                 true, true));

    addTask(TutorialState::Initial,
            new InfoTask(this, TutorialIds::MovementLookInfo, TutorialIds::DefaultMessageGroup, true, 0,
                         static_cast<ETelemetryChallenges>(0)));
    addTask(TutorialState::Initial,
            new InfoTask(this, TutorialIds::MovementForwardInfo, TutorialIds::DefaultMessageGroup, true, 0,
                         static_cast<ETelemetryChallenges>(0)));
    addTask(TutorialState::Initial,
            new InfoTask(this, TutorialIds::InventoryInfo, TutorialIds::DefaultMessageGroup, true, 0,
                         static_cast<ETelemetryChallenges>(0)));

    Item* completeUsingFoodItems[15] = {
        Items::mushroomStew, Items::apple,       Items::bread,        Items::porkchop,
        Items::cookedPorkchop, Items::goldenApple, Items::fish,        Items::cookedFish,
        Items::cookie,       Items::cookedBeef,  Items::beef,          Items::cookedChicken,
        Items::chicken,      Items::melon,        Items::rottenFlesh
    };
    addTask(TutorialState::Initial,
            new CompleteUsingItemTask(this, TutorialIds::FoodUse, completeUsingFoodItems, 15, true));

    int mappingSingle40[1] = { 40 };
    addTask(TutorialState::Initial,
            new ControllerTask(this, TutorialIds::ControllerInventory, false, false, mappingSingle40, 1, NULL, 0, NULL,
                                0));

    addTask(TutorialState::Initial,
            new ProgressFlagTask(this, &mTutorialStarted, 1,
                                  ProgressFlagTask::eProgressFlagAction_Set));

    addTask(TutorialState::Initial,
            new CraftTask(Item::byBlock(Blocks::planks), -1, 1, this, TutorialIds::PlanksCraft, true, NULL,
                           false, true, true));
    addTask(TutorialState::Initial,
            new CraftTask(Item::byBlock(Blocks::crafting_table), -1, 1, this, TutorialIds::CraftingTableCraft, true,
                           NULL, false, true, true));
    addTask(TutorialState::Initial,
            new InfoTask(this, TutorialIds::CraftingBasicsInfo, TutorialIds::DefaultMessageGroup, true, 0,
                         static_cast<ETelemetryChallenges>(0)));
    addTask(TutorialState::Initial,
            new UseItemTask(Item::byBlock(Blocks::crafting_table), this, TutorialIds::CraftingTableUseItem, true, NULL,
                             false, true, true));
    addTask(TutorialState::Initial,
            new InfoTask(this, TutorialIds::CraftingTableUseInfo, TutorialIds::DefaultMessageGroup, true, 0,
                         static_cast<ETelemetryChallenges>(0)));
    addTask(TutorialState::Initial,
            new InfoTask(this, TutorialIds::CraftingInfoAdvanced, TutorialIds::DefaultMessageGroup, true, 0,
                         static_cast<ETelemetryChallenges>(0)));
    addTask(TutorialState::Initial,
            new InfoTask(this, TutorialIds::CraftingTableInfo, TutorialIds::DefaultMessageGroup, true, 0,
                         static_cast<ETelemetryChallenges>(0)));
    addTask(TutorialState::Initial,
            new ChoiceTask(this, TutorialIds::CraftingChoice, TutorialIds::CraftingChoiceFollowup, true, 0, 1,
                           static_cast<eTutorial_CompletionAction>(3),
                           static_cast<ETelemetryChallenges>(2)));
    addTask(TutorialState::Initial,
            new UseBlockTask(Blocks::crafting_table, this, TutorialIds::CraftingTableUse, false, NULL, false, true,
                              true));

    addTask(TutorialState::Initial,
            new ProgressFlagTask(this, &mTutorialStarted, 2,
                                  ProgressFlagTask::eProgressFlagAction_Set));

    addTask(TutorialState::Initial,
            new CraftTask(Items::stick, -1, 1, this, TutorialIds::StickCraft, true, NULL, false, true, true));

    Item* shovelSetItems[5] = { Items::woodenShovel, Items::stoneShovel, Items::ironShovel,
                                 Items::goldenHorseArmor, Items::diamondShovel };
    int shovelSetAux[5] = { -1, -1, -1, -1, -1 };
    addTask(TutorialState::Initial,
            new CraftTask(shovelSetItems, shovelSetAux, 5, 1, this, TutorialIds::ShovelSetCraft, true, NULL, false,
                           true, true));

    Item* axeSetItems[5] = { Items::woodenAxe, Items::stoneAxe, Items::ironAxe, Items::goldenAxe,
                              Items::diamondAxe };
    int axeSetAux[5] = { -1, -1, -1, -1, -1 };
    addTask(TutorialState::Initial,
            new CraftTask(axeSetItems, axeSetAux, 5, 1, this, TutorialIds::AxeSetCraft, true, NULL, false, true,
                           true));

    Item* pickaxeSetItems[5] = { Items::woodenPickaxe, Items::stonePickaxe, Items::ironPickaxe,
                                  Items::goldenPickaxe, Items::diamondPickaxe };
    int pickaxeSetAux[5] = { -1, -1, -1, -1, -1 };
    addTask(TutorialState::Initial,
            new CraftTask(pickaxeSetItems, pickaxeSetAux, 5, 1, this, TutorialIds::PickaxeSetCraft, true, NULL,
                           false, true, true));

    Item* cobblestonePickupItems[1] = { Item::byBlock(Blocks::cobblestone) };
    addTask(TutorialState::Initial,
            new PickupTask(cobblestonePickupItems, 1, 8, -1, this, TutorialIds::CobblestonePickup, true, NULL, false,
                            true, true, NULL));

    addTask(TutorialState::Initial,
            new ProgressFlagTask(this, &mTutorialStarted, 4,
                                  ProgressFlagTask::eProgressFlagAction_Set));

    addTask(TutorialState::Initial,
            new CraftTask(Item::byBlock(Blocks::furnace), -1, 1, this, TutorialIds::FurnaceCraft, true, NULL,
                           false, true, true));
    addTask(TutorialState::Initial,
            new UseBlockTask(Blocks::furnace, this, TutorialIds::FurnaceUse, false, NULL, false, true, true));

    addTask(TutorialState::Initial,
            new ProgressFlagTask(this, &mTutorialStarted, 8,
                                  ProgressFlagTask::eProgressFlagAction_Set));

    addTask(TutorialState::Initial,
            new StateChangeTask(TutorialState::Furnace, this, -1, false, NULL, false,
                                 true, true));

    addTask(TutorialState::Initial,
            new CraftTask(Items::coal, -1, 1, this, TutorialIds::CoalCraft, true, NULL, false, true, true));
    addTask(TutorialState::Initial,
            new CraftTask(Item::byBlock(Blocks::glass), -1, 1, this, TutorialIds::GlassCraft, true, NULL,
                           false, true, true));

    Item* doorSetItems[6] = { Items::woodenDoor, Items::acaciaDoor, Items::birchDoor,
                               Items::darkOakDoor, Items::jungleDoor, Items::spruceDoor };
    int doorSetAux[6] = { -1, -1, -1, -1, -1, -1 };
    addTask(TutorialState::Initial,
            new CraftTask(doorSetItems, doorSetAux, 6, 1, this, TutorialIds::DoorSetCraft, true, NULL, false,
                           true, true));
    addTask(TutorialState::Initial,
            new UseItemTask(doorSetItems, 6, this, TutorialIds::DoorUse, false, NULL, false, true, true));

    addTask(TutorialState::Initial,
            new CraftTask(Item::byBlock(Blocks::torch), -1, 1, this, TutorialIds::TorchCraft, true, NULL, false,
                           true, true));

    if (gApp.getGameRuleDefinitions() != NULL)
    {
        AABB* areaTutorial = gApp.getGameRuleDefinitions()->getNamedArea(L"tutorialArea");
        if (areaTutorial != NULL)
        {
            std::vector<TutorialConstraint*>* tutorialAreaConstraints =
                new std::vector<TutorialConstraint*>();
            tutorialAreaConstraints->push_back(new AreaConstraint(
                TutorialIds::TutorialArea, areaTutorial->mMinX, areaTutorial->mMinY, areaTutorial->mMinZ,
                areaTutorial->mMaxX, areaTutorial->mMaxY, areaTutorial->mMaxZ, true, true, false,
                2));
            addTask(TutorialState::Initial,
                    new AreaTask(TutorialState::Initial, this, tutorialAreaConstraints,
                                 -1, AreaTask::CompletionStateOnStateChange));
        }
    }

    ProcedureCompoundTask* compound0 = new ProcedureCompoundTask(this);
    compound0->AddTask(new InfoTask(this, TutorialIds::WelcomeMessage, TutorialIds::DefaultMessageGroup, true, 0,
                                     static_cast<ETelemetryChallenges>(3)));
    compound0->AddTask(new InfoTask(this, TutorialIds::CraftingInfo, TutorialIds::DefaultMessageGroup, true, 0,
                                     static_cast<ETelemetryChallenges>(0)));
    compound0->AddTask(new InfoTask(this, TutorialIds::CraftingTableInfoAdvanced, TutorialIds::DefaultMessageGroup, true, 0,
                                     static_cast<ETelemetryChallenges>(0)));
    addTask(TutorialState::Initial, compound0);

    addTask(TutorialState::Controller,
            new FullTutorialActiveTask(this, static_cast<eTutorial_CompletionAction>(1)));
    addTask(TutorialState::Controller,
            new InfoTask(this, TutorialIds::ChallengeIntroInfo, -1, false, 1, static_cast<ETelemetryChallenges>(0)));

    addTask(TutorialState::Crafting,
            new FullTutorialActiveTask(this, static_cast<eTutorial_CompletionAction>(1)));
    addTask(TutorialState::Crafting,
            new ProgressFlagTask(this, &mTutorialStarted, 1,
                                  ProgressFlagTask::eProgressFlagAction_Check));
    addTask(TutorialState::Crafting,
            new FullTutorialActiveTask(this, static_cast<eTutorial_CompletionAction>(1)));
    addTask(TutorialState::Crafting,
            new CraftTask(Item::byBlock(Blocks::planks), -1, 1, this, TutorialIds::AdditionalPlanksCraft, true, NULL,
                           false, true, true));

    ProcedureCompoundTask* compound1 = new ProcedureCompoundTask(this);
    compound1->AddTask(new UICraftingTask(this, TutorialIds::StructuresUi, Recipe::eGroupType_Structures, false,
                                           NULL, false, true, true));
    compound1->AddTask(new UICraftingTask(this, TutorialIds::CraftingTableUi, Item::byBlock(Blocks::crafting_table),
                                           false, NULL, false, true, true));
    compound1->AddTask(new CraftTask(Item::byBlock(Blocks::crafting_table), -1, 1, this,
                                      TutorialIds::CraftingTableCraft, true, NULL, false, true, true));
    addTask(TutorialState::Crafting, compound1);

    addTask(TutorialState::Crafting,
            new InfoTask(this, TutorialIds::SurvivalBasicsInfo, -1, false, 1, static_cast<ETelemetryChallenges>(0)));

    addTask(TutorialState::Tools,
            new FullTutorialActiveTask(this, static_cast<eTutorial_CompletionAction>(1)));
    addTask(TutorialState::Tools,
            new ProgressFlagTask(this, &mTutorialStarted, 2,
                                  ProgressFlagTask::eProgressFlagAction_Check));
    addTask(TutorialState::Tools,
            new CraftTask(Items::stick, -1, 1, this, TutorialIds::StickCraft, true, NULL, false, true, true));

    ProcedureCompoundTask* compound2 = new ProcedureCompoundTask(this);
    compound2->AddTask(new UICraftingTask(this, TutorialIds::ToolsUi, Recipe::eGroupType_Tools, true, NULL,
                                           false, true, true));
    compound2->AddTask(new UICraftingTask(this, TutorialIds::WoodenShovelUi, Items::woodenShovel, false, NULL,
                                           false, true, true));
    compound2->AddTask(new CraftTask(shovelSetItems, shovelSetAux, 5, 1, this, TutorialIds::ShovelSetCraft, true,
                                      NULL, false, true, true));
    addTask(TutorialState::Tools, compound2);

    addTask(TutorialState::Tools,
            new CraftTask(axeSetItems, axeSetAux, 5, 1, this, TutorialIds::AxeSetCraft, true, NULL, false, true,
                           true));
    addTask(TutorialState::Tools,
            new CraftTask(pickaxeSetItems, pickaxeSetAux, 5, 1, this, TutorialIds::PickaxeSetCraft, true, NULL,
                           false, true, true));
    addTask(TutorialState::Tools,
            new InfoTask(this, TutorialIds::ToolBasicsInfo, -1, false, 1, static_cast<ETelemetryChallenges>(0)));
    addTask(TutorialState::Tools,
            new ProgressFlagTask(this, &mTutorialStarted, 4,
                                  ProgressFlagTask::eProgressFlagAction_Check));
    addTask(TutorialState::Tools,
            new CraftTask(Item::byBlock(Blocks::furnace), -1, 1, this, TutorialIds::FurnaceCraftCreative, true, NULL,
                           false, true, true));
    addTask(TutorialState::Tools,
            new InfoTask(this, TutorialIds::SmeltingBasicsInfo, -1, false, 1, static_cast<ETelemetryChallenges>(0)));
    addTask(TutorialState::Tools,
            new CraftTask(doorSetItems, doorSetAux, 6, 1, this, TutorialIds::DoorSetCraft, true, NULL, false,
                           true, true));
    addTask(TutorialState::Tools,
            new CraftTask(Item::byBlock(Blocks::torch), -1, 1, this, TutorialIds::TorchCraft, true, NULL, false,
                           true, true));

    addTask(TutorialState::SurvivalCrafting,
            new FullTutorialActiveTask(this, static_cast<eTutorial_CompletionAction>(1)));
    addTask(TutorialState::SurvivalCrafting,
            new ProgressFlagTask(this, &mTutorialStarted, 2,
                                  ProgressFlagTask::eProgressFlagAction_Check));
    addTask(TutorialState::SurvivalCrafting,
            new CraftTask(Items::stick, -1, 1, this, TutorialIds::StickCraft, true, NULL, false, true, true));
    addTask(TutorialState::SurvivalCrafting,
            new CraftTask(shovelSetItems, shovelSetAux, 5, 1, this, TutorialIds::ShovelSetCraft, true, NULL, false,
                           true, true));
    addTask(TutorialState::SurvivalCrafting,
            new CraftTask(axeSetItems, axeSetAux, 5, 1, this, TutorialIds::AxeSetCraft, true, NULL, false, true,
                           true));
    addTask(TutorialState::SurvivalCrafting,
            new CraftTask(pickaxeSetItems, pickaxeSetAux, 5, 1, this, TutorialIds::PickaxeSetCraft, true, NULL,
                           false, true, true));
    addTask(TutorialState::SurvivalCrafting,
            new InfoTask(this, TutorialIds::ToolBasicsInfo, -1, false, 1, static_cast<ETelemetryChallenges>(0)));
    addTask(TutorialState::SurvivalCrafting,
            new ProgressFlagTask(this, &mTutorialStarted, 4,
                                  ProgressFlagTask::eProgressFlagAction_Check));
    addTask(TutorialState::SurvivalCrafting,
            new CraftTask(Item::byBlock(Blocks::furnace), -1, 1, this, TutorialIds::FurnaceCraftCreative, true, NULL,
                           false, true, true));
    addTask(TutorialState::SurvivalCrafting,
            new InfoTask(this, TutorialIds::SmeltingBasicsInfo, -1, false, 1, static_cast<ETelemetryChallenges>(0)));
    addTask(TutorialState::SurvivalCrafting,
            new CraftTask(doorSetItems, doorSetAux, 6, 1, this, TutorialIds::DoorSetCraft, true, NULL, false,
                           true, true));
    addTask(TutorialState::SurvivalCrafting,
            new CraftTask(Item::byBlock(Blocks::torch), -1, 1, this, TutorialIds::TorchCraft, true, NULL, false,
                           true, true));

    addTask(TutorialState::Furnace,
            new FullTutorialActiveTask(this, static_cast<eTutorial_CompletionAction>(1)));
    addTask(TutorialState::Furnace,
            new ProgressFlagTask(this, &mTutorialStarted, 8,
                                  ProgressFlagTask::eProgressFlagAction_Check));
    addTask(TutorialState::Furnace,
            new CraftTask(Items::coal, -1, 1, this, TutorialIds::CoalCraftCreative, true, NULL, false, true, true));
    addTask(TutorialState::Furnace,
            new InfoTask(this, TutorialIds::SmeltingInfo, TutorialIds::DefaultMessageGroup, true, 0,
                         static_cast<ETelemetryChallenges>(0)));
    addTask(TutorialState::Furnace,
            new CraftTask(Item::byBlock(Blocks::glass), -1, 1, this, TutorialIds::GlassCraftCreative, true, NULL,
                           false, true, true));

    if (gApp.getGameRuleDefinitions() != NULL)
    {
        AABB* areaMinecart = gApp.getGameRuleDefinitions()->getNamedArea(L"minecartArea");
        if (areaMinecart != NULL)
        {
            addHint(TutorialState::Initial,
                    new AreaHint(static_cast<eTutorial_Hint>(0x2c), this,
                                 TutorialState::Initial,
                                 TutorialState::Minecart, TutorialIds::MinecartArea, areaMinecart->mMinX,
                                 areaMinecart->mMinY, areaMinecart->mMinZ, areaMinecart->mMaxX,
                                 areaMinecart->mMaxY, areaMinecart->mMaxZ, true, true));
        }
    }

    if (gApp.getGameRuleDefinitions() != NULL)
    {
        AABB* areaBoat = gApp.getGameRuleDefinitions()->getNamedArea(L"boatArea");
        if (areaBoat != NULL)
        {
            addHint(TutorialState::Initial,
                    new AreaHint(static_cast<eTutorial_Hint>(0x2c), this,
                                 TutorialState::Initial,
                                 TutorialState::Boat, TutorialIds::BoatArea, areaBoat->mMinX,
                                 areaBoat->mMinY, areaBoat->mMinZ, areaBoat->mMaxX,
                                 areaBoat->mMaxY, areaBoat->mMaxZ, true, true));
        }
    }

    if (gApp.getGameRuleDefinitions() != NULL)
    {
        AABB* areaFishing = gApp.getGameRuleDefinitions()->getNamedArea(L"fishingArea");
        if (areaFishing != NULL)
        {
            addHint(TutorialState::Initial,
                    new AreaHint(static_cast<eTutorial_Hint>(0x2c), this,
                                 TutorialState::Initial,
                                 TutorialState::Fishing, TutorialIds::FishingArea, areaFishing->mMinX,
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
            addHint(TutorialState::Initial,
                    new AreaHint(static_cast<eTutorial_Hint>(0x2c), this,
                                 TutorialState::Initial,
                                 TutorialState::Invalid, TutorialIds::PistonBridgeArea,
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
            addHint(TutorialState::Initial,
                    new AreaHint(static_cast<eTutorial_Hint>(0x2c), this,
                                 TutorialState::Initial,
                                 TutorialState::Invalid, TutorialIds::ElytraArea,
                                 areaEmeraldElytra->mMinX, areaEmeraldElytra->mMinY,
                                 areaEmeraldElytra->mMinZ, areaEmeraldElytra->mMaxX,
                                 areaEmeraldElytra->mMaxY, areaEmeraldElytra->mMaxZ, true, true));
        }
        if (areaGoldElytra != NULL)
        {
            addHint(TutorialState::Initial,
                    new AreaHint(static_cast<eTutorial_Hint>(0x2c), this,
                                 TutorialState::Initial,
                                 TutorialState::Invalid, TutorialIds::ElytraArea,
                                 areaGoldElytra->mMinX, areaGoldElytra->mMinY,
                                 areaGoldElytra->mMinZ, areaGoldElytra->mMaxX,
                                 areaGoldElytra->mMaxY, areaGoldElytra->mMaxZ, true, true));
        }
        if (areaDiamondElytra != NULL)
        {
            addHint(TutorialState::Initial,
                    new AreaHint(static_cast<eTutorial_Hint>(0x2c), this,
                                 TutorialState::Initial,
                                 TutorialState::Invalid, TutorialIds::ElytraArea,
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
            eTutorial_State creativeExitStates[1] = { TutorialState::Initial };
            gameModeConstraints->AddGlobalConstraint(new ChangeStateConstraint(
                this, TutorialState::Creative, creativeExitStates, 1,
                areaCreative->mMinX, areaCreative->mMinY, areaCreative->mMinZ,
                areaCreative->mMaxX, areaCreative->mMaxY, areaCreative->mMaxZ, true, true,
                GameType::CREATIVE));

            addTask(TutorialState::Creative,
                    new ChoiceTask(this, TutorialIds::CreativeModeChoice, TutorialIds::CreativeModeChoiceFollowup, true, 0, 1,
                                   static_cast<eTutorial_CompletionAction>(3),
                                   static_cast<ETelemetryChallenges>(0xe)));
            addTask(TutorialState::Creative,
                    new InfoTask(this, TutorialIds::CreativeInventoryInfo, TutorialIds::DefaultMessageGroup, true, 0,
                                 static_cast<ETelemetryChallenges>(0)));
            addTask(TutorialState::Creative,
                    new InfoTask(this, TutorialIds::CreativeInfo, TutorialIds::DefaultMessageGroup, true, 0,
                                 static_cast<ETelemetryChallenges>(0)));

            int creativeMapping[1] = { 40 };
            addTask(TutorialState::Creative,
                    new ControllerTask(this, TutorialIds::CreativeController, false, false, creativeMapping, 1, NULL,
                                        0, NULL, 0));

            addTask(TutorialState::Creative,
                    new StateChangeTask(TutorialState::CreativeCompletion, this, -1, false, NULL,
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
                compound3->AddTask(new AreaTask(TutorialState::Creative, this,
                                                 creativeExitConstraints, TutorialIds::CreativeExitArea,
                                                 AreaTask::CompletionStateOnConstraints));
            }

            std::vector<TutorialConstraint*>* creativeAreaConstraints =
                new std::vector<TutorialConstraint*>();
            creativeAreaConstraints->push_back(new AreaConstraint(
                TutorialIds::TutorialArea, areaCreative->mMinX, areaCreative->mMinY, areaCreative->mMinZ,
                areaCreative->mMaxX, areaCreative->mMaxY, areaCreative->mMaxZ, true, true, false,
                2));
            compound3->AddTask(new AreaTask(TutorialState::Creative, this,
                                             creativeAreaConstraints, -1,
                                             AreaTask::CompletionStateOnStateChange));

            compound3->AddTask(new InfoTask(this, TutorialIds::CreativeExitInfo, TutorialIds::DefaultMessageGroup, true, 0,
                                             static_cast<ETelemetryChallenges>(0)));

            addTask(TutorialState::Creative, compound3);
        }
    }

    AddTutorialArea(TutorialState::Nether);
    AddTutorialArea(TutorialState::End);
    AddTutorialArea(TutorialState::AreaState11);
    AddTutorialArea(TutorialState::AreaState13);
    AddTutorialArea(TutorialState::AreaState1A);
    AddTutorialArea(TutorialState::AreaState18);
    AddTutorialArea(TutorialState::AreaState23);
    AddTutorialArea(TutorialState::AreaState21);
    AddTutorialArea(TutorialState::AreaState1F);
    AddTutorialArea(TutorialState::AreaState1C);
    AddTutorialArea(TutorialState::AreaState15);
    AddTutorialArea(TutorialState::AreaState16);
    AddTutorialArea(TutorialState::AreaState17);
    AddTutorialArea(TutorialState::AreaState26);
    AddTutorialArea(TutorialState::AreaState27);
    AddTutorialArea(TutorialState::AreaState28);
    AddTutorialArea(TutorialState::AreaState2A);
    AddTutorialArea(TutorialState::AreaState2B);
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

        if (state == TutorialState::Completed ||
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
