#include "game/ui/UIScene_MapVoteMenu.hpp"
#include "game/core/StringIds.hpp"

#include <cwchar>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ClientMasterGameMode.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/DLCFile.hpp"
#include "game/core/DLCManager.hpp"
#include "game/core/DLCPack.hpp"
#include "game/core/HtmlString.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/core/GameSettings.hpp"

extern ConsoleUIController gConsoleUIController;

UIScene_MapVoteMenu::UIScene_MapVoteMenu(int type, void*, UILayer* layer)
    : UIScene(type, layer), m_splitScreen(type == 1), m_votableType(0), m_mapIds(),
      m_levelRuleGroups(), m_defaultTexture(L"MapSelectMenu"), m_selectedVote(-1),
      m_previousSelection(-1), m_updateVoteInfo(false), m_voteTallyText(), m_mapList(), m_mapListTitle(),
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
    m_mapListTitle.init(UIString(IDS_MG_SELECT_MAP));
    ClientMasterGameMode* mode = Minecraft::GetInstance()->GetClientMasterGameMode();
    m_mapIds.clear();
    LevelRules::ELevelRuleType type = LevelRules::GetRuleTypeForMiniGame(Minecraft::GetMiniGameId());
    g_pApp->getLevelRules()->getRulesByType(type, &m_levelRuleGroups, false);
    m_mapTitle.init(UIString(L10N::GetString(m_splitScreen ? IDS_MG_MAP_DESC : IDS_MG_MAP_VOTES)));
    m_mapDescription.init(UIString(L""));
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
            name += L" (Huge)";
        else if (g_pApp->DebugArtToolsOn() && size == 0)
            name += L" (Small)";
        bool checked = mode->GetMapVote(m_playerId, playlist[i].uniqueId) != 0;
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
    MiniGameDef* definition = MiniGameDef::GetCustomGameModeById(mode->GetNextGameMode(), true);
    m_mapTitle.init(UIString(L10N::GetString(m_splitScreen ? 0xd5923bd8 : 0xbeee0712)));
    m_mapDescription.init(UIString(L"MapSelect"));
    int count = (int)definition->GetGameVariantCount() - 2;
    if (definition->IsCustomGame())
        count = definition->GetLootSetCount();
    for (int i = 0; i < count; ++i)
    {
        std::wstring icon = MiniGameDef::GetVariantIconPath(mode->GetNextGameMode(), i);
        if (g_pApp->hasArchiveFile(icon, false))
        {
            arrayWithLength<unsigned char> file = g_pApp->getArchiveFile(icon, false);
            if (file.data != 0 && file.length != 0)
                registerSubstitutionTexture(icon, file.data, file.length, true);
        }
        std::wstring label = definition->GetLootSetText(i);
        bool checked = mode->GetMapVote(m_playerId, i) != 0;
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
        std::wstring icon = MiniGameDef::GetVariantIconPath(mode->GetNextGameMode(), i);
        if (g_pApp->hasArchiveFile(icon, false))
        {
            arrayWithLength<unsigned char> file = g_pApp->getArchiveFile(icon, false);
            if (file.data != 0 && file.length != 0)
                registerSubstitutionTexture(icon, file.data, file.length, true);
        }
        std::wstring label = L"";
        bool checked = mode->GetMapVote(m_playerId, i) != 0;
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
    gConsoleUIController.SetTooltips((unsigned int)m_playerId, IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK,
                                     -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                     -1, -1, false, false);
}

void UIScene_MapVoteMenu::updateComponents()
{
    m_layer->showComponent(m_playerId, EUIScene_Logo, false);
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
        LevelRules::ELevelRuleType type = LevelRules::GetRuleTypeForMiniGame(Minecraft::GetMiniGameId());
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
        DLCFile* file = options->getPack()->getFile(DLCManager::eDLCType_Texture, image, true, true);
        if (file == 0)
        {
            m_mapIcon.setTextureName(L"MapSel", true);
            return;
        }
        unsigned int imageSize = 0;
        unsigned char* imageData = file->getData(imageSize);
        if (imageData == 0 || imageSize == 0)
        {
            m_mapIcon.setTextureName(L"MapSel", true);
            return;
        }
        wchar_t textureName[64];
        std::swprintf(textureName, 64, L"mapSelect_%08x", options->getUniqueId());
        registerSubstitutionTexture(std::wstring(textureName), imageData, imageSize, false);
        m_mapIcon.setTextureName(std::wstring(textureName), true);
    }
    else if (m_votableType == 1)
    {
        MiniGameDef* definition = MiniGameDef::GetCustomGameModeById(
            (EMiniGameId)Minecraft::GetInstance()->GetClientMasterGameMode()->GetNextGameMode(), true);
        if (m_splitScreen)
        {
            m_mapTitle.setLabel(UIString(definition->GetLootSetText(selection)), false, false);
            m_mapDescription.setLabel(UIString(definition->GetLootSetDescription(selection)), false, false);
            if (selection == 0)
                m_mapDescription.setLabel(UIString(L10N::GetString(0xea1ca730)), false, false);
            else if (selection == 1)
                m_mapDescription.setLabel(UIString(L10N::GetString(0x28850ed6)), false, false);
        }
    }
    else
    {
        m_mapIcon.setTextureName(L"MapSel", true);
        if (m_splitScreen)
        {
            m_mapTitle.setLabel(UIString(L"MapSel"), false, false);
            m_mapDescription.setLabel(UIString(L"MapSel"), false, false);
        }
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
    {
        std::vector<HtmlString> lines;
        ClientMasterGameMode* mode = Minecraft::GetInstance()->GetClientMasterGameMode();
        if (m_votableType == 0)
            mode->GetCurrentMapVotesText(lines);
        else if (m_votableType == 1)
            mode->GetCurrentVariantVotesText(lines);
        else if (m_votableType == 2)
            mode->GetCurrentThemeVotesText(lines);
        std::wstring text = HtmlString::Compose(&lines);
        if (text.compare(m_voteTallyText) != 0)
        {
            m_mapDescription.setLabel(UIString(text), false, false);
            m_voteTallyText = text;
        }
    }
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
    switch (action)
    {
    case EControllerActions_MenuUp:
    case EControllerActions_MenuDown:
    case EControllerActions_MenuRight:
    case EControllerActions_MenuLeft:
    case EControllerActions_MenuOtherStickUp:
    case EControllerActions_MenuOtherStickDown:
    case EControllerActions_MenuOk:
        sendInputToMovie(action, repeat, released, pressed);
        break;
    case EControllerActions_MenuCancel:
        if (released)
        {
            navigateBack();
            handled = true;
        }
        break;
    }
}

void UIScene_MapVoteMenu::handlePress(int, int childId)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
    if (childId < 0 || childId >= (int)m_mapIds.size())
        return;
    if (m_selectedVote == childId)
    {
        m_selectedVote = -1;
        Minecraft::GetInstance()->GetClientMasterGameMode()->SetMapVote(m_playerId, m_mapIds[childId], true);
        return;
    }
    m_previousSelection = m_selectedVote;
    m_selectedVote = childId;
    ClientMasterGameMode* mode = Minecraft::GetInstance()->GetClientMasterGameMode();
    if (m_votableType == 2)
        mode->SetBuildOffThemeVote(m_playerId, m_mapIds[childId], 1);
    else
        mode->SetMapVote(m_playerId, m_mapIds[childId], false);
}

void UIScene_MapVoteMenu::handleFocusChange(int, int childFocus)
{
    m_mapList.updateChildFocus(childFocus);
    m_updateVoteInfo = true;
}

int UIScene_MapVoteMenu::getSceneType()
{
    return EUIScene_MapVoteMenu;
}
