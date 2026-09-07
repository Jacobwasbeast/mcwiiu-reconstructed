#ifndef GAME_UI_UISCENE_LEADERBOARDSMENU_HPP
#define GAME_UI_UISCENE_LEADERBOARDSMENU_HPP

#include <string>
#include <vector>

#include "game/core/LeaderboardInterface.hpp"
#include "game/network/LeaderboardReadListener.hpp"
#include "game/ui/UIControl.hpp"
#include "game/ui/UIControl_Label.hpp"
#include "game/ui/UIControl_LeaderboardList.hpp"
#include "game/ui/UIScene.hpp"
#include "game/world/generation/PlayerUID.hpp"
#include "game/world/LevelRules.hpp"
#include "platform/wiiu/C4JStorage.hpp"

class UILayer;

class UIScene_LeaderboardsMenu : public UIScene, public LeaderboardReadListener
{
public:
    struct LeaderboardEntry
    {
        LeaderboardEntry() : uid(0), rank(0), score(0), complete(false), selected(false), loading(false), visible(false), messageStringId(0) {}
        PlayerUID uid;
        unsigned int rank;
        unsigned int score;
        std::wstring gamertag;
        unsigned int stats[8];
        std::wstring statText[8];
        bool complete;
        bool selected;
        bool loading;
        bool visible;
        int messageStringId;
    };

    UIScene_LeaderboardsMenu(int type, void* data, UILayer* layer);
    virtual ~UIScene_LeaderboardsMenu();
    virtual bool OnStatsReadComplete(LeaderboardManager::eStatsReturn result, int userIndex,
                                     LeaderboardManager::ReadView readView);
    virtual std::wstring getMoviePath();
    virtual unsigned int mapElementsAndNames();
    virtual int getSceneType();
    virtual void tick();
    virtual void handleTimerComplete(int timerId);
    virtual bool hasFocus(int sceneType);
    virtual void updateTooltips();
    virtual void updateComponents();
    virtual void customDraw(const char* name, fuiRect* rect);
    virtual void handleInput(int playerId, int action, bool repeat, bool released, bool pressed,
                             bool& handled);
    virtual void handleReload();
    virtual void handleSelectionChanged(int selection);
    virtual void handleRequestMoreData(int id, bool forward);

    unsigned int GetEntryStartIndex();
    unsigned int LanguageUsesCommaRadix(int language);
    void CopyLeaderboardEntry(LeaderboardManager::ReadScore* score, int index,
                              LeaderboardManager::EStatsType statsType);
    void RetrieveStats();
    void SetLeaderboardHeader();
    void ReadStats(int startRank);
    void SetLeaderboardTitleIcons();
    void PopulateLeaderboard(LeaderboardManager::eStatsReturn result);
    void finishUpdatingLeaderboards();
    static void staticCtor();
    static int ExitLeaderboards(void* context, int button, C4JStorage::EMessageResult result);

private:
    unsigned int m_unknownC8;
    unsigned int m_leaderboardId;
    std::vector<LeaderboardEntry> m_entries;
    LeaderboardInterface m_leaderboardInterface;
    UIControl_LeaderboardList m_leaderboardList;
    UIControl_Label m_filterLabel;
    UIControl_Label m_leaderboardLabel;
    UIControl_Label m_entriesLabel;
    UIControl_Label m_infoLabel;
    UIControl m_tabNavigation;
    std::vector<LevelRules::LevelRulesetGroup*> m_levelRuleGroups;
    unsigned int m_numStats;
    unsigned int m_leaderboardIndex;
    int m_readType;
    int m_statsType;
    int m_startRank;
    int m_numRows;
    int m_totalRows;
    int m_selectedEntry;
    bool m_loading;
    bool m_hasEntries;
    bool m_titleIconsSet;
    bool m_updatePending;
    bool m_showInfo;
    bool m_canRequestMore;
    bool m_initialized;
    unsigned int m_result;
};

#endif
