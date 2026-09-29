#include "game/ui/UIScene_LeaderboardsMenu.hpp"
#include "game/core/StringIds.hpp"

#include <cwchar>
#include <cstring>

#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/L10N.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/UILayer.hpp"
#include "game/item/Item.hpp"
#include "game/item/Items.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/LevelRuleset.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "game/core/XLanguage.hpp"


struct LeaderboardDescriptor
{
    unsigned int numStats;
    unsigned int id;
    unsigned int themeId;
};

static std::vector<LeaderboardDescriptor> gLeaderboardDescriptors[4];
static int gLeaderboardIconIds[28];

UIScene_LeaderboardsMenu::UIScene_LeaderboardsMenu(int type, void* data, UILayer* layer)
    : UIScene(type, layer), m_visibleRowCount(0), m_numStats(0), m_statsType(0), m_readType(0),
      m_leaderboardIndex(0), m_startRank(0), m_numRows(0), m_loadedRowCount(0),
      m_visibleStartRank(-1), m_selectedEntry(-1), m_leaderboardPopulated(false),
      m_statsIconsValid(false), m_leaderboardInterface(g_pLeaderboardManager), m_totalRows(0),
      m_numScores(0), m_pReadScores(NULL), m_loading(true),
      m_result(static_cast<unsigned int>(-1))
{
    initialiseMovie();
    m_selectedEntry = -1;
    m_leaderboardPopulated = false;
    m_statsIconsValid = false;
    m_visibleStartRank = -1;
    m_loading = true;
    g_pApp->SetLiveLinkRequired(true);
    m_leaderboardIndex = 0;
    m_statsType = 0;
    SetLeaderboardHeader();
    m_readType = 0;
    m_filterLabel.init(UIString(L10N::GetString(IDS_LEADERBOARD_FILTER)));
    m_entriesLabel.init(UIString(L10N::GetString(IDS_LEADERBOARD_ENTRIES)));
    ReadStats(-1);
    addTimer(0, 100);
    if (g_pApp->StartInstallDLCProcess(gProfile->GetPrimaryPad()) == 0)
        g_pApp->getLevelRules()->getRulesByType(LevelRules::eLevelRuleType_Unknown,
                                                  &m_levelRuleGroups, false);
    m_result = static_cast<unsigned int>(-1);
}

UIScene_LeaderboardsMenu::~UIScene_LeaderboardsMenu()
{
    g_pApp->SetLiveLinkRequired(false);
}

bool UIScene_LeaderboardsMenu::OnStatsReadComplete(LeaderboardManager::eStatsReturn result,
                                                   int userIndex,
                                                   LeaderboardManager::ReadView readView)
{
    m_totalRows = userIndex;
    m_numScores = readView.m_uiNumScores;
    m_pReadScores = readView.m_pScores;
    m_result = result;
    RetrieveStats();
    return true;
}

unsigned int UIScene_LeaderboardsMenu::GetEntryStartIndex()
{
    if (!m_entries.empty())
        return m_entries.front().rank;
    return 0;
}

unsigned int UIScene_LeaderboardsMenu::LanguageUsesCommaRadix(int language)
{
    switch (language)
    {
    case XC_LANGUAGE_GERMAN:
    case XC_LANGUAGE_FRENCH:
    case XC_LANGUAGE_SPANISH:
    case XC_LANGUAGE_PORTUGUESE:
    case XC_LANGUAGE_POLISH:
    case XC_LANGUAGE_RUSSIAN:
    case XC_LANGUAGE_SWEDISH:
    case XC_LANGUAGE_NORWEGIAN:
    case XC_LANGUAGE_DUTCH:
    case XC_LANGUAGE_FINNISH:
    case XC_LANGUAGE_GREEK:
    case XC_LANGUAGE_DANISH:
    case XC_LANGUAGE_CZECH:
    case XC_LANGUAGE_SLOVAK:
        return 1;
    default:
        return 0;
    }
}

void UIScene_LeaderboardsMenu::CopyLeaderboardEntry(LeaderboardManager::ReadScore* score,
                                                    int index,
                                                    LeaderboardManager::EStatsType statsType)
{
    LeaderboardEntry& entry = m_entries[index];
    entry.uid = score->m_uid;
    entry.gamertag = score->m_wsGamertag;
    entry.score = score->m_uiRating;
    entry.messageStringId = score->m_iMessageStringId;
    if (m_readType != LeaderboardManager::eReadType_Friends)
    {
        unsigned int provisionalRank = entry.score - 1;
        if (index < 1 || m_entries[index - 1].rank < provisionalRank)
            entry.rank = provisionalRank;
        else
            entry.rank = m_entries[index - 1].rank + 1;
    }
    else
    {
        entry.rank = static_cast<unsigned int>(index);
    }
    entry.complete = true;
    entry.selected = false;
    entry.loading = false;
    entry.visible = true;
    for (unsigned int i = 0; i < 7; ++i)
        entry.statText[i].clear();
    unsigned int numStats = score->m_usNumStats;
    if (numStats > 7)
        numStats = 7;
    for (unsigned int i = 0; i < numStats; ++i)
    {
        entry.stats[i] = score->m_auiStats[i];
        wchar_t buf[12];
        if ((i == 0 && statsType == LeaderboardManager::eStatsType_TimePlayed) ||
            (i == 2 && statsType == LeaderboardManager::eStatsType_DistanceTravelled))
        {
            unsigned int value = entry.stats[i];
            unsigned int minutes = value / 60000;
            unsigned int seconds = (value / 1000) % 60;
            unsigned int millis = value % 1000;
            swprintf(buf, 12, L"%i:%i%i.%i%i%i", minutes, seconds / 10, seconds % 10,
                     millis / 100, (millis / 10) % 10, millis % 10);
        }
        else if (statsType == LeaderboardManager::eStatsType_Score)
        {
            unsigned int value = entry.stats[i];
            unsigned int digits = 0;
            unsigned int temp = value;
            while (temp != 0)
            {
                temp /= 10;
                ++digits;
            }
            if (digits <= 3)
                swprintf(buf, 12, L"%um", value);
            else if (digits < 8)
                swprintf(buf, 12, L"%.1fkm", static_cast<float>(value) / 1000.0f);
            else
                swprintf(buf, 12, L"%.0fkm", static_cast<float>(value) / 1000.0f);
        }
        else
        {
            swprintf(buf, 12, L"%u", entry.stats[i]);
        }
        entry.statText[i] = buf;
    }
}

void UIScene_LeaderboardsMenu::RetrieveStats()
{
    if (g_pApp->DebugSettingsOn() &&
        (GameSettings::GetGameSettingsDebugMask(-1, false) & 0x10000) != 0)
    {
        m_entries.clear();
        m_numStats = gLeaderboardDescriptors[m_statsType][m_leaderboardIndex].numStats;
        for (unsigned int rank = 0; rank < 0x65; ++rank)
        {
            LeaderboardEntry entry;
            entry.uid = PlayerUID(0);
            entry.rank = rank;
            entry.score = rank + 1;
            entry.gamertag = L"12345678";
            for (unsigned int c = 0; c < 7; ++c)
            {
                if (m_numStats != 0)
                {
                    if (LanguageUsesCommaRadix(XGetLanguage()) == 0)
                    {
                        entry.statText[c] = L"WWWWWWWWWWWW";
                    }
                    else
                    {
                        wchar_t* buf = new wchar_t[12];
                        swprintf(buf, 12, L"WWWWWWWWWWWW");
                        for (int k = 0; k < 12; ++k)
                            if (buf[k] == L'.')
                                buf[k] = L',';
                        entry.statText[c] = buf;
                        delete[] buf;
                    }
                }
            }
            entry.complete = (rank == 0);
            entry.selected = !entry.complete;
            entry.loading = false;
            entry.visible = false;
            m_entries.push_back(entry);
        }
        m_startRank = 0;
        m_loadedRowCount = 0x65;
        m_visibleRowCount = 0x65;
        return;
    }

    m_loadedRowCount = m_numScores;
    if (m_visibleRowCount == 0)
    {
        m_entries.clear();
        unsigned int newTotal;
        if (m_readType == LeaderboardManager::eReadType_Friends)
            newTotal = m_numScores;
        else
            newTotal = static_cast<unsigned int>(m_totalRows);
        m_visibleRowCount = newTotal;
        if (newTotal == 0 || m_numScores == 0)
            return;
        m_numStats = m_pReadScores[0].m_usNumStats;
        for (unsigned int i = 0; i < m_numScores; ++i)
        {
            m_entries.push_back(LeaderboardEntry());
            CopyLeaderboardEntry(&m_pReadScores[i], static_cast<int>(i),
                                  static_cast<LeaderboardManager::EStatsType>(m_statsType));
        }
        m_startRank = 0;
        m_visibleStartRank = -1;
        m_selectedEntry = -1;
        if (m_readType == LeaderboardManager::eReadType_MyScore)
        {
            unsigned int firstRank = GetEntryStartIndex();
            unsigned int selectedRank = firstRank;
            unsigned int i = firstRank;
            for (;; ++i)
            {
                unsigned int loaded = m_entries.empty() ? 0 : static_cast<unsigned int>(m_entries.size());
                if (firstRank + loaded <= i)
                {
                    selectedRank = GetEntryStartIndex();
                    break;
                }
                if (m_entries[i - firstRank].complete)
                {
                    selectedRank = i;
                    break;
                }
            }
            m_visibleStartRank = static_cast<int>(selectedRank);
            if (static_cast<int>(selectedRank - firstRank) > 9)
                m_visibleStartRank = static_cast<int>(selectedRank - 9);
            m_selectedEntry = static_cast<int>(selectedRank);
        }
        else
        {
            m_visibleStartRank = static_cast<int>(GetEntryStartIndex());
            m_selectedEntry = static_cast<int>(GetEntryStartIndex());
        }
        return;
    }

    unsigned int previousStart = static_cast<unsigned int>(m_startRank);
    unsigned int readCount = m_loadedRowCount;
    unsigned int firstVisible = GetEntryStartIndex();
    if (previousStart < firstVisible && previousStart == 1)
    {
        readCount = firstVisible;
        m_loadedRowCount = firstVisible;
        firstVisible = GetEntryStartIndex();
    }

    bool trimFront = false;
    bool trimBack = false;
    const bool overCapacity = static_cast<unsigned int>(m_entries.size()) + readCount >= 0x65;
    unsigned int insertRank = 0;
    if (previousStart < firstVisible)
    {
        if (overCapacity)
            trimBack = true;
    }
    else
    {
        insertRank = static_cast<unsigned int>(m_entries.size());
        if (overCapacity)
            trimFront = true;
    }
    m_startRank = static_cast<int>(insertRank);

    for (unsigned int i = 0; i < m_loadedRowCount; ++i)
    {
        m_entries.insert(m_entries.begin() + insertRank + i, LeaderboardEntry());
        CopyLeaderboardEntry(&m_pReadScores[i], static_cast<int>(insertRank + i),
                              static_cast<LeaderboardManager::EStatsType>(m_statsType));
    }

    if (trimFront)
    {
        m_entries.erase(m_entries.begin(), m_entries.begin() + 15);
        m_startRank -= m_numRows;
    }
    else if (trimBack)
    {
        m_entries.erase(m_entries.end() - 15, m_entries.end());
    }
}

void UIScene_LeaderboardsMenu::SetLeaderboardHeader()
{
    const LeaderboardDescriptor& descriptor = gLeaderboardDescriptors[m_statsType][m_leaderboardIndex];
    std::wstring label = L10N::GetString(descriptor.id);
    if (m_statsType == LeaderboardManager::eStatsType_TimePlayed ||
        m_statsType == LeaderboardManager::eStatsType_DistanceTravelled)
    {
        unsigned int uniqueId = LevelGenerationOptions::CreateUniqueId(static_cast<int>(descriptor.themeId), 0);
        if (m_levelRuleGroups.empty())
            g_pApp->getLevelRules()->getRulesByType(LevelRules::eLevelRuleType_Unknown,
                                                      &m_levelRuleGroups, false);
        if (!m_levelRuleGroups.empty())
        {
            std::vector<LevelRules::LevelRulesetGroup*>::iterator group = m_levelRuleGroups.begin();
            LevelGenerationOptions* options = (*group)->rulesets.front()->getGenerationOptions();
            unsigned int currentId = options->getUniqueId();
            while (currentId != uniqueId)
            {
                ++group;
                if (group == m_levelRuleGroups.end())
                {
                    m_leaderboardLabel.setLabel(UIString(label), false, false);
                    return;
                }
                options = (*group)->rulesets.front()->getGenerationOptions();
                currentId = options->getUniqueId();
            }
            label = label + L" : " + options->getWorldName();
            m_leaderboardLabel.setLabel(UIString(label), false, false);
            return;
        }
    }
    m_leaderboardLabel.setLabel(UIString(label), false, false);
}

void UIScene_LeaderboardsMenu::ReadStats(int startRank)
{
    m_result = static_cast<unsigned int>(-1);
    if (startRank == -1)
    {
        m_startRank = 1;
        m_numRows = 0xf;
        m_visibleRowCount = 0;
        m_loadedRowCount = 0;
        m_leaderboardList.clearList();
    }
    else
    {
        m_startRank = startRank;
    }

    PlayerUID uid(0);
    if (m_readType == LeaderboardManager::eReadType_Friends)
    {
        m_leaderboardInterface.ReadStats_Friends(
            this, m_leaderboardIndex, static_cast<LeaderboardManager::EStatsType>(m_statsType),
            uid, static_cast<unsigned int>(m_startRank), static_cast<unsigned int>(m_numRows));
    }
    else if (m_readType == LeaderboardManager::eReadType_MyScore && startRank == -1)
    {
        m_leaderboardInterface.ReadStats_MyScore(
            this, m_leaderboardIndex, static_cast<LeaderboardManager::EStatsType>(m_statsType),
            uid, static_cast<unsigned int>(m_numRows));
        m_infoLabel.setLabel(UIString(L10N::GetString(0xf14f12e4)), false, false);
        m_infoLabel.setVisible(true);
        return;
    }
    else
    {
        m_leaderboardInterface.ReadStats_TopRank(
            this, m_leaderboardIndex, static_cast<LeaderboardManager::EStatsType>(m_statsType),
            static_cast<unsigned int>(m_startRank), static_cast<unsigned int>(m_numRows));
        m_infoLabel.setLabel(UIString(L10N::GetString(0xf14f12e4)), false, false);
        m_infoLabel.setVisible(true);
        return;
    }
    m_infoLabel.setLabel(UIString(L10N::GetString(0xf14f12e4)), false, false);
    m_infoLabel.setVisible(true);
}

void UIScene_LeaderboardsMenu::staticCtor()
{
    const LeaderboardDescriptor first[] = {
        {4, 0x42511ff4, 0}, {4, 0x5f8f0bf7, 0}, {4, 0x6771baea, 0}, {4, 0xa506abee, 0}
    };
    const LeaderboardDescriptor second[] = {
        {7, IDS_LEADERBOARD_MINING_BLOCKS_PEACEFUL, 0}, {7, IDS_LEADERBOARD_MINING_BLOCKS_EASY, 0}, {7, IDS_LEADERBOARD_MINING_BLOCKS_NORMAL, 0}, {7, IDS_LEADERBOARD_MINING_BLOCKS_HARD, 0}
    };
    const LeaderboardDescriptor third[] = {
        {6, IDS_LEADERBOARD_FARMING_PEACEFUL, 0}, {6, IDS_LEADERBOARD_FARMING_EASY, 0}, {6, IDS_LEADERBOARD_FARMING_NORMAL, 0}, {6, IDS_LEADERBOARD_FARMING_HARD, 0}
    };
    const LeaderboardDescriptor fourth[] = {
        {0, 0xffffffff, 0}, {7, IDS_LEADERBOARD_KILLS_EASY, 0}, {7, IDS_LEADERBOARD_KILLS_NORMAL, 0}, {7, IDS_LEADERBOARD_KILLS_HARD, 0}
    };
    gLeaderboardDescriptors[0].assign(first, first + 4);
    gLeaderboardDescriptors[1].assign(second, second + 4);
    gLeaderboardDescriptors[2].assign(third, third + 4);
    gLeaderboardDescriptors[3].assign(fourth, fourth + 4);

    gLeaderboardIconIds[0] = 32003;
    gLeaderboardIconIds[1] = 32002;
    gLeaderboardIconIds[2] = Item::getId(Items::minecart);
    gLeaderboardIconIds[3] = Item::getId(Items::boat);
    gLeaderboardIconIds[7] = Block::getId(Blocks::dirt);
    gLeaderboardIconIds[8] = Block::getId(Blocks::cobblestone);
    gLeaderboardIconIds[9] = Block::getId(Blocks::sand);
    gLeaderboardIconIds[10] = Block::getId(Blocks::stone);
    gLeaderboardIconIds[11] = Block::getId(Blocks::gravel);
    gLeaderboardIconIds[12] = Block::getId(Blocks::clay);
    gLeaderboardIconIds[13] = Block::getId(Blocks::obsidian);
    gLeaderboardIconIds[14] = Item::getId(Items::egg);
    gLeaderboardIconIds[15] = Item::getId(Items::wheat);
    gLeaderboardIconIds[16] = Block::getId(Blocks::brown_mushroom);
    gLeaderboardIconIds[17] = Item::getId(Items::reeds);
    gLeaderboardIconIds[18] = Item::getId(Items::milkBucket);
    gLeaderboardIconIds[19] = Block::getId(Blocks::pumpkin);
}

unsigned int UIScene_LeaderboardsMenu::mapElementsAndNames()
{
    fuiRenderNode* root = m_movie->getRootNode();
    m_leaderboardList.setupControl(this, root, "Gamers");
    m_controls.push_back(&m_leaderboardList);
    m_filterLabel.setupControl(this, root, "Filter");
    m_controls.push_back(&m_filterLabel);
    m_leaderboardLabel.setupControl(this, root, "Leaderboard");
    m_controls.push_back(&m_leaderboardLabel);
    m_entriesLabel.setupControl(this, root, "Entries");
    m_controls.push_back(&m_entriesLabel);
    m_infoLabel.setupControl(this, root, "Info");
    m_controls.push_back(&m_infoLabel);
    m_tabNavigation.setupControl(this, root, "TabNavigation");
    m_controls.push_back(&m_tabNavigation);
    return 1;
}

int UIScene_LeaderboardsMenu::ExitLeaderboards(void* context, int pad,
                                                         C4JStorage::EMessageResult result)
{
    static_cast<UIScene*>(context)->navigateBack();
    return 0;
}

void UIScene_LeaderboardsMenu::updateTooltips()
{
    gConsoleUIController.SetTooltips(static_cast<unsigned int>(m_playerId), -1, IDS_TOOLTIPS_BACK,
        IDS_TOOLTIPS_CHANGE_FILTER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0);
}

void UIScene_LeaderboardsMenu::updateComponents()
{
    m_layer->showComponent(m_playerId, EUIScene_Panorama, !g_pApp->GetGameStarted());
    m_layer->showComponent(m_playerId, EUIScene_Logo, false);
}

void UIScene_LeaderboardsMenu::handleTimerComplete(int timerId)
{
    if (timerId == 0)
    {
        unsigned int primaryPad = static_cast<unsigned int>(gProfile->GetPrimaryPad());
        if (!gProfile->IsSignedInLive(static_cast<int>(primaryPad)))
        {
            int messageParameters[1];
            messageParameters[0] = IDS_CONFIRM_OK;
            int messageId = CGameNetworkManager::sInstance.CorrectErrorIDS(0x16cb58e9);
            unsigned int userIndex = static_cast<unsigned int>(gProfile->GetPrimaryPad());
            gConsoleUIController.RequestErrorMessage(IDS_CONNECTION_LOST, messageId, messageParameters, 1,
                                                      userIndex, ExitLeaderboards, this, 0, true);
        }
    }
}

std::wstring UIScene_LeaderboardsMenu::getMoviePath()
{
    return std::wstring(L"LeaderboardMenu");
}

void UIScene_LeaderboardsMenu::SetLeaderboardTitleIcons()
{
    for (int column = 0; column < 7; ++column)
    {
        int icon = gLeaderboardIconIds[m_statsType * 7 + column];
        if (icon != 0)
            m_leaderboardList.setColumnIcon(column, icon);
    }
}

void UIScene_LeaderboardsMenu::PopulateLeaderboard(LeaderboardManager::eStatsReturn result)
{
    SetLeaderboardTitleIcons();
    if (result == LeaderboardManager::eStatsReturn_Success && m_visibleRowCount != 0)
    {
        m_leaderboardList.setupTitles(L10N::GetString(IDS_LEADERBOARD_RANK), L10N::GetString(IDS_LEADERBOARD_GAMERTAG));
        if (!g_pApp->DebugSettingsOn() ||
            (GameSettings::GetGameSettingsDebugMask(-1, false) &
             0x10000) == 0)
        {
            wchar_t buf[40];
            swprintf(buf, 40, L"%ls%i", L10N::GetString(IDS_LEADERBOARD_ENTRIES), m_numRows);
            m_entriesLabel.setLabel(UIString(buf), false, false);
        }
        else
        {
            wchar_t buf[40];
            swprintf(buf, 40, L"%ls12345678", L10N::GetString(IDS_LEADERBOARD_ENTRIES));
            m_entriesLabel.setLabel(UIString(buf), false, false);
        }
        m_infoLabel.setLabel(UIString(L""), false, false);
        m_infoLabel.setVisible(false);

        m_leaderboardList.initLeaderboard(m_selectedEntry, static_cast<int>(m_visibleRowCount),
                                           static_cast<int>(gLeaderboardDescriptors[m_statsType][m_leaderboardIndex].numStats));

        unsigned int start = static_cast<unsigned int>(m_startRank);
        unsigned int count = m_loadedRowCount;
        for (unsigned int i = 0; i < count && (start + i) < m_entries.size(); ++i)
        {
            const LeaderboardEntry& entry = m_entries[start + i];
            bool isLast = (i == count - 1);
            if (entry.messageStringId >= 1)
            {
                std::wstring text = L10N::GetString(entry.messageStringId);
                m_leaderboardList.addDataSet(isLast, static_cast<int>(entry.rank),
                                              static_cast<int>(entry.score), entry.gamertag, true,
                                              text, L"", L"", L"", L"", L"", L"");
            }
            else
            {
                m_leaderboardList.addDataSet(
                    isLast, static_cast<int>(entry.rank), static_cast<int>(entry.score),
                    entry.gamertag, false, entry.statText[0], entry.statText[1], entry.statText[2],
                    entry.statText[3], entry.statText[4], entry.statText[5], entry.statText[6]);
            }
        }
        m_leaderboardPopulated = true;
        return;
    }

    m_leaderboardList.setupTitles(L"", L"");
    wchar_t buf[40];
    swprintf(buf, 40, L"%ls0", L10N::GetString(IDS_LEADERBOARD_ENTRIES));
    m_entriesLabel.setLabel(UIString(buf), false, false);
    if (result == LeaderboardManager::eStatsReturn_NoScores)
    {
        m_infoLabel.setLabel(UIString(L10N::GetString(IDS_ERROR_NETWORK)), false, false);
        m_infoLabel.setVisible(true);
    }
    else
    {
        m_infoLabel.setLabel(UIString(L10N::GetString(IDS_LEADERBOARD_NORESULTS)), false, false);
        m_infoLabel.setVisible(true);
    }
    m_leaderboardPopulated = true;
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
    if (m_result != static_cast<unsigned int>(-1) && g_pApp->DLCInstallPending() == 0)
    {
        finishUpdatingLeaderboards();
        m_result = static_cast<unsigned int>(-1);
        m_loading = false;
    }
}

void UIScene_LeaderboardsMenu::handleReload()
{
    navigateBack();
}

void UIScene_LeaderboardsMenu::handleInput(int playerId, int action, bool repeat, bool pressed,
                                           bool released, bool& handled)
{
    if (m_loading && action != EControllerActions_MenuCancel)
        return;

    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, pressed, released);
    if (!pressed)
        return;

    switch (action)
    {
    case EControllerActions_MenuA:
    case EControllerActions_MenuY:
        handled = true;
        break;
    case EControllerActions_MenuB:
        break;
    case EControllerActions_MenuX:
        if (m_leaderboardPopulated && g_pLeaderboardManager->isIdle())
        {
            gConsoleUIController.PlayUISFX(SoundEvents::UI_SCROLL);
            if (m_readType == LeaderboardManager::eReadType_Friends)
            {
                m_readType = LeaderboardManager::eReadType_MyScore;
                wchar_t buf[40];
                swprintf(buf, 40, L"%ls%ls", L10N::GetString(IDS_LEADERBOARD_FILTER),
                         L10N::GetString(0x81f49d21));
                m_filterLabel.setLabel(UIString(buf), false, false);
            }
            else if (m_readType == LeaderboardManager::eReadType_MyScore)
            {
                m_readType = LeaderboardManager::eReadType_TopRank;
                wchar_t buf[40];
                swprintf(buf, 40, L"%ls%ls", L10N::GetString(IDS_LEADERBOARD_FILTER),
                         L10N::GetString(IDS_LEADERBOARD_FILTER_OVERALL));
                m_filterLabel.setLabel(UIString(buf), false, false);
            }
            else if (m_readType == LeaderboardManager::eReadType_TopRank)
            {
                m_readType = LeaderboardManager::eReadType_Friends;
                wchar_t buf[40];
                swprintf(buf, 40, L"%ls%ls", L10N::GetString(IDS_LEADERBOARD_FILTER),
                         L10N::GetString(IDS_LEADERBOARD_FILTER_FRIENDS));
                m_filterLabel.setLabel(UIString(buf), false, false);
            }
            ReadStats(-1);
            gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
        }
        handled = true;
        break;
    case EControllerActions_MenuUp:
        if (m_selectedEntry > 0)
        {
            --m_selectedEntry;
            sendInputToMovie(action, repeat, pressed, released);
            return;
        }
        m_selectedEntry = 0;
        sendInputToMovie(action, repeat, pressed, released);
        return;
    case EControllerActions_MenuDown:
        if (static_cast<unsigned int>(m_selectedEntry + 1) < m_visibleRowCount)
        {
            ++m_selectedEntry;
        }
        else
        {
            m_selectedEntry = static_cast<int>(m_visibleRowCount) - 1;
        }
        sendInputToMovie(action, repeat, pressed, released);
        return;
    case EControllerActions_MenuRight:
    case EControllerActions_MenuLeft:
        if (m_leaderboardPopulated && g_pLeaderboardManager->isIdle())
        {
            gConsoleUIController.PlayUISFX(SoundEvents::UI_SCROLL);
            m_statsIconsValid = false;
            int newStatsType = m_statsType;
            if (action == EControllerActions_MenuRight)
            {
                ++newStatsType;
                if (newStatsType == 4)
                    newStatsType = 0;
            }
            else
            {
                if (newStatsType == 0)
                    newStatsType = 4;
                --newStatsType;
            }
            m_statsType = newStatsType;
            unsigned int index = m_leaderboardIndex;
            if (index >= gLeaderboardDescriptors[m_statsType].size())
                index = 0;
            if (m_statsType == 3 && index == 0)
                index = 1;
            m_leaderboardIndex = index;
            SetLeaderboardHeader();
            ReadStats(-1);
            gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
            handled = true;
            return;
        }
        handled = true;
        break;
    case EControllerActions_MenuPageUp:
    case EControllerActions_MenuPageDown:
        if (m_leaderboardPopulated && g_pLeaderboardManager->isIdle())
        {
            gConsoleUIController.PlayUISFX(SoundEvents::UI_SCROLL);
            if (m_visibleRowCount < 0xb)
                return;
            sendInputToMovie(action, repeat, pressed, released);
            handled = true;
            return;
        }
        handled = true;
        break;
    case EControllerActions_MenuRightScroll:
    case EControllerActions_MenuLeftScroll:
        if (m_leaderboardPopulated && g_pLeaderboardManager->isIdle())
        {
            unsigned int previousIndex = m_leaderboardIndex;
            gConsoleUIController.PlayUISFX(SoundEvents::UI_SCROLL);
            unsigned int listSize = static_cast<unsigned int>(gLeaderboardDescriptors[m_statsType].size());
            unsigned int newIndex = m_leaderboardIndex;
            if (action == EControllerActions_MenuRightScroll)
            {
                ++newIndex;
                if (newIndex == listSize)
                    newIndex = 0;
                if (m_statsType == 3 && newIndex == 0)
                    newIndex = 1;
            }
            else
            {
                if (newIndex == 0)
                    newIndex = listSize;
                --newIndex;
                if (m_statsType == 3 && newIndex == 0)
                    newIndex = 3;
            }
            m_leaderboardIndex = newIndex;
            if (previousIndex != newIndex)
            {
                m_tabNavigation.setVisible(true);
                SetLeaderboardHeader();
                ReadStats(-1);
                gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
                handled = true;
                return;
            }
            m_tabNavigation.setVisible(false);
            handled = true;
            return;
        }
        handled = true;
        break;
    case EControllerActions_MenuCancel:
        navigateBack();
        handled = true;
        return;
    default:
        break;
    }
}

void UIScene_LeaderboardsMenu::customDraw(const char* name, fuiRect* rect)
{
    int slot = -1;
    std::sscanf(name, "slot_%d", &slot);
    if (slot == -1)
        return;

    int iconId = gLeaderboardIconIds[m_statsType * 7 + slot];
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
    if (m_visibleRowCount == 0)
        return;

    unsigned int start = GetEntryStartIndex();
    if (static_cast<unsigned int>(id + 1) < start)
    {
        if (g_pLeaderboardManager->isIdle())
        {
            int newStart = static_cast<int>(GetEntryStartIndex()) - 14;
            if (newStart < 1)
                newStart = 1;
            ReadStats(newStart);
        }
    }
    else
    {
        unsigned int loaded = static_cast<unsigned int>(m_entries.size());
        if (start + loaded <= static_cast<unsigned int>(id + 1) && g_pLeaderboardManager->isIdle())
        {
            loaded = static_cast<unsigned int>(m_entries.size());
            ReadStats(static_cast<int>(GetEntryStartIndex() + loaded + 1));
        }
    }
}

int UIScene_LeaderboardsMenu::getSceneType()
{
    return EUIScene_LeaderboardsMenu;
}

bool UIScene_LeaderboardsMenu::hasFocus(int sceneType)
{
    return m_hasFocus;
}
