#include "game/minigame/Tutorial.hpp"

#include "game/core/ChoiceTask.hpp"
#include "game/core/GameModeConstraints.hpp"
#include "game/core/InfoTask.hpp"
#include "game/core/ProcedureCompoundTask.hpp"
#include "game/entity/mob/AbstractHorse.hpp"
#include "game/entity/mob/HorseChoiceTask.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/world/generation/Player.hpp"
#include "game/minigame/TutorialHint.hpp"
#include "game/minigame/TutorialMessage.hpp"
#include "game/minigame/TutorialTask.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/L10N.hpp"
#include "game/world/block/LookAtBlockHint.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "vendor/_TutorialPopupInfo.hpp"

unsigned int GetTickCount();

extern ConsoleUIController gConsoleUIController;

namespace
{
    unsigned int gTutorialVisibleTime = 7000;
    unsigned int gTutorialMinimumDisplayTime = 2000;
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
    : currentState(0), stateChanged(false), started(false), startTime(0),
      gameModeConstraints(constraints), messages(), tasks(), currentTask(),
      completedStates(), overridesGameHostOptions(false), gameHostOptionsOverridden(false),
      gameMode(static_cast<eTutorial_GameMode>(4)), gameModeChanged(false), hintShown(false),
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

void Tutorial::isHintCompleted(eTutorial_Hint hint)
{
    getCompleted((int)hint);
}

void Tutorial::addHint(eTutorial_State state, TutorialHint* hint)
{
    stateHints[state].push_back(hint);
}

unsigned int Tutorial::addHintIfIncomplete(eTutorial_State state, TutorialHint* hint)
{
    int hintId = *reinterpret_cast<int*>(reinterpret_cast<char*>(hint) + 0xc);
    if (getCompleted(hintId) == 0)
    {
        addHint(state, hint);
        return 0;
    }
    delete hint;
    return 0;
}

unsigned int Tutorial::AddHints()
{
    addHintIfIncomplete((eTutorial_State)0,
                        new TutorialHint((eTutorial_Hint)45, this,
                                         (int)0xA3540000 - 7424,
                                         (TutorialHint::eHintType)1,
                                         true, true, -1));
    addHintIfIncomplete((eTutorial_State)0,
                        new TutorialHint((eTutorial_Hint)46, this,
                                         (int)0x71390000 - 14709,
                                         (TutorialHint::eHintType)3,
                                         true, true, -1));
    addHintIfIncomplete((eTutorial_State)0,
                        new TutorialHint((eTutorial_Hint)47, this,
                                         (int)0xB4980000 + 23009,
                                         (TutorialHint::eHintType)8,
                                         true, true, -1));
    addHint((eTutorial_State)2,
            new TutorialHint((eTutorial_Hint)44, this,
                             (int)0xDBD70000 + 9206,
                             (TutorialHint::eHintType)2,
                             true, true, -1));
    const BlockState* stoneState[] = { Blocks::stone->defaultBlockState() };
    addHintIfIncomplete((eTutorial_State)0,
                        new LookAtBlockHint((eTutorial_Hint)57, this,
                                            stoneState, 1, 0, 0, -1, 0, true));
    const BlockState* cobblestoneState[] = { Blocks::cobblestone->defaultBlockState() };
    addHintIfIncomplete((eTutorial_State)0,
                        new LookAtBlockHint((eTutorial_Hint)58, this,
                                            cobblestoneState, 1, 0, 0, -1, 0, true));
    const BlockState* planksState[] = { Blocks::planks->defaultBlockState() };
    addHintIfIncomplete((eTutorial_State)0,
                        new LookAtBlockHint((eTutorial_Hint)59, this,
                                            planksState, 1, 0, 0, -1, 0, true));
    const BlockState* saplingState[] = { Blocks::sapling->defaultBlockState() };
    addHintIfIncomplete((eTutorial_State)0,
                        new LookAtBlockHint((eTutorial_Hint)60, this,
                                            saplingState, 1, 0, 0, -1, 0, true));
    const BlockState* bedrockState[] = { Blocks::bedrock->defaultBlockState() };
    addHintIfIncomplete((eTutorial_State)0,
                        new LookAtBlockHint((eTutorial_Hint)61, this,
                                            bedrockState, 1, 0, 0, -1, 0, true));
    const BlockState* flowingLavaState[] = { Blocks::flowing_lava->defaultBlockState(),
                                             Blocks::lava->defaultBlockState() };
    addHintIfIncomplete((eTutorial_State)0,
                        new LookAtBlockHint((eTutorial_Hint)62, this,
                                            flowingLavaState, 2, 0, 0, -1, 0, true));
    const BlockState* magmaState[] = { Blocks::magma->defaultBlockState() };
    addHintIfIncomplete((eTutorial_State)0,
                        new LookAtBlockHint((eTutorial_Hint)63, this,
                                            magmaState, 1, 0, 0, -1, 0, true));
    const BlockState* sandState[] = { Blocks::sand->defaultBlockState() };
    addHintIfIncomplete((eTutorial_State)0,
                        new LookAtBlockHint((eTutorial_Hint)64, this,
                                            sandState, 1, 0, 0, -1, 0, true));
    const BlockState* gravelState[] = { Blocks::gravel->defaultBlockState() };
    addHintIfIncomplete((eTutorial_State)0,
                        new LookAtBlockHint((eTutorial_Hint)65, this,
                                            gravelState, 1, 0, 0, -1, 0, true));
    const BlockState* goldOreState[] = { Blocks::gold_ore->defaultBlockState() };
    addHintIfIncomplete((eTutorial_State)0,
                        new LookAtBlockHint((eTutorial_Hint)66, this,
                                            goldOreState, 1, 0, 0, -1, 0, true));
    const BlockState* ironOreState[] = { Blocks::iron_ore->defaultBlockState() };
    addHintIfIncomplete((eTutorial_State)0,
                        new LookAtBlockHint((eTutorial_Hint)67, this,
                                            ironOreState, 1, 0, 0, -1, 0, true));
    const BlockState* coalOreState[] = { Blocks::coal_ore->defaultBlockState() };
    addHintIfIncomplete((eTutorial_State)0,
                        new LookAtBlockHint((eTutorial_Hint)68, this,
                                            coalOreState, 1, 0, 0, -1, 0, true));
    addHintIfIncomplete((eTutorial_State)0,
                        new LookAtBlockHint((eTutorial_Hint)69, this,
                                            coalOreState, 1, 0, 0, -1, 0, true));
    return 0;
}

void Tutorial::addTask(eTutorial_State state, TutorialTask* task)
{
    if (state == eTutorial_State_None)
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
    if (!enabledValue && isStateCompleted(static_cast<eTutorial_State>(1)))
        goto check_state_0xE;
    create_state_1:
    compound_0 = new ProcedureCompoundTask(this);
    compound_0->AddTask(new ChoiceTask(this, 0xB77FC340, 0x9E1E8969, true, 0, 1, static_cast<eTutorial_CompletionAction>(1), static_cast<ETelemetryChallenges>(0x4)));
    compound_0->AddTask(new InfoTask(this, 0xC18416E0, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    compound_0->AddTask(new InfoTask(this, 0x99B85F72, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    compound_0->AddTask(new InfoTask(this, 0x1A28A6CA, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    compound_0->AddTask(new InfoTask(this, 0xEC1649D5, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    addTask(static_cast<eTutorial_State>(1), compound_0);
    if (!enabledValue)
        goto check_state_0xE;
    create_state_0xE:
    compound_1 = new ProcedureCompoundTask(this);
    compound_1->AddTask(new ChoiceTask(this, 0x25598D4, 0x46859B85, true, 0, 1, static_cast<eTutorial_CompletionAction>(1), static_cast<ETelemetryChallenges>(0x14)));
    compound_1->AddTask(new InfoTask(this, 0x11BE3D54, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    compound_1->AddTask(new InfoTask(this, 0x5F3FE8A6, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    compound_1->AddTask(new InfoTask(this, 0x94985DDE, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    compound_1->AddTask(new InfoTask(this, 0x3F6BE86, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    compound_1->AddTask(new InfoTask(this, 0x893A28E1, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    addTask(static_cast<eTutorial_State>(0xE), compound_1);
    if (!enabledValue)
        goto check_state_2;
    create_state_2:
    compound_2 = new ProcedureCompoundTask(this);
    compound_2->AddTask(new ChoiceTask(this, 0x9A252193, 0x7F0C21BA, true, 0, 1, static_cast<eTutorial_CompletionAction>(1), static_cast<ETelemetryChallenges>(0x5)));
    compound_2->AddTask(new InfoTask(this, 0x27A2B969, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    compound_2->AddTask(new InfoTask(this, 0xA42613B4, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    compound_2->AddTask(new InfoTask(this, 0x723936AB, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    compound_2->AddTask(new InfoTask(this, 0x1F10DDC2, 0xA0D0169C, false, 2, static_cast<ETelemetryChallenges>(0x0)));
    compound_2->AddTask(new InfoTask(this, 0xB135E36, 0xFC8730E4, false, 2, static_cast<ETelemetryChallenges>(0x0)));
    compound_2->AddTask(new InfoTask(this, 0x3AF6EF3E, 0xAF244F78, false, 2, static_cast<ETelemetryChallenges>(0x0)));
    addTask(static_cast<eTutorial_State>(2), compound_2);
    if (!enabledValue)
        goto check_state_4;
    create_state_4:
    compound_3 = new ProcedureCompoundTask(this);
    compound_3->AddTask(new ChoiceTask(this, 0x6CE6D19, 0x4C0CF470, true, 0, 1, static_cast<eTutorial_CompletionAction>(1), static_cast<ETelemetryChallenges>(0x6)));
    compound_3->AddTask(new InfoTask(this, 0x9BB6558D, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    compound_3->AddTask(new InfoTask(this, 0xC6B63D93, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    compound_3->AddTask(new InfoTask(this, 0x5466C434, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    addTask(static_cast<eTutorial_State>(4), compound_3);
    if (!enabledValue)
        goto check_state_0x12;
    create_state_0x12:
    compound_4 = new ProcedureCompoundTask(this);
    compound_4->AddTask(new ChoiceTask(this, 0x9253A77, 0x9E500506, true, 0, 1, static_cast<eTutorial_CompletionAction>(1), static_cast<ETelemetryChallenges>(0xF)));
    compound_4->AddTask(new InfoTask(this, 0x96BEBCB3, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    compound_4->AddTask(new InfoTask(this, 0xAED3450F, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    compound_4->AddTask(new InfoTask(this, 0xC02F0030, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    compound_4->AddTask(new InfoTask(this, 0xFB48ACEA, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    compound_4->AddTask(new InfoTask(this, 0xFCFE0427, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    addTask(static_cast<eTutorial_State>(0x12), compound_4);
    if (!enabledValue)
        goto check_state_0x14;
    create_state_0x14:
    compound_5 = new ProcedureCompoundTask(this);
    compound_5->AddTask(new ChoiceTask(this, 0x18A28D4E, 0x690D3FD7, true, 0, 1, static_cast<eTutorial_CompletionAction>(1), static_cast<ETelemetryChallenges>(0x15)));
    compound_5->AddTask(new InfoTask(this, 0xD1EDBF15, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    compound_5->AddTask(new InfoTask(this, 0xCA85FE57, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    compound_5->AddTask(new InfoTask(this, 0x244CF0FE, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    compound_5->AddTask(new InfoTask(this, 0x731167E6, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    compound_5->AddTask(new InfoTask(this, 0x697EA620, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    addTask(static_cast<eTutorial_State>(0x14), compound_5);
    if (!enabledValue)
        goto check_state_0x1B;
    create_state_0x1B:
    compound_6 = new ProcedureCompoundTask(this);
    compound_6->AddTask(new ChoiceTask(this, 0xD381DD9, 0x645B6F8, true, 0, 1, static_cast<eTutorial_CompletionAction>(1), static_cast<ETelemetryChallenges>(0x2E)));
    compound_6->AddTask(new InfoTask(this, 0xC9DE21A, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    compound_6->AddTask(new InfoTask(this, 0x4F577027, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    compound_6->AddTask(new InfoTask(this, 0x5E84B4A1, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    compound_6->AddTask(new InfoTask(this, 0x87DCC839, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    compound_6->AddTask(new InfoTask(this, 0x190AE839, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    compound_6->AddTask(new InfoTask(this, 0x67CD9B1F, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    compound_6->AddTask(new InfoTask(this, 0x51ED81D1, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    addTask(static_cast<eTutorial_State>(0x1B), compound_6);
    if (!enabledValue)
        goto check_state_0x19;
    create_state_0x19:
    compound_7 = new ProcedureCompoundTask(this);
    compound_7->AddTask(new ChoiceTask(this, 0x821611E6, 0x789B40B7, true, 0, 1, static_cast<eTutorial_CompletionAction>(1), static_cast<ETelemetryChallenges>(0x30)));
    compound_7->AddTask(new InfoTask(this, 0x54091CCD, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    compound_7->AddTask(new InfoTask(this, 0xB7242ECF, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    compound_7->AddTask(new InfoTask(this, 0x7C03162F, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    compound_7->AddTask(new InfoTask(this, 0x7B8B7B8F, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    compound_7->AddTask(new InfoTask(this, 0x4896FB7B, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    addTask(static_cast<eTutorial_State>(0x19), compound_7);
    if (!enabledValue)
        goto check_state_0x1D;
    create_state_0x1D:
    addTask(static_cast<eTutorial_State>(0x1D), new HorseChoiceTask(this, 0x4DCDE902, 0x80BBE3E3, 0xDB419510, 0xE9B948F4, 0x7595B01B, true, 0, 1, static_cast<eTutorial_CompletionAction>(2), static_cast<ETelemetryChallenges>(0x32)));
    addTask(static_cast<eTutorial_State>(0x1D), new InfoTask(this, 0x8DFE2C47, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    addTask(static_cast<eTutorial_State>(0x1D), new InfoTask(this, 0xCE6D12A7, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    addTask(static_cast<eTutorial_State>(0x1D), new InfoTask(this, 0xFB9D6DB1, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    addTask(static_cast<eTutorial_State>(0x1D), new InfoTask(this, 0x86C858BB, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    addTask(static_cast<eTutorial_State>(0x1D), new InfoTask(this, 0x32AEB3B, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    addTask(static_cast<eTutorial_State>(0x1D), new InfoTask(this, 0xD0B71241, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    addTask(static_cast<eTutorial_State>(0x1D), new InfoTask(this, 0xB916B23D, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    addTask(static_cast<eTutorial_State>(0x1D), new InfoTask(this, 0x3ED848DD, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    if (!enabledValue)
        goto check_state_0x1E;
    create_state_0x1E:
    compound_9 = new ProcedureCompoundTask(this);
    compound_9->AddTask(new ChoiceTask(this, 0x7C8AEDB6, 0xEB9A80F7, true, 0, 1, static_cast<eTutorial_CompletionAction>(1), static_cast<ETelemetryChallenges>(0x33)));
    compound_9->AddTask(new InfoTask(this, 0xE4582E6F, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    compound_9->AddTask(new InfoTask(this, 0xAC55BC2F, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    compound_9->AddTask(new InfoTask(this, 0xAD1032A9, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    addTask(static_cast<eTutorial_State>(0x1E), compound_9);
    if (!enabledValue)
        goto check_state_0x24;
    create_state_0x24:
    compound_10 = new ProcedureCompoundTask(this);
    compound_10->AddTask(new ChoiceTask(this, 0xECBB7BAC, 0xB0ED8A95, true, 0, 1, static_cast<eTutorial_CompletionAction>(1), static_cast<ETelemetryChallenges>(0x35)));
    compound_10->AddTask(new InfoTask(this, 0x8C159652, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    compound_10->AddTask(new InfoTask(this, 0xFE1D058D, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    compound_10->AddTask(new InfoTask(this, 0x22299E7D, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    compound_10->AddTask(new InfoTask(this, 0xE44F42D0, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    compound_10->AddTask(new InfoTask(this, 0x13FD0693, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    compound_10->AddTask(new InfoTask(this, 0x859377B3, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    compound_10->AddTask(new InfoTask(this, 0xD0C84AA4, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    compound_10->AddTask(new InfoTask(this, 0x36518124, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    compound_10->AddTask(new InfoTask(this, 0x306CF96D, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    addTask(static_cast<eTutorial_State>(0x24), compound_10);
    if (!enabledValue)
        goto check_state_0x22;
    create_state_0x22:
    compound_11 = new ProcedureCompoundTask(this);
    compound_11->AddTask(new ChoiceTask(this, 0x6FF43521, 0xBA245C8, true, 0, 1, static_cast<eTutorial_CompletionAction>(1), static_cast<ETelemetryChallenges>(0x37)));
    compound_11->AddTask(new InfoTask(this, 0x934C3015, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    compound_11->AddTask(new InfoTask(this, 0xF87CF280, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    compound_11->AddTask(new InfoTask(this, 0xC6447326, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    addTask(static_cast<eTutorial_State>(0x22), compound_11);
    if (!enabledValue)
        goto check_state_5;
    create_state_5:
    addTask(static_cast<eTutorial_State>(5), new ChoiceTask(this, 0x97628A42, 0x16DFAE83, true, 0, 1, static_cast<eTutorial_CompletionAction>(2), static_cast<ETelemetryChallenges>(0x8)));
    addTask(static_cast<eTutorial_State>(5), new InfoTask(this, 0x6D8D73AC, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    addTask(static_cast<eTutorial_State>(5), new InfoTask(this, 0xF965D585, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    addTask(static_cast<eTutorial_State>(5), new InfoTask(this, 0x5C8E319F, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    if (!enabledValue)
        goto check_state_6;
    create_state_6:
    addTask(static_cast<eTutorial_State>(6), new ChoiceTask(this, 0x25C06AF9, 0x811A4288, true, 0, 1, static_cast<eTutorial_CompletionAction>(2), static_cast<ETelemetryChallenges>(0x9)));
    addTask(static_cast<eTutorial_State>(6), new InfoTask(this, 0xE6A88FAF, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    if (!enabledValue)
        goto check_state_7;
    create_state_7:
    addTask(static_cast<eTutorial_State>(7), new ChoiceTask(this, 0x2C532B5D, 0x1B284F34, true, 0, 1, static_cast<eTutorial_CompletionAction>(2), static_cast<ETelemetryChallenges>(0x7)));
    addTask(static_cast<eTutorial_State>(7), new InfoTask(this, 0xC7681D23, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    addTask(static_cast<eTutorial_State>(7), new InfoTask(this, 0x4F274F92, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    addTask(static_cast<eTutorial_State>(7), new InfoTask(this, 0x314B8DCE, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    if (!enabledValue)
        goto check_state_8;
    create_state_8:
    addTask(static_cast<eTutorial_State>(8), new InfoTask(this, 0xAD017F4C, 0x3F9463F, true, 0x3A, static_cast<ETelemetryChallenges>(0x0)));
    if (!enabledValue)
        goto check_state_9;
    goto add_state_9;
    check_state_0xE:
    if (!isStateCompleted(static_cast<eTutorial_State>(0xE)))
        goto create_state_0xE;
    check_state_2:
    if (!isStateCompleted(static_cast<eTutorial_State>(2)))
        goto create_state_2;
    check_state_4:
    if (!isStateCompleted(static_cast<eTutorial_State>(4)))
        goto create_state_4;
    check_state_0x12:
    if (!isStateCompleted(static_cast<eTutorial_State>(0x12)))
        goto create_state_0x12;
    check_state_0x14:
    if (!isStateCompleted(static_cast<eTutorial_State>(0x14)))
        goto create_state_0x14;
    check_state_0x1B:
    if (!isStateCompleted(static_cast<eTutorial_State>(0x1B)))
        goto create_state_0x1B;
    check_state_0x19:
    if (!isStateCompleted(static_cast<eTutorial_State>(0x19)))
        goto create_state_0x19;
    check_state_0x1D:
    if (!isStateCompleted(static_cast<eTutorial_State>(0x1D)))
        goto create_state_0x1D;
    check_state_0x1E:
    if (!isStateCompleted(static_cast<eTutorial_State>(0x1E)))
        goto create_state_0x1E;
    check_state_0x24:
    if (!isStateCompleted(static_cast<eTutorial_State>(0x24)))
        goto create_state_0x24;
    check_state_0x22:
    if (!isStateCompleted(static_cast<eTutorial_State>(0x22)))
        goto create_state_0x22;
    check_state_5:
    if (!isStateCompleted(static_cast<eTutorial_State>(5)))
        goto create_state_5;
    check_state_6:
    if (!isStateCompleted(static_cast<eTutorial_State>(6)))
        goto create_state_6;
    check_state_7:
    if (!isStateCompleted(static_cast<eTutorial_State>(7)))
        goto create_state_7;
    check_state_8:
    if (!isStateCompleted(static_cast<eTutorial_State>(8)))
        goto create_state_8;
    check_state_9:
    if (!isStateCompleted(static_cast<eTutorial_State>(9)))
        goto add_state_9;
    goto add_state_f;
    add_state_9:
    addTask(static_cast<eTutorial_State>(9), new ChoiceTask(this, 0xA76E95BA, 0x2C631D03, true, 0, 1, static_cast<eTutorial_CompletionAction>(2), static_cast<ETelemetryChallenges>(0xA)));
    addTask(static_cast<eTutorial_State>(9), new InfoTask(this, 0x54D286F0, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    addTask(static_cast<eTutorial_State>(9), new InfoTask(this, 0xA62DA153, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    if (enabledValue)
        return 0;
    add_state_f:
    if (!isStateCompleted(static_cast<eTutorial_State>(0xf)))
    {
    addTask(static_cast<eTutorial_State>(0xF), new ChoiceTask(this, 0x82BFAADD, 0x2B6F086C, true, 0, 1, static_cast<eTutorial_CompletionAction>(2), static_cast<ETelemetryChallenges>(0xD)));
    addTask(static_cast<eTutorial_State>(0xF), new InfoTask(this, 0x67613737, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    addTask(static_cast<eTutorial_State>(0xF), new InfoTask(this, 0xD756C6A6, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
    addTask(static_cast<eTutorial_State>(0xF), new InfoTask(this, 0x642C1BB4, 0x3F9463F, true, 0, static_cast<ETelemetryChallenges>(0x0)));
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
             static_cast<unsigned int>(visibleTime) + gTutorialVisibleTime))
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
        (GameSettings::GetGameSettings(playerId, static_cast<eGameSetting>(0xf)) == 0 ||
         GameSettings::GetGameSettings(playerId, static_cast<eGameSetting>(0x17)) == 0))
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
            if (now - static_cast<unsigned int>(visibleTime) <= 2000)
            {
                return 0;
            }
        }
        else if (now - static_cast<unsigned int>(messageTime) <= 14000)
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
    return currentState == 0 || currentState >= 7 ? 1U : 0U;
}

void Tutorial::tick()
{
    if (!started)
    {
        unsigned int now = GetTickCount();
        if (startTime == 0)
        {
            startTime = (int)now;
            return;
        }
        if (now - (unsigned int)startTime < 1501)
        {
            return;
        }
        started = true;
    }

    if (!popupVisible)
    {
        gConsoleUIController.SetTutorialVisible(playerId, false);
        return;
    }

    if (!uiControllerSet)
    {
        gConsoleUIController.SetTutorial(playerId, this);
        uiControllerSet = true;
    }

    if (gConsoleUIController.IsPauseMenuDisplayed(playerId))
        return;

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

    if (currentTask[currentState] == NULL && !stateTasks[currentState].empty())
    {
        currentTask[currentState] = stateTasks[currentState].front();
        currentTask[currentState]->setAsCurrentTask(true);
        stateChanged = true;
    }

    TutorialTask* task = currentTask[currentState];
    if (task == NULL || gameModeConstraints->HasFailedConstraint(currentState))
    {
        gConsoleUIController.SetTutorialVisible(playerId, false);
        return;
    }

    if (stateChanged)
    {
        PopupMessageDetails* details = new PopupMessageDetails();
        details->titleId = task->getDescriptionId();
        details->messageId = task->getPromptId();
        details->repeat = task->AllowFade();
        setMessage(details);
        stateChanged = false;
    }

    if (task->ShowMinimumTime() &&
        GetTickCount() > static_cast<unsigned int>(visibleTime) + gTutorialMinimumDisplayTime)
    {
        task->setShownForMinimumTime();
        if (currentMessage != NULL && currentMessage->messageId != task->getPromptId())
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
    if (currentState == (int)state)
    {
        currentScene = scene;
        return;
    }
    setMessage((PopupMessageDetails*)NULL);
    currentScene = scene;
    currentState = (int)state;
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
    for (int state = 0; state < 44; ++state)
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
    for (int state = 0; state < 44; ++state)
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
    for (int state = 0; state < 44; ++state)
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
        changeTutorialState(static_cast<eTutorial_State>(0), NULL);
        return 0;
    }
    Item* itemType = item->getItem();
    if (itemType == Items::fishingRod)
    {
        changeTutorialState(static_cast<eTutorial_State>(7), NULL);
        return 0;
    }
    if (itemType == Items::carrotOnAStick)
    {
        changeTutorialState(static_cast<eTutorial_State>(8), NULL);
        return 0;
    }
    changeTutorialState(static_cast<eTutorial_State>(0), NULL);
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
        if (!handled && currentState == 0 && state != NULL && state->getBlock() == Blocks::bed)
            changeTutorialState(static_cast<eTutorial_State>(9), NULL);
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
        if (!hintHandled && currentState == 0 && entity.get() != NULL)
        {
            unsigned long long entityType = entity->GetType();
            if ((entityType & 0x0D690000ULL) == 0x0D690000ULL
                && entityType != 0x000000002D690002ULL
                && entityType != 0x000000002D690003ULL
                && (!splitScreen || hintShown))
            {
                boost::shared_ptr<AbstractHorse> horse =
                    boost::static_pointer_cast<AbstractHorse>(entity);
                if (!horse->isTamed())
                    changeTutorialState(static_cast<eTutorial_State>(0x1D), NULL);
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
    if (currentState == 0)
    {
        unsigned long long type = entity->GetType();
        if (type == 0x8000008ULL)
        {
            changeTutorialState(static_cast<eTutorial_State>(6), NULL);
        }
        else if (type == 0xa800001ULL)
        {
            changeTutorialState(static_cast<eTutorial_State>(5), NULL);
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
    if (gameMode == static_cast<eTutorial_GameMode>(10) && !hintShown)
    {
        return;
    }
    gameMode = newGameMode;
    gameModeChanged = true;
    if (newGameMode != static_cast<eTutorial_GameMode>(10))
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
