#include "game/minigame/Tutorial.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/ChoiceTask.hpp"
#include "game/core/CoralFanHang.hpp"
#include "game/core/Direction.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/GameModeConstraints.hpp"
#include "game/core/InfoTask.hpp"
#include "game/core/ProcedureCompoundTask.hpp"
#include "game/entity/mob/AbstractHorse.hpp"
#include "game/entity/mob/HorseChoiceTask.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LookAtEntityHint.hpp"
#include "game/item/DiggerItemHint.hpp"
#include "game/item/Item.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/world/generation/Player.hpp"
#include "game/minigame/TutorialHint.hpp"
#include "game/minigame/TutorialMessage.hpp"
#include "game/minigame/TutorialTask.hpp"
#include "game/minigame/TutorialConstraint.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/L10N.hpp"
#include "game/world/block/LookAtBlockHint.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/CoralBlock.hpp"
#include "game/world/block/DirtBlock.hpp"
#include "game/world/block/DoublePlantBlock.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "vendor/_TutorialPopupInfo.hpp"
#include "platform/wiiu/WiiU_Runtime.hpp"

extern ConsoleUIController gConsoleUIController;

namespace
{
    const unsigned int gTutorialStartDelay = 1500;
    unsigned int gTutorialHintRepeatTime = 14000;
    unsigned int gTutorialVisibleTime = 7000;
    unsigned int gTutorialMinimumDisplayTime = 2000;
    unsigned int gTutorialMessageRepeatDelay = 13000;
    long long gTutorialDayTime = 8000;
    unsigned int gTutorialMessageRepeatTime = gTutorialVisibleTime + gTutorialMessageRepeatDelay;
    std::vector<int> gTutorialCompletionIds;
    static const int gTutorialCompletionIdValues[] = { 1, 2, 3, 4, 5, 6, 7, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 65, 66, 67, 68, 69, 70, 72, 71, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 101, 102, 103, 104, 105, 107, 106, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 256, 257, 258, 259, 260, 261, 262, 263, 264, 265, 266, 267, 268, 269, 270, 271, 272, 273, 274, 275, 276, 277, 278, 279, 280, 281, 282, 283, 284, 285, 286, 287, 288, 289, 290, 291, 292, 293, 294, 295, 296, 297, 386, 387, 388, 389, 390, 391, 392, 393, 394, 395, 396, 397, 398, 20, 21, 22, 23, 24, 25, 26, 27, 28, 30, 32, 188, 190, 191, 192, 298, 299, 300, 301, 302, 303, 304, 305, 306, 307, 34, 36, 29, 31, 33, 35, 189, 8, 193, 194, 195, 309, 310, 311, 312, 313, 308, 196, 100, 64, 314, 315, 316, 317, 318, 319, 149, 150, 151, 152, 153, 197, 198, 199, 200, 38, 39, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 350, 351, 352, 353, 354, 355, 356, 357, 358, 359, 360, 361, 362, 363, 364, 365, 366, 367, 368, 369, 370, 371, 372, 373, 374, 375, 376, 377, 378, 379, 380, 381, 383, 384 };
}

Tutorial::PopupMessageDetails::PopupMessageDetails()
    : titleId(-1), messageId(-1), promptId(-1), title(), message(), itemDescription(),
      itemInstance(), displayTime(-1), priority(0), repeat(true), unknown(false),
      displayed(false), allowFade(false), isItem(false), force(false), state(-1)
{
}

unsigned int Tutorial::PopupMessageDetails::isSameContent(PopupMessageDetails* details)
{
    if (details == NULL)
    {
        return 0;
    }
    return (titleId == details->titleId && messageId == details->messageId &&
            promptId == details->promptId && title == details->title &&
            message == details->message && itemDescription == details->itemDescription) ? 1U : 0U;
}

Tutorial::Tutorial(int playerIdValue, GameModeConstraints* constraints, bool enabledValue,
                   bool skipSetup)
    : currentState(eTutorial_State_Gameplay), stateChanged(false), started(false), startTime(0),
      gameModeConstraints(constraints), messages(), tasks(), currentTask(),
      completedStates(), overridesGameHostOptions(false), gameHostOptionsOverridden(false),
      gameMode(eTutorial_GameMode_ChestRefill), gameModeChanged(false), hintShown(false),
      playerId(playerIdValue), visibleTime(0), currentMessage(NULL), messageState(0),
      messageRepeatCount(0), popupVisible(true), popupPending(false), uiControllerSet(false),
      tutorialHidden(false), currentScene(NULL), tasksChecked(false), completed(false),
      enabled(enabledValue)
{
    if (!skipSetup)
    {
        AddHints();
        AddTasks(enabledValue);
    }
}

Tutorial::~Tutorial()
{
    boost::unordered_map<int, TutorialMessage*>::iterator message = messages.begin();
    while (message != messages.end())
    {
        delete message->second;
        ++message;
    }
    delete currentMessage;
}

void Tutorial::addMessage(int messageId, bool repeatMessage, unsigned char maxDisplayCount)
{
    if (messageId == -1 || messages.find(messageId) != messages.end())
    {
        return;
    }
    messages[messageId] = new TutorialMessage(messageId, repeatMessage, maxDisplayCount);
}

unsigned int Tutorial::getCompleted(int completionId)
{
    unsigned int index = static_cast<unsigned int>(-1);
    for (std::vector<int>::const_iterator it = gTutorialCompletionIds.begin();
         it != gTutorialCompletionIds.end(); ++it)
    {
        ++index;
        if (*it == completionId)
            return index < 0x200 ? (GameSettings::GetTutorialCompletionFlag(playerId, static_cast<int>(index)) ? 1U : 0U) : 0U;
    }
    return 0;
}

unsigned int Tutorial::isHintCompleted(eTutorial_Hint hint)
{
    return getCompleted(hint);
}

void Tutorial::addHint(eTutorial_State state, TutorialHint* hint)
{
    stateHints[state].push_back(hint);
}

void Tutorial::addHintIfIncomplete(eTutorial_State state, TutorialHint* hint)
{
    if (!isHintCompleted(hint->hint))
    {
        addHint(state, hint);
        return;
    }
    delete hint;
}

void Tutorial::AddHints()
{
    addHintIfIncomplete(eTutorial_State_Gameplay, new TutorialHint(eTutorial_Hint_HoldToMine, this, IDS_TUTORIAL_HINT_HOLD_TO_MINE, TutorialHint::HINT_START_DESTROY_BLOCK, true, true, -1));
    addHintIfIncomplete(eTutorial_State_Gameplay, new TutorialHint(eTutorial_Hint_ToolDamaged, this, IDS_TUTORIAL_HINT_TOOL_DAMAGED, TutorialHint::HINT_ITEM_DAMAGED, true, true, -1));
    addHintIfIncomplete(eTutorial_State_Gameplay, new TutorialHint(eTutorial_Hint_SwimUp, this, IDS_TUTORIAL_HINT_SWIM_UP, TutorialHint::HINT_WATER_MOVEMENT, true, true, -1));
    const BlockState* stoneStates[] = { Blocks::stone->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Stone, this, stoneStates, 1, NULL, 0, -1, -1, false));
    const BlockState* cobblestoneStates[] = { Blocks::cobblestone->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Cobblestone, this, cobblestoneStates, 1, NULL, 0, -1, -1, false));
    const BlockState* planksStates[] = { Blocks::planks->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Planks, this, planksStates, 1, NULL, 0, -1, -1, false));
    const BlockState* saplingStates[] = { Blocks::sapling->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Sapling, this, saplingStates, 1, NULL, 0, -1, -1, false));
    const BlockState* bedrockStates[] = { Blocks::bedrock->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Bedrock, this, bedrockStates, 1, NULL, 0, -1, -1, false));
    const BlockState* waterStates[] = { Blocks::water->defaultBlockState(), Blocks::flowing_water->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Water, this, waterStates, 2, NULL, 0, -1, -1, false));
    const BlockState* lavaStates[] = { Blocks::lava->defaultBlockState(), Blocks::flowing_lava->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Lava, this, lavaStates, 2, NULL, 0, -1, -1, false));
    const BlockState* magmaStates[] = { Blocks::magma->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Magma, this, magmaStates, 1, NULL, 0, -1, -1, false));
    const BlockState* sandStates[] = { Blocks::sand->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Sand, this, sandStates, 1, NULL, 0, -1, -1, false));
    const BlockState* gravelStates[] = { Blocks::gravel->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Gravel, this, gravelStates, 1, NULL, 0, -1, -1, false));
    const BlockState* goldOreStates[] = { Blocks::gold_ore->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_GoldOre, this, goldOreStates, 1, NULL, 0, -1, -1, false));
    const BlockState* ironOreStates[] = { Blocks::iron_ore->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_IronOre, this, ironOreStates, 1, NULL, 0, -1, -1, false));
    const BlockState* coalOreStates[] = { Blocks::coal_ore->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_CoalOre, this, coalOreStates, 1, NULL, 0, -1, -1, false));
    const BlockState* logStates[] = { Blocks::log->defaultBlockState(), Blocks::log2->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Log, this, logStates, 2, NULL, 0, -1, -1, false));
    const BlockState* leavesStates[] = { Blocks::leaves->defaultBlockState(), Blocks::leaves2->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Leaves, this, leavesStates, 2, NULL, 0, -1, -1, false));
    const BlockState* glassStates[] = { Blocks::glass->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Glass, this, glassStates, 1, NULL, 0, -1, -1, false));
    const BlockState* lapisOreStates[] = { Blocks::lapis_ore->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_LapisOre, this, lapisOreStates, 1, NULL, 0, -1, -1, false));
    const BlockState* lapisBlockStates[] = { Blocks::lapis_block->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_LapisBlock, this, lapisBlockStates, 1, NULL, 0, -1, -1, false));
    const BlockState* dispenserStates[] = { Blocks::dispenser->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Dispenser, this, dispenserStates, 1, NULL, 0, -1, -1, false));
    const BlockState* sandstoneStates[] = { Blocks::sandstone->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Sandstone, this, sandstoneStates, 1, NULL, 0, -1, -1, false));
    const BlockState* noteblockStates[] = { Blocks::noteblock->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Noteblock, this, noteblockStates, 1, NULL, 0, -1, -1, false));
    const BlockState* goldenRailStates[] = { Blocks::golden_rail->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_GoldenRail, this, goldenRailStates, 1, NULL, 0, -1, -1, false));
    const BlockState* detectorRailStates[] = { Blocks::detector_rail->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_DetectorRail, this, detectorRailStates, 1, NULL, 0, -1, -1, false));
    const BlockState* tallgrassStates[] = { Blocks::tallgrass->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Tallgrass, this, tallgrassStates, 1, NULL, 0, -1, -1, false));
    const BlockState* woolStates[] = { Blocks::wool->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Wool, this, woolStates, 1, NULL, 0, -1, -1, false));
    const BlockState* redFlowerStates[] = { Blocks::red_flower->defaultBlockState(), Blocks::yellow_flower->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_RedFlower, this, redFlowerStates, 2, NULL, 0, -1, -1, false));
    const BlockState* redMushroomStates[] = { Blocks::red_mushroom->defaultBlockState(), Blocks::brown_mushroom->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_RedMushroom, this, redMushroomStates, 2, NULL, 0, -1, -1, false));
    const BlockState* goldBlockStates[] = { Blocks::gold_block->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_GoldBlock, this, goldBlockStates, 1, NULL, 0, -1, -1, false));
    const BlockState* ironBlockStates[] = { Blocks::iron_block->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_IronBlock, this, ironBlockStates, 1, NULL, 0, -1, -1, false));
    const BlockState* stoneSlabStates[] = { Blocks::stone_slab->defaultBlockState(), Blocks::stone_slab2->defaultBlockState(), Blocks::double_stone_slab->defaultBlockState(), Blocks::double_stone_slab2->defaultBlockState(), Blocks::purpur_slab->defaultBlockState(), Blocks::prismarine_slab->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_StoneSlab, this, stoneSlabStates, 6, NULL, 0, -1, -1, false));
    const BlockState* woodenSlabStates[] = { Blocks::wooden_slab->defaultBlockState(), Blocks::double_wooden_slab->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_StoneSlab, this, woodenSlabStates, 2, NULL, 0, -1, -1, false));
    const BlockState* brickBlockStates[] = { Blocks::brick_block->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_BrickBlock, this, brickBlockStates, 1, NULL, 0, -1, -1, false));
    const BlockState* tntStates[] = { Blocks::tnt->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Tnt, this, tntStates, 1, NULL, 0, -1, -1, false));
    const BlockState* bookshelfStates[] = { Blocks::bookshelf->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Bookshelf, this, bookshelfStates, 1, NULL, 0, -1, -1, false));
    const BlockState* mossyCobblestoneStates[] = { Blocks::mossy_cobblestone->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_MossyCobblestone, this, mossyCobblestoneStates, 1, NULL, 0, -1, -1, false));
    const BlockState* obsidianStates[] = { Blocks::obsidian->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Obsidian, this, obsidianStates, 1, NULL, 0, -1, -1, false));
    const BlockState* torchStates[] = { Blocks::torch->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Torch, this, torchStates, 1, NULL, 0, -1, -1, false));
    const BlockState* mobSpawnerStates[] = { Blocks::mob_spawner->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_MobSpawner, this, mobSpawnerStates, 1, NULL, 0, -1, -1, false));
    const BlockState* chestStates[] = { Blocks::chest->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Chest, this, chestStates, 1, NULL, 0, -1, -1, false));
    const BlockState* redstoneWireStates[] = { Blocks::redstone_wire->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_RedstoneWire, this, redstoneWireStates, 1, NULL, 0, Item::getId(Items::redstone), -1, false));
    const BlockState* diamondOreStates[] = { Blocks::diamond_ore->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_DiamondOre, this, diamondOreStates, 1, NULL, 0, -1, -1, false));
    const BlockState* diamondBlockStates[] = { Blocks::diamond_block->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_DiamondBlock, this, diamondBlockStates, 1, NULL, 0, -1, -1, false));
    const BlockState* craftingTableStates[] = { Blocks::crafting_table->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_CraftingTable, this, craftingTableStates, 1, NULL, 0, -1, -1, false));
    const BlockState* wheatStates[] = { Blocks::wheat->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Wheat, this, wheatStates, 1, NULL, 0, -1, 7, false));
    const BlockState* beetrootsStates[] = { Blocks::beetroots->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Beetroots, this, beetrootsStates, 1, NULL, 0, -1, 7, false));
    const BlockState* farmlandStates[] = { Blocks::farmland->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Farmland, this, farmlandStates, 1, NULL, 0, -1, -1, false));
    const BlockState* furnaceStates[] = { Blocks::furnace->defaultBlockState(), Blocks::lit_furnace->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Furnace, this, furnaceStates, 2, NULL, 0, -1, -1, false));
    const BlockState* wallSignStates[] = { Blocks::wall_sign->defaultBlockState(), Blocks::standing_sign->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_WallSign, this, wallSignStates, 2, NULL, 0, Item::getId(Items::sign), -1, false));
    const BlockState* woodenDoorStates[] = { Blocks::wooden_door->defaultBlockState(), Blocks::spruce_door->defaultBlockState(), Blocks::birch_door->defaultBlockState(), Blocks::jungle_door->defaultBlockState(), Blocks::acacia_door->defaultBlockState(), Blocks::dark_oak_door->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_WoodenDoor, this, woodenDoorStates, 6, NULL, 0, -1, -1, true));
    const BlockState* ladderStates[] = { Blocks::ladder->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Ladder, this, ladderStates, 1, NULL, 0, -1, -1, false));
    const BlockState* stoneStairsStates[] = { Blocks::stone_stairs->defaultBlockState(), Blocks::brick_stairs->defaultBlockState(), Blocks::stone_brick_stairs->defaultBlockState(), Blocks::nether_brick_stairs->defaultBlockState(), Blocks::red_sandstone_stairs->defaultBlockState(), Blocks::quartz_stairs->defaultBlockState(), Blocks::purpur_stairs->defaultBlockState(), Blocks::oak_stairs->defaultBlockState(), Blocks::spruce_stairs->defaultBlockState(), Blocks::birch_stairs->defaultBlockState(), Blocks::jungle_stairs->defaultBlockState(), Blocks::acacia_stairs->defaultBlockState(), Blocks::dark_oak_stairs->defaultBlockState(), Blocks::prismarine_stairs->defaultBlockState(), Blocks::prismarine_bricks_stairs->defaultBlockState(), Blocks::dark_prismarine_stairs->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_StoneStairs, this, stoneStairsStates, 16, NULL, 0, -1, -1, false));
    const BlockState* railStates[] = { Blocks::rail->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Rail, this, railStates, 1, NULL, 0, -1, -1, false));
    const BlockState* leverStates[] = { Blocks::lever->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Lever, this, leverStates, 1, NULL, 0, -1, -1, false));
    const BlockState* stonePressurePlateStates[] = { Blocks::stone_pressure_plate->defaultBlockState(), Blocks::wooden_pressure_plate->defaultBlockState(), Blocks::acacia_pressure_plate->defaultBlockState(), Blocks::birch_pressure_plate->defaultBlockState(), Blocks::dark_oak_pressure_plate->defaultBlockState(), Blocks::jungle_pressure_plate->defaultBlockState(), Blocks::spruce_pressure_plate->defaultBlockState(), Blocks::light_weighted_pressure_plate->defaultBlockState(), Blocks::heavy_weighted_pressure_plate->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_StonePressurePlate, this, stonePressurePlateStates, 9, NULL, 0, -1, -1, false));
    const BlockState* ironDoorStates[] = { Blocks::iron_door->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_IronDoor, this, ironDoorStates, 1, NULL, 0, Item::getId(Items::ironDoor), -1, false));
    const BlockState* redstoneOreStates[] = { Blocks::redstone_ore->defaultBlockState(), Blocks::lit_redstone_ore->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_RedstoneOre, this, redstoneOreStates, 2, NULL, 0, -1, -1, false));
    const BlockState* redstoneTorchStates[] = { Blocks::redstone_torch->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_RedstoneTorch, this, redstoneTorchStates, 1, NULL, 0, -1, -1, false));
    const BlockState* stoneButtonStates[] = { Blocks::stone_button->defaultBlockState(), Blocks::wooden_button->defaultBlockState(), Blocks::acacia_button->defaultBlockState(), Blocks::birch_button->defaultBlockState(), Blocks::dark_oak_button->defaultBlockState(), Blocks::jungle_button->defaultBlockState(), Blocks::spruce_button->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_StoneButton, this, stoneButtonStates, 7, NULL, 0, -1, -1, false));
    if (!isHintCompleted(eTutorial_Hint_Snow))
    {
        const BlockState* snowStates[] = { Blocks::snow->defaultBlockState() };
        addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Snow, this, snowStates, 1, NULL, 0, -1, -1, false));
    }
    const BlockState* iceStates[] = { Blocks::ice->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Ice, this, iceStates, 1, NULL, 0, -1, -1, false));
    const BlockState* cactusStates[] = { Blocks::cactus->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Cactus, this, cactusStates, 1, NULL, 0, -1, -1, false));
    const BlockState* clayStates[] = { Blocks::clay->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Clay, this, clayStates, 1, NULL, 0, -1, -1, false));
    const BlockState* reedsStates[] = { Blocks::reeds->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Reeds, this, reedsStates, 1, NULL, 0, -1, -1, false));
    const BlockState* jukeboxStates[] = { Blocks::jukebox->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Jukebox, this, jukeboxStates, 1, NULL, 0, -1, -1, false));
    const BlockState* pumpkinStates[] = { Blocks::pumpkin->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Pumpkin, this, pumpkinStates, 1, NULL, 0, -1, 0, false));
    const BlockState* netherrackStates[] = { Blocks::netherrack->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Netherrack, this, netherrackStates, 1, NULL, 0, -1, -1, false));
    const BlockState* soulSandStates[] = { Blocks::soul_sand->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_SoulSand, this, soulSandStates, 1, NULL, 0, -1, -1, false));
    const BlockState* glowstoneStates[] = { Blocks::glowstone->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Glowstone, this, glowstoneStates, 1, NULL, 0, -1, -1, false));
    const BlockState* portalStates[] = { Blocks::portal->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Portal, this, portalStates, 1, NULL, 0, -1, -1, false));
    const BlockState* litPumpkinStates[] = { Blocks::lit_pumpkin->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_LitPumpkin, this, litPumpkinStates, 1, NULL, 0, -1, 0, false));
    const BlockState* cakeStates[] = { Blocks::cake->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Cake, this, cakeStates, 1, NULL, 0, -1, -1, false));
    const BlockState* poweredRepeaterStates[] = { Blocks::powered_repeater->defaultBlockState(), Blocks::unpowered_repeater->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_PoweredRepeater, this, poweredRepeaterStates, 2, NULL, 0, Item::getId(Items::repeater), -1, false));
    const BlockState* trapdoorStates[] = { Blocks::trapdoor->defaultBlockState(), Blocks::spruce_trapdoor->defaultBlockState(), Blocks::birch_trapdoor->defaultBlockState(), Blocks::jungle_trapdoor->defaultBlockState(), Blocks::acacia_trapdoor->defaultBlockState(), Blocks::dark_oak_trapdoor->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Trapdoor, this, trapdoorStates, 6, NULL, 0, -1, -1, false));
    const BlockState* pistonStates[] = { Blocks::piston->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Piston, this, pistonStates, 1, NULL, 0, -1, -1, false));
    const BlockState* stickyPistonStates[] = { Blocks::sticky_piston->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_StickyPiston, this, stickyPistonStates, 1, NULL, 0, -1, -1, false));
    const BlockState* monsterEggStates[] = { Blocks::monster_egg->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_MonsterEgg, this, monsterEggStates, 1, NULL, 0, -1, -1, false));
    const BlockState* stonebrickStates[] = { Blocks::stonebrick->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Stonebrick, this, stonebrickStates, 1, NULL, 0, -1, -1, false));
    const BlockState* brownMushroomBlockStates[] = { Blocks::brown_mushroom_block->defaultBlockState(), Blocks::red_mushroom_block->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_BrownMushroomBlock, this, brownMushroomBlockStates, 2, NULL, 0, -1, -1, false));
    const BlockState* ironBarsStates[] = { Blocks::iron_bars->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_IronBars, this, ironBarsStates, 1, NULL, 0, -1, -1, false));
    const BlockState* glassPaneStates[] = { Blocks::glass_pane->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_GlassPane, this, glassPaneStates, 1, NULL, 0, -1, -1, false));
    const BlockState* melonBlockStates[] = { Blocks::melon_block->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_MelonBlock, this, melonBlockStates, 1, NULL, 0, -1, -1, false));
    const BlockState* vineStates[] = { Blocks::vine->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Vine, this, vineStates, 1, NULL, 0, -1, -1, false));
    const BlockState* fenceGateStates[] = { Blocks::fence_gate->defaultBlockState(), Blocks::spruce_fence_gate->defaultBlockState(), Blocks::birch_fence_gate->defaultBlockState(), Blocks::jungle_fence_gate->defaultBlockState(), Blocks::acacia_fence_gate->defaultBlockState(), Blocks::dark_oak_fence_gate->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_FenceGate, this, fenceGateStates, 6, NULL, 0, -1, -1, false));
    const BlockState* myceliumStates[] = { Blocks::mycelium->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Mycelium, this, myceliumStates, 1, NULL, 0, -1, -1, false));
    const BlockState* waterlilyStates[] = { Blocks::waterlily->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Waterlily, this, waterlilyStates, 1, NULL, 0, -1, -1, false));
    const BlockState* netherBrickStates[] = { Blocks::nether_brick->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_NetherBrick, this, netherBrickStates, 1, NULL, 0, -1, -1, false));
    const BlockState* netherBrickFenceStates[] = { Blocks::nether_brick_fence->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_NetherBrickFence, this, netherBrickFenceStates, 1, NULL, 0, -1, -1, false));
    const BlockState* netherWartStates[] = { Blocks::nether_wart->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_NetherWart, this, netherWartStates, 1, NULL, 0, -1, -1, false));
    const BlockState* enchantingTableStates[] = { Blocks::enchanting_table->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_EnchantingTable, this, enchantingTableStates, 1, NULL, 0, -1, -1, false));
    const BlockState* brewingStandStates[] = { Blocks::brewing_stand->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_BrewingStand, this, brewingStandStates, 1, NULL, 0, Item::getId(Items::brewingStand), -1, false));
    const BlockState* cauldronStates[] = { Blocks::cauldron->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Cauldron, this, cauldronStates, 1, NULL, 0, Item::getId(Items::cauldron), -1, false));
    const BlockState* endPortalStates[] = { Blocks::end_portal->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_EndPortal, this, endPortalStates, 1, NULL, 0, -2, -1, false));
    const BlockState* endPortalFrameStates[] = { Blocks::end_portal_frame->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_EndPortalFrame, this, endPortalFrameStates, 1, NULL, 0, -1, -1, false));
    const BlockState* endStoneStates[] = { Blocks::end_stone->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_EndStone, this, endStoneStates, 1, NULL, 0, -1, -1, false));
    const BlockState* dragonEggStates[] = { Blocks::dragon_egg->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_DragonEgg, this, dragonEggStates, 1, NULL, 0, -1, -1, false));
    const BlockState* redstoneLampStates[] = { Blocks::redstone_lamp->defaultBlockState(), Blocks::lit_redstone_lamp->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_RedstoneLamp, this, redstoneLampStates, 2, NULL, 0, -1, -1, false));
    const BlockState* cocoaStates[] = { Blocks::cocoa->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Cocoa, this, cocoaStates, 1, NULL, 0, Item::getId(Items::dye), DyeColor::BROWN->getItemData(), false));
    const BlockState* emeraldOreStates[] = { Blocks::emerald_ore->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_EmeraldOre, this, emeraldOreStates, 1, NULL, 0, -1, -1, false));
    const BlockState* emeraldBlockStates[] = { Blocks::emerald_block->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_EmeraldBlock, this, emeraldBlockStates, 1, NULL, 0, -1, -1, false));
    const BlockState* enderChestStates[] = { Blocks::ender_chest->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_EnderChest, this, enderChestStates, 1, NULL, 0, -1, -1, false));
    const BlockState* tripwireHookStates[] = { Blocks::tripwire_hook->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_TripwireHook, this, tripwireHookStates, 1, NULL, 0, -1, -1, false));
    const BlockState* tripwireStates[] = { Blocks::tripwire->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Tripwire, this, tripwireStates, 1, NULL, 0, Item::getId(Items::string), -1, false));
    const BlockState* cobblestoneWallStates[] = { Blocks::cobblestone_wall->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_CobblestoneWall, this, cobblestoneWallStates, 1, NULL, 0, -1, -1, false));
    const BlockState* flowerPotStates[] = { Blocks::flower_pot->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_FlowerPot, this, flowerPotStates, 1, NULL, 0, Item::getId(Items::flowerPot), -1, false));
    const BlockState* anvilStates[] = { Blocks::anvil->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Anvil, this, anvilStates, 1, NULL, 0, -1, -1, false));
    const BlockState* quartzOreStates[] = { Blocks::quartz_ore->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_QuartzOre, this, quartzOreStates, 1, NULL, 0, -1, -1, false));
    const BlockState* quartzBlockStates[] = { Blocks::quartz_block->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_QuartzBlock, this, quartzBlockStates, 1, NULL, 0, -1, -1, false));
    const BlockState* carpetStates[] = { Blocks::carpet->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Carpet, this, carpetStates, 1, NULL, 0, -1, -1, false));
    const BlockState* potatoesStates[] = { Blocks::potatoes->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Potatoes, this, potatoesStates, 1, NULL, 0, -1, 7, false));
    const BlockState* carrotsStates[] = { Blocks::carrots->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Carrots, this, carrotsStates, 1, NULL, 0, -1, 7, false));
    const BlockState* commandBlockStates[] = { Blocks::command_block->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_CommandBlock, this, commandBlockStates, 1, NULL, 0, -1, -1, false));
    const BlockState* beaconStates[] = { Blocks::beacon->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Beacon, this, beaconStates, 1, NULL, 0, -1, -1, false));
    const BlockState* activatorRailStates[] = { Blocks::activator_rail->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_ActivatorRail, this, activatorRailStates, 1, NULL, 0, -1, -1, false));
    const BlockState* redstoneBlockStates[] = { Blocks::redstone_block->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_RedstoneBlock, this, redstoneBlockStates, 1, NULL, 0, -1, -1, false));
    const BlockState* daylightDetectorStates[] = { Blocks::daylight_detector->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_DaylightDetector, this, daylightDetectorStates, 1, NULL, 0, -1, -1, false));
    const BlockState* dropperStates[] = { Blocks::dropper->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Dropper, this, dropperStates, 1, NULL, 0, -1, -1, false));
    const BlockState* hopperStates[] = { Blocks::hopper->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Hopper, this, hopperStates, 1, NULL, 0, -1, -1, false));
    const BlockState* poweredComparatorStates[] = { Blocks::powered_comparator->defaultBlockState(), Blocks::unpowered_comparator->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_PoweredComparator, this, poweredComparatorStates, 2, NULL, 0, Item::getId(Items::comparator), -1, false));
    const BlockState* trappedChestStates[] = { Blocks::trapped_chest->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_TrappedChest, this, trappedChestStates, 1, NULL, 0, -1, -1, false));
    const BlockState* hayBlockStates[] = { Blocks::hay_block->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_HayBlock, this, hayBlockStates, 1, NULL, 0, -1, -1, false));
    const BlockState* hardenedClayStates[] = { Blocks::hardened_clay->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_HardenedClay, this, hardenedClayStates, 1, NULL, 0, -1, -1, false));
    const BlockState* stainedHardenedClayStates[] = { Blocks::stained_hardened_clay->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_StainedHardenedClay, this, stainedHardenedClayStates, 1, NULL, 0, -1, -1, false));
    const BlockState* coalBlockStates[] = { Blocks::coal_block->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_CoalBlock, this, coalBlockStates, 1, NULL, 0, -1, -1, false));
    const BlockState* spongeStates[] = { Blocks::sponge->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Sponge, this, spongeStates, 1, NULL, 0, -1, -1, false));
    const BlockState* prismarineStates[] = { Blocks::prismarine->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Prismarine, this, prismarineStates, 1, NULL, 0, -1, -1, false));
    const BlockState* seaLanternStates[] = { Blocks::sea_lantern->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_SeaLantern, this, seaLanternStates, 1, NULL, 0, -1, -1, false));
    const BlockState* slimeStates[] = { Blocks::slime->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_SlimeBlock, this, slimeStates, 1, NULL, 0, -1, -1, false));
    TypedBoxed<DoublePlantBlock::Variant*> doublePlantValue(DoublePlantBlock::Variant::byData(0));
    TypedBoxed<DoublePlantBlock::Variant*> doublePlantValue2(DoublePlantBlock::Variant::byData(1));
    TypedBoxed<DoublePlantBlock::Variant*> doublePlantValue3(DoublePlantBlock::Variant::byData(2));
    TypedBoxed<DoublePlantBlock::Variant*> doublePlantValue4(DoublePlantBlock::Variant::byData(3));
    TypedBoxed<DoublePlantBlock::Variant*> doublePlantValue5(DoublePlantBlock::Variant::byData(4));
    TypedBoxed<DoublePlantBlock::Variant*> doublePlantValue6(DoublePlantBlock::Variant::byData(5));
    const BlockState* doublePlantStates[] = { Blocks::double_plant->defaultBlockState()->setBoxedValue(DoublePlantBlock::VARIANT, &doublePlantValue), Blocks::double_plant->defaultBlockState()->setBoxedValue(DoublePlantBlock::VARIANT, &doublePlantValue2), Blocks::double_plant->defaultBlockState()->setBoxedValue(DoublePlantBlock::VARIANT, &doublePlantValue3), Blocks::double_plant->defaultBlockState()->setBoxedValue(DoublePlantBlock::VARIANT, &doublePlantValue4), Blocks::double_plant->defaultBlockState()->setBoxedValue(DoublePlantBlock::VARIANT, &doublePlantValue5), Blocks::double_plant->defaultBlockState()->setBoxedValue(DoublePlantBlock::VARIANT, &doublePlantValue6) };
    const Property* doublePlantProperties[] = { DoublePlantBlock::VARIANT };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_DoublePlant, this, doublePlantStates, 6, doublePlantProperties, 1, -1, -1, true));
    const BlockState* redSandstoneStates[] = { Blocks::red_sandstone->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_RedSandstone, this, redSandstoneStates, 1, NULL, 0, -1, -1, false));
    const BlockState* endBricksStates[] = { Blocks::end_bricks->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_EndBricks, this, endBricksStates, 1, NULL, 0, -1, -1, false));
    const BlockState* redNetherBrickStates[] = { Blocks::red_nether_brick->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_RedNetherBrick, this, redNetherBrickStates, 1, NULL, 0, -1, -1, false));
    const BlockState* boneBlockStates[] = { Blocks::bone_block->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_BoneBlock, this, boneBlockStates, 1, NULL, 0, -1, -1, false));
    const BlockState* grassPathStates[] = { Blocks::grass_path->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_GrassPath, this, grassPathStates, 1, NULL, 0, -1, -1, false));
    const BlockState* netherWartBlockStates[] = { Blocks::nether_wart_block->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_NetherWartBlock, this, netherWartBlockStates, 1, NULL, 0, -1, -1, false));
    const BlockState* standingBannerStates[] = { Blocks::standing_banner->defaultBlockState(), Blocks::wall_banner->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_StandingBanner, this, standingBannerStates, 2, NULL, 0, -1, -1, false));
    const BlockState* chorusFlowerStates[] = { Blocks::chorus_flower->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_ChorusFlower, this, chorusFlowerStates, 1, NULL, 0, -1, -1, false));
    const BlockState* chorusPlantStates[] = { Blocks::chorus_plant->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_ChorusPlant, this, chorusPlantStates, 1, NULL, 0, -1, -1, false));
    const BlockState* endGatewayStates[] = { Blocks::end_gateway->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_EndGateway, this, endGatewayStates, 1, NULL, 0, -2, -1, false));
    const BlockState* endRodStates[] = { Blocks::end_rod->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_EndRod, this, endRodStates, 1, NULL, 0, -1, Direction::UP->get3DDataValue(), false));
    const BlockState* purpurBlockStates[] = { Blocks::purpur_block->defaultBlockState(), Blocks::purpur_pillar->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_PurpurBlock, this, purpurBlockStates, 2, NULL, 0, -1, -1, false));
    const BlockState* grassStates[] = { Blocks::grass->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Grass, this, grassStates, 1, NULL, 0, -1, -1, false));
    TypedBoxed<DirtBlock::Variant*> dirtValue(DirtBlock::Variant::byData(0));
    const BlockState* dirtStates[] = { Blocks::dirt->defaultBlockState()->setBoxedValue(DirtBlock::VARIANT, &dirtValue) };
    const Property* dirtProperties[] = { DirtBlock::VARIANT };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Dirt, this, dirtStates, 1, dirtProperties, 1, -1, -1, true));
    TypedBoxed<DirtBlock::Variant*> dirtValue2(DirtBlock::Variant::byData(1));
    const BlockState* dirtStates2[] = { Blocks::dirt->defaultBlockState()->setBoxedValue(DirtBlock::VARIANT, &dirtValue2) };
    const Property* dirtProperties2[] = { DirtBlock::VARIANT };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_CoarseDirt, this, dirtStates2, 1, dirtProperties2, 1, -1, -1, true));
    TypedBoxed<DirtBlock::Variant*> dirtValue3(DirtBlock::Variant::byData(2));
    const BlockState* dirtStates3[] = { Blocks::dirt->defaultBlockState()->setBoxedValue(DirtBlock::VARIANT, &dirtValue3) };
    const Property* dirtProperties3[] = { DirtBlock::VARIANT };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Podzol, this, dirtStates3, 1, dirtProperties3, 1, -1, -1, true));
    const BlockState* fenceStates[] = { Blocks::fence->defaultBlockState(), Blocks::spruce_fence->defaultBlockState(), Blocks::birch_fence->defaultBlockState(), Blocks::jungle_fence->defaultBlockState(), Blocks::acacia_fence->defaultBlockState(), Blocks::dark_oak_fence->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Fence, this, fenceStates, 6, NULL, 0, -1, -1, false));
    const BlockState* packedIceStates[] = { Blocks::packed_ice->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_PackedIce, this, packedIceStates, 1, NULL, 0, -1, -1, false));
    const BlockState* ironTrapdoorStates[] = { Blocks::iron_trapdoor->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_IronTrapdoor, this, ironTrapdoorStates, 1, NULL, 0, -1, -1, false));
    const BlockState* skullStates[] = { Blocks::skull->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Skull, this, skullStates, 1, NULL, 0, -1, -1, false));
    const BlockState* deadbushStates[] = { Blocks::deadbush->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Deadbush, this, deadbushStates, 1, NULL, 0, -1, -1, false));
    const BlockState* webStates[] = { Blocks::web->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Web, this, webStates, 1, NULL, 0, -1, -1, false));
    const BlockState* concreteStates[] = { Blocks::concrete->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Concrete, this, concreteStates, 1, NULL, 0, -1, -1, false));
    const BlockState* concretePowderStates[] = { Blocks::concrete_powder->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_ConcretePowder, this, concretePowderStates, 1, NULL, 0, -1, -1, false));
    const BlockState* whiteGlazedTerracottaStates[] = { Blocks::white_glazed_terracotta->defaultBlockState(), Blocks::orange_glazed_terracotta->defaultBlockState(), Blocks::magenta_glazed_terracotta->defaultBlockState(), Blocks::light_blue_glazed_terracotta->defaultBlockState(), Blocks::yellow_glazed_terracotta->defaultBlockState(), Blocks::lime_glazed_terracotta->defaultBlockState(), Blocks::pink_glazed_terracotta->defaultBlockState(), Blocks::gray_glazed_terracotta->defaultBlockState(), Blocks::silver_glazed_terracotta->defaultBlockState(), Blocks::cyan_glazed_terracotta->defaultBlockState(), Blocks::purple_glazed_terracotta->defaultBlockState(), Blocks::blue_glazed_terracotta->defaultBlockState(), Blocks::brown_glazed_terracotta->defaultBlockState(), Blocks::green_glazed_terracotta->defaultBlockState(), Blocks::red_glazed_terracotta->defaultBlockState(), Blocks::black_glazed_terracotta->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_WhiteGlazedTerracotta, this, whiteGlazedTerracottaStates, 16, NULL, 0, -1, -1, false));
    const BlockState* whiteShulkerBoxStates[] = { Blocks::white_shulker_box->defaultBlockState(), Blocks::orange_shulker_box->defaultBlockState(), Blocks::magenta_shulker_box->defaultBlockState(), Blocks::light_blue_shulker_box->defaultBlockState(), Blocks::yellow_shulker_box->defaultBlockState(), Blocks::lime_shulker_box->defaultBlockState(), Blocks::pink_shulker_box->defaultBlockState(), Blocks::gray_shulker_box->defaultBlockState(), Blocks::silver_shulker_box->defaultBlockState(), Blocks::cyan_shulker_box->defaultBlockState(), Blocks::purple_shulker_box->defaultBlockState(), Blocks::blue_shulker_box->defaultBlockState(), Blocks::brown_shulker_box->defaultBlockState(), Blocks::green_shulker_box->defaultBlockState(), Blocks::red_shulker_box->defaultBlockState(), Blocks::black_shulker_box->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_WhiteShulkerBox, this, whiteShulkerBoxStates, 16, NULL, 0, -1, -1, false));
    const BlockState* observerStates[] = { Blocks::observer->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Observer, this, observerStates, 1, NULL, 0, -1, -1, false));
    const BlockState* coralBlockStates[] = { Blocks::coral_block->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_CoralBlock, this, coralBlockStates, 1, NULL, 0, -1, -1, false));
    TypedBoxed<bool> coralBlockValue(true);
    const BlockState* coralBlockStates2[] = { Blocks::coral_block->defaultBlockState()->setBoxedValue(CoralBlock::DEAD, &coralBlockValue) };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_DeadCoralBlock, this, coralBlockStates2, 1, NULL, 0, -1, -1, false));
    const BlockState* coralStates[] = { Blocks::coral->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Coral, this, coralStates, 1, NULL, 0, -1, -1, false));
    const BlockState* coralFanStates[] = { Blocks::coral_fan->defaultBlockState(), Blocks::coral_fan_hang->defaultBlockState(), Blocks::coral_fan_hang2->defaultBlockState(), Blocks::coral_fan_hang3->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_CoralFan, this, coralFanStates, 4, NULL, 0, -1, -1, false));
    TypedBoxed<bool> coralFanHangValue(true);
    TypedBoxed<bool> coralFanHang2Value(true);
    TypedBoxed<bool> coralFanHang3Value(true);
    const BlockState* coralFanDeadStates[] = { Blocks::coral_fan_dead->defaultBlockState(), Blocks::coral_fan_hang->defaultBlockState()->setBoxedValue(CoralFanHang::DEAD, &coralFanHangValue), Blocks::coral_fan_hang2->defaultBlockState()->setBoxedValue(CoralFanHang::DEAD, &coralFanHang2Value), Blocks::coral_fan_hang3->defaultBlockState()->setBoxedValue(CoralFanHang::DEAD, &coralFanHang3Value) };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_CoralFanDead, this, coralFanDeadStates, 4, NULL, 0, -1, -1, false));
    const BlockState* kelpStates[] = { Blocks::kelp->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Kelp, this, kelpStates, 1, NULL, 0, -1, -1, false));
    const BlockState* seaGrassStates[] = { Blocks::sea_grass->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_SeaGrass, this, seaGrassStates, 1, NULL, 0, -1, -1, false));
    const BlockState* seaPickleStates[] = { Blocks::sea_pickle->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_SeaPickle, this, seaPickleStates, 1, NULL, 0, -1, -1, false));
    const BlockState* blueIceStates[] = { Blocks::blue_ice->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_BlueIce, this, blueIceStates, 1, NULL, 0, -1, -1, false));
    const BlockState* turtleEggStates[] = { Blocks::turtle_egg->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_TurtleEgg, this, turtleEggStates, 1, NULL, 0, -1, -1, false));
    const BlockState* driedKelpBlockStates[] = { Blocks::dried_kelp_block->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_DriedKelpBlock, this, driedKelpBlockStates, 1, NULL, 0, -1, -1, false));
    const BlockState* strippedLogOakStates[] = { Blocks::stripped_log_oak->defaultBlockState(), Blocks::stripped_log_spruce->defaultBlockState(), Blocks::stripped_log_birch->defaultBlockState(), Blocks::stripped_log_jungle->defaultBlockState(), Blocks::stripped_log_acacia->defaultBlockState(), Blocks::stripped_log_dark_oak->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_StrippedLogOak, this, strippedLogOakStates, 6, NULL, 0, -1, -1, false));
    const BlockState* conduitStates[] = { Blocks::conduit->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_Conduit, this, conduitStates, 1, NULL, 0, -1, -1, false));
    const BlockState* carvedPumpkinStates[] = { Blocks::carved_pumpkin->defaultBlockState() };
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtBlockHint(eTutorial_Hint_CarvedPumpkin, this, carvedPumpkinStates, 1, NULL, 0, -1, -1, false));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_Squid, this, 0x8311C378, IDS_SQUID, EntityType::SQUID));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_Cow, this, 0x6DFF13B9, IDS_COW, EntityType::COW));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_Sheep, this, 0x184D1C09, IDS_SHEEP, EntityType::SHEEP));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_Chicken, this, 0x6CAF5F1B, IDS_CHICKEN, EntityType::CHICKEN));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_Pig, this, 0xFFD20CD4, IDS_PIG, EntityType::PIG));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_Wolf, this, IDS_DESC_WOLF, IDS_WOLF, EntityType::WOLF));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_Creeper, this, 0x3E4CD914, IDS_CREEPER, EntityType::CREEPER));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_Skeleton, this, 0x7D30125B, IDS_SKELETON, EntityType::SKELETON));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_Spider, this, 0x4029D63B, IDS_SPIDER, EntityType::SPIDER));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_Zombie, this, 0x45497554, IDS_ZOMBIE, EntityType::ZOMBIE));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_PigZombie, this, 0xD2243BD2, 0xF566D3CC, EntityType::PIG_ZOMBIE));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_Ghast, this, 0x2E34B42B, IDS_GHAST, EntityType::GHAST));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_Slime, this, IDS_DESC_SLIME, IDS_SLIME, EntityType::SLIME));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_EnderMan, this, 0x576863C8, IDS_ENDERMAN, EntityType::ENDER_MAN));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_Silverfish, this, IDS_DESC_SILVERFISH, IDS_SILVERFISH, EntityType::SILVERFISH));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_CaveSpider, this, 0xB0C858E5, IDS_CAVE_SPIDER, EntityType::CAVE_SPIDER));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_MushroomCow, this, 0xF541B352, 0x2775807C, EntityType::MUSHROOM_COW));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_SnowMan, this, 0xDBD1B7DD, 0x55D2D0E3, EntityType::SNOW_MAN));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_VillagerGolem, this, IDS_DESC_IRONGOLEM, IDS_IRONGOLEM, EntityType::VILLAGER_GOLEM));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_EnderDragon, this, IDS_DESC_ENDERDRAGON, IDS_ENDERDRAGON, EntityType::ENDER_DRAGON));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_Blaze, this, IDS_DESC_BLAZE, IDS_BLAZE, EntityType::BLAZE));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_MagmaCube, this, 0xA0138101, 0x9E8D900F, EntityType::MAGMA_CUBE));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_Ocelot, this, 0x4C0C6E55, 0xAA44AD8B, EntityType::OCELOT));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_Villager, this, IDS_DESC_VILLAGER, IDS_VILLAGER, EntityType::VILLAGER));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_WitherBoss, this, IDS_DESC_WITHER, IDS_WITHER, EntityType::WITHER_BOSS));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_WitherSkeleton, this, IDS_DESC_WITHER_SKELETON, IDS_WITHER_SKELETON, EntityType::WITHER_SKELETON));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_Witch, this, 0xE6AF0FB, IDS_WITCH, EntityType::WITCH));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_Bat, this, 0x48211895, IDS_BAT, EntityType::BAT));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_Horse, this, 0x3C0567A9, IDS_HORSE, EntityType::HORSE));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_Guardian, this, 0xB34C27D1, IDS_GUARDIAN, EntityType::GUARDIAN));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_Rabbit, this, IDS_DESC_RABBIT, IDS_RABBIT, EntityType::RABBIT));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_Endermite, this, IDS_DESC_ENDERMITE, IDS_ENDERMITE, EntityType::ENDERMITE));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_PolarBear, this, 0xA657167E, IDS_POLARBEAR, EntityType::POLAR_BEAR));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_Shulker, this, 0xA4CA6434, IDS_SHULKER, EntityType::SHULKER));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_Stray, this, 0x3D28EFA9, 0xC9DE5707, EntityType::STRAY));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_Husk, this, 0xC1C90A7, IDS_HUSK, EntityType::HUSK));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_SkeletonHorse, this, 0x5844482F, IDS_SKELETON_HORSE, EntityType::SKELETON_HORSE));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_ZombieHorse, this, 0x6231F2C0, IDS_ZOMBIE_HORSE, EntityType::ZOMBIE_HORSE));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_Painting, this, 0xE2BB6816, IDS_ITEM_PAINTING, EntityType::PAINTING));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_ItemFrame, this, IDS_DESC_ITEMFRAME, IDS_ITEM_ITEMFRAME, EntityType::ITEM_FRAME));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_MinecartRideable, this, 0xDF071B81, IDS_ITEM_MINECART, EntityType::MINECART_RIDEABLE));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_MinecartChest, this, 0x3ED319BA, IDS_ITEM_MINECART_CHEST, EntityType::MINECART_CHEST));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_MinecartFurnace, this, 0xBB5D88CB, IDS_ITEM_MINECART_FURNACE, EntityType::MINECART_FURNACE));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_MinecartHopper, this, IDS_DESC_MINECART_HOPPER, IDS_ITEM_MINECART_HOPPER, EntityType::MINECART_HOPPER));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_MinecartTnt, this, IDS_DESC_MINECART_TNT, IDS_ITEM_MINECART_TNT, EntityType::MINECART_TNT));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_Boat, this, 0x1BAB2B22, IDS_ITEM_BOAT_OAK, EntityType::BOAT));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_ArmorStand, this, IDS_DESC_ARMORSTAND, IDS_ARMORSTAND, EntityType::ARMOR_STAND));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_LeashFenceKnotEntity, this, 0x729DE426, IDS_ITEM_LEAD, EntityType::LEASH_FENCE_KNOT_ENTITY));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_EnderCrystal, this, 0xD396A19A, IDS_ITEM_END_CRYSTAL, EntityType::ENDER_CRYSTAL));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_Llama, this, IDS_DESC_LLAMA, IDS_LLAMA, EntityType::LLAMA));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_Parrot, this, IDS_DESC_PARROT, IDS_PARROT, EntityType::PARROT));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_Mule, this, 0x317E6BC3, IDS_MULE, EntityType::MULE));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_Donkey, this, 0x4F219CD8, IDS_DONKEY, EntityType::DONKEY));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_Vex, this, IDS_DESC_VEX, IDS_VEX, EntityType::VEX));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_EvocationIllager, this, 0xA2978A87, 0xD9AA9869, EntityType::EVOCATION_ILLAGER));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_VindicationIllager, this, 0x3AE301AF, 0xF3D4F291, EntityType::VINDICATION_ILLAGER));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_ElderGuardian, this, IDS_DESC_ELDER_GUARDIAN, IDS_ELDER_GUARDIAN, EntityType::ELDER_GUARDIAN));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_ZombieVillager, this, 0x5DCF8BDD, IDS_ZOMBIE_VILLAGER, EntityType::ZOMBIE_VILLAGER));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_Turtle, this, IDS_DESC_TURTLE, IDS_TURTLE, EntityType::TURTLE));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_Dolphin, this, IDS_DESC_DOLPHIN, IDS_DOLPHIN, EntityType::DOLPHIN));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_Fish, this, 0x33D9E792, IDS_COD, EntityType::FISH));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_Fish, this, 0x9D69BB40, IDS_SALMON, EntityType::SALMON));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_Fish, this, IDS_DESC_PUFFERFISH, IDS_PUFFERFISH, EntityType::PUFFERFISH));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_Fish, this, IDS_DESC_TROPICALFISH, IDS_TROPICALFISH, EntityType::TROPICAL_FISH));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_Drowned, this, IDS_DESC_DROWNED, IDS_DROWNED, EntityType::DROWNED));
    addHintIfIncomplete(eTutorial_State_Gameplay, new LookAtEntityHint(eTutorial_Hint_Phantom, this, 0x1D435D3, IDS_PHANTOM, EntityType::PHANTOM));
    Item* shovelItems[] = { Items::woodenShovel, Items::stoneShovel, Items::ironShovel, Items::goldenShovel, Items::diamondShovel };
    addHintIfIncomplete(eTutorial_State_Gameplay, new DiggerItemHint(eTutorial_Hint_Shovel, this, 0x6B4D1717, shovelItems, 5));
    Item* axeItems[] = { Items::woodenAxe, Items::stoneAxe, Items::ironAxe, Items::goldenAxe, Items::diamondAxe };
    addHintIfIncomplete(eTutorial_State_Gameplay, new DiggerItemHint(eTutorial_Hint_Axe, this, 0x70599EAB, axeItems, 5));
    Item* pickaxeItems[] = { Items::woodenPickaxe, Items::stonePickaxe, Items::ironPickaxe, Items::goldenPickaxe, Items::diamondPickaxe };
    addHintIfIncomplete(eTutorial_State_Gameplay, new DiggerItemHint(eTutorial_Hint_Pickaxe, this, 0xC5B6F219, pickaxeItems, 5));
    addHint(eTutorial_State_CraftingMenu, new TutorialHint(eTutorial_Hint_General, this, 0xDBD723F6, TutorialHint::HINT_CREATE_ITEM_SELECTED, true, true, -1));
    addHint(eTutorial_State_CraftingTableMenu, new TutorialHint(eTutorial_Hint_General, this, 0xDBD723F6, TutorialHint::HINT_CREATE_ITEM_SELECTED, true, true, -1));
}

void Tutorial::addTask(eTutorial_State state, TutorialTask* task)
{
    if (state == eTutorial_State_Gameplay)
    {
        tasks.push_back(task);
    }
    stateTasks[state].push_back(task);
}

unsigned int Tutorial::AddTasks(bool enabledValue)
{
    ProcedureCompoundTask* compound_0;
    ProcedureCompoundTask* compound_1;
    ProcedureCompoundTask* compound_2;
    ProcedureCompoundTask* compound_3;
    ProcedureCompoundTask* compound_4;
    ProcedureCompoundTask* compound_5;
    ProcedureCompoundTask* compound_6;
    ProcedureCompoundTask* compound_7;
    ProcedureCompoundTask* compound_9;
    ProcedureCompoundTask* compound_10;
    ProcedureCompoundTask* compound_11;
    if (!enabledValue && isStateCompleted(eTutorial_State_InventoryMenu))
        goto check_state_0xE;
    create_state_1:
    compound_0 = new ProcedureCompoundTask(this);
    compound_0->AddTask(new ChoiceTask(this, 0xB77FC340, 0x9E1E8969, true, 0, 1, eTutorial_CompletionAction_CompleteState, static_cast<ETelemetryChallenges>(0x4)));
    compound_0->AddTask(new InfoTask(this, 0xC18416E0, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_0->AddTask(new InfoTask(this, 0x99B85F72, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_0->AddTask(new InfoTask(this, 0x1A28A6CA, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_0->AddTask(new InfoTask(this, 0xEC1649D5, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    addTask(eTutorial_State_InventoryMenu, compound_0);
    if (!enabledValue)
        goto check_state_0xE;
    create_state_0xE:
    compound_1 = new ProcedureCompoundTask(this);
    compound_1->AddTask(new ChoiceTask(this, 0x25598D4, 0x46859B85, true, 0, 1, eTutorial_CompletionAction_CompleteState, static_cast<ETelemetryChallenges>(0x14)));
    compound_1->AddTask(new InfoTask(this, 0x11BE3D54, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_1->AddTask(new InfoTask(this, 0x5F3FE8A6, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_1->AddTask(new InfoTask(this, 0x94985DDE, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_1->AddTask(new InfoTask(this, 0x3F6BE86, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_1->AddTask(new InfoTask(this, 0x893A28E1, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    addTask(eTutorial_State_CreativeInventoryMenu, compound_1);
    if (!enabledValue)
        goto check_state_2;
    create_state_2:
    compound_2 = new ProcedureCompoundTask(this);
    compound_2->AddTask(new ChoiceTask(this, 0x9A252193, 0x7F0C21BA, true, 0, 1, eTutorial_CompletionAction_CompleteState, static_cast<ETelemetryChallenges>(0x5)));
    compound_2->AddTask(new InfoTask(this, 0x27A2B969, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_2->AddTask(new InfoTask(this, 0xA42613B4, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_2->AddTask(new InfoTask(this, 0x723936AB, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_2->AddTask(new InfoTask(this, 0x1F10DDC2, 0xA0D0169C, false, 2, eTelemetryChallenges_None));
    compound_2->AddTask(new InfoTask(this, 0xB135E36, 0xFC8730E4, false, 2, eTelemetryChallenges_None));
    compound_2->AddTask(new InfoTask(this, 0x3AF6EF3E, 0xAF244F78, false, 2, eTelemetryChallenges_None));
    addTask(eTutorial_State_CraftingMenu, compound_2);
    if (!enabledValue)
        goto check_state_4;
    create_state_4:
    compound_3 = new ProcedureCompoundTask(this);
    compound_3->AddTask(new ChoiceTask(this, 0x6CE6D19, 0x4C0CF470, true, 0, 1, eTutorial_CompletionAction_CompleteState, static_cast<ETelemetryChallenges>(0x6)));
    compound_3->AddTask(new InfoTask(this, 0x9BB6558D, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_3->AddTask(new InfoTask(this, 0xC6B63D93, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_3->AddTask(new InfoTask(this, 0x5466C434, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    addTask(eTutorial_State_FurnaceMenu, compound_3);
    if (!enabledValue)
        goto check_state_0x12;
    create_state_0x12:
    compound_4 = new ProcedureCompoundTask(this);
    compound_4->AddTask(new ChoiceTask(this, 0x9253A77, 0x9E500506, true, 0, 1, eTutorial_CompletionAction_CompleteState, static_cast<ETelemetryChallenges>(0xF)));
    compound_4->AddTask(new InfoTask(this, 0x96BEBCB3, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_4->AddTask(new InfoTask(this, 0xAED3450F, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_4->AddTask(new InfoTask(this, 0xC02F0030, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_4->AddTask(new InfoTask(this, 0xFB48ACEA, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_4->AddTask(new InfoTask(this, 0xFCFE0427, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    addTask(eTutorial_State_BrewingMenu, compound_4);
    if (!enabledValue)
        goto check_state_0x14;
    create_state_0x14:
    compound_5 = new ProcedureCompoundTask(this);
    compound_5->AddTask(new ChoiceTask(this, 0x18A28D4E, 0x690D3FD7, true, 0, 1, eTutorial_CompletionAction_CompleteState, static_cast<ETelemetryChallenges>(0x15)));
    compound_5->AddTask(new InfoTask(this, 0xD1EDBF15, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_5->AddTask(new InfoTask(this, 0xCA85FE57, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_5->AddTask(new InfoTask(this, 0x244CF0FE, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_5->AddTask(new InfoTask(this, 0x731167E6, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_5->AddTask(new InfoTask(this, 0x697EA620, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    addTask(eTutorial_State_EnchantingMenu, compound_5);
    if (!enabledValue)
        goto check_state_0x1B;
    create_state_0x1B:
    compound_6 = new ProcedureCompoundTask(this);
    compound_6->AddTask(new ChoiceTask(this, 0xD381DD9, 0x645B6F8, true, 0, 1, eTutorial_CompletionAction_CompleteState, static_cast<ETelemetryChallenges>(0x2E)));
    compound_6->AddTask(new InfoTask(this, 0xC9DE21A, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_6->AddTask(new InfoTask(this, 0x4F577027, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_6->AddTask(new InfoTask(this, 0x5E84B4A1, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_6->AddTask(new InfoTask(this, 0x87DCC839, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_6->AddTask(new InfoTask(this, 0x190AE839, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_6->AddTask(new InfoTask(this, 0x67CD9B1F, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_6->AddTask(new InfoTask(this, 0x51ED81D1, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    addTask(eTutorial_State_AnvilMenu, compound_6);
    if (!enabledValue)
        goto check_state_0x19;
    create_state_0x19:
    compound_7 = new ProcedureCompoundTask(this);
    compound_7->AddTask(new ChoiceTask(this, 0x821611E6, 0x789B40B7, true, 0, 1, eTutorial_CompletionAction_CompleteState, static_cast<ETelemetryChallenges>(0x30)));
    compound_7->AddTask(new InfoTask(this, 0x54091CCD, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_7->AddTask(new InfoTask(this, 0xB7242ECF, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_7->AddTask(new InfoTask(this, 0x7C03162F, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_7->AddTask(new InfoTask(this, 0x7B8B7B8F, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_7->AddTask(new InfoTask(this, 0x4896FB7B, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    addTask(eTutorial_State_TradingMenu, compound_7);
    if (!enabledValue)
        goto check_state_0x1D;
    create_state_0x1D:
    addTask(eTutorial_State_Horse, new HorseChoiceTask(this, 0x4DCDE902, 0x80BBE3E3, 0xDB419510, 0xE9B948F4, 0x7595B01B, true, 0, 1, eTutorial_CompletionAction_CompleteStateAndConstraints, static_cast<ETelemetryChallenges>(0x32)));
    addTask(eTutorial_State_Horse, new InfoTask(this, 0x8DFE2C47, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    addTask(eTutorial_State_Horse, new InfoTask(this, 0xCE6D12A7, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    addTask(eTutorial_State_Horse, new InfoTask(this, 0xFB9D6DB1, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    addTask(eTutorial_State_Horse, new InfoTask(this, 0x86C858BB, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    addTask(eTutorial_State_Horse, new InfoTask(this, 0x32AEB3B, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    addTask(eTutorial_State_Horse, new InfoTask(this, 0xD0B71241, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    addTask(eTutorial_State_Horse, new InfoTask(this, 0xB916B23D, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    addTask(eTutorial_State_Horse, new InfoTask(this, 0x3ED848DD, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    if (!enabledValue)
        goto check_state_0x1E;
    create_state_0x1E:
    compound_9 = new ProcedureCompoundTask(this);
    compound_9->AddTask(new ChoiceTask(this, 0x7C8AEDB6, 0xEB9A80F7, true, 0, 1, eTutorial_CompletionAction_CompleteState, static_cast<ETelemetryChallenges>(0x33)));
    compound_9->AddTask(new InfoTask(this, 0xE4582E6F, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_9->AddTask(new InfoTask(this, 0xAC55BC2F, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_9->AddTask(new InfoTask(this, 0xAD1032A9, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    addTask(eTutorial_State_HorseMenu, compound_9);
    if (!enabledValue)
        goto check_state_0x24;
    create_state_0x24:
    compound_10 = new ProcedureCompoundTask(this);
    compound_10->AddTask(new ChoiceTask(this, 0xECBB7BAC, 0xB0ED8A95, true, 0, 1, eTutorial_CompletionAction_CompleteState, static_cast<ETelemetryChallenges>(0x35)));
    compound_10->AddTask(new InfoTask(this, 0x8C159652, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_10->AddTask(new InfoTask(this, 0xFE1D058D, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_10->AddTask(new InfoTask(this, 0x22299E7D, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_10->AddTask(new InfoTask(this, 0xE44F42D0, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_10->AddTask(new InfoTask(this, 0x13FD0693, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_10->AddTask(new InfoTask(this, 0x859377B3, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_10->AddTask(new InfoTask(this, 0xD0C84AA4, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_10->AddTask(new InfoTask(this, 0x36518124, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_10->AddTask(new InfoTask(this, 0x306CF96D, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    addTask(eTutorial_State_FireworksMenu, compound_10);
    if (!enabledValue)
        goto check_state_0x22;
    create_state_0x22:
    compound_11 = new ProcedureCompoundTask(this);
    compound_11->AddTask(new ChoiceTask(this, 0x6FF43521, 0xBA245C8, true, 0, 1, eTutorial_CompletionAction_CompleteState, static_cast<ETelemetryChallenges>(0x37)));
    compound_11->AddTask(new InfoTask(this, 0x934C3015, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_11->AddTask(new InfoTask(this, 0xF87CF280, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    compound_11->AddTask(new InfoTask(this, 0xC6447326, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    addTask(eTutorial_State_BeaconMenu, compound_11);
    if (!enabledValue)
        goto check_state_5;
    create_state_5:
    addTask(eTutorial_State_RidingMinecart, new ChoiceTask(this, 0x97628A42, 0x16DFAE83, true, 0, 1, eTutorial_CompletionAction_CompleteStateAndConstraints, static_cast<ETelemetryChallenges>(0x8)));
    addTask(eTutorial_State_RidingMinecart, new InfoTask(this, 0x6D8D73AC, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    addTask(eTutorial_State_RidingMinecart, new InfoTask(this, 0xF965D585, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    addTask(eTutorial_State_RidingMinecart, new InfoTask(this, 0x5C8E319F, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    if (!enabledValue)
        goto check_state_6;
    create_state_6:
    addTask(eTutorial_State_RidingBoat, new ChoiceTask(this, 0x25C06AF9, 0x811A4288, true, 0, 1, eTutorial_CompletionAction_CompleteStateAndConstraints, static_cast<ETelemetryChallenges>(0x9)));
    addTask(eTutorial_State_RidingBoat, new InfoTask(this, 0xE6A88FAF, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    if (!enabledValue)
        goto check_state_7;
    create_state_7:
    addTask(eTutorial_State_Fishing, new ChoiceTask(this, 0x2C532B5D, 0x1B284F34, true, 0, 1, eTutorial_CompletionAction_CompleteStateAndConstraints, static_cast<ETelemetryChallenges>(0x7)));
    addTask(eTutorial_State_Fishing, new InfoTask(this, 0xC7681D23, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    addTask(eTutorial_State_Fishing, new InfoTask(this, 0x4F274F92, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    addTask(eTutorial_State_Fishing, new InfoTask(this, 0x314B8DCE, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    if (!enabledValue)
        goto check_state_8;
    create_state_8:
    addTask(eTutorial_State_NameTag, new InfoTask(this, 0xAD017F4C, 0x3F9463F, true, 0x3A, eTelemetryChallenges_None));
    if (!enabledValue)
        goto check_state_9;
    goto add_state_9;
    check_state_0xE:
    if (!isStateCompleted(eTutorial_State_CreativeInventoryMenu))
        goto create_state_0xE;
    check_state_2:
    if (!isStateCompleted(eTutorial_State_CraftingMenu))
        goto create_state_2;
    check_state_4:
    if (!isStateCompleted(eTutorial_State_FurnaceMenu))
        goto create_state_4;
    check_state_0x12:
    if (!isStateCompleted(eTutorial_State_BrewingMenu))
        goto create_state_0x12;
    check_state_0x14:
    if (!isStateCompleted(eTutorial_State_EnchantingMenu))
        goto create_state_0x14;
    check_state_0x1B:
    if (!isStateCompleted(eTutorial_State_AnvilMenu))
        goto create_state_0x1B;
    check_state_0x19:
    if (!isStateCompleted(eTutorial_State_TradingMenu))
        goto create_state_0x19;
    check_state_0x1D:
    if (!isStateCompleted(eTutorial_State_Horse))
        goto create_state_0x1D;
    check_state_0x1E:
    if (!isStateCompleted(eTutorial_State_HorseMenu))
        goto create_state_0x1E;
    check_state_0x24:
    if (!isStateCompleted(eTutorial_State_FireworksMenu))
        goto create_state_0x24;
    check_state_0x22:
    if (!isStateCompleted(eTutorial_State_BeaconMenu))
        goto create_state_0x22;
    check_state_5:
    if (!isStateCompleted(eTutorial_State_RidingMinecart))
        goto create_state_5;
    check_state_6:
    if (!isStateCompleted(eTutorial_State_RidingBoat))
        goto create_state_6;
    check_state_7:
    if (!isStateCompleted(eTutorial_State_Fishing))
        goto create_state_7;
    check_state_8:
    if (!isStateCompleted(eTutorial_State_NameTag))
        goto create_state_8;
    check_state_9:
    if (!isStateCompleted(eTutorial_State_Bed))
        goto add_state_9;
    goto add_state_f;
    add_state_9:
    addTask(eTutorial_State_Bed, new ChoiceTask(this, 0xA76E95BA, 0x2C631D03, true, 0, 1, eTutorial_CompletionAction_CompleteStateAndConstraints, static_cast<ETelemetryChallenges>(0xA)));
    addTask(eTutorial_State_Bed, new InfoTask(this, 0x54D286F0, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    addTask(eTutorial_State_Bed, new InfoTask(this, 0xA62DA153, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    if (enabledValue)
        return 0;
    add_state_f:
    if (!isStateCompleted(eTutorial_State_FoodBar))
    {
    addTask(eTutorial_State_FoodBar, new ChoiceTask(this, 0x82BFAADD, 0x2B6F086C, true, 0, 1, eTutorial_CompletionAction_CompleteStateAndConstraints, static_cast<ETelemetryChallenges>(0xD)));
    addTask(eTutorial_State_FoodBar, new InfoTask(this, 0x67613737, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    addTask(eTutorial_State_FoodBar, new InfoTask(this, 0xD756C6A6, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    addTask(eTutorial_State_FoodBar, new InfoTask(this, 0x642C1BB4, 0x3F9463F, true, 0, eTelemetryChallenges_None));
    }
    return 0;
}


unsigned int Tutorial::setMessage(PopupMessageDetails* details)
{
    if (details == NULL)
    {
        if (currentMessage != NULL && currentMessage->titleId != -1 &&
            currentMessage->titleId != -2)
        {
            _TutorialPopupInfo info;
            info.scene = currentScene;
            gConsoleUIController.SetTutorialDescription(playerId, &info);
        }
        delete currentMessage;
        currentMessage = NULL;
        popupPending = false;
        return 0;
    }
    if (!details->allowFade && messageState == currentState && currentMessage != NULL &&
        details->isSameContent(currentMessage) != 0 &&
        (!details->unknown || GetTickCount() <
             static_cast<unsigned int>(visibleTime) + gTutorialMessageRepeatTime))
    {
        delete details;
        return 0;
    }

    messageState = currentState;
    if (!details->displayed)
        visibleTime = static_cast<int>(GetTickCount());

    if ((details->titleId != -1 && details->titleId != -2) || !details->title.empty())
    {
        std::wstring title;
        if (!details->title.empty())
            title = details->title;
        else
        {
            boost::unordered_map<int, TutorialMessage*>::iterator found = messages.find(details->titleId);
            title = found != messages.end() && !details->force
                ? found->second->getMessageForDisplay()
                : L10N::GetString(details->titleId);
        }

        if (!details->message.empty())
            title += details->message;
        else if (details->messageId != -1)
        {
            boost::unordered_map<int, TutorialMessage*>::iterator found = messages.find(details->messageId);
            if (found != messages.end())
                title += found->second->getMessageForDisplay();
        }

        std::wstring description;
        if (!details->itemDescription.empty())
            description = details->itemDescription;
        else if (details->promptId != -1 && details->promptId != -2)
            description = L10N::GetString(details->promptId);

        _TutorialPopupInfo info;
        info.scene = currentScene;
        info.title = title.c_str();
        info.description = description.c_str();
        info.icon = details->itemInstance;
        info.iconItemId = details->itemId;
        info.showTitle = details->repeat;
        info.showExitScreenshot = details->unknown;
        info.descriptionId = details->state;
        gConsoleUIController.SetTutorialDescription(playerId, &info);
    }

    delete currentMessage;
    currentMessage = details;
    popupPending = true;
    return 1;
}

void Tutorial::setCompleted(int completionId)
{
    if (getCompleted(completionId) == 0)
    {
        completedStates.push_back((eTutorial_State)completionId);
    }
}

void Tutorial::setHintCompleted(eTutorial_Hint hint)
{
    setCompleted((int)hint);
}

unsigned int Tutorial::setHintCompleted(TutorialHint* hint)
{
    if (hint == NULL)
    {
        return 0;
    }
    return 1;
}

int Tutorial::setMessage(TutorialHint* hint, PopupMessageDetails* details)
{
    bool eligible = true;
    if (!enabled &&
        (GameSettings::GetGameSettings(playerId, eGameSetting_Hints) == 0 ||
         GameSettings::GetGameSettings(playerId, eGameSetting_DisplayHUD) == 0))
    {
        eligible = false;
    }
    unsigned int now = GetTickCount();
    if (details == NULL)
    {
        return 0;
    }
    if (!details->allowFade && !eligible)
    {
        return 0;
    }
    if (details->isItem)
    {
        if (!popupPending)
        {
            if (now - static_cast<unsigned int>(visibleTime) <= gTutorialMinimumDisplayTime)
            {
                return 0;
            }
        }
        else if (now - static_cast<unsigned int>(messageTime) <= gTutorialHintRepeatTime)
        {
            return 0;
        }
    }
    int result = static_cast<int>(setMessage(details));
    if (result != 0)
    {
        messageTime = static_cast<int>(now);
        popupPending = true;
        if (hint != NULL)
        {
            setHintCompleted(hint);
        }
    }
    return result;
}

unsigned int Tutorial::isSelectedItemState()
{
    return currentState == eTutorial_State_Gameplay || currentState >= eTutorial_State_Fishing ? 1U : 0U;
}

void Tutorial::tick()
{
    if (!started)
    {
        unsigned int now = GetTickCount();
        if (startTime == 0)
        {
            startTime = static_cast<int>(now);
            return;
        }
        if (now - static_cast<unsigned int>(startTime) <= gTutorialStartDelay)
            return;
        started = true;
    }

    Minecraft* minecraft = Minecraft::GetInstance();
    if (overridesGameHostOptions)
    {
        if (gameHostOptionsOverridden)
        {
            if (completed)
            {
                MinecraftServer::s_setDayTimeAtEndOfTick = true;
                MinecraftServer::s_setDayTime = gTutorialDayTime;
                minecraft->mLevel->setDayTime(gTutorialDayTime);
                gApp.SetGameHostOption(eGameHostOption_DoDaylightCycle, 1);
                gameHostOptionsOverridden = false;
            }
        }
        else if (!completed)
        {
            MinecraftServer::s_setDayTimeAtEndOfTick = true;
            MinecraftServer::s_setDayTime = gTutorialDayTime;
            minecraft->mLevel->setDayTime(gTutorialDayTime);
            gApp.SetGameHostOption(eGameHostOption_DoDaylightCycle, 0);
            gameHostOptionsOverridden = true;
        }
    }

    if (popupVisible)
    {
        bool taskChanged = false;
        if (!uiControllerSet)
        {
            gConsoleUIController.SetTutorial(playerId, this);
            uiControllerSet = true;
        }
        else if (splitScreen != (gApp.GetLocalPlayerCount() > 1))
        {
            gConsoleUIController.SetTutorial(playerId, this);
        }

        if (!gConsoleUIController.IsPauseMenuDisplayed(playerId))
        {
            if (tutorialHidden)
            {
                if (!gameModeConstraints->HasFailedConstraint(currentState) &&
                    currentTask[currentState] != NULL)
                {
                    gConsoleUIController.SetTutorialVisible(playerId, true);
                    visibleTime = static_cast<int>(GetTickCount());
                }
                tutorialHidden = false;
            }

            if (!popupPending)
            {
                const bool hintsEnabled =
                    enabled || GameSettings::GetGameSettings(playerId, eGameSetting_Hints) != 0;
                if ((gameModeChanged && gameMode == eTutorial_GameMode_Fireworks) || hintsEnabled)
                {
                    std::vector<TutorialHint*>& hints = stateHints[currentState];
                    for (std::vector<TutorialHint*>::iterator it = hints.begin(); it != hints.end(); ++it)
                    {
                        TutorialHint* hint = *it;
                        int hintId = hint->tick();
                        if (gameModeChanged && !gConsoleUIController.GetMenuDisplayed(playerId))
                            hintId = static_cast<int>(hint->onGameModeChange(gameMode));
                        if (hintId != -1)
                        {
                            hintShown = true;
                            gameModeChanged = false;
                            PopupMessageDetails* details = new PopupMessageDetails();
                            details->titleId = hintId;
                            details->repeat = hint->allowFade() != 0;
                            details->allowFade = true;
                            details->state = static_cast<int>(hint->getMinimumDisplayTime());
                            setMessage(hint, details);
                            break;
                        }
                    }
                }
            }

            if (!tasksChecked)
            {
                if (!gameModeConstraints->HasFailedConstraint(currentState))
                {
                    std::vector<TutorialTask*>& tasks = stateTasks[currentState];
                    bool checkTask = true;
                    std::vector<TutorialTask*>::iterator it = tasks.begin();
                    while (!tasks.empty() && it != tasks.end())
                    {
                        TutorialTask* task = *it;
                        if (!checkTask && !task->isPreCompletionEnabled())
                        {
                            ++it;
                            continue;
                        }
                        checkTask = false;
                        if ((!task->ShowMinimumTime() ||
                             (task->hasBeenActivated() &&
                              static_cast<unsigned int>(visibleTime) + gTutorialMinimumDisplayTime <
                                  GetTickCount())) &&
                            task->isCompleted())
                        {
                            const eTutorial_CompletionAction action = task->getCompletionAction();
                            it = tasks.erase(it);
                            delete task;
                            if (!tasks.empty() &&
                                (action == eTutorial_CompletionAction_CompleteState ||
                                 action == eTutorial_CompletionAction_CompleteStateAndConstraints))
                            {
                                if (action == eTutorial_CompletionAction_CompleteStateAndConstraints)
                                    gameModeConstraints->MoveDelayedConstraints(currentState, 0);
                                for (std::vector<TutorialTask*>::iterator rest = tasks.begin(); rest != tasks.end();
                                     ++rest)
                                {
                                    delete *rest;
                                }
                                tasks.clear();
                                it = tasks.end();
                                if (currentState == eTutorial_State_Gameplay)
                                    gApp.SetTutorialMode(false, minecraft->mLocalPlayers[playerId]);
                            }
                            else if (!tasks.empty() &&
                                     action == eTutorial_CompletionAction_CompleteStateExceptLastTask)
                            {
                                TutorialTask* lastTask = tasks.at(tasks.size() - 1);
                                tasks.pop_back();
                                for (std::vector<TutorialTask*>::iterator rest = tasks.begin(); rest != tasks.end();
                                     ++rest)
                                {
                                    delete *rest;
                                }
                                tasks.clear();
                                tasks.push_back(lastTask);
                                it = tasks.begin();
                                if (currentState == eTutorial_State_Gameplay)
                                    gApp.SetTutorialMode(false, minecraft->mLocalPlayers[playerId]);
                            }

                            taskChanged = true;
                            if (tasks.empty())
                            {
                                setStateCompleted(currentState);
                                currentTask[currentState] = NULL;
                            }
                            else
                            {
                                currentTask[currentState] = tasks.front();
                                currentTask[currentState]->setAsCurrentTask(true);
                                if (currentTask[currentState]->isPreCompletionEnabled())
                                    checkTask = true;
                            }
                        }
                        else
                        {
                            ++it;
                            if (task->ShowMinimumTime() && task->hasBeenActivated() &&
                                static_cast<unsigned int>(visibleTime) + gTutorialMinimumDisplayTime <
                                    GetTickCount())
                            {
                                task->setShownForMinimumTime();
                                if (!popupPending)
                                {
                                    PopupMessageDetails* details = new PopupMessageDetails();
                                    details->titleId = task->getDescriptionId();
                                    details->messageId = task->getPromptId();
                                    details->repeat = task->AllowFade();
                                    details->displayed = true;
                                    setMessage(details);
                                }
                            }
                        }
                    }

                    if (currentTask[currentState] == NULL && !tasks.empty())
                    {
                        currentTask[currentState] = tasks.front();
                        currentTask[currentState]->setAsCurrentTask(true);
                        taskChanged = true;
                    }
                }

                if (!tasksChecked && (taskChanged || stateChanged))
                {
                    bool allTasksDone = true;
                    for (int state = 0; state < eTutorial_State_Max; ++state)
                    {
                        if (!stateTasks[state].empty())
                        {
                            allTasksDone = false;
                            break;
                        }
                        if (state == eTutorial_State_Gameplay)
                        {
                            completed = true;
                            gApp.SetTutorialMode(false, minecraft->mLocalPlayers[playerId]);
                            Minecraft::GetInstance()->playerLeftTutorial(playerId);
                        }
                    }
                    if (allTasksDone)
                        tasksChecked = true;
                }
            }

            bool showTaskMessage =
                gameModeConstraints->HasFailedConstraint(currentState) || taskChanged || stateChanged;
            if (!showTaskMessage && !popupPending && !gameModeConstraints->HasFailedConstraint(currentState) &&
                currentTask[currentState] != NULL &&
                (currentMessage == NULL || currentTask[currentState]->getDescriptionId() != currentMessage->titleId))
            {
                showTaskMessage = true;
            }

            if (showTaskMessage)
            {
                if (!gameModeConstraints->HasFailedConstraint(currentState))
                {
                    TutorialTask* task = currentTask[currentState];
                    if (task == NULL)
                    {
                        setMessage(static_cast<PopupMessageDetails*>(NULL));
                    }
                    else
                    {
                        PopupMessageDetails* details = new PopupMessageDetails();
                        details->titleId = task->getDescriptionId();
                        details->messageId = task->getPromptId();
                        details->repeat = task->AllowFade();
                        setMessage(details);
                        messageRepeatCount = currentTask[currentState]->repeatMessage;
                    }
                }
                else
                {
                    PopupMessageDetails* details = new PopupMessageDetails();
                    details->titleId = gameModeConstraints->GetFailedConstraint(currentState)->messageId;
                    details->repeat = false;
                    setMessage(details);
                }
            }

            if (popupPending && static_cast<unsigned int>(visibleTime) + gTutorialVisibleTime < GetTickCount())
                popupPending = false;

            if (!gameModeConstraints->HasFailedConstraint(currentState))
            {
                TutorialTask* task = currentTask[currentState];
                if (task != NULL && messageRepeatCount != 0 &&
                    static_cast<unsigned int>(visibleTime) + messageRepeatCount * gTutorialMessageRepeatTime <
                        GetTickCount())
                {
                    PopupMessageDetails* details = new PopupMessageDetails();
                    details->titleId = task->getDescriptionId();
                    details->messageId = task->getPromptId();
                    details->repeat = task->AllowFade();
                    details->unknown = true;
                    setMessage(details);
                    ++messageRepeatCount;
                    if (static_cast<unsigned int>(messageRepeatCount) > 1)
                        messageRepeatCount = 1;
                }
            }
            stateChanged = false;

            if (currentState != eTutorial_State_Gameplay && stateTasks[currentState].empty() &&
                (isSelectedItemState() || !gConsoleUIController.GetMenuDisplayed(playerId)))
            {
                completedStates.push_back(currentState);
                changeTutorialState(eTutorial_State_Gameplay, NULL);
            }
            return;
        }
    }

    TutorialTask* task = currentTask[currentState];
    if (task != NULL &&
        (!task->AllowFade() || static_cast<unsigned int>(visibleTime) + gTutorialVisibleTime > GetTickCount()))
    {
        tutorialHidden = true;
    }
    gConsoleUIController.SetTutorialVisible(playerId, false);
}

unsigned char Tutorial::isStateCompleted(eTutorial_State state)
{
    return (unsigned char)(getCompleted((int)state) != 0);
}

void Tutorial::setStateCompleted(eTutorial_State state)
{
    setCompleted((int)state);
}

void Tutorial::changeTutorialState(eTutorial_State state, UIScene* scene)
{
    if (currentState == state)
    {
        currentScene = scene;
        return;
    }
    setMessage((PopupMessageDetails*)NULL);
    currentScene = scene;
    currentState = state;
    if (gameModeConstraints != NULL)
    {
        gameModeConstraints->SetCurrentState(currentState);
    }
    popupPending = false;
    stateChanged = true;
}

void Tutorial::setMessage(const std::wstring& messageValue, int displayTimeValue,
                          int priorityValue)
{
    PopupMessageDetails* details = new PopupMessageDetails();
    details->message = messageValue;
    details->displayTime = displayTimeValue;
    details->priority = priorityValue;
    details->allowFade = true;
    setMessage(details);
}

void Tutorial::showTutorialPopup(bool show)
{
    popupVisible = show;
    if (!show)
    {
        TutorialTask* task = currentTask[currentState];
        if (task != NULL
            && (!task->AllowFade()
                || GetTickCount() < (unsigned int)visibleTime + gTutorialVisibleTime))
        {
            tutorialHidden = true;
        }
        gConsoleUIController.SetTutorialVisible(playerId, false);
    }
}

unsigned int Tutorial::useItemOn(Level* level, not_null_ptr<ItemInstance> item,
                                  const BlockPos& pos, bool cancelled)
{
    if (cancelled)
    {
        return 0xffffffffU;
    }
    return useItemOn(item, cancelled);
}

unsigned int Tutorial::useItemOn(not_null_ptr<ItemInstance> item, bool cancelled)
{
    if (cancelled)
    {
        return 0xffffffffU;
    }
    return 0;
}

unsigned int Tutorial::completeUsingItem(not_null_ptr<ItemInstance> item)
{
    if (item.get() == NULL)
        return 0;
    bool handled = false;
    TutorialTask* activeTask = NULL;
    if (currentState >= 0 && currentState < static_cast<int>(tasks.size()))
    {
        activeTask = tasks[currentState];
        if (activeTask != NULL)
        {
            activeTask->completeUsingItem(item);
            handled = true;
        }
    }
    for (std::vector<TutorialTask*>::iterator it = tasks.begin(); it != tasks.end(); ++it)
    {
        if (*it != NULL && *it != activeTask)
        {
            (*it)->completeUsingItem(item);
            handled = true;
        }
    }
    return handled ? 1U : 0U;
}

unsigned int Tutorial::startDestroyBlock(not_null_ptr<ItemInstance> item, const BlockState* state)
{
    for (std::vector<TutorialHint*>::iterator it = stateHints[currentState].begin();
         it != stateHints[currentState].end(); ++it)
    {
        TutorialHint* hint = *it;
        if (hint == NULL)
            continue;
        int result = hint->startDestroyBlock(item, state);
        if (result != -1)
        {
            PopupMessageDetails* details = new PopupMessageDetails();
            details->titleId = result;
            setMessage(hint, details);
            return 0;
        }
    }
    return 0xffffffffU;
}

unsigned int Tutorial::destroyBlock(Block* block)
{
    for (std::vector<TutorialHint*>::iterator it = stateHints[currentState].begin();
         it != stateHints[currentState].end(); ++it)
    {
        TutorialHint* hint = *it;
        if (hint == NULL)
            continue;
        int result = static_cast<int>(hint->destroyBlock(block));
        if (result != -1)
        {
            PopupMessageDetails* details = new PopupMessageDetails();
            details->titleId = result;
            setMessage(hint, details);
            return 0;
        }
    }
    return 0xffffffffU;
}

unsigned int Tutorial::attack(boost::shared_ptr<Player> player, boost::shared_ptr<Entity> entity)
{
    for (std::vector<TutorialHint*>::iterator it = stateHints[currentState].begin();
         it != stateHints[currentState].end(); ++it)
    {
        TutorialHint* hint = *it;
        if (hint == NULL)
            continue;
        not_null_ptr<ItemInstance> selected = player->mInventory->getSelected();
        int result = hint->attack(selected, entity);
        if (result != -1)
        {
            PopupMessageDetails* details = new PopupMessageDetails();
            details->titleId = result;
            setMessage(hint, details);
            return 0;
        }
    }
    return 0xffffffffU;
}

unsigned int Tutorial::itemDamaged(not_null_ptr<ItemInstance> item)
{
    for (std::vector<TutorialHint*>::iterator it = stateHints[currentState].begin();
         it != stateHints[currentState].end(); ++it)
    {
        TutorialHint* hint = *it;
        if (hint == NULL)
            continue;
        int result = hint->itemDamaged(item);
        if (result != -1)
        {
            PopupMessageDetails* details = new PopupMessageDetails();
            details->titleId = result;
            setMessage(hint, details);
            return 0;
        }
    }
    return 0xffffffffU;
}

void Tutorial::handleUIInput(int input)
{
    if (popupPending && currentMessage != NULL)
    {
        currentMessage->displayed = true;
        popupPending = false;
    }
}

void Tutorial::createItemSelected(not_null_ptr<ItemInstance> item, bool selected)
{
    for (std::vector<TutorialHint*>::iterator it = stateHints[currentState].begin();
         it != stateHints[currentState].end(); ++it)
    {
        TutorialHint* hint = *it;
        if (hint == NULL)
            continue;
        int result = hint->createItemSelected(item, selected);
        if (result != -1)
        {
            PopupMessageDetails* details = new PopupMessageDetails();
            details->titleId = result;
            setMessage(hint, details);
            return;
        }
    }
}

unsigned int Tutorial::onCrafted(not_null_ptr<ItemInstance> item)
{
    for (int state = 0; state < eTutorial_State_Max; ++state)
    {
        for (std::vector<TutorialTask*>::iterator it = stateTasks[state].begin();
             it != stateTasks[state].end(); ++it)
        {
            if (*it != NULL)
                (*it)->onCrafted(item);
        }
    }
    return 0xffffffffU;
}

unsigned int Tutorial::onEquip(not_null_ptr<ItemInstance> item)
{
    for (int state = 0; state < eTutorial_State_Max; ++state)
    {
        for (std::vector<TutorialTask*>::iterator it = stateTasks[state].begin();
             it != stateTasks[state].end(); ++it)
        {
            if (*it != NULL)
                (*it)->onEquip(item);
        }
    }
    return 0xffffffffU;
}

unsigned int Tutorial::onTakeFromAnvil(not_null_ptr<ItemInstance> item)
{
    for (int state = 0; state < eTutorial_State_Max; ++state)
    {
        for (std::vector<TutorialTask*>::iterator it = stateTasks[state].begin();
             it != stateTasks[state].end(); ++it)
        {
            if (*it != NULL)
                (*it)->onTakeFromAnvil(item);
        }
    }
    return 0xffffffffU;
}

unsigned int Tutorial::onTake(not_null_ptr<ItemInstance> item, unsigned int itemCount,
                               unsigned int exactCount)
{
    if (!enabled)
    {
        for (std::vector<TutorialHint*>::iterator it = stateHints[currentState].begin();
             it != stateHints[currentState].end(); ++it)
        {
            if (*it != NULL && (*it)->onTake(item) != 0)
                break;
        }
    }
    for (int state = 0; state < 22; ++state)
    {
        for (std::vector<TutorialTask*>::iterator it = stateTasks[state].begin();
             it != stateTasks[state].end(); ++it)
        {
            if (*it != NULL)
                (*it)->onTake(item, itemCount, exactCount);
        }
        for (std::vector<TutorialHint*>::iterator it = stateHints[state].begin();
             it != stateHints[state].end(); ++it)
        {
            if (*it != NULL)
                (*it)->onTake(item);
        }
    }
    return 0xffffffffU;
}

unsigned int Tutorial::onSelectedItemChanged(not_null_ptr<ItemInstance> item)
{
    if (isSelectedItemState() == 0)
        return 0;
    if (item.get() == NULL || item->isEmpty())
    {
        changeTutorialState(eTutorial_State_Gameplay, NULL);
        return 0;
    }
    Item* itemType = item->getItem();
    if (itemType == Items::fishingRod)
    {
        changeTutorialState(eTutorial_State_Fishing, NULL);
        return 0;
    }
    if (itemType == Items::carrotOnAStick)
    {
        changeTutorialState(eTutorial_State_NameTag, NULL);
        return 0;
    }
    changeTutorialState(eTutorial_State_Gameplay, NULL);
    return 0;
}

void Tutorial::onLookAt(const BlockState* state)
{
    if (!enabled)
    {
        bool handled = false;
        for (std::vector<TutorialHint*>::iterator it = stateHints[currentState].begin();
             it != stateHints[currentState].end(); ++it)
        {
            if (*it != NULL && (*it)->onLookAt(state) != 0)
            {
                handled = true;
                break;
            }
        }
        if (!handled && currentState == eTutorial_State_Gameplay && state != NULL && state->getBlock() == Blocks::bed)
            changeTutorialState(eTutorial_State_Bed, NULL);
    }
}

void Tutorial::onLookAtSkull(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (level == NULL || state == NULL)
    {
        popupPending = false;
    }
}

unsigned int Tutorial::onLookAtEntity(boost::shared_ptr<Entity> entity)
{
    if (!enabled)
    {
        bool hintHandled = false;
        for (std::vector<TutorialHint*>::iterator it = stateHints[currentState].begin();
             it != stateHints[currentState].end(); ++it)
        {
            if (*it != NULL && (*it)->onLookAtEntity(entity) != 0)
            {
                hintHandled = true;
                break;
            }
        }
        if (!hintHandled && currentState == eTutorial_State_Gameplay && entity.get() != NULL)
        {
            unsigned long long entityType = entity->GetType();
            if ((entityType & EntityType::ABSTRACT_HORSE) == EntityType::ABSTRACT_HORSE
                && entityType != EntityType::SKELETON_HORSE
                && entityType != EntityType::ZOMBIE_HORSE
                && (!splitScreen || hintShown))
            {
                boost::shared_ptr<AbstractHorse> horse =
                    boost::static_pointer_cast<AbstractHorse>(entity);
                if (!horse->isTamed())
                    changeTutorialState(eTutorial_State_Horse, NULL);
            }
        }
        for (std::vector<TutorialTask*>::iterator it = stateTasks[currentState].begin();
             it != stateTasks[currentState].end(); ++it)
        {
            if (*it != NULL)
                (*it)->onLookAtEntity(entity);
        }
    }
    return 0xffffffffU;
}

unsigned int Tutorial::onRideEntity(boost::shared_ptr<Entity> entity)
{
    if (currentState == eTutorial_State_Gameplay)
    {
        unsigned long long type = entity->GetType();
        if (type == EntityType::BOAT)
        {
            changeTutorialState(eTutorial_State_RidingBoat, NULL);
        }
        else if (type == EntityType::MINECART_RIDEABLE)
        {
            changeTutorialState(eTutorial_State_RidingMinecart, NULL);
        }
    }
    for (std::vector<TutorialTask*>::iterator it = stateTasks[currentState].begin();
         it != stateTasks[currentState].end(); ++it)
    {
        if (*it != NULL)
            (*it)->onRideEntity(entity);
    }
    return 0xffffffffU;
}

void Tutorial::onEffectChanged(MobEffect* effect, bool added)
{
    if (effect == NULL)
    {
        return;
    }
    enabled = added || enabled;
}

void Tutorial::onGameModeChange(eTutorial_GameMode newGameMode)
{
    if (gameMode == newGameMode)
    {
        return;
    }
    if (gameMode == eTutorial_GameMode_Fireworks && !hintShown)
    {
        return;
    }
    gameMode = newGameMode;
    gameModeChanged = true;
    if (newGameMode != eTutorial_GameMode_Fireworks)
    {
        return;
    }
    hintShown = false;
}

unsigned int Tutorial::staticCtor()
{
    if (gTutorialCompletionIds.empty())
    {
        const unsigned int count = sizeof(gTutorialCompletionIdValues) / sizeof(gTutorialCompletionIdValues[0]);
        for (unsigned int i = 0; i < count; ++i)
            gTutorialCompletionIds.push_back(gTutorialCompletionIdValues[i]);
    }
    return 0;
}
