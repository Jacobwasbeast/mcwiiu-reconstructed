#ifndef GAME_UI_UISCENE_GAMEMODESCOREBOARDMENU_HPP
#define GAME_UI_UISCENE_GAMEMODESCOREBOARDMENU_HPP

#include <stdint.h>
#include <string>
#include <boost/shared_ptr.hpp>
#include "game/world/generation/IUIScene_PlayerList.hpp"
#include "game/world/generation/PlayerUID.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/UIControl.hpp"
#include "game/ui/UIControl_Label.hpp"
#include "game/ui/UIControl_Progress.hpp"
#include "game/ui/UIControl_ScoreboardButtonList.hpp"
#include "game/core/IScoreboardLayout.hpp"
#include "game/core/ClientScoreboard.hpp"

class INetworkPlayer;
class MultiplayerLocalPlayer;
class StatsUID;
class UILayer;
class fuiRect;

class UIScene_GameModeScoreboardMenu : public IUIScene_PlayerList, public UIScene
{
public:
    UIScene_GameModeScoreboardMenu(int type, void* data, UILayer* layer);
    virtual ~UIScene_GameModeScoreboardMenu();
    virtual std::wstring getMoviePath();
    virtual unsigned int mapElementsAndNames();
    boost::shared_ptr<MultiplayerLocalPlayer> GetPlayer();
    ClientScoreboard* GetScoreboard();
    void registerTexture(int index);
    void UpdateStats(const StatsUID& uid);
    void SetStatIconTextureName(int index, const std::wstring& name);
    void SetTitleIconTextureName(int index, const std::wstring& name);
    void InitialiseScoreboard(INetworkPlayer* player);
    void SetHasPlayerBeenKilled(bool killed);
    virtual void tick();
    bool CanKickPlayer(INetworkPlayer* player);
    virtual void updateTooltips();
    virtual void handleGainFocus(bool hadFocus);
    virtual unsigned int GetMainPanel();
    virtual void customDraw(const char* name, fuiRect* rect);
    virtual void handleInput(int playerId, int action, bool repeat, bool released, bool pressed, bool& handled);
    virtual void handleDestroy();
    void setScoreboardType(int type);
    void initNumColumns(int columns);
    virtual void handleReload();
    virtual void handlePress(int first, int second);
    virtual void handleFocusChange(int focus, int childFocus);
    uint64_t CheckReloadMovie();
    virtual void BuildPlayerInfo(INetworkPlayer* player, PlayerInfo* info);
    virtual PlayerInfo* CreatePlayerInfo();
    virtual int GetPlayerCount();
    virtual PlayerInfo* GetPlayer(int index);
    virtual bool RemovePlayer(unsigned char smallId);
    virtual void AddPlayer(unsigned char smallId);
    virtual UIControl_ButtonList* GetPlayerList();
    virtual void UpdatePlayer(int index, PlayerInfo* oldInfo, PlayerInfo* newInfo);
    virtual int getSceneType();
    virtual int GetPad();
    int getScoreboardType();
    void SetTitleSlotText(int index, const std::wstring& text);
    void ShowTitleSlot(int index, bool show);

private:
    PlayerUID m_playerUid;
    unsigned char m_playerAvailable;
    unsigned char m_canKickPlayers;
    unsigned char m_kickVoteEnabled;
    unsigned char m_playerStatsValid;
    unsigned char m_scoreboardInitialised;
    unsigned char m_playerSelectionState;
    unsigned char m_playerSelectionPadding;
    IScoreboardLayout* m_scoreboardLayout;
    unsigned char m_scoreboardMode;
    unsigned char m_registeredTextures[55];
    int m_killBarValue;
    unsigned char m_unusedState[16];
    unsigned char m_hasFocusSelection;
    unsigned char m_unusedFocusState[15];
    int m_numPlayers;
    int m_selectedPlayer;
    int m_numColumns;
    unsigned char m_unusedColumnState[16];
    int m_columnCount;
    UIControl m_awardPanel;
    UIControl m_mainPanel;
    UIControl_Progress m_killBar;
    UIControl_Label m_playerTitleLabel;
    UIControl_Label m_stat1Label;
    UIControl_Label m_stat2Label;
    UIControl_Label m_stat3Label;
    UIControl_Label m_stat4Label;
    UIControl_Label m_stat5Label;
    UIControl_Label m_killedByLabel;
    UIControl_Label m_gamertagLabel;
    UIControl_ScoreboardButtonList m_playerList;
};

#endif
