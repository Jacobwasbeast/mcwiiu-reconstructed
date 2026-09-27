#ifndef GAME_UI_UISCENE_INGAMEPLAYEROPTIONSMENU_HPP
#define GAME_UI_UISCENE_INGAMEPLAYEROPTIONSMENU_HPP

#include "game/ui/UIScene.hpp"
#include "game/ui/UIControl_Label.hpp"
#include "game/ui/UIControl_MultiList.hpp"

class INetworkPlayer;

class UIScene_InGamePlayerOptionsMenu : public UIScene
{
public:
    UIScene_InGamePlayerOptionsMenu(int sceneType, void* data, UILayer* layer);
    void resetCheatCheckboxes();
    static void OnPlayerChanged(void* context, INetworkPlayer* player, bool added);
    virtual std::wstring getMoviePath();
    virtual unsigned int mapElementsAndNames();
    virtual ~UIScene_InGamePlayerOptionsMenu();
    virtual int getSceneType();
    virtual void tick();
    virtual void handleTimerComplete(int timerId);
    virtual void updateTooltips();
    virtual void handleInput(int playerId, int action, bool repeat, bool released, bool pressed,
                             bool& handled);
    virtual void handleDestroy();
    virtual void handleReload();
    virtual void handlePress(int controlId, int childId);
    virtual void handleCheckboxToggled(int id, bool checked);
    virtual void handleCheckboxElementToggled(int first, int second, bool checked);
    virtual void handleSliderElementMove(int first, int second, int value);

    bool m_navigatingBack;
    bool m_isHost;
    unsigned char m_smallId;
    unsigned int m_privileges;
    UIControl_Label m_gamertag;
    UIControl_MultiList m_playerOptions;
    bool m_cheatsEnabled;
};

#endif
