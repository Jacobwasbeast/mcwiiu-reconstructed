#ifndef GAME_MINIGAME_TUTORIAL_HPP
#define GAME_MINIGAME_TUTORIAL_HPP

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

#include "game/item/not_null_ptr.hpp"

class Block;
class BlockPos;
class BlockState;
class Entity;
class GameModeConstraints;
class ItemInstance;
class Level;
class MobEffect;
class Player;
class TutorialHint;
class TutorialMessage;
class TutorialTask;
class UIScene;

enum eTutorial_Hint
{
    eTutorial_Hint_None = 0
};

enum eTutorial_State
{
    eTutorial_State_None = 0
};

enum eTutorial_GameMode
{
    eTutorial_GameMode_None = 0
};

enum eTutorial_CompletionAction
{
    eTutorial_CompletionAction_None = 0
};

class Tutorial
{
public:
    class PopupMessageDetails
    {
    public:
        PopupMessageDetails();
        unsigned int isSameContent(PopupMessageDetails* details);

        int titleId;
        int messageId;
        union
        {
            int promptId;
            int itemDescriptionId;
        };
        std::wstring title;
        std::wstring message;
        std::wstring itemDescription;
        boost::shared_ptr<ItemInstance> itemInstance;
        union
        {
            int displayTime;
            int itemId;
        };
        union
        {
            int priority;
            int itemAuxValue;
        };
        bool repeat;
        bool unknown;
        bool displayed;
        bool allowFade;
        bool isItem;
        bool force;
        int state;
    };

    Tutorial(int playerId, GameModeConstraints* constraints, bool enabled, bool skipSetup);
    virtual ~Tutorial();

    void addMessage(int messageId, bool repeatMessage, unsigned char maxDisplayCount);
    unsigned int getCompleted(int completionId);
    void isHintCompleted(eTutorial_Hint hint);
    void addHint(eTutorial_State state, TutorialHint* hint);
    unsigned int addHintIfIncomplete(eTutorial_State state, TutorialHint* hint);
    unsigned int AddHints();
    void addTask(eTutorial_State state, TutorialTask* task);
    unsigned int AddTasks(bool enabled);
    unsigned int setMessage(PopupMessageDetails* details);
    void setCompleted(int completionId);
    void setHintCompleted(eTutorial_Hint hint);
    unsigned int setHintCompleted(TutorialHint* hint);
    int setMessage(TutorialHint* hint, PopupMessageDetails* details);
    unsigned int isSelectedItemState();
    virtual void tick();
    virtual unsigned char isStateCompleted(eTutorial_State state);
    virtual void setStateCompleted(eTutorial_State state);
    virtual void changeTutorialState(eTutorial_State state, UIScene* scene);
    void setMessage(const std::wstring& message, int displayTime, int priority);
    void showTutorialPopup(bool show);
    unsigned int useItemOn(Level* level, not_null_ptr<class ItemInstance> item,
                           const BlockPos& pos, bool cancelled);
    unsigned int useItemOn(not_null_ptr<class ItemInstance> item, bool cancelled);
    unsigned int completeUsingItem(not_null_ptr<class ItemInstance> item);
    unsigned int startDestroyBlock(not_null_ptr<class ItemInstance> item, const BlockState* state);
    unsigned int destroyBlock(Block* block);
    unsigned int attack(boost::shared_ptr<Player> player, boost::shared_ptr<Entity> entity);
    unsigned int itemDamaged(not_null_ptr<class ItemInstance> item);
    void handleUIInput(int input);
    void createItemSelected(not_null_ptr<class ItemInstance> item, bool selected);
    unsigned int onCrafted(not_null_ptr<class ItemInstance> item);
    unsigned int onEquip(not_null_ptr<class ItemInstance> item);
    unsigned int onTakeFromAnvil(not_null_ptr<class ItemInstance> item);
    unsigned int onTake(not_null_ptr<class ItemInstance> item, unsigned int itemCount,
                        unsigned int exactCount);
    unsigned int onSelectedItemChanged(not_null_ptr<class ItemInstance> item);
    void onLookAt(const BlockState* state);
    void onLookAtSkull(Level* level, const BlockPos& pos, const BlockState* state);
    unsigned int onLookAtEntity(boost::shared_ptr<Entity> entity);
    unsigned int onRideEntity(boost::shared_ptr<Entity> entity);
    void onEffectChanged(MobEffect* effect, bool added);
    void onGameModeChange(eTutorial_GameMode gameMode);
    static unsigned int staticCtor();

    std::vector<TutorialTask*>* getTasks();
    int getCurrentTaskIndex();
    static void debugResetPlayerSavedProgress(int playerId);

    int currentState;
    bool stateChanged;
    bool splitScreen;
    bool started;
    int startTime;
    GameModeConstraints* gameModeConstraints;
    boost::unordered_map<int, TutorialMessage*> messages;
    std::vector<TutorialTask*> tasks;
    std::vector<TutorialTask*> stateTasks[44];
    std::vector<TutorialHint*> stateHints[44];
    TutorialTask* currentTask[44];
    std::vector<eTutorial_State> completedStates;
    bool overridesGameHostOptions;
    bool gameHostOptionsOverridden;
    eTutorial_GameMode gameMode;
    bool gameModeChanged;
    bool hintShown;
    int playerId;
    int visibleTime;
    int messageTime;
    PopupMessageDetails* currentMessage;
    int messageState;
    int messageRepeatCount;
    bool popupVisible;
    bool popupPending;
    bool uiControllerSet;
    bool tutorialHidden;
    UIScene* currentScene;
    bool tasksChecked;
    bool completed;
    bool enabled;
};

#endif
