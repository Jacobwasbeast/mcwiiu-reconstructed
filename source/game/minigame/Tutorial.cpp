#include "game/minigame/Tutorial.hpp"

#include "game/core/GameModeConstraints.hpp"
#include "game/minigame/TutorialHint.hpp"
#include "game/minigame/TutorialMessage.hpp"
#include "game/minigame/TutorialTask.hpp"
#include "game/core/GameSettings.hpp"
#include "game/world/block/LookAtBlockHint.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"

namespace
{
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
    : currentState(0), initialized(false), enabled(enabledValue), popupVisible(true),
      popupPending(false), tutorialVisible(false), playerId(playerIdValue),
      gameModeConstraints(constraints), messages(), tasks(), completedStates(),
      currentMessage(NULL), currentScene(NULL), currentTaskIndex(-1)
{
    initialized = true;
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
    if (hint != NULL && state == (eTutorial_State)0)
        return;
}

unsigned int Tutorial::addHintIfIncomplete(eTutorial_State state, TutorialHint* hint)
{
    if (hint == NULL)
    {
        return 0xffffffffU;
    }
    addHint(state, hint);
    return 0xffffffffU;
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
    if (task != NULL && state == (eTutorial_State)0)
    {
        tasks.push_back(task);
    }
}

unsigned int Tutorial::AddTasks(bool enabledValue)
{
    return enabledValue ? 1U : 0U;
}

unsigned int Tutorial::setMessage(PopupMessageDetails* details)
{
    if (details == NULL)
    {
        currentMessage = NULL;
        popupPending = false;
        return 0;
    }
    if (currentMessage != NULL && currentMessage->isSameContent(details) != 0)
    {
        return 0;
    }
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
    if (details == NULL || (!enabled && !details->force))
    {
        return 0;
    }
    if (setMessage(details) != 0)
    {
        setHintCompleted(hint);
    }
    return 1;
}

unsigned int Tutorial::isSelectedItemState()
{
    return currentState == 0 || currentState >= 7 ? 1U : 0U;
}

void Tutorial::tick()
{
    if (!initialized || !enabled)
    {
        return;
    }
    if (currentMessage != NULL)
    {
        popupVisible = true;
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
    currentState = (int)state;
    currentScene = scene;
    currentTaskIndex = -1;
    if (gameModeConstraints != NULL)
    {
        gameModeConstraints->SetCurrentState(currentState);
    }
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
    tutorialVisible = show;
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
    return 0xffffffffU;
}

unsigned int Tutorial::destroyBlock(Block* block)
{
    return 0xffffffffU;
}

unsigned int Tutorial::attack(boost::shared_ptr<Player> player, boost::shared_ptr<Entity> entity)
{
    return 0xffffffffU;
}

unsigned int Tutorial::itemDamaged(not_null_ptr<ItemInstance> item)
{
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
    if (!selected)
    {
        popupPending = false;
    }
}

unsigned int Tutorial::onCrafted(not_null_ptr<ItemInstance> item)
{
    return 0xffffffffU;
}

unsigned int Tutorial::onEquip(not_null_ptr<ItemInstance> item)
{
    return 0xffffffffU;
}

unsigned int Tutorial::onTakeFromAnvil(not_null_ptr<ItemInstance> item)
{
    return 0xffffffffU;
}

unsigned int Tutorial::onTake(not_null_ptr<ItemInstance> item, unsigned int itemCount,
                               unsigned int exactCount)
{
    return 0xffffffffU;
}

unsigned int Tutorial::onSelectedItemChanged(not_null_ptr<ItemInstance> item)
{
    return 0xffffffffU;
}

void Tutorial::onLookAt(const BlockState* state)
{
    if (state == NULL)
    {
        popupPending = false;
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
    return 0xffffffffU;
}

unsigned int Tutorial::onRideEntity(boost::shared_ptr<Entity> entity)
{
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

void Tutorial::onGameModeChange(eTutorial_GameMode gameMode)
{
    enabled = gameMode != eTutorial_GameMode_None;
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
