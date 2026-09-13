#include "game/ui/UIScene_LaunchMoreOptionsMenu.hpp"

#include <cwchar>

#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/GameModeOptions.hpp"
#include "game/core/L10N.hpp"
#include "game/core/MasterGameMode.hpp"
#include "game/core/StringUtils.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/ui/FJ_LaunchMoreOptionsMenu.hpp"
#include "game/ui/LaunchMoreOptionsMenuInitData.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/SuperflatMenu.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/world/generation/FlatGeneratorInfo.hpp"
#include "game/client/render/Renderer.hpp"
#include "platform/wiiu/CInput.hpp"
#include "platform/wiiu/CProfile.hpp"

extern ConsoleUIController gConsoleUIController;
extern CMinecraftApp* g_pApp;

UIScene_LaunchMoreOptionsMenu::UIScene_LaunchMoreOptionsMenu(int sceneType, void* data, UILayer* layer)
    : UIScene(sceneType, layer), m_worldSize(), m_worldType(), m_gameMode(), m_lootSet(),
      m_foodBalance(), m_mapSize(), m_roundLength(), m_worldTab(), m_gameTab(), m_worldOptions(),
      m_gameOptions(), m_worldBackground(), m_gameBackground(), m_worldOptionsList(),
      m_gameOptionsList(), m_worldLabel(), m_gameLabel(), m_resizeLabel(), m_worldDescription(),
      m_gameDescription(), m_worldTabLabel(), m_gameTabLabel(), m_textPanelLabel(), m_optionsLabel(),
      m_initData(static_cast<LaunchMoreOptionsMenuInitData*>(data)), m_online(false),
      m_keyboardActive(false), m_menuType(0), m_requestKeyboard(false), m_updateCheats(false),
      m_updateOnline(false), m_seed()
{
    initialiseMovie();
    m_online = gProfile->IsSignedInLive(m_initData->mValue18) &&
               gProfile->AllowedToPlayMultiplayer(m_initData->mValue18);
    m_menuType = static_cast<int>(m_initData->mFlag3);
    _init();
    addTimer(0, 100);
}

UIScene_LaunchMoreOptionsMenu::~UIScene_LaunchMoreOptionsMenu()
{
}

void UIScene_LaunchMoreOptionsMenu::EnableCustomOptions(bool enabled)
{
    int miniGameId = m_initData->mValue5C;
    if (miniGameId != 1 && miniGameId != 2)
        return;
    if (m_initData->mFlag56)
    {
        m_gameOptionsList.EnableItem(0x22, enabled, true);
        m_gameOptionsList.EnableItem(0x1e, enabled, true);
        miniGameId = m_initData->mValue5C;
    }
    if (miniGameId == 1)
    {
        if (m_initData->mFlag57 || !m_initData->mFlag56)
            return;
        m_gameOptionsList.EnableItem(0x1f, enabled, true);
        m_gameOptionsList.EnableItem(0x20, enabled, true);
        m_gameOptionsList.EnableItem(0x21, enabled, true);
        m_gameOptionsList.EnableItem(0x23, enabled, true);
        m_gameOptionsList.EnableItem(0x24, enabled, true);
        m_gameOptionsList.EnableItem(0x25, enabled, true);
        m_gameOptionsList.EnableItem(0x27, enabled, true);
        m_gameOptionsList.EnableItem(0x28, enabled, true);
        m_gameOptionsList.EnableItem(0x29, enabled, true);
        unsigned int noSmallSkins = m_initData->mPlayerSettings.Get(eCustomGameModeHostOption_NoSmallSkins);
        m_gameOptionsList.SetCheckboxValue(0x1f, noSmallSkins == 0, true);
        m_gameOptionsList.SetCheckboxValue(0x28, m_initData->mPlayerSettings.mTakeEverythingEnabled, true);
        m_gameOptionsList.SetCheckboxValue(0x29, m_initData->mPlayerSettings.mChestRefillEnabled, true);
    }
    else if (miniGameId == 2 && !m_initData->mFlag57 && m_initData->mFlag56)
    {
        m_gameOptionsList.EnableItem(0x20, enabled, true);
        m_gameOptionsList.EnableItem(0x2b, enabled, true);
        m_gameOptionsList.EnableItem(0x2c, enabled, true);
        m_gameOptionsList.EnableItem(0x2a, enabled, true);
    }
}

void UIScene_LaunchMoreOptionsMenu::UpdateCheats()
{
    bool cheatsEnabled = m_initData->mFlagC;
    if (!m_initData->mFlag56 && !m_initData->mFlag57)
    {
        m_gameOptionsList.EnableItem(0xc, cheatsEnabled, true);
        m_gameOptionsList.EnableItem(0xd, cheatsEnabled, true);
        m_gameOptionsList.EnableItem(0xf, cheatsEnabled, true);
        m_gameOptionsList.EnableItem(0x12, cheatsEnabled, true);
        m_gameOptionsList.EnableItem(0x13, cheatsEnabled, true);
    }
    if (!cheatsEnabled)
    {
        m_initData->mFlag11 = true;
        m_initData->mFlagF = true;
        m_initData->mFlag10 = false;
        m_initData->mFlag15 = true;
        m_initData->mFlag16 = true;
        if (!m_initData->mFlag56 && !m_initData->mFlag57)
        {
            m_gameOptionsList.SetCheckboxValue(0xc, false, true);
            m_gameOptionsList.SetCheckboxValue(0xd, m_initData->mFlag11, true);
            m_gameOptionsList.SetCheckboxValue(0xf, m_initData->mFlagF, true);
            m_gameOptionsList.SetCheckboxValue(0x12, m_initData->mFlag15, true);
            m_gameOptionsList.SetCheckboxValue(0x13, m_initData->mFlag16, true);
        }
    }
}

void UIScene_LaunchMoreOptionsMenu::UpdateDescriptionText()
{
    if (m_focusedItemId == -1)
        return;

    static const unsigned int descriptionIds[] = {
        0x1a93e279, 0x0f7151d3, 0x7b994768, 0x2df040b0, 0x2bdbddba, 0xf2e6dbcb,
        0xc85a085b, 0x8f41adb5, 0xa112a17c, 0xcb6d8de1, 0x30f1cb60, 0xc61e984d,
        0xe0163d88, 0xcf045afa, 0x03d32989, 0x544ad58e, 0xaf1fca77, 0x33b6226b,
        0x80e6ea53, 0x2bd798a9, 0xc36b9de0, 0x6d6eba08, 0xa6874a79, 0x8f88ae9a,
        0xffe92161, 0x814065bc, 0x4f8bedac, 0x75cf2428, 0x4c17ef92, 0x18a5cddd,
        0xfe516d5e, 0xbe68971f, 0, 0, 0x42d4ae0f, 0x5e1bb988, 0x2a5c55c8,
        0xb66f0561, 0xf7c16b2c, 0xe7ba74fc, 0xe8f6de23, 0xbfd9bf09, 0xa541f7ca,
        0x0d7dae19, 0x35ef3122
    };
    const int index = m_focusedItemId;
    if (index < 0 || index >= static_cast<int>(sizeof(descriptionIds) / sizeof(descriptionIds[0])))
        return;
    const unsigned int descriptionId = descriptionIds[index];
    std::wstring description;
    if (descriptionId != static_cast<unsigned int>(-1) && descriptionId != static_cast<unsigned int>(-2))
        description = g_pApp->FormatHTMLString(m_playerId, L10N::GetString(static_cast<int>(descriptionId)), -1);
    if (descriptionId == 0xc36b9de0u)
        description = replaceAll(description, L"{*DISABLES_ACHIEVEMENTS*}", L10N::GetString(0xbf244113));
    const bool hiDef = gRenderer->IsHiDef();
    if (!hiDef)
        gRenderer->IsWidescreen();
    if (index == 0x15)
    {
        if (!m_initData->mFlag5)
            description = L10N::GetString(0xaa2e57af);
        else
            description = std::wstring(L"\n\n") + FlatGeneratorInfo(&m_initData->mSuperflatConfig).generateDescription();
    }
    else if (index == 0x20)
        description = MiniGameDef::GetCustomGameModeById(static_cast<EMiniGameId>(m_initData->mValue5C), true)->GetLootSetDescription(m_gameOptionsList.GetSliderValue(0x20));
    else if (index == 0x21)
        description = MiniGameDef::GetCustomGameModeById(static_cast<EMiniGameId>(m_initData->mValue5C), true)->GetFoodBalanceIndexDescription(m_gameOptionsList.GetSliderValue(0x21));
    if (descriptionId == 0xc36b9de0u)
        description = replaceAll(description, L"{*DISABLES_ACHIEVEMENTS*}", L10N::GetString(0xbf244113));
    wchar_t prefix[64];
    swprintf(prefix, 64, L"<font color=\"#%08x\">", g_pApp->GetHTMLColour(static_cast<eMinecraftColour>(0x1b5)));
    description.insert(0, prefix);
    UIString value(description);
    if (!m_initData->mFlag56 && !m_initData->mFlag57 && m_menuType == 1)
        m_worldDescription.setLabel(value, false, false);
    else
        m_gameDescription.setLabel(value, false, false);
    if (m_initData->mFlag3)
        m_worldOptionsList.SetItemLabel(0x16, UIString(m_initData->mDisplayName), false);
}
void UIScene_LaunchMoreOptionsMenu::_init()
{
    mapElementsAndNames();

    m_worldLabel.init(UIString(L10N::GetString(0x66331121)));
    m_gameLabel.init(UIString(L10N::GetString(0xf2f331d5)));
    m_focusedControlId = -1;
    m_focusedItemId = -1;

    const int miniGameId = m_initData->mValue5C;
    const bool custom = m_initData->mFlag56;
    const bool joining = m_initData->mFlag57;
    const std::wstring blank(L" ");
    const std::wstring section(L10N::GetString(0xf2f331d5));

    if (!custom && !joining)
    {
        m_worldOptionsList.AddNewLabel(std::wstring(L10N::GetString(0xa60b975e)), 0x34);
        m_worldOptionsList.AddNewTextInput(m_initData->mDisplayName, 0x16, 0x3c);
        m_worldOptionsList.AddNewLabel(std::wstring(L10N::GetString(0xae4164b2)), 0x34);
        m_worldOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(0xcf25ae14)), 0x17,
                                          m_initData->mFlag40);
        m_worldOptionsList.AddNewLabel(blank, 0x34);
        m_worldOptionsList.AddNewLabel(std::wstring(L10N::GetString(0xa0e64dbf)), 0x34);
        const int biomeLabels[] = { static_cast<int>(0x6353b2e3), static_cast<int>(0xd797b179), static_cast<int>(0x34a11679) };
        m_worldOptionsList.AddNewSlider(std::wstring(L10N::GetString(biomeLabels[m_initData->mFlagD4])), 0x18, 0, 2,
                                        m_initData->mFlagD4, false, 7, 3);
        m_worldOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(0x9c2f3dbe)), 10,
                                          m_initData->mFlag9);
        m_worldOptionsList.EnableItem(10, !m_initData->mFlagA, true);
        m_worldOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(0xf1fc8097)), 0xb,
                                          m_initData->mFlag6);
        m_worldOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(0x6c3259af)), 0x14,
                                          m_initData->mFlag5);
        m_worldOptionsList.EnableItem(0x14, !m_initData->mFlag7, true);
        m_worldOptionsList.AddNewButton(std::wstring(L10N::GetString(0x9cfef661)), 0x15, true);
        m_worldOptionsList.EnableItem(0x15, m_initData->mFlag5 && !m_initData->mFlag7, true);
        m_worldOptionsList.AddNewLabel(blank, 0x34);
        m_worldOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(0x1de3701)), 5,
                                          m_initData->mFlagA);
        m_worldOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(0xef311201)), 6,
                                          m_initData->mFlagB);
        m_worldOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(0x846fa807)), 4,
                                          m_initData->mFlag9);
        if (m_initData->mFlag3)
            m_focusedItemId = 0x16;
    }

    if (custom || joining)
    {
        m_gameOptionsList.AddNewLabel(section, 0x34);
        MiniGameDef* game = MiniGameDef::GetCustomGameModeById(static_cast<EMiniGameId>(miniGameId), true);
        if (game->HasGameVariants())
        {
            m_gameMode = SliderLogic(0x1d, 0x5bd476fb, eCustomGameModeHostOption_GameVariant,
                                     static_cast<EMiniGameId>(miniGameId), true);
            m_gameMode.AddTo(m_gameOptionsList, m_initData->mValue64);
        }
        if (game->HasMultipleMaps() && miniGameId != 2)
        {
            m_gameOptionsList.AddNewButton(std::wstring(L10N::GetString(0x88c46bf1)), 0x1b, true);
            m_gameOptionsList.EnableItem(0x1b, true, true);
        }
        if (game->HasRoundLengthOption())
        {
            m_roundLength = SliderLogic(0x1c, 0x908811f4, eCustomGameModeHostOption_RoundLength,
                                        static_cast<EMiniGameId>(miniGameId), true);
            m_roundLength.AddTo(m_gameOptionsList, m_initData->mValue60);
        }
        if (game->HasCentralSpawnOption())
            m_gameOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(0xef2a2f6a)), 0x1a,
                                             m_initData->mPlayerSettings.Get(eCustomGameModeHostOption_CentreSpawn) != 0);
        if (miniGameId == 1 || miniGameId == 2)
        {
            m_gameOptionsList.AddNewLabel(blank, 0x34);
            m_gameOptionsList.AddNewLabel(std::wstring(L10N::GetString(0x8932699d)), 0x34);
            m_gameOptionsList.AddNewSlider(std::wstring(L10N::GetString(0x8d5cbb71)), 0x22, 0, 10, 0, false, 7, 3);
            m_gameOptionsList.AddNewSlider(std::wstring(L10N::GetString(0x901c1fe8)), 0x1e, 0, 10,
                                           m_initData->mPlayerSettings.Get(eCustomGameModeHostOption_SpectateMode), false, 7, 3);
        }
        if (miniGameId == 1)
        {
            m_gameOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(0xa329d6e9)), 0x1f,
                                             m_initData->mPlayerSettings.Get(eCustomGameModeHostOption_NoSmallSkins) == 0);
            m_gameOptionsList.AddNewSlider(std::wstring(L10N::GetString(0x5ccdb447)), 0x20, 0, 5, static_cast<int>(m_initData->mPlayerSettings.mLootSetId), false, 7, 3);
            m_gameOptionsList.AddNewSlider(std::wstring(L10N::GetString(0x1cbfab4a)), 0x21, 0, 7, m_initData->mPlayerSettings.mFoodBalance, false, 7, 3);
            m_gameOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(0x7356945d)), 0x28,
                                             m_initData->mPlayerSettings.mTakeEverythingEnabled);
            m_gameOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(0xaa27b127)), 0x29,
                                             m_initData->mPlayerSettings.mChestRefillEnabled);
        }
        else if (miniGameId == 2)
        {
            m_gameOptionsList.AddNewSlider(std::wstring(L10N::GetString(0x5ccdb447)), 0x20, 0, 3, 0, false, 7, 3);
            m_gameOptionsList.AddNewSlider(std::wstring(L10N::GetString(0x36df2177)), 0x2b, 0, 5, 0, false, 7, 3);
            m_gameOptionsList.AddNewSlider(std::wstring(L10N::GetString(0x50849384)), 0x2c, 0, 8, 0, false, 7, 3);
            m_gameOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(0x40ea8efc)), 0x2a,
                                             m_initData->mFlagD2);
        }
    }
    else
    {
        m_gameOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(0x2c692b72)), 0, m_initData->mFlag0);
        m_gameOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(0x4d7ec0cc)), 2, m_initData->mFlag2);
        m_gameOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(0xfbb3ea51)), 3, m_initData->mFlag8);
        m_gameOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(0x7873433f)), 7, m_initData->mFlagC);
        m_gameOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(0x5e940e19)), 0x12, m_initData->mFlag15);
        m_gameOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(0xfaaaeb73)), 0x13, m_initData->mFlag16);
        m_gameOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(0x7a2d3b2)), 0xc, m_initData->mFlag10);
        m_gameOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(0x3decac90)), 0xd, m_initData->mFlag11);
        m_gameOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(0x2a7c56c4)), 0xf, m_initData->mFlagF);
        m_gameOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(0x4aac0bc3)), 0xe, m_initData->mFlag12);
        m_gameOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(0x78e0a40d)), 0x10, m_initData->mFlag13);
        m_gameOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(0x6f7d8591)), 0x11, m_initData->mFlag14);
    }

    static_cast<FJ_LaunchMoreOptionsMenu*>(m_movie->getRootNode()->getFuiNode())->SetMenuType(!m_initData->mFlag3);
    if (m_initData->mFlag56 || m_initData->mFlag57)
        static_cast<FJ_LaunchMoreOptionsMenu*>(m_movie->getRootNode()->getFuiNode())->HideWorldTab();
    if (!m_initData->mFlag56 && !m_initData->mFlag57)
    {
        if (!m_online)
        {
            m_gameOptionsList.EnableItem(0, false, true);
            m_gameOptionsList.EnableItem(2, false, true);
        }
        if (!m_initData->mFlag0)
        {
            m_gameOptionsList.EnableItem(2, false, true);
            m_gameOptionsList.EnableItem(0, false, true);
        }
    }

    UpdateCheats();
    UpdateDescriptionText();
    m_worldTab.init(0x34);
    m_gameTab.init(0x35);
    updateTooltips();
    updateComponents();
    gConsoleUIController.TouchBoxRebuild(this);
}

unsigned int UIScene_LaunchMoreOptionsMenu::mapElementsAndNames()
{
    if (m_movie == NULL)
        return 0;
    fuiRenderNode* root = m_movie->getRootNode();
    m_gameLabel.setupControl(this, root, std::string("LabelGame"));
    m_worldLabel.setupControl(this, root, std::string("LabelWorld"));
    m_worldOptions.setupControl(this, root, std::string("WorldOptions"));
    m_worldOptionsList.setupControl(this, m_worldOptions.getFuiRenderNode(), std::string("WorldOptionsList"));
    m_worldTab.setupControl(this, m_worldOptions.getFuiRenderNode(), std::string("TouchTabWorld"));
    m_worldDescription.setupControl(this, m_worldOptions.getFuiRenderNode(), std::string("Description_WorldOptions"));
    m_worldTabLabel.setupControl(this, m_worldOptions.getFuiRenderNode(), std::string("WorldSize"));
    m_resizeLabel.setupControl(this, root, std::string("ResizeLabel"));
    m_gameOptions.setupControl(this, root, std::string("GameOptions"));
    m_gameOptionsList.setupControl(this, m_gameOptions.getFuiRenderNode(), std::string("GameOptionsList"));
    m_gameTab.setupControl(this, m_gameOptions.getFuiRenderNode(), std::string("TouchTabGame"));
    m_gameDescription.setupControl(this, m_gameOptions.getFuiRenderNode(), std::string("Description_GameOptions"));
    m_controls.push_back(&m_gameLabel);
    m_controls.push_back(&m_worldLabel);
    m_controls.push_back(&m_worldOptions);
    m_controls.push_back(&m_worldOptionsList);
    m_controls.push_back(&m_worldTab);
    m_controls.push_back(&m_worldDescription);
    m_controls.push_back(&m_worldTabLabel);
    m_controls.push_back(&m_resizeLabel);
    m_controls.push_back(&m_gameOptions);
    m_controls.push_back(&m_gameOptionsList);
    m_controls.push_back(&m_gameTab);
    m_controls.push_back(&m_gameDescription);
    return 1;
}

void UIScene_LaunchMoreOptionsMenu::updateTooltips()
{
    if (!m_initData->mFlag56 && !m_initData->mFlag57)
    {
        const int tabLabel = m_menuType == 0 ? 0x66331121 : 0xf2f331d5;
        gConsoleUIController.SetTooltips(0, 0x8d6323ed, 0x4f6bcf5e, -1, -1, -1, -1, -2, tabLabel, -1, -1, -1, -1, -1, -1, -1, -1, false, false);
    }
    else
    {
        gConsoleUIController.SetTooltips(0, 0x8d6323ed, 0x4f6bcf5e, -1, -1, -1, -1,
                                         -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, false, false);
    }
}

void UIScene_LaunchMoreOptionsMenu::updateComponents()
{
    if (m_layer != NULL)
    {
        m_layer->showComponent(m_playerId, static_cast<EUIScene>(0x42), true);
        m_layer->showComponent(m_playerId, static_cast<EUIScene>(0x43), false);
    }
}

std::wstring UIScene_LaunchMoreOptionsMenu::getMoviePath()
{
    if (g_pApp != NULL && g_pApp->GetLocalPlayerCount() > 1)
        return std::wstring(L"LaunchMoreOptionsMenuSplit");
    return std::wstring(L"LaunchMoreOptionsMenu");
}

int UIScene_LaunchMoreOptionsMenu::KeyboardCompleteSeedCallback(void* data, bool accepted)
{
    UIScene_LaunchMoreOptionsMenu* scene = static_cast<UIScene_LaunchMoreOptionsMenu*>(data);
    scene->m_keyboardActive = false;
    if (accepted)
    {
        wchar_t text[128];
        g_pInput->GetText(text, 0x7f);
        scene->m_worldOptionsList.SetItemLabel(0x16, UIString(text), false);
        scene->m_initData->mDisplayName.assign(text);
        scene->m_initData->mFlag55 = true;
    }
    return 0;
}

int UIScene_LaunchMoreOptionsMenu::HandleRequestKeyboard()
{
    m_keyboardActive = true;
    m_seed.assign(m_initData->mDisplayName);
    if (m_seed == L" ")
        m_seed.assign(L"");
    const wchar_t* title = L10N::GetString(0xa60b975e);
    g_pInput->RequestKeyboard(title, m_seed.c_str(), m_playerId, 0x3c,
                              KeyboardCompleteSeedCallback, this,
                              C_4JInput::EKeyboardMode_0);
    return 0;
}

void UIScene_LaunchMoreOptionsMenu::UpdateOnline()
{
    if (m_initData->mFlag56 && m_initData->mFlag58)
        return;
    if (m_initData->mFlag57)
        return;
    m_gameOptionsList.EnableItem(2, m_initData->mFlag0, true);
}

void UIScene_LaunchMoreOptionsMenu::tick()
{
    UIScene::tick();
    bool online = gProfile->IsSignedInLive(m_initData->mValue18) &&
                  gProfile->AllowedToPlayMultiplayer(m_initData->mValue18);
    if (online != m_online)
    {
        m_gameOptionsList.EnableItem(0, online, true);
        m_gameOptionsList.EnableItem(2, online, true);
        if (online)
        {
            m_gameOptionsList.SetCheckboxValue(0, true, true);
            m_gameOptionsList.SetCheckboxValue(2, true, true);
        }
        m_online = online;
    }
    if (m_requestKeyboard)
    {
        HandleRequestKeyboard();
        m_requestKeyboard = false;
    }
    if (m_updateCheats)
    {
        UpdateCheats();
        m_updateCheats = false;
    }
    if (m_updateOnline)
    {
        UpdateOnline();
        m_updateOnline = false;
    }
}

void UIScene_LaunchMoreOptionsMenu::handleDestroy()
{
    if (g_pInput != NULL)
        g_pInput->DestroyKeyboard();
}

void UIScene_LaunchMoreOptionsMenu::handleReload()
{
    int savedItemId = m_focusedItemId;
    _init();
    static_cast<FJ_LaunchMoreOptionsMenu*>(m_movie->getRootNode()->getFuiNode())->SetActiveTab(m_menuType, false);
    m_focusedItemId = savedItemId;
    if (m_menuType == 0)
    {
        int selection = m_gameOptionsList.getListIndex(savedItemId);
        m_gameOptionsList.setCurrentSelection(selection, false, true);
    }
    else
    {
        int selection = m_worldOptionsList.getListIndex(savedItemId);
        m_worldOptionsList.setCurrentSelection(selection, false, true);
    }
}

void UIScene_LaunchMoreOptionsMenu::handleInput(int, int action, bool repeat, bool released,
                                                bool pressed, bool& handled)
{
    if (m_keyboardActive)
        return;
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, released, pressed);
    if (action < 0xe)
    {
        if (action <= 3)
            return;
        if (action < 10)
        {
            sendInputToMovie(action, repeat, released, pressed);
            handled = true;
            return;
        }
        if (action > 0xb)
            return;
        if (released)
        {
            gConsoleUIController.PlayUISFX(NULL);
            m_menuType = m_menuType == 0;
            updateTooltips();
            static_cast<FJ_LaunchMoreOptionsMenu*>(m_movie->getRootNode()->getFuiNode())->ChangeTab();
            gConsoleUIController.TouchBoxRebuild(this);
        }
        return;
    }
    if (action < 0x10 || action == 0x14)
    {
        sendInputToMovie(action, repeat, released, pressed);
        handled = true;
        return;
    }
    if (action == 0x15 && released)
    {
        if (m_initData->mFlag56 || m_initData->mFlag57)
            MasterGameMode::SaveDefaults(*m_initData);
        navigateBack();
        handled = true;
    }
}

void UIScene_LaunchMoreOptionsMenu::handleFocusChange(int focus, int childFocus)
{
    m_focusedItemId = childFocus;
    m_focusedControlId = focus;
    UpdateDescriptionText();
}

void UIScene_LaunchMoreOptionsMenu::handleTimerComplete(int)
{
}

void UIScene_LaunchMoreOptionsMenu::handlePress(int first, int second)
{
    if (m_keyboardActive)
        return;
    gConsoleUIController.PlayUISFX(NULL);
    if (first == 0x16)
    {
        m_requestKeyboard = true;
        return;
    }
    if (second == 0x15)
    {
        gConsoleUIController.NavigateToScene(m_playerId, static_cast<EUIScene>(0x39), m_initData,
                                             static_cast<EUILayer>(6), static_cast<EUIGroup>(7));
        return;
    }
    if (second == 0x19)
    {
        gConsoleUIController.NavigateToScene(m_playerId, static_cast<EUIScene>(0x40), m_initData,
                                             static_cast<EUILayer>(6), static_cast<EUIGroup>(7));
        return;
    }
    if (second == 0x1b)
    {
        gConsoleUIController.NavigateToScene(m_playerId, static_cast<EUIScene>(0x3f), m_initData,
                                             static_cast<EUILayer>(6), static_cast<EUIGroup>(7));
        return;
    }
    if (second != 0x2d)
        return;
    if (m_initData->mFlag57)
    {
        MiniGameDef* miniGame = MiniGameDef::GetCustomGameModeById(static_cast<EMiniGameId>(m_initData->mValue5C), true);
        miniGame->SetDefaultMatchmakingSearchData(true, m_initData);
        m_gameOptionsList.SetSliderValue(0x2e, m_initData->mValue60, false);
        m_lootSet.Update(m_gameOptionsList, m_initData->mValue60);
        m_gameOptionsList.SetSliderValue(0x2f, m_initData->mValue64, false);
        m_foodBalance.Update(m_gameOptionsList, m_initData->mValue64);
        UpdateDescriptionText();
        return;
    }
    if (m_initData->mFlag56 && m_initData->mFlag58)
    {
        MiniGameDef* miniGame = MiniGameDef::GetCustomGameModeById(static_cast<EMiniGameId>(m_initData->mValue5C), true);
        miniGame->SetDefaultMatchmakingSearchData(false, m_initData);
        m_gameOptionsList.SetSliderValue(0x2e, m_initData->mValue60, false);
        m_lootSet.Update(m_gameOptionsList, m_initData->mValue60);
        m_gameOptionsList.SetSliderValue(0x2f, m_initData->mValue64, false);
        m_foodBalance.Update(m_gameOptionsList, m_initData->mValue64);
        UpdateDescriptionText();
    }
}

void UIScene_LaunchMoreOptionsMenu::handleSliderElementMove(int, int second, int value)
{
    if (second == 0x2e)
        m_worldSize.Update(m_gameOptionsList, value);
    else if (second == 0x2f)
        m_roundLength.Update(m_gameOptionsList, value);
    UpdateDescriptionText();
}

void UIScene_LaunchMoreOptionsMenu::handleGainFocus(bool)
{
    UpdateDescriptionText();
}

void UIScene_LaunchMoreOptionsMenu::handleCheckboxElementToggled(int, int second, bool checked)
{
    gConsoleUIController.PlayUISFX(NULL);
    switch (second)
    {
    case 0:
        m_initData->mFlag0 = checked;
        m_updateOnline = true;
        m_initData->mFlag55 = true;
        break;
    case 1:
        m_initData->mFlag55 = true;
        m_initData->mFlag1 = checked;
        break;
    case 2:
        m_initData->mFlag55 = true;
        m_initData->mFlag2 = checked;
        break;
    case 3:
        m_initData->mFlag55 = true;
        m_initData->mFlag8 = checked;
        break;
    case 4:
        m_initData->mFlag55 = true;
        m_initData->mFlag9 = checked;
        break;
    case 5:
        m_initData->mFlag55 = true;
        m_initData->mFlagA = checked;
        break;
    case 6:
        m_initData->mFlag55 = true;
        m_initData->mFlagB = checked;
        break;
    case 7:
        m_initData->mFlagC = checked;
        m_updateCheats = true;
        m_initData->mFlag55 = true;
        break;
    case 8:
        m_initData->mFlag55 = true;
        m_initData->mFlagD = checked;
        break;
    case 9:
        m_initData->mFlag55 = true;
        m_initData->mFlagE = checked;
        break;
    case 10:
        m_initData->mFlag4 = m_initData->mFlag5 || checked;
        m_initData->mFlag55 = true;
        break;
    case 11:
        m_initData->mFlag55 = true;
        m_initData->mFlag6 = checked;
        break;
    case 12:
        m_initData->mFlag55 = true;
        m_initData->mFlag10 = checked;
        break;
    case 13:
        m_initData->mFlag55 = true;
        m_initData->mFlag11 = checked;
        break;
    case 14:
        m_initData->mFlag55 = true;
        m_initData->mFlag12 = checked;
        break;
    case 15:
        m_initData->mFlag55 = true;
        m_initData->mFlagF = checked;
        break;
    case 16:
        m_initData->mFlag55 = true;
        m_initData->mFlag13 = checked;
        break;
    case 17:
        m_initData->mFlag55 = true;
        m_initData->mFlag14 = checked;
        break;
    case 18:
        m_initData->mFlag55 = true;
        m_initData->mFlag15 = checked;
        break;
    case 19:
        m_initData->mFlag55 = true;
        m_initData->mFlag16 = checked;
        break;
    case 20:
        m_initData->mFlag4 = true;
        m_initData->mFlag5 = checked;
        if (!m_initData->mFlag7)
        {
            m_worldOptionsList.EnableItem(0x15, checked, false);
            m_worldOptionsList.EnableItem(10, !m_initData->mFlag5, false);
            if (!m_initData->mFlag5)
            {
                m_worldOptionsList.SetCheckboxValue(10, true, false);
                m_initData->mFlag55 = true;
                break;
            }
            SuperflatMenu::PresetCache::GetCachedPreset(m_playerId, m_initData->mSuperflatConfig);
        }
        m_initData->mFlag55 = true;
        break;
    case 23:
        m_initData->mFlag55 = true;
        m_initData->mFlag40 = checked;
        break;
    case 26:
        m_initData->mPlayerSettings.Set(eCustomGameModeHostOption_CentreSpawn, checked);
        m_initData->mFlag55 = true;
        break;
    case 0x1f:
        if (MiniGameDef::IsCustomGame(static_cast<EMiniGameId>(m_initData->mValue5C), m_initData->mPlayerSettings))
            m_initData->mPlayerSettings.Set(eCustomGameModeHostOption_NoSmallSkins, checked ? 0 : 1);
        m_initData->mFlag55 = true;
        break;
    case 0x28:
        if (MiniGameDef::IsCustomGame(static_cast<EMiniGameId>(m_initData->mValue5C), m_initData->mPlayerSettings))
            m_initData->mPlayerSettings.mTakeEverythingEnabled = checked;
        m_initData->mFlag55 = true;
        break;
    case 0x29:
        if (MiniGameDef::IsCustomGame(static_cast<EMiniGameId>(m_initData->mValue5C), m_initData->mPlayerSettings))
            m_initData->mPlayerSettings.mChestRefillEnabled = checked;
        m_initData->mFlag55 = true;
        break;
    default:
        m_initData->mFlag55 = true;
        break;
    }
    UpdateDescriptionText();
}

void UIScene_LaunchMoreOptionsMenu::handleTouchInput(unsigned int, int action, int, int, bool repeat,
                                                     bool, bool pressed)
{
    if (pressed && action > 0x33 && action < 0x36)
    {
        gConsoleUIController.PlayUISFX(NULL);
        bool world = action == 0x34;
        if (m_menuType != static_cast<int>(world))
        {
            m_menuType = world;
            updateTooltips();
            static_cast<FJ_LaunchMoreOptionsMenu*>(m_movie->getRootNode()->getFuiNode())->ChangeTab();
        }
        gConsoleUIController.TouchBoxRebuild(this);
    }
    (void)repeat;
}

UIControl* UIScene_LaunchMoreOptionsMenu::GetMainPanel()
{
    return m_menuType == 1 ? &m_worldBackground : &m_gameBackground;
}

int UIScene_LaunchMoreOptionsMenu::getSceneType()
{
    return 0x2b;
}



