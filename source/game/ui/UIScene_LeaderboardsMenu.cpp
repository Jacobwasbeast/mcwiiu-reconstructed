#include "game/ui/UIScene_LeaderboardsMenu.hpp"

#include <cwchar>
#include <cstring>

#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/L10N.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/UILayer.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/world/LevelGenerationOptions.hpp"

extern unsigned int XGetLanguage();

struct LeaderboardDescriptor
{
    unsigned int statsType;
    unsigned int id;
    unsigned int icon;
};

static std::vector<LeaderboardDescriptor> gLeaderboardDescriptors[4];
static int gLeaderboardIconIds[28];

UIScene_LeaderboardsMenu::UIScene_LeaderboardsMenu(int type, void* data, UILayer* layer)
    : UIScene(type, layer), m_leaderboardInterface(g_pLeaderboardManager), m_leaderboardId(0),
      m_numStats(0), m_leaderboardIndex(0), m_readType(0), m_statsType(0), m_startRank(0),
      m_numRows(0), m_totalRows(0), m_selectedEntry(-1), m_loading(true), m_hasEntries(false),
      m_titleIconsSet(false), m_updatePending(false), m_showInfo(false), m_canRequestMore(false),
      m_initialized(false), m_result(static_cast<unsigned int>(-1))
{
    initialiseMovie();
    m_selectedEntry = -1;
    m_loading = true;
    m_hasEntries = false;
    m_titleIconsSet = false;
    m_updatePending = false;
    m_initialized = true;
    SetLeaderboardHeader();
    m_filterLabel.init(UIString(L10N::GetString(0x5589e932)));
    m_entriesLabel.init(UIString(L10N::GetString(0x254b61c0)));
    ReadStats(-1);
    addTimer(0, 100);
    if (g_pApp->StartInstallDLCProcess(g_pApp->GetPrimaryUserIndex()) == 0)
        g_pApp->getLevelRules()->getRulesByType(LevelRules::eLevelRuleType_Unknown,
                                                  &m_levelRuleGroups, false);
    m_result = static_cast<unsigned int>(-1);
}

UIScene_LeaderboardsMenu::~UIScene_LeaderboardsMenu()
{
    g_pApp->SetLiveLinkRequired(false);
}

unsigned int UIScene_LeaderboardsMenu::GetEntryStartIndex()
{
    if (!m_entries.empty())
        return m_entries.front().rank;
    return 0;
}

unsigned int UIScene_LeaderboardsMenu::LanguageUsesCommaRadix(int language)
{
    return (language > 2 && language < 6) || language == 9 ||
           (language >= 11 && language < 14) || (language > 14 && language < 17) ||
           (language > 240 && language < 246);
}

void UIScene_LeaderboardsMenu::CopyLeaderboardEntry(LeaderboardManager::ReadScore* score,
                                                    int index,
                                                    LeaderboardManager::EStatsType statsType)
{
    if (score == NULL || index < 0)
        return;
    if (static_cast<size_t>(index) >= m_entries.size())
        m_entries.resize(index + 1);
    LeaderboardEntry& entry = m_entries[index];
    entry.uid = score->m_uid;
    entry.gamertag = score->m_wsGamertag;
    entry.score = score->m_uiRating;
    entry.messageStringId = score->m_iMessageStringId;
    entry.rank = score->m_uiRank;
    entry.complete = true;
    entry.selected = false;
    entry.loading = false;
    entry.visible = true;
    entry.stats[0] = score->m_uiRating;
    for (unsigned int i = 0; i < 8; ++i)
        entry.statText[i].clear();
    for (unsigned int i = 0; i < score->m_usNumStats && i < 8; ++i) {
        entry.stats[i] = score->m_auiStats[i];
        if (statsType == LeaderboardManager::eStatsType_Score)
            swprintf(&entry.statText[i][0], 1, L"");
    }
}

void UIScene_LeaderboardsMenu::RetrieveStats()
{
    if (m_result == LeaderboardManager::eStatsReturn_Success)
        m_hasEntries = true;
    m_loading = false;
    m_updatePending = true;
}

bool UIScene_LeaderboardsMenu::OnStatsReadComplete(LeaderboardManager::eStatsReturn result,
                                                   int userIndex,
                                                   LeaderboardManager::ReadView readView)
{
    m_totalRows = userIndex;
    m_startRank = static_cast<int>(readView.m_uiNumScores);
    m_numRows = readView.m_pScores == NULL ? 0 : static_cast<int>(readView.m_uiNumScores);
    m_result = result;
    RetrieveStats();
    return true;
}

void UIScene_LeaderboardsMenu::SetLeaderboardHeader()
{
    m_leaderboardLabel.setLabel(UIString(L10N::GetString(0x5589e932)), false, false);
}

void UIScene_LeaderboardsMenu::ReadStats(int startRank)
{
    m_result = static_cast<unsigned int>(-1);
    if (startRank == -1) {
        m_loading = true;
        m_totalRows = 0;
        m_numRows = 0;
        m_leaderboardList.clearList();
    }
    PlayerUID uid(0);
    if (m_readType == LeaderboardManager::eReadType_Friends)
        m_leaderboardInterface.ReadStats_Friends(this, m_leaderboardId,
            static_cast<LeaderboardManager::EStatsType>(m_statsType), uid, startRank, m_numRows);
    else if (m_readType == LeaderboardManager::eReadType_MyScore)
        m_leaderboardInterface.ReadStats_MyScore(this, m_leaderboardId,
            static_cast<LeaderboardManager::EStatsType>(m_statsType), uid, m_numRows);
    else if (m_readType == LeaderboardManager::eReadType_TopRank)
        m_leaderboardInterface.ReadStats_TopRank(this, m_leaderboardId,
            static_cast<LeaderboardManager::EStatsType>(m_statsType), startRank, m_numRows);
    m_infoLabel.setLabel(UIString(L10N::GetString(0xf14f12e4)), false, false);
    m_infoLabel.setVisible(true);
}

void UIScene_LeaderboardsMenu::staticCtor()
{
    const LeaderboardDescriptor first[] = {
        {4, 0x42511ff4, 0}, {4, 0x5f8f0bf7, 0}, {4, 0x6771baea, 0}, {4, 0xa506abee, 0}
    };
    const LeaderboardDescriptor second[] = {
        {7, 0x40cdc147, 0}, {7, 0x93926fe4, 0}, {7, 0x7e732429, 0}, {7, 0xb203d3dd, 0}
    };
    const LeaderboardDescriptor third[] = {
        {6, 0x0170b0d8, 0}, {6, 0x2a285bbb, 0}, {6, 0x896d8ee6, 0}, {6, 0x17517772, 0}
    };
    const LeaderboardDescriptor fourth[] = {
        {0, 0xffffffff, 0}, {7, 0x56a47b1a, 0}, {7, 0x4e067c37, 0}, {7, 0xbc14f8d3, 0}
    };
    gLeaderboardDescriptors[0].assign(first, first + 4);
    gLeaderboardDescriptors[1].assign(second, second + 4);
    gLeaderboardDescriptors[2].assign(third, third + 4);
    gLeaderboardDescriptors[3].assign(fourth, fourth + 4);
}

unsigned int UIScene_LeaderboardsMenu::mapElementsAndNames()
{
    fuiRenderNode* root = m_movie == NULL ? NULL : m_movie->getRootNode();
    m_leaderboardList.setupControl(this, root, "Gamers");
    m_filterLabel.setupControl(this, root, "Filter");
    m_leaderboardLabel.setupControl(this, root, "Leaderboard");
    m_entriesLabel.setupControl(this, root, "Entries");
    m_infoLabel.setupControl(this, root, "Info");
    m_tabNavigation.setupControl(this, root, "TabNavigation");
    return 1;
}

int UIScene_LeaderboardsMenu::ExitLeaderboards(void* context, int button,
                                                         C4JStorage::EMessageResult result)
{
    static_cast<UIScene*>(context)->navigateBack();
    return 0;
}

void UIScene_LeaderboardsMenu::updateTooltips()
{
    gConsoleUIController.SetTooltips(static_cast<unsigned int>(m_playerId), -1, 0x4f6bcf5e,
        0x4990ea8c, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0);
}

void UIScene_LeaderboardsMenu::updateComponents()
{
    m_layer->showComponent(m_playerId, static_cast<EUIScene>(0x42), !g_pApp->GetGameStarted());
    m_layer->showComponent(m_playerId, static_cast<EUIScene>(0x43), false);
}

void UIScene_LeaderboardsMenu::handleTimerComplete(int timerId)
{
    if (timerId == 0 && g_pApp->DLCInstallPending() == 0)
        gConsoleUIController.RequestErrorMessage(0xf1c0f3d9, 0, NULL, 0,
                                                  m_playerId, ExitLeaderboards, this, 0, true);
}

std::wstring UIScene_LeaderboardsMenu::getMoviePath()
{
    return std::wstring(L"LeaderboardMenu");
}

void UIScene_LeaderboardsMenu::SetLeaderboardTitleIcons()
{
    unsigned int count = m_numStats;
    if (count > 7)
        count = 7;
    for (unsigned int i = 0; i < count; ++i)
        m_leaderboardList.setColumnIcon(static_cast<int>(i), 0);
    m_titleIconsSet = count != 0;
}

void UIScene_LeaderboardsMenu::PopulateLeaderboard(LeaderboardManager::eStatsReturn result)
{
    m_leaderboardList.clearList();
    for (std::vector<LeaderboardEntry>::const_iterator it = m_entries.begin(); it != m_entries.end(); ++it)
        m_leaderboardList.addDataSet(!it->complete, static_cast<int>(it->rank),
            static_cast<int>(it->score), it->gamertag, it->complete, it->statText[0],
            it->statText[1], it->statText[2], it->statText[3], it->statText[4],
            it->statText[5], it->statText[6]);
    SetLeaderboardTitleIcons();
}

void UIScene_LeaderboardsMenu::finishUpdatingLeaderboards()
{
    PopulateLeaderboard(static_cast<LeaderboardManager::eStatsReturn>(m_result));
    updateComponents();
}

void UIScene_LeaderboardsMenu::tick()
{
    UIScene::tick();
    m_leaderboardInterface.tick();
    if (m_result != static_cast<unsigned int>(-1) && g_pApp->DLCInstallPending() == 0) {
        finishUpdatingLeaderboards();
        m_result = static_cast<unsigned int>(-1);
        m_loading = false;
    }
}

void UIScene_LeaderboardsMenu::handleReload()
{
    navigateBack();
}

void UIScene_LeaderboardsMenu::handleInput(int playerId, int action, bool repeat, bool released,
                                           bool pressed, bool& handled)
{
    if (pressed)
        SetLeaderboardHeader();
    handled = false;
}

void UIScene_LeaderboardsMenu::customDraw(const char* name, fuiRect* rect)
{
    int slot = -1;
    std::sscanf(name, "slot_%d", &slot);
    if (slot == -1)
        return;

    int iconId = gLeaderboardIconIds[m_leaderboardIndex * 7 + slot];
    not_null_ptr<ItemInstance> item(new ItemInstance(Item::byId(iconId), 1, 0));
    customDrawSlotControlFui(rect, m_playerId, item, 0.0f, false, false, false);
}

void UIScene_LeaderboardsMenu::handleSelectionChanged(int selection)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_FOCUS);
    m_selectedEntry = selection;
}

void UIScene_LeaderboardsMenu::handleRequestMoreData(int id, bool forward)
{
    if (!m_canRequestMore)
        return;
    unsigned int start = GetEntryStartIndex();
    if (forward)
        ReadStats(static_cast<int>(start + m_numRows + 1));
    else
        ReadStats(static_cast<int>(start > 14 ? start - 14 : 1));
}

int UIScene_LeaderboardsMenu::getSceneType()
{
    return 0x16;
}

bool UIScene_LeaderboardsMenu::hasFocus(int sceneType)
{
    return m_hasFocus;
}
