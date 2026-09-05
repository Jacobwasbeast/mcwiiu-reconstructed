#include "game/minigame/Tutorial.hpp"

#include "game/core/GameModeConstraints.hpp"
#include "game/minigame/TutorialHint.hpp"
#include "game/minigame/TutorialMessage.hpp"
#include "game/minigame/TutorialTask.hpp"

Tutorial::PopupMessageDetails::PopupMessageDetails()
    : titleId(-1), messageId(-1), promptId(-1), title(), message(), prompt(),
      displayTime(0), fadeTime(0), priority(0), soundId(0), unknownId(-1),
      repeat(false), allowFade(true), force(false), displayed(false), state(-1)
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
            message == details->message && prompt == details->prompt) ? 1U : 0U;
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
    return 0;
}

void Tutorial::isHintCompleted(eTutorial_Hint hint)
{
    getCompleted((int)hint);
}

void Tutorial::addHint(eTutorial_State state, TutorialHint* hint)
{
    if (hint != NULL && state == (eTutorial_State)0)
    {
        return;
    }
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
    return 0;
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
    return 1;
}
