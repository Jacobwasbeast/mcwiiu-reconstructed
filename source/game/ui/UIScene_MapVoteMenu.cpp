#include "game/ui/UIScene_MapVoteMenu.hpp"

#include <cwchar>

#include "game/core/CMinecraftApp.hpp"
#include "game/core/ClientMasterGameMode.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/HtmlString.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/world/LevelGenerationOptions.hpp"

extern ConsoleUIController gConsoleUIController;

UIScene_MapVoteMenu::UIScene_MapVoteMenu(int type, void*, UILayer* layer)
    : UIScene(type, layer), m_splitScreen(type == 1), m_votableType(0), m_mapIds(),
      m_levelRuleGroups(), m_defaultTexture(L"MapSelectMenu"), m_selectedVote(-1),
      m_previousSelection(-1), m_updateVoteInfo(false), m_mapList(), m_mapListTitle(),
      m_mapTitle(), m_mapDescription(), m_mapIcon()
{
    initialiseMovie();
    m_votableType = Minecraft::GetInstance()->GetFirstMiniGame()->VotableType();
    Init();
}

UIScene_MapVoteMenu::~UIScene_MapVoteMenu()
{
}

unsigned int UIScene_MapVoteMenu::mapElementsAndNames()
{
    UIScene::mapElementsAndNames();
    fuiRenderNode* root = m_movie->getRootNode();
    m_mapList.setupControl(this, root, "MapList");
    m_mapListTitle.setupControl(this, root, "MapListTitle");
    m_mapTitle.setupControl(this, root, "MapTitle");
    m_mapDescription.setupControl(this, root, "HTMLMapText");
    m_mapIcon.setupControl(this, root, "MapPic");
    m_controls.push_back(&m_mapList);
    m_controls.push_back(&m_mapListTitle);
    m_controls.push_back(&m_mapTitle);
    m_controls.push_back(&m_mapDescription);
    m_controls.push_back(&m_mapIcon);
    return 1;
}

void UIScene_MapVoteMenu::InitForMap()
{
    m_mapList.init(0);
    m_mapListTitle.init(UIString(0x0f7ae41a));
    ClientMasterGameMode* mode = Minecraft::GetInstance()->GetClientMasterGameMode();
    m_mapIds.clear();
    LevelRules::ELevelRuleType type = (LevelRules::ELevelRuleType)Minecraft::GetMiniGameId();
    if (Minecraft::GetMiniGameId() == 0 || Minecraft::GetMiniGameId() > 4)
        type = (LevelRules::ELevelRuleType)0;
    g_pApp->getLevelRules()->getRulesByType(type, &m_levelRuleGroups, false);
    m_mapTitle.init(UIString(L10N::GetString(m_splitScreen ? 0xd5923bd8 : 0xbeee0712)));
    m_mapDescription.init(UIString(L"MapSelect"));
    std::vector<ClientMasterGameMode::MapSelectInfo> playlist;
    mode->GetUniquePlaylist(playlist);
    for (unsigned int i = 0; i < playlist.size(); ++i)
    {
        std::wstring texture = MiniGameDef::GetVariantIconPath(Minecraft::GetMiniGameId(), i);
        LevelGenerationOptions* options = g_pApp->getLevelRules()->getLevelGenFromUniqueId(type, playlist[i].uniqueId);
        std::wstring name = options != 0 ? options->getWorldName() : L"";
        if (name.empty())
            name = playlist[i].name;
        int size = LevelGenerationOptions::GetMapSizeFromUniqueId(playlist[i].uniqueId);
        if (g_pApp->DebugArtToolsOn() && size == 2)
            name += L" Huge";
        else if (g_pApp->DebugArtToolsOn() && size == 0)
            name += L" Small";
        bool checked = mode->GetMapVote(m_resolution, playlist[i].uniqueId) != 0;
        if (checked)
            m_selectedVote = (int)i;
        m_mapList.AddNewItem(name, (int)i, checked, texture);
        m_mapIds.push_back(playlist[i].uniqueId);
    }
}

void UIScene_MapVoteMenu::InitForVariant()
{
    m_mapList.init(0);
    m_mapListTitle.init(UIString(0x66f20471));
    Minecraft* minecraft = Minecraft::GetInstance();
    ClientMasterGameMode* mode = minecraft->GetClientMasterGameMode();
    MiniGameDef* definition = MiniGameDef::GetCustomGameModeById((EMiniGameId)mode->GetNextGameMode(), true);
    m_mapTitle.init(UIString(L10N::GetString(m_splitScreen ? 0xd5923bd8 : 0xbeee0712)));
    m_mapDescription.init(UIString(L"MapSelect"));
    int count = (int)definition->GetGameVariantCount() - 2;
    if (definition->IsCustomGame())
        count = definition->GetLootSetCount();
    for (int i = 0; i < count; ++i)
    {
        std::wstring icon = MiniGameDef::GetVariantIconPath((EMiniGameId)mode->GetNextGameMode(), i);
        std::wstring label = definition->GetLootSetText(i);
        bool checked = mode->GetMapVote(m_resolution, i) != 0;
        if (checked)
            m_selectedVote = i;
        m_mapList.AddNewItem(label, i, checked, icon);
        m_mapIds.push_back(i);
    }
}

void UIScene_MapVoteMenu::InitForTheme()
{
    m_mapList.init(0);
    m_mapListTitle.init(UIString(0x66f20471));
    ClientMasterGameMode* mode = Minecraft::GetInstance()->GetClientMasterGameMode();
    m_mapTitle.init(UIString(L10N::GetString(m_splitScreen ? 0xd5923bd8 : 0xbeee0712)));
    m_mapDescription.init(UIString(L"MapSel"));
    for (unsigned int i = 0; i < 1; ++i)
    {
        std::wstring icon = MiniGameDef::GetVariantIconPath((EMiniGameId)mode->GetNextGameMode(), i);
        std::wstring label = L"";
        bool checked = mode->GetMapVote(m_resolution, i) != 0;
        if (checked)
            m_selectedVote = (int)i;
        m_mapList.AddNewItem(label, (int)i, checked, icon);
        m_mapIds.push_back((int)i);
    }
}

void UIScene_MapVoteMenu::Init()
{
    m_mapIds.clear();
    m_levelRuleGroups.clear();
    m_selectedVote = -1;
    if (m_votableType == 0)
        InitForMap();
    else if (m_votableType == 1)
        InitForVariant();
    else if (m_votableType == 2)
        InitForTheme();
}

void UIScene_MapVoteMenu::updateTooltips()
{
    gConsoleUIController.SetTooltips((unsigned int)m_resolution, 0x8d6323ed, 0x4f6bcf5e,
                                     -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                     -1, -1, false, false);
}

void UIScene_MapVoteMenu::updateComponents()
{
    m_layer->showComponent(m_resolution, (EUIScene)0x43, false);
}

void UIScene_MapVoteMenu::handleReload()
{
    Init();
}

void UIScene_MapVoteMenu::UpdateVotableInfo()
{
    int selection = m_mapList.getCurrentSelection();
    if (selection < 0 || selection >= (int)m_mapIds.size())
        return;
    int id = m_mapIds[selection];
    if (m_votableType == 0)
    {
        LevelRules::ELevelRuleType type = (LevelRules::ELevelRuleType)Minecraft::GetMiniGameId();
        LevelGenerationOptions* options = g_pApp->getLevelRules()->getLevelGenFromUniqueId(type, id);
        if (options == 0)
        {
            m_mapIcon.setTextureName(L"MapSel", true);
            if (m_splitScreen)
            {
                m_mapTitle.setLabel(UIString(L"MapSel"), false, false);
                m_mapDescription.setLabel(UIString(L"MapSel"), false, false);
            }
            return;
        }
        if (m_splitScreen)
        {
            m_mapTitle.setLabel(UIString(options->getWorldName()), false, false);
            m_mapDescription.setLabel(UIString(options->getWorldDescription()), false, false);
        }
        std::wstring image = options->getWorldPreviewImage();
        arrayWithLength<unsigned char> file = g_pApp->getArchiveFile(image, true);
        if (file.data != 0 && file.length != 0)
            registerSubstitutionTexture(image, file.data, file.length, true);
        else
            m_mapIcon.setTextureName(L"MapSel", true);
    }
    else if (m_votableType == 1 && m_splitScreen)
    {
        MiniGameDef* definition = MiniGameDef::GetCustomGameModeById(
            (EMiniGameId)Minecraft::GetInstance()->GetClientMasterGameMode()->GetNextGameMode(), true);
        m_mapTitle.setLabel(UIString(definition->GetLootSetText(selection)), false, false);
        m_mapDescription.setLabel(UIString(definition->GetLootSetDescription(selection)), false, false);
    }
}

void UIScene_MapVoteMenu::tick()
{
    if (m_previousSelection >= 0)
    {
        m_mapList.SetCheckbox(m_previousSelection, false);
        m_previousSelection = -1;
    }
    if (!m_splitScreen)
        UpdateVotableInfo();
    if (m_updateVoteInfo)
    {
        m_updateVoteInfo = false;
        UpdateVotableInfo();
    }
    UIScene::tick();
}

std::wstring UIScene_MapVoteMenu::getMoviePath()
{
    return g_pApp->GetLocalPlayerCount() < 2 ? L"MapSelectMenu" : L"MapSelectMenuSplit";
}

void UIScene_MapVoteMenu::handleInput(int playerId, int action, bool repeat, bool released, bool pressed,
                                      bool& handled)
{
    gConsoleUIController.AnimateKeyPress(playerId, action, repeat, released, pressed);
    if (action < 4 || (action > 7 && action < 14))
        return;
    if (action > 15 && action != 20 && action != 21)
        return;
    if (action == 21)
    {
        if (!released)
            return;
        navigateBack();
        handled = true;
        return;
    }
    sendInputToMovie(action, repeat, released, pressed);
}

void UIScene_MapVoteMenu::handlePress(int, int selection)
{
    gConsoleUIController.PlayUISFX(NULL);
    if (selection < 0 || selection >= (int)m_mapIds.size())
        return;
    if (m_selectedVote == selection)
    {
        m_selectedVote = -1;
        Minecraft::GetInstance()->GetClientMasterGameMode()->SetMapVote(m_resolution, m_mapIds[selection], true);
        return;
    }
    m_previousSelection = m_selectedVote;
    m_selectedVote = selection;
    ClientMasterGameMode* mode = Minecraft::GetInstance()->GetClientMasterGameMode();
    if (m_votableType == 2)
        mode->SetBuildOffThemeVote(m_resolution, m_mapIds[selection], 1);
    else
        mode->SetMapVote(m_resolution, m_mapIds[selection], false);
}

void UIScene_MapVoteMenu::handleFocusChange(int, int childFocus)
{
    m_mapList.updateChildFocus(childFocus);
    m_updateVoteInfo = true;
}

int UIScene_MapVoteMenu::getSceneType()
{
    return 0x3e;
}
