#include "game/ui/UIScene_HUD.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cwchar>
#include <list>
#include <utility>
#include <vector>

#include "game/core/BossEvent.hpp"
#include "game/core/BossHealthOverlay.hpp"
#include "game/core/ClientMasterGameMode.hpp"
#include "game/core/ClientScoreboard.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/FoodData.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/GameType.hpp"
#include "game/core/GhostController.hpp"
#include "game/core/Gui.hpp"
#include "game/core/HumanoidArm.hpp"
#include "game/core/L10N.hpp"
#include "game/core/LerpingBossEvent.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/core/StringFormat.hpp"
#include "game/core/StringUtils.hpp"
#include "game/core/System.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/ui/FJ_Hud.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fui.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/generation/GhostPlayer.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"
#include "platform/wiiu/CProfile.hpp"

UIScene_HUD::UIScene_HUD(int sceneType, void*, UILayer* layer)
    : UIScene(sceneType, layer), m_splitScreen(false), m_animatedSplashIcon(-1),
      m_hudSize(-1), m_air(10), m_hudGameMode(-1), m_selectedLabelTicks(0),
      m_timedSplashTicks(0), m_ridingHorse(true), m_displayNameVisible(true),
      m_scoreIcon(0), m_displayFlag(true), m_animatedSplashVisible(true),
      m_resultSplashShown(false), m_timedSplashLocked(false), m_checkpointsInitialised(false),
      m_wrongWayTicks(0), m_wrongWayVisible(false), m_levelGenerationId(-1),
      m_previousHealth(0), m_comparisonScore(0), m_personalBestTime(0)
{
    initialiseMovie();
    SetSelectedLabel(std::wstring());
    for (int i = 0; i < 10; ++i)
        m_messageLabels[i].init(UIString(L""));
    m_selectedLabel.init(UIString(L""));
    for (int i = 0; i < 4; ++i)
        m_armorIcons[i] = m_shownArmorIcons[i] = !HideMainHUD();
    m_healthVisible = !HideMainHUD();
    m_foodVisible = !HideMainHUD();
    m_airVisible = !HideMainHUD();
    m_armourVisible = !HideMainHUD();
    m_quickSelectVisible = !HideMainHUD();
    for (int i = 0; i < 4; ++i)
        m_lastCheckpointIds[i] = 0;
    addTimer(0, 100);
}

UIScene_HUD::~UIScene_HUD()
{
}

FJ_Hud* UIScene_HUD::getFJ_Hud()
{
    if (m_movie == 0)
        return 0;
    return m_movie->getRootNode()->getFuiNode()->asFJ_Hud();
}

void UIScene_HUD::SetSelectedLabel(const std::wstring& label)
{
    if (getFJ_Hud() != 0) getFJ_Hud()->SetSelectedLabel(label, true);
}

bool UIScene_HUD::HideMainHUD()
{
    return Minecraft::InMiniGame(eMiniGameId_Any, true) &&
        Minecraft::GetInstance()->GetMiniGame()->IsHUDDisabled();
}

unsigned int UIScene_HUD::mapElementsAndNames()
{
    fuiRenderNode* root = m_movie->getRootNode();

    m_messageLabels[0].setupControl(this, root, std::string("Label1"));
    m_controls.push_back(&m_messageLabels[0]);
    m_messageLabels[1].setupControl(this, root, std::string("Label2"));
    m_controls.push_back(&m_messageLabels[1]);
    m_messageLabels[2].setupControl(this, root, std::string("Label3"));
    m_controls.push_back(&m_messageLabels[2]);
    m_messageLabels[3].setupControl(this, root, std::string("Label4"));
    m_controls.push_back(&m_messageLabels[3]);
    m_messageLabels[4].setupControl(this, root, std::string("Label5"));
    m_controls.push_back(&m_messageLabels[4]);
    m_messageLabels[5].setupControl(this, root, std::string("Label6"));
    m_controls.push_back(&m_messageLabels[5]);
    m_messageLabels[6].setupControl(this, root, std::string("Label7"));
    m_controls.push_back(&m_messageLabels[6]);
    m_messageLabels[7].setupControl(this, root, std::string("Label8"));
    m_controls.push_back(&m_messageLabels[7]);
    m_messageLabels[8].setupControl(this, root, std::string("Label9"));
    m_controls.push_back(&m_messageLabels[8]);
    m_messageLabels[9].setupControl(this, root, std::string("Label10"));
    m_controls.push_back(&m_messageLabels[9]);

    m_messageControls[0].setupControl(this, root, std::string("Label1Background"));
    m_controls.push_back(&m_messageControls[0]);
    m_messageControls[1].setupControl(this, root, std::string("Label2Background"));
    m_controls.push_back(&m_messageControls[1]);
    m_messageControls[2].setupControl(this, root, std::string("Label3Background"));
    m_controls.push_back(&m_messageControls[2]);
    m_messageControls[3].setupControl(this, root, std::string("Label4Background"));
    m_controls.push_back(&m_messageControls[3]);
    m_messageControls[4].setupControl(this, root, std::string("Label5Background"));
    m_controls.push_back(&m_messageControls[4]);
    m_messageControls[5].setupControl(this, root, std::string("Label6Background"));
    m_controls.push_back(&m_messageControls[5]);
    m_messageControls[6].setupControl(this, root, std::string("Label7Background"));
    m_controls.push_back(&m_messageControls[6]);
    m_messageControls[7].setupControl(this, root, std::string("Label8Background"));
    m_controls.push_back(&m_messageControls[7]);
    m_messageControls[8].setupControl(this, root, std::string("Label9Background"));
    m_controls.push_back(&m_messageControls[8]);
    m_messageControls[9].setupControl(this, root, std::string("Label10Background"));
    m_controls.push_back(&m_messageControls[9]);

    m_selectedLabel.setupControl(this, root, std::string("Jukebox"));
    m_controls.push_back(&m_selectedLabel);

    if (m_splitScreen)
    {
        m_displayName.setupControl(this, root, std::string("LabelGamertag"));
        m_controls.push_back(&m_displayName);
    }

    m_healthLabel.setupControl(this, root, std::string("ScoreLabel"));
    m_controls.push_back(&m_healthLabel);
    m_hudComponent.setupControl(this, root, std::string("ScoreIcon"));
    m_controls.push_back(&m_hudComponent);
    m_foodLabel.setupControl(this, root, std::string("SpeedLabel"));
    m_controls.push_back(&m_foodLabel);
    m_armourLabel.setupControl(this, root, std::string("TimeLabel"));
    m_controls.push_back(&m_armourLabel);

    m_positionLabels[0].setupControl(this, root, std::string("PositionOne"));
    m_controls.push_back(&m_positionLabels[0]);
    m_positionLabels[1].setupControl(this, root, std::string("PositionTwo"));
    m_controls.push_back(&m_positionLabels[1]);
    m_positionLabels[2].setupControl(this, root, std::string("PositionThree"));
    m_controls.push_back(&m_positionLabels[2]);

    m_positionControls[0].setupControl(this, root, std::string("Gold"));
    m_controls.push_back(&m_positionControls[0]);
    m_positionControls[1].setupControl(this, root, std::string("Silver"));
    m_controls.push_back(&m_positionControls[1]);
    m_positionControls[2].setupControl(this, root, std::string("Bronze"));
    m_controls.push_back(&m_positionControls[2]);

    m_effectLabel.setupControl(this, root, std::string("WrongWayLabel"));
    m_controls.push_back(&m_effectLabel);

    m_splashLabel.setupControl(this, root, std::string("RoundNumber"));
    m_controls.push_back(&m_splashLabel);

    return 1;
}

void UIScene_HUD::ShowArmorIcons(int armour, bool visible)
{
    if (armour < 4 && m_shownArmorIcons[armour] != visible)
    {
        m_shownArmorIcons[armour] = visible;
        if (getFJ_Hud() != 0)
            getFJ_Hud()->ShowArmorIcons(armour, visible);
    }
}

bool UIScene_HUD::SORTBYTIMELEFT(MobEffectInstance* first, MobEffectInstance* second)
{
    return second->getDuration() < first->getDuration();
}

void UIScene_HUD::updateEffectsDisplay()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    boost::shared_ptr<MultiplayerLocalPlayer> player = minecraft->mLocalPlayers[getPad()];
    if (player == 0)
        return;

    std::vector<MobEffectInstance*>* effects = player->getActiveEffects();
    if (effects == 0)
        return;

    if (effects->empty())
    {
        FJ_Hud* hud = getFJ_Hud();
        if (hud != 0)
            hud->UpdateStatusEffectTile(0, 0, 0, false, 0.0f);
        delete effects;
        return;
    }

    std::sort(effects->begin(), effects->end(), SORTBYTIMELEFT);
    const int activeCount = static_cast<int>(effects->size());
    int slot = 0;
    for (std::vector<MobEffectInstance*>::iterator it = effects->begin(); it != effects->end(); ++it)
    {
        MobEffectInstance* instance = *it;
        MobEffect* effect = instance->getEffect();
        if (!effect->hasIcon())
            continue;

        float alpha = 1.0f;
        const int duration = instance->getDuration();
        if (duration < 200)
        {
            const float durationValue = static_cast<float>(duration);
            const float clampedDuration = Mth::clamp(durationValue / 10.0f, 0.0f, 1.0f);
            const float cosValue = Mth::cos((durationValue * 3.1415927f) / 5.0f);
            const float blink = Mth::clamp((10.0f - static_cast<float>(duration) / 20.0f) / 10.0f, 0.0f, 1.0f);
            alpha = cosValue * blink + clampedDuration;
        }

        FJ_Hud* hud = getFJ_Hud();
        if (hud != 0)
            hud->UpdateStatusEffectTile(activeCount, slot, effect->getIcon(), instance->isAmbient(), alpha);
        ++slot;
    }

    delete effects;
}

void UIScene_HUD::SetNumBossBars(int count)
{
    if (getFJ_Hud() != 0) getFJ_Hud()->SetNumBossBars(count);
}

void UIScene_HUD::SetBossBar(int index, float progress, const std::wstring& name)
{
    if (getFJ_Hud() != 0) getFJ_Hud()->SetBossBar(index, progress, name);
}

void UIScene_HUD::HideSelectedLabel()
{
    if (getFJ_Hud() != 0) getFJ_Hud()->HideSelectedLabel();
}

void UIScene_HUD::tick()
{
    if (getFJ_Hud() != 0)
        UIScene::tick();

    if (m_movie == 0)
        return;

    if (!g_pApp->GetGameStarted())
    {
        SetNumBossBars(0);
        if (m_selectedLabelTicks != 0)
        {
            m_selectedLabelTicks = 0;
            HideSelectedLabel();
        }
        return;
    }

    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft->mLocalPlayers[getPad()] == 0 || minecraft->mGameModes[getPad()] == 0)
        return;

    if (m_armorIcons[0] != m_shownArmorIcons[0])
        ShowArmorIcons(0, m_armorIcons[0]);
    if (m_armorIcons[1] != m_shownArmorIcons[1])
        ShowArmorIcons(1, m_armorIcons[1]);
    if (m_armorIcons[2] != m_shownArmorIcons[2])
        ShowArmorIcons(2, m_armorIcons[2]);
    if (m_armorIcons[3] != m_shownArmorIcons[3])
        ShowArmorIcons(3, m_armorIcons[3]);

    updateEffectsDisplay();

    boost::unordered_map<std::wstring, LerpingBossEvent*> bossEvents =
        minecraft->mGui->getBossOverlay(getPad())->mBossEvents;
    SetNumBossBars(bossEvents.size() > 5 ? 5 : static_cast<int>(bossEvents.size()));
    if (bossEvents.size() == 0)
        return;

    int index = 0;
    for (boost::unordered_map<std::wstring, LerpingBossEvent*>::iterator it = bossEvents.begin();
         it != bossEvents.end(); ++it, ++index)
    {
        if (index >= 5)
            continue;
        BossEvent* bossEvent = it->second;
        std::wstring name = L10N::GetString(bossEvent->getNameId());
        if (!bossEvent->getName().empty())
            name = bossEvent->getName();
        SetBossBar(index, bossEvent->getPercent(), name);
    }
}

void UIScene_HUD::updateSafeZone()
{
    switch (m_layer->getViewport()) {
    default:
    case C4JRender::eViewportType_FullScreen:
        setSafeZone(getSafeZoneHalfHeight(), getSafeZoneHalfHeight(), getSafeZoneHalfWidth(),
                    getSafeZoneHalfWidth());
        break;
    case C4JRender::eViewportType_TopHalf:
        setSafeZone(getSafeZoneHalfHeight(), 0.0, 0.0, 0.0);
        break;
    case C4JRender::eViewportType_BottomHalf:
        setSafeZone(0.0, getSafeZoneHalfHeight(), 0.0, 0.0);
        break;
    case C4JRender::eViewportType_LeftHalf:
        setSafeZone(0.0, 0.0, getSafeZoneHalfWidth(), 0.0);
        break;
    case C4JRender::eViewportType_RightHalf:
        setSafeZone(0.0, 0.0, 0.0, getSafeZoneHalfWidth());
        break;
    case C4JRender::eViewportType_TopLeftQuarter:
        setSafeZone(getSafeZoneHalfHeight(), 0.0, getSafeZoneHalfWidth(), 0.0);
        break;
    case C4JRender::eViewportType_TopRightQuarter:
        setSafeZone(getSafeZoneHalfHeight(), 0.0, 0.0, getSafeZoneHalfWidth());
        break;
    case C4JRender::eViewportType_BottomLeftQuarter:
        setSafeZone(0.0, getSafeZoneHalfHeight(), getSafeZoneHalfWidth(), 0.0);
        break;
    case C4JRender::eViewportType_BottomRightQuarter:
        setSafeZone(0.0, getSafeZoneHalfHeight(), 0.0, getSafeZoneHalfWidth());
        break;
    }
}

std::wstring UIScene_HUD::getMoviePath()
{
    const C4JRender::eViewportType viewport = m_layer->getViewport();
    if (viewport == 0 || viewport > 8)
    {
        m_splitScreen = false;
        return std::wstring(L"HUD");
    }
    m_splitScreen = true;
    return std::wstring(L"HUDSplit");
}

void UIScene_HUD::customDraw(const char* name, fuiRect*)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft->mLocalPlayers[getPad()] != 0 && minecraft->mGameModes[getPad()] != 0)
    {
        int slot = -1;
        std::sscanf(name, "slot_%d", &slot);
        int armorSlot = -1;
        std::sscanf(name, "armor_slot_%d", &armorSlot);
        int pickupSlot = -1;
        std::sscanf(name, "pickup_slot_%d", &pickupSlot);
    }
}

void UIScene_HUD::SetHudSize(int size)
{
    if (getFJ_Hud() != 0) getFJ_Hud()->LoadHud(size);
}

void UIScene_HUD::SetDisplayName(const std::wstring& name)
{
    if (name.compare(m_displayNameText) != 0 && m_splitScreen)
    {
        m_displayNameText = name;
        if (getFJ_Hud() != 0)
            getFJ_Hud()->SetGamertag(name);
        m_displayName.setVisible(m_displayNameVisible);
    }
}

void UIScene_HUD::repositionHud()
{
    if (!m_splitScreen)
        return;

    int width = 0;
    int height = 0;
    m_layer->getRenderDimensions(width, height);
    const C4JRender::eViewportType viewport = m_layer->getViewport();
    if (viewport == C4JRender::eViewportType_TopHalf ||
        viewport == C4JRender::eViewportType_BottomHalf)
    {
        int fullHeight;
        gConsoleUIController.getRenderDimensions(C4JRender::eViewportType_FullScreen,
                                                 width, fullHeight);
    }
    else if (viewport == C4JRender::eViewportType_LeftHalf ||
             viewport == C4JRender::eViewportType_RightHalf)
    {
        int fullWidth;
        gConsoleUIController.getRenderDimensions(C4JRender::eViewportType_FullScreen,
                                                 fullWidth, height);
    }

    if (getFJ_Hud() != 0)
        getFJ_Hud()->RepositionHud(width, height);
}

void UIScene_HUD::ShowGameModePlayers(bool visible)
{
    if (getFJ_Hud() != 0) getFJ_Hud()->ShowGameModePlayers(visible);
}

bool UIScene_HUD::ShowHudComponent(bool wasVisible, bool visible)
{
    if (!wasVisible && visible && Minecraft::InMiniGame(eMiniGameId_Lobby, true) &&
        Minecraft::GetInstance()->GetClientMasterGameMode()->IsRoundEnding())
        visible = false;
    return visible;
}

void UIScene_HUD::ShowHealth(bool visible) { if (getFJ_Hud() != 0) getFJ_Hud()->ShowHealth(visible); }
void UIScene_HUD::ShowFood(bool visible) { if (getFJ_Hud() != 0) getFJ_Hud()->ShowFood(visible); }
void UIScene_HUD::ShowArmour(bool visible) { if (getFJ_Hud() != 0) getFJ_Hud()->ShowArmour(visible); }
void UIScene_HUD::ShowArmorBar(bool visible) { if (getFJ_Hud() != 0) getFJ_Hud()->ShowArmorBar(visible); }
void UIScene_HUD::ShowQuickSelect(bool visible)
{
    if (HideMainHUD())
        visible = false;
    bool quickSelectVisible = ShowHudComponent(m_quickSelectVisible, visible);
    bool showArmorBar = false;
    if (Minecraft::InMiniGame(eMiniGameId_Battle, false))
        showArmorBar = quickSelectVisible;
    ShowArmorBar(showArmorBar);
    m_quickSelectVisible = quickSelectVisible;
    if (getFJ_Hud() != 0)
        getFJ_Hud()->ShowQuickSelect(quickSelectVisible);
}

bool UIScene_HUD::GetQuickSelectVisible() const
{
    return m_quickSelectVisible;
}

void UIScene_HUD::ShowExpBar(bool visible) { if (getFJ_Hud() != 0) getFJ_Hud()->ShowExpBar(visible); }
void UIScene_HUD::ShowHorseHealth(bool visible) { if (getFJ_Hud() != 0) getFJ_Hud()->ShowHorseHealth(visible); }
void UIScene_HUD::ShowAir(bool visible) { if (getFJ_Hud() != 0) getFJ_Hud()->ShowAir(visible); }
void UIScene_HUD::SetRidingHorse(bool riding, bool showJump, int health) { if (getFJ_Hud() != 0) getFJ_Hud()->SetRidingHorse(riding, showJump, health); }
void UIScene_HUD::SetRegenerationEffect(bool enabled) { if (getFJ_Hud() != 0) getFJ_Hud()->SetRegenerationEffect(enabled); }
void UIScene_HUD::SetHealthAbsorb(int health) { if (getFJ_Hud() != 0) getFJ_Hud()->SetHealthAbsorb(true, health); }
void UIScene_HUD::InitMainHUD()
{
    const bool visible = !HideMainHUD();
    ShowGameModePlayers(true);
    ShowHealth(visible);
    ShowFood(visible);
    ShowArmour(visible);
    ShowQuickSelect(visible);
    ShowExpBar(visible);
    ShowHorseHealth(visible);
    ShowAir(false);
    SetRidingHorse(false, false, 0);
    SetRegenerationEffect(false);
    SetHealthAbsorb(0);
    m_shownArmorIcons[0] = visible;
    m_armorIcons[0] = visible;
    m_shownArmorIcons[1] = visible;
    m_armorIcons[1] = visible;
    m_shownArmorIcons[2] = visible;
    m_armorIcons[2] = visible;
    m_shownArmorIcons[3] = visible;
    m_armorIcons[3] = visible;
}
void UIScene_HUD::SetTooltipsEnabled(bool enabled) { if (getFJ_Hud() != 0) getFJ_Hud()->SetTooltipsEnabled(enabled); }
void UIScene_HUD::handleReload()
{
    m_hudSize = -1;
    m_air = 10;
    m_airVisible = true;
    m_armourVisible = true;
    m_foodVisible = true;
    m_selectedLabelTicks = 0;
    m_healthVisible = true;
    m_displayNameText.clear();
    m_displayFlag = true;
    m_hudGameMode = -1;
    m_quickSelectVisible = true;
    m_ridingHorse = true;
    m_animatedSplashVisible = true;
    m_cachedLabel.clear();
    m_animatedSplashText.clear();
    m_displayNameVisible = true;
    m_scoreIcon = 0;
    m_animatedSplashIcon = -1;
    m_checkpointsInitialised = false;

    if (m_splitScreen)
        m_displayName.setVisible(true);

    Minecraft* minecraft = Minecraft::GetInstance();

    SetSelectedLabel(std::wstring());

    m_messageLabels[0].init(UIString(L""));
    m_messageLabels[1].init(UIString(L""));
    m_messageLabels[2].init(UIString(L""));
    m_messageLabels[3].init(UIString(L""));
    m_messageLabels[4].init(UIString(L""));
    m_messageLabels[5].init(UIString(L""));
    m_messageLabels[6].init(UIString(L""));
    m_messageLabels[7].init(UIString(L""));
    m_messageLabels[8].init(UIString(L""));
    m_messageLabels[9].init(UIString(L""));
    m_selectedLabel.init(UIString(L""));

    const unsigned int playerId = getPad();
    boost::shared_ptr<MultiplayerLocalPlayer> player = minecraft->mLocalPlayers[playerId];

    if (player != 0 && player->mViewport != 0)
        SetHudSize(GameSettings::GetGameSettings(playerId, eGameSetting_HUDSizeSplitscreen));
    else
        SetHudSize(GameSettings::GetGameSettings(playerId, eGameSetting_HUDSize));

    wchar_t* profileName = gProfile->GetMiiName(playerId);
    SetDisplayName(std::wstring(profileName));

    repositionHud();
    minecraft->mGui->clearSingleMessages();

    if (Minecraft::InMiniGame(eMiniGameId_Any, true))
        InitMainHUD();

    bool tooltipsEnabled = true;
    if (!gConsoleUIController.GetMenuDisplayed(playerId) &&
        GameSettings::GetGameSettings(playerId, eGameSetting_Tooltips) == 0)
        tooltipsEnabled = false;
    SetTooltipsEnabled(tooltipsEnabled);
}

unsigned int UIScene_HUD::getPad() { return static_cast<unsigned int>(m_playerId); }
void UIScene_HUD::SetOpacity(float opacity) { if (getFJ_Hud() != 0) getFJ_Hud()->SetAlpha(opacity); }
void UIScene_HUD::SetVisible(bool visible) { setVisible(visible); }
unsigned char UIScene_HUD::GetHudLoading() { return getFJ_Hud() == 0 ? 0 : getFJ_Hud()->GetHudLoading(); }
void UIScene_HUD::SetExpBarProgress(float progress, int) { if (getFJ_Hud() != 0) getFJ_Hud()->SetExpBarProgress(progress); }
void UIScene_HUD::SetExpLevel(int level) { if (getFJ_Hud() != 0) getFJ_Hud()->SetPlayerLevel(level); }
void UIScene_HUD::SetActiveSlot(int slot) { if (getFJ_Hud() != 0) getFJ_Hud()->SetActiveSlot(slot); }
void UIScene_HUD::SetGlideHealth(int health) { if (getFJ_Hud() != 0) getFJ_Hud()->SetGlideHealth(health); }
void UIScene_HUD::SetHealth(int health, int minimumHealth, bool highlight, bool hardcore, bool hide)
{
    if (health < minimumHealth)
        health = minimumHealth;

    if (getFJ_Hud() != 0)
        getFJ_Hud()->SetHealth(health, highlight, hardcore, hide);
}
void UIScene_HUD::SetFood(int food, int, bool highlight) { if (getFJ_Hud() != 0) getFJ_Hud()->SetFood(food, highlight); }
void UIScene_HUD::SetAir(int air, int)
{
    if (m_air != air)
    {
        m_air = air;
        if (getFJ_Hud() != 0)
            getFJ_Hud()->SetAir(air);
    }
}
void UIScene_HUD::SetArmour(int armour) { if (getFJ_Hud() != 0) getFJ_Hud()->SetArmour(armour); }
void UIScene_HUD::SetRoundNumber(unsigned int round, unsigned int total, bool visible)
{
    wchar_t text[32];

    if (visible)
        std::swprintf(text, 32, L"%u/%u", round, total);
    else
        text[0] = 0;

    m_splashLabel.setLabel(UIString(text), false, false);
}
void UIScene_HUD::ShowCrosshair(bool visible) { if (getFJ_Hud() != 0) getFJ_Hud()->ShowCrosshair(visible); }
void UIScene_HUD::SetFoodSaturationLevel(int level) { if (getFJ_Hud() != 0) getFJ_Hud()->SetFoodSaturationLevel(level); }
void UIScene_HUD::SetHorseHealth(int health, bool highlight) { if (getFJ_Hud() != 0) getFJ_Hud()->SetHorseHealth(health, highlight); }
void UIScene_HUD::SetHorseJumpBarProgress(float progress) { if (getFJ_Hud() != 0) getFJ_Hud()->SetHorseJumpBarProgress(progress); }
void UIScene_HUD::ShowDisplayName(bool visible) { if (getFJ_Hud() != 0) getFJ_Hud()->ShowGameModePlayers(visible); }
void UIScene_HUD::SetHudGameMode(int gameMode) { if (getFJ_Hud() != 0) getFJ_Hud()->SetHudGameMode(gameMode); }
void UIScene_HUD::SetGameModePlayerState(int player, eGameModePlayerState state, bool visible) { if (getFJ_Hud() != 0) getFJ_Hud()->SetGameModePlayerState(player, (int)state, visible); }
void UIScene_HUD::SetGameModeNumPlayers(int count, bool visible) { if (getFJ_Hud() != 0) getFJ_Hud()->SetGameModeNumPlayers(count, visible); }
int UIScene_HUD::GetHotBarX() { return getFJ_Hud() == 0 ? 0 : getFJ_Hud()->GetHotBarX(); }
int UIScene_HUD::GetHotBarY() { return getFJ_Hud() == 0 ? 0 : getFJ_Hud()->GetHotBarY(); }
int UIScene_HUD::GetHotBarWidth() { return getFJ_Hud() == 0 ? 0 : getFJ_Hud()->GetHotBarWidth(); }
int UIScene_HUD::GetHotBarHeight() { return getFJ_Hud() == 0 ? 0 : getFJ_Hud()->GetHotBarHeight(); }
void UIScene_HUD::ShowDirectionalDamage(int direction, int ticks, float alpha) { if (getFJ_Hud() != 0) getFJ_Hud()->ShowDirectionalDamage(direction, ticks, alpha); }
void UIScene_HUD::SetAnimatedSplashTexture(const std::wstring& texture) { if (getFJ_Hud() != 0) getFJ_Hud()->SetAnimatedSplashTexture(texture); }
void UIScene_HUD::SetAnimatedSplashIcon(int icon) { std::wstring texture; if (icon == 2) texture = L"Graphics/GameModeGraphics/AwardIcon.png"; SetAnimatedSplashTexture(texture); }
void UIScene_HUD::ShowAnimatedSplash(const std::wstring& label, bool visible, int icon)
{
    if (m_animatedSplashText != label || m_animatedSplashVisible != visible)
    {
        m_animatedSplashText = label;
        m_animatedSplashVisible = visible;
        if (icon >= 0 && m_animatedSplashIcon != icon)
        {
            SetAnimatedSplashIcon(icon);
            m_animatedSplashIcon = icon;
        }
        if (getFJ_Hud() != 0)
            getFJ_Hud()->ShowAnimatedSplash(label, visible);
    }
}
void UIScene_HUD::SetIsRightHanded(bool rightHanded) { if (getFJ_Hud() != 0) getFJ_Hud()->SetIsRightHanded(rightHanded); }
void UIScene_HUD::ShowOffhandSlot(bool visible) { if (getFJ_Hud() != 0) getFJ_Hud()->ShowOffhandSlot(visible); }
void UIScene_HUD::SetScore(const std::wstring& score) { m_healthLabel.setLabel(UIString(score), false, false); }
void UIScene_HUD::ShowScore(bool visible) { if (getFJ_Hud() != 0) getFJ_Hud()->ShowScore(visible); }
void UIScene_HUD::SetSpeed(const std::wstring& speed) { m_foodLabel.setLabel(UIString(speed), false, false); }
void UIScene_HUD::SetTime(const std::wstring& time) { m_armourLabel.setLabel(UIString(time), false, false); }
void UIScene_HUD::SetTimerIcon(int icon, int time) { if (getFJ_Hud() != 0) getFJ_Hud()->SetTimerIcon(icon, time); }
void UIScene_HUD::SetWrongWay(const std::wstring& label) { if (getFJ_Hud() != 0) getFJ_Hud()->SetWrongWayLabel(label); }
void UIScene_HUD::SetPercentageMapTraversed(int position, double percentage) { percentage = std::max(0.0, std::min(100.0, percentage)); if (getFJ_Hud() != 0) getFJ_Hud()->SetPercentageMapTraversed(position, (float)percentage); }
void UIScene_HUD::AddCheckpointAtPercentage(double percentage) { if (getFJ_Hud() != 0) getFJ_Hud()->AddCheckpointAtPercentage((float)percentage); }
void UIScene_HUD::ClearCheckpointPositions() { if (getFJ_Hud() != 0) getFJ_Hud()->ClearCheckpointPositions(); }
void UIScene_HUD::AnimateCheckpoint(unsigned int index) { if (getFJ_Hud() != 0) getFJ_Hud()->AnimateCheckpoint((int)index); }
void UIScene_HUD::SetPositions(unsigned int position, const std::wstring& label, bool visible) { if (getFJ_Hud() != 0) getFJ_Hud()->SetPosition((int)position, label, visible); }
void UIScene_HUD::SetSpeedometer(unsigned int value) { if (getFJ_Hud() != 0) getFJ_Hud()->SetSpeedometer((int)value); }
void UIScene_HUD::SetScoreIcon(unsigned int value) { if (getFJ_Hud() != 0) getFJ_Hud()->SetScoreIcon((int)value); }
unsigned int UIScene_HUD::GetStatusEffectsYOffset() { return getFJ_Hud() == 0 ? 0 : getFJ_Hud()->GetStatusEffectsYOffset(); }
void UIScene_HUD::renderPlayerHealth()
{
    ShowHealth(true);
    Minecraft* minecraft = Minecraft::GetInstance();
    boost::shared_ptr<MultiplayerLocalPlayer> player = minecraft->mLocalPlayers[getPad()];
    if (player == 0)
        return;

    SetRegenerationEffect(player->hasEffect(MobEffects::REGENERATION));

    const int blinkCycleTime = player->mInvulnerableTime;
    const int blinkActiveTime = player->mInvulnerableTime;
    const int health = (int)std::ceil(player->getHealth());
    const bool poisoned = player->hasEffect(MobEffects::POISON);
    const bool withered = player->hasEffect(MobEffects::WITHER);
    const int absorption = (int)player->getAbsorptionAmount();
    const int armour = player->getArmorValue();
    SetHealth(health, m_previousHealth,
              blinkActiveTime > 9 && (blinkCycleTime / 3) % 2 == 1,
              poisoned || withered, withered);
    m_previousHealth = health;
    SetHealthAbsorb(absorption);
    ShowArmour(armour > 0);
    if (armour > 0)
        SetArmour(armour);

    boost::shared_ptr<Entity> vehicle = player->getVehicle();
    if (vehicle == 0 || (vehicle->GetType() & EntityType::LIVING_ENTITY) != EntityType::LIVING_ENTITY)
    {
        SetRidingHorse(false, false, 0);
        ShowFood(true);
        ShowHorseHealth(false);
        FoodData* food = player->getFoodData();
        const int foodLevel = food->getFoodLevel();
        const int lastFoodLevel = food->getLastFoodLevel();
        const bool hungry = player->hasEffect(MobEffects::HUNGER);
        const int saturation = (int)food->getSaturationLevel();
        SetFood(foodLevel, lastFoodLevel, hungry);
        SetFoodSaturationLevel(saturation);
    }

    if (!player->isUnderLiquid(Material::WATER) && !player->isUnderLiquid(Material::LAVA)
        && player->getAirSupply() >= (int)player->getMaxAirSupply())
    {
        ShowAir(false);
        return;
    }

    ShowAir(true);
    const int air = (int)std::ceil((float)(player->getAirSupply() - 2) * 10.0f / 300.0f);
    const int popping = (int)std::ceil((float)player->getAirSupply() * 10.0f / 300.0f) - air;
    SetAir(air, popping);
}
void UIScene_HUD::renderVehicleHealth()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    boost::shared_ptr<MultiplayerLocalPlayer> player = minecraft->mLocalPlayers[getPad()];
    if (player == 0)
        return;
    boost::shared_ptr<Entity> vehicle = player->getVehicle();
    if (vehicle == 0 || (vehicle->GetType() & EntityType::LIVING_ENTITY) != EntityType::LIVING_ENTITY)
    {
        SetRidingHorse(false, false, 0);
        ShowHorseHealth(false);
        return;
    }
    boost::shared_ptr<LivingEntity> livingVehicle = boost::static_pointer_cast<LivingEntity>(vehicle);
    SetRidingHorse(true, player->isRidingJumpable(), (int)livingVehicle->getMaxHealth());
    SetHorseHealth((int)std::ceil(livingVehicle->getHealth()), false);
    ShowHorseHealth(true);
}
std::wstring UIScene_HUD::formatTime(long long time, bool showHours)
{
    wchar_t text[32];
    unsigned int milliseconds = (unsigned int)(time % 1000);
    unsigned int seconds = (unsigned int)((time / 1000) % 60);
    unsigned int minutes = (unsigned int)((time / 60000) % 60);

    if (showHours)
    {
        unsigned int hours = (unsigned int)(time / 3600000);
        std::swprintf(text, 32, L"%u:%02u:%02u", hours, minutes, seconds);
    }
    else if (milliseconds == 0)
    {
        std::swprintf(text, 32, L"%u:%02u", minutes, seconds);
    }
    else
    {
        std::swprintf(text, 32, L"%u:%02u.%03u", minutes, seconds, milliseconds);
    }

    return std::wstring(text);
}
void UIScene_HUD::updateGameModePlayers()
{
    const int pad = static_cast<int>(getPad());
    Minecraft* minecraft = Minecraft::GetInstance();
    const int networkPlayerCount = CGameNetworkManager::sInstance.GetPlayerCount();
    int slotStates[8];
    for (int i = 0; i < 8; ++i)
        slotStates[i] = 8;

    boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = minecraft->mLocalPlayers[pad];
    bool showNumPlayers = true;
    if (localPlayer != 0 && localPlayer->GetGameType() != GameType::LOBBY)
        showNumPlayers = false;

    bool glideLayout = false;
    if (Minecraft::InMiniGame(eMiniGameId_Glide, true) && showNumPlayers)
        glideLayout = true;

    bool showGhost = false;
    const MiniGameDef* miniGame = minecraft->GetMiniGame();
    if (miniGame->GetHostOption(eCustomGameModeHostOption_TimeTrial) != 0)
    {
        GhostController* ghostController = minecraft->GetGhostController();
        if (ghostController != 0 && !showNumPlayers && ghostController->mLoaded[pad] != 0)
        {
            if (GameSettings::GetGameSettings(pad, eGameSetting_ShowGlideGhost) != 0)
                showGhost = true;
        }
    }

    for (int index = 0; index < 8; ++index)
    {
        boost::shared_ptr<Player> player = minecraft->GetPlayerByPlayerIndex(index);
        if (player == 0)
            continue;
        if (player->isSpectator())
            continue;

        bool dead = !player->isAlive();
        if (Minecraft::InMiniGame(eMiniGameId_Any, true) && dead)
        {
            if (miniGame->GetLivesPerRound() != 0)
                dead = !dead;
        }

        if (dead)
            slotStates[player->mPlayerIndex] = 9;
        else
            slotStates[player->mPlayerIndex] = player->mPlayerIndex;
    }

    ClientMasterGameMode* gameMode = minecraft->GetClientMasterGameMode();
    SetGameModeNumPlayers(showGhost ? networkPlayerCount + 1 : networkPlayerCount, showNumPlayers);

    const int localIndex = localPlayer != 0 ? localPlayer->mPlayerIndex : 0;
    int displaySlot = 0;

    if (glideLayout)
    {
        bool visible = true;
        if (!gameMode->IsPlayerReady(static_cast<unsigned int>(localIndex)) && gameMode->IsInLobbyState())
            visible = false;
        SetGameModePlayerState(0, static_cast<eGameModePlayerState>(slotStates[localIndex]), visible);
        m_gameModePlayerSlots[localIndex] = 0;
        displaySlot = 1;
    }

    for (int group = 0; group < 4; ++group)
    {
        for (int pair = 0; pair < 2; ++pair)
        {
            const int playerIndex = group * 2 + pair;
            bool visible = true;
            if (!gameMode->IsPlayerReady(static_cast<unsigned int>(playerIndex)) && gameMode->IsInLobbyState())
                visible = false;

            boost::shared_ptr<Player> player = minecraft->GetPlayerByPlayerIndex(playerIndex);
            if (glideLayout && player != 0 && player->isSpectator())
                visible = false;

            const int state = slotStates[playerIndex];
            if (state != 9 && state != 8 && (localIndex != playerIndex || !glideLayout))
            {
                SetGameModePlayerState(displaySlot, static_cast<eGameModePlayerState>(state), visible);
                m_gameModePlayerSlots[playerIndex] = displaySlot;
                ++displaySlot;
            }
        }
    }

    for (int group = 0; group < 4; ++group)
    {
        for (int pair = 0; pair < 2; ++pair)
        {
            const int playerIndex = group * 2 + pair;
            bool visible = true;
            if (!gameMode->IsPlayerReady(static_cast<unsigned int>(playerIndex)) && gameMode->IsInLobbyState())
                visible = false;

            boost::shared_ptr<Player> player = minecraft->GetPlayerByPlayerIndex(playerIndex);
            if (glideLayout && player != 0 && player->isSpectator())
                visible = false;

            const int state = slotStates[playerIndex];
            if ((state == 9 || state == 8) && (localIndex != playerIndex || !glideLayout))
            {
                SetGameModePlayerState(displaySlot, static_cast<eGameModePlayerState>(state), visible);
                m_gameModePlayerSlots[playerIndex] = displaySlot;
                ++displaySlot;
            }
        }
    }

    if (!showGhost)
    {
        m_ghostPlayerSlot = -1;
        return;
    }

    m_ghostPlayerSlot = networkPlayerCount;
    SetGameModePlayerState(networkPlayerCount, static_cast<eGameModePlayerState>(9), true);
}
void UIScene_HUD::ClearGlideHUD(int) { ShowScore(false); ShowExpBar(false); SetHudGameMode(1); ShowGameModePlayers(true); updateGameModePlayers(); SetRoundNumber(0, 0, false); }
void UIScene_HUD::showTimedSplash(const std::wstring& label, unsigned int duration, int icon) { ShowAnimatedSplash(label, true, (int)duration); SetAnimatedSplashIcon(icon); }
void UIScene_HUD::updateFrameTick()
{
    const int pad = static_cast<int>(getPad());
    Minecraft* minecraft = Minecraft::GetInstance();
    boost::shared_ptr<MultiplayerLocalPlayer> player = minecraft->mLocalPlayers[pad];
    if (player == 0)
        return;

    MultiPlayerGameMode* gameMode = minecraft->mGameModes[pad];
    if (player->mViewport == 0)
        SetHudSize(static_cast<int>(GameSettings::GetGameSettings(pad, eGameSetting_HUDSize)));
    else
        SetHudSize(static_cast<int>(GameSettings::GetGameSettings(pad, eGameSetting_HUDSizeSplitscreen)));

    SetDisplayName(gProfile->GetMiiName(pad));

    bool tooltipsEnabled = true;
    if (gConsoleUIController.GetMenuDisplayed(pad) == 0 &&
        GameSettings::GetGameSettings(pad, eGameSetting_Tooltips) == 0)
        tooltipsEnabled = false;
    SetTooltipsEnabled(tooltipsEnabled);

    SetActiveSlot(player->mInventory->m_selected);

    not_null_ptr<ItemInstance> offhandItem = player->getOffhandItem();
    const HumanoidArm* oppositeArm = player->getMainArm()->getOpposite();
    if (!offhandItem->isEmpty())
    {
        SetIsRightHanded(oppositeArm == HumanoidArm::RIGHT);
        ShowOffhandSlot(true);
    }
    else
    {
        ShowOffhandSlot(false);
    }

    if (!player->isSpectator())
    {
        ShowCrosshair(!player->isUsingItem());
        ShowQuickSelect(true);
        if (gameMode == 0 || !gameMode->canHurtPlayer())
        {
            ShowHealth(false);
            ShowFood(false);
            ShowAir(false);
            ShowArmour(false);
            ShowExpBar(false);
            SetHealthAbsorb(0);
            renderVehicleHealth();
        }
        else
        {
            renderPlayerHealth();
            renderVehicleHealth();
        }
    }
    else
    {
        ShowGameModePlayers(true);
        ShowQuickSelect(false);
        bool showCrosshair = false;
        const MiniGameDef* miniGame = minecraft->GetMiniGame();
        if (miniGame->HasSpectatorParticipation())
        {
            if (!player->IsSpectatingOtherPlayer())
                showCrosshair = !player->isUsingItem();
        }
        ShowCrosshair(showCrosshair);
        ShowHealth(false);
        ShowFood(false);
        ShowHorseHealth(false);
        ShowAir(false);
        ShowArmour(false);
        ShowExpBar(false);
        SetRidingHorse(false, false, 0);
        SetRegenerationEffect(false);
        SetHealthAbsorb(0);
    }

    if (!player->isRidingJumpable())
    {
        if (gameMode == 0 || !gameMode->hasExperience())
        {
            ShowExpBar(false);
            SetExpLevel(0);
        }
        else
        {
            if (g_pApp->GetGameHostOption(eGameHostOption_MiniGame) == 0)
                ShowExpBar(true);
            SetExpBarProgress(player->mExperienceProgress, player->getXpNeededForNextLevel());
            SetExpLevel(player->mExperienceLevel);
        }
    }
    else
    {
        SetHorseJumpBarProgress(static_cast<float>(player->getJumpRidingScale()));
    }

    if (Minecraft::InMiniGame(eMiniGameId_Any, true))
    {
        if (!player->mDamageIndicatorAllDirections)
        {
            for (int segment = 0; segment < 8; ++segment)
            {
                if (player->mDamageIndicatorSegments[segment])
                {
                    ShowDirectionalDamage(segment, static_cast<int>(minecraft->GetMiniGame()->GetDamageIndicatorDelay()),
                                          static_cast<float>(player->getDamageTaken()));
                }
            }
        }
        else
        {
            ShowDirectionalDamage(8, static_cast<int>(minecraft->GetMiniGame()->GetDamageIndicatorDelay()),
                                  static_cast<float>(player->getDamageTaken()));
        }
    }

    if (m_selectedLabelTicks != 0)
    {
        --m_selectedLabelTicks;
        if (m_selectedLabelTicks < 0x14)
        {
            HideSelectedLabel();
            m_selectedLabelTicks = 0;
        }
    }

    unsigned int hudOpacity = GameSettings::GetGameSettings(gProfile->GetPrimaryPad(), eGameSetting_InterfaceOpacity);
    if (hudOpacity < 0x50)
    {
        if (gConsoleUIController.GetMenuDisplayed(pad) != 0 && hudOpacity < 0xf)
            hudOpacity = 0xf;
        const unsigned int opacityTimer = g_pApp->GetOpacityTimer(pad);
        if (opacityTimer == 0)
        {
            SetOpacity(static_cast<float>(hudOpacity) * 0.01f);
        }
        else if (opacityTimer < 10)
        {
            const float baseOpacity = static_cast<float>(hudOpacity) * 0.01f;
            const float fade = (10.0f - static_cast<float>(opacityTimer)) / 10.0f;
            SetOpacity(baseOpacity * (1.0f - fade * (1.0f - 0.8f)));
        }
        else
        {
            SetOpacity(0.8f);
        }
    }
    else
    {
        gConsoleUIController.GetMenuDisplayed(pad);
        SetOpacity(static_cast<float>(hudOpacity) * 0.01f);
    }

    if (g_pApp->GetGameStarted() == 0 ||
        gConsoleUIController.GetMenuDisplayed(pad) != 0 ||
        gConsoleUIController.IsSceneInStack(pad, EUIScene_ConnectingProgress) != 0 ||
        g_pApp->GetXuiAction(pad) == eXuiAction_AutoSave ||
        GameSettings::GetGameSettings(pad, eGameSetting_DisplayHUD) == 0)
    {
        SetVisible(false);
    }
    else
    {
        SetVisible(true);
    }

    if (g_pApp->GetGameHostOption(eGameHostOption_MiniGame) == 0)
    {
        m_comparisonScore = 0;
        m_personalBestTime = 0;
        SetHudGameMode(0);
        ShowGameModePlayers(false);
        ShowAnimatedSplash(L"", false, -1);
    }
    else if (!Minecraft::InMiniGame(eMiniGameId_Glide, true))
    {
        ClearGlideHUD(pad);
    }
    else
    {
        LevelGenerationOptions* levelOptions = g_pApp->getLevelGenerationOptions();
        if (levelOptions == 0 || levelOptions->getUniqueId() == 0)
        {
            ClearGlideHUD(pad);
        }
        else
        {
            MultiPlayerLevel* level = player->GetLevel();
            ClientScoreboard* scoreboard = level != 0 ? level->GetClientScoreboard() : 0;
            ClientMasterGameMode* clientGameMode = minecraft->GetClientMasterGameMode();
            const MiniGameDef* miniGame = minecraft->GetMiniGame();
            boost::shared_ptr<Player> glidePlayer = player;
            if (player->isSpectator())
            {
                boost::shared_ptr<Entity> camera = player->GetCamera();
                if (camera != 0 && (camera->GetType() & EntityType::PLAYER) == EntityType::PLAYER)
                    glidePlayer = boost::static_pointer_cast<Player>(camera);
            }

            SetHudGameMode(2);
            const int currentRound = scoreboard != 0 ? scoreboard->mCurrentRound : 0;
            SetRoundNumber(static_cast<unsigned int>(currentRound + 1), miniGame->GetRoundCount(), true);

            double speed = 0.0;
            if (glidePlayer != 0 && glidePlayer->isAlive())
            {
                speed = std::sqrt(glidePlayer->mXd * glidePlayer->mXd +
                                  glidePlayer->mYd * glidePlayer->mYd +
                                  glidePlayer->mZd * glidePlayer->mZd);
            }
            speed *= 20.0;

            std::wstring speedTemplate = L10N::GetString(0xc6565405);
            std::wstring speedValue = formatwstr(L"%.2f", speed);
            std::wstring speedText = replaceAll(speedTemplate, L"%f", speedValue);
            SetSpeed(speedText);
            SetSpeedometer(static_cast<unsigned int>((speed / 50.0f) * 12.0f + 0.5f));

            if (miniGame->ArePointsActive())
            {
                StatsUID uid = glidePlayer->getSUID();
                const int points = scoreboard->GetPoints(uid, -1);
                const int lastPoints = scoreboard->GetLastPointsAdded(uid, -1);
                SetScore(formatwstr(L"%d", points));
                ShowScore(true);
                if (lastPoints == 7)
                {
                    SetScoreIcon(3);
                    m_scoreIcon = 3;
                }
                else if (lastPoints == 5)
                {
                    SetScoreIcon(2);
                    m_scoreIcon = 2;
                }
                else
                {
                    const unsigned int icon = lastPoints == 3 ? 1U : 0U;
                    SetScoreIcon(icon);
                    m_scoreIcon = static_cast<int>(icon);
                }
            }
            else
            {
                ShowScore(false);
            }

            SetGlideHealth(static_cast<int>(std::ceil(glidePlayer->getHealth())));

            unsigned int elapsedMs = 0;
            bool showTimer = true;
            StatsUID uid = glidePlayer->getSUID();
            if (scoreboard != 0 && scoreboard->GetLifeSpan(uid, -1) != 0)
            {
                elapsedMs = static_cast<unsigned int>(scoreboard->GetLifeSpan(uid, -1));
            }
            else if (clientGameMode->IsRoundEnding())
            {
                showTimer = false;
                SetTime(L10N::GetString(IDS_DNF));
                SetTimerIcon(0, 0);
            }
            else if (clientGameMode->IsWaitingToStart() || clientGameMode->IsRoundStarting())
            {
                SetTime(formatTime(0, false));
                SetTimerIcon(0, 0);
            }
            else
            {
                const unsigned long long now = System::processTimeInMilliSecs();
                const unsigned long long start = clientGameMode->GetLocalRoundTimeStart();
                elapsedMs = static_cast<unsigned int>(now - start);
                SetTime(formatTime(static_cast<long long>(elapsedMs), true));
                SetTimerIcon(static_cast<int>((elapsedMs / 60000) % 0x3c), static_cast<int>((elapsedMs / 1000) % 0x3c));
            }

            if (showTimer && scoreboard != 0)
            {
                if (clientGameMode->IsRoundEnding())
                {
                    SetTime(L10N::GetString(IDS_DNF));
                    SetTimerIcon(0, 0);
                }
                else
                {
                    SetTime(formatTime(static_cast<long long>(elapsedMs), true));
                    SetTimerIcon(static_cast<int>((elapsedMs / 60000) % 0x3c), static_cast<int>((elapsedMs / 1000) % 0x3c));
                }
            }

            std::list<std::pair<std::wstring, int> > sortedPlayers;
            if (scoreboard != 0)
            {
                if (miniGame->ArePointsActive())
                    scoreboard->GetPlayersSortedByPoints(sortedPlayers, -1);
                else if (clientGameMode->IsWaitingToStart() || clientGameMode->IsRoundStarting())
                    scoreboard->GetPlayersSortedByPosition(sortedPlayers, currentRound - 1);
                else
                    scoreboard->GetPlayersSortedByPosition(sortedPlayers, -1);
            }

            unsigned int positionSlot = 0;
            const std::wstring emptyLabel;
            GhostController* ghostController = minecraft->GetGhostController();
            if (player->mViewport == 0 && sortedPlayers.size() < 2 &&
                miniGame->GetHostOption(eCustomGameModeHostOption_TimeTrial) != 0)
            {
                if (ghostController != 0)
                {
                    boost::shared_ptr<GhostPlayer> ghostPlayer = ghostController->getGhostPlayer(pad);
                    GhostController::GhostRecord* ghostRecord = ghostController->mLoaded[pad];
                    const bool hasLifeSpan = scoreboard != 0 && scoreboard->GetLifeSpan(uid, -1) != 0;
                    if (ghostRecord != 0 &&
                        (ghostPlayer != 0 || hasLifeSpan) &&
                        (ghostController->mRecordingSlotFlags2[pad] ||
                         ghostController->HasFinished(ghostRecord)))
                    {
                        int comparisonDelta = 0;
                        if (sortedPlayers.size() <= 1 || !hasLifeSpan)
                        {
                            comparisonDelta = static_cast<int>(elapsedMs - ghostRecord->mElapsed);
                            m_comparisonScore = static_cast<unsigned int>(ghostRecord->mElapsed);
                        }
                        else if (!sortedPlayers.empty())
                        {
                            comparisonDelta = sortedPlayers.front().second - static_cast<int>(m_comparisonScore);
                        }

                        bool ahead = false;
                        if (ghostPlayer != 0)
                        {
                            const double ghostProgress = ghostPlayer->getProgress();
                            const double playerProgress =
                                clientGameMode->GetPlayerMapProgress(glidePlayer->mXuid);
                            ahead = playerProgress < ghostProgress;
                        }
                        if (comparisonDelta > 0)
                            ahead = true;

                        std::wstring positionLabel = emptyLabel;
                        if (ahead)
                        {
                            positionLabel = formatwstr(L"%ls %ls", formatTime(m_comparisonScore, true).c_str(),
                                                       L10N::GetString(0x72cede7d));
                        }
                        else if (comparisonDelta < 0)
                        {
                            positionLabel = formatwstr(L"- %ls %ls", formatTime(-comparisonDelta, false).c_str(),
                                                       player->getDisplayName().c_str());
                        }
                        else
                        {
                            positionLabel = formatwstr(L"+ %ls %ls", formatTime(comparisonDelta, true).c_str(),
                                                       player->getDisplayName().c_str());
                        }
                        SetPositions(0, positionLabel, true);
                        if (ahead)
                        {
                            if (comparisonDelta < 0)
                                SetPositions(1, formatwstr(L"- %ls %ls", formatTime(-comparisonDelta, false).c_str(),
                                                             player->getDisplayName().c_str()), true);
                            else
                                SetPositions(1, formatwstr(L"+ %ls %ls", formatTime(comparisonDelta, true).c_str(),
                                                             player->getDisplayName().c_str()), true);
                        }
                        else
                        {
                            SetPositions(1, formatwstr(L"%ls %ls", formatTime(m_comparisonScore, true).c_str(),
                                                       L10N::GetString(0x72cede7d)), true);
                        }
                        positionSlot = 2;
                    }
                }
            }
            else if (miniGame->ArePointsActive() && ghostController != 0)
            {
                const int lifeSpan = scoreboard != 0 ? scoreboard->GetLifeSpan(uid, -1) : 0;
                unsigned int bestScore = m_comparisonScore;
                if (lifeSpan == 0)
                {
                    bestScore = ghostController->GetBestScore(pad);
                    m_comparisonScore = bestScore;
                }
                if (bestScore != 0)
                {
                    if (sortedPlayers.size() <= 1)
                    {
                        SetPositions(0, formatwstr(L"%d %ls", bestScore, L10N::GetString(0x72cede7d)), true);
                    }
                    else if (static_cast<unsigned int>(sortedPlayers.front().second) > bestScore)
                    {
                        SetPositions(0, formatwstr(L"%d %ls", sortedPlayers.front().second,
                                                   sortedPlayers.front().first.c_str()), true);
                        SetPositions(1, formatwstr(L"%d %ls", bestScore, L10N::GetString(0x72cede7d)), true);
                    }
                    else
                    {
                        SetPositions(0, formatwstr(L"%d %ls", bestScore, L10N::GetString(0x72cede7d)), true);
                        SetPositions(1, formatwstr(L"%d %ls", sortedPlayers.front().second,
                                                     sortedPlayers.front().first.c_str()), true);
                    }
                    positionSlot = 2;
                }
            }
            else
            {
                int leaderValue = 0;
                for (std::list<std::pair<std::wstring, int> >::iterator it = sortedPlayers.begin();
                     it != sortedPlayers.end() && positionSlot < 3; ++it)
                {
                    std::wstring label = emptyLabel;
                    if (miniGame->ArePointsActive())
                        label = formatwstr(L"%d %ls", it->second, it->first.c_str());
                    else if (positionSlot == 0)
                        label = formatwstr(L"%ls %ls", formatTime(it->second, true).c_str(), it->first.c_str());
                    else
                        label = formatwstr(L"+ %ls %ls", formatTime(it->second - leaderValue, true).c_str(), it->first.c_str());
                    SetPositions(positionSlot, label, true);
                    leaderValue = it->second;
                    ++positionSlot;
                }
            }

            while (positionSlot < 3)
            {
                SetPositions(positionSlot, emptyLabel, false);
                ++positionSlot;
            }

            if (g_pApp->getLevelGenerationOptions() == 0)
            {
                ClearGlideHUD(pad);
                m_levelGenerationId = static_cast<unsigned int>(-1);
            }
            else if (!m_checkpointsInitialised)
            {
                const double totalDistance = clientGameMode->getTotalDistance();
                std::vector<double>* checkpointDistances = clientGameMode->getCheckpointDistances();
                double accumulated = 0.0;
                if (checkpointDistances != 0)
                {
                    for (std::vector<double>::iterator it = checkpointDistances->begin() + 1;
                         it != checkpointDistances->end(); ++it)
                    {
                        accumulated += *it;
                        AddCheckpointAtPercentage((accumulated / totalDistance) * 100.0);
                    }
                }
                m_checkpointsInitialised = true;
                m_levelGenerationId = g_pApp->getLevelGenerationOptions()->getUniqueId();
            }
            else
            {
                const unsigned int uniqueId = g_pApp->getLevelGenerationOptions()->getUniqueId();
                if (uniqueId != m_levelGenerationId)
                {
                    ClearGlideHUD(pad);
                }
            }

            if (clientGameMode->IsInActiveRoundState() && scoreboard != 0)
            {
                StatsUID checkpointUid = player->getSUID();
                const int checkpointId = scoreboard->GetLastBankedCheckpointID(checkpointUid, -1);
                if (checkpointId > m_lastCheckpointIds[pad])
                {
                    m_lastCheckpointIds[pad] = checkpointId;
                    const int spawnableId = clientGameMode->convertCheckpointIDToSpawnableID(checkpointId);
                    AnimateCheckpoint(static_cast<unsigned int>(spawnableId));
                    updateGameModePlayers();
                }
            }
            else
            {
                updateGameModePlayers();
            }

            if (!gConsoleUIController.IsPauseMenuDisplayed(pad) && !gConsoleUIController.GetMenuDisplayed(pad) &&
                !player->isSpectator())
            {
                const double progress = clientGameMode->GetPlayerMapProgress(player->mXuid);
                if (!clientGameMode->IsWaitingToStart() && !clientGameMode->IsRoundStarting() &&
                    scoreboard != 0 && scoreboard->GetLifeSpan(uid, -1) == 0)
                {
                    if (progress <= player->GetPercentageTraversed())
                    {
                        m_wrongWayTicks = 0;
                        SetWrongWay(L"");
                        m_wrongWayVisible = false;
                    }
                    else if (player->isCreative())
                    {
                        ++m_wrongWayTicks;
                        if (m_wrongWayTicks < 0x3d)
                            SetWrongWay(L"");
                        else if (!m_wrongWayVisible)
                        {
                            SetWrongWay(L10N::GetString(0xe39432b1));
                            m_wrongWayVisible = true;
                        }
                    }
                    else
                    {
                        m_wrongWayTicks = 0;
                        SetWrongWay(L"");
                        m_wrongWayVisible = false;
                    }
                }
                else
                {
                    m_wrongWayTicks = 0;
                    SetWrongWay(L"");
                    m_wrongWayVisible = false;
                }
                player->SetPercentageTraversed(progress);
            }

            for (int index = 0; index < 8; ++index)
            {
                boost::shared_ptr<Player> trackedPlayer = minecraft->GetPlayerByPlayerIndex(index);
                if (trackedPlayer == 0)
                    continue;
                const double mapProgress = clientGameMode->GetPlayerMapProgress(trackedPlayer->mXuid);
                SetPercentageMapTraversed(m_gameModePlayerSlots[index], mapProgress);
            }

            if (m_ghostPlayerSlot < 0 || ghostController == 0)
            {
                SetPercentageMapTraversed(m_ghostPlayerSlot, 0.0);
            }
            else
            {
                boost::shared_ptr<GhostPlayer> ghostPlayer = ghostController->getGhostPlayer(pad);
                if (ghostPlayer != 0)
                    SetPercentageMapTraversed(m_ghostPlayerSlot, ghostPlayer->getProgress());
                else
                    SetPercentageMapTraversed(m_ghostPlayerSlot, 0.0);
            }
        }
    }

    ClientScoreboard* scoreboard = 0;
    MultiPlayerLevel* level = player->GetLevel();
    if (level != 0)
        scoreboard = level->GetClientScoreboard();
    ClientMasterGameMode* clientGameMode = minecraft->GetClientMasterGameMode();
    LevelGenerationOptions* levelOptions = g_pApp->getLevelGenerationOptions();
    if (levelOptions == 0 || levelOptions->getUniqueId() == 0)
    {
        m_comparisonScore = 0;
        m_personalBestTime = 0;
        ShowAnimatedSplash(L"", false, -1);
        m_resultSplashShown = false;
    }
    else if (clientGameMode->IsRoundStarting())
    {
        if (Minecraft::InMiniGame(eMiniGameId_Glide, true) && scoreboard != 0)
        {
            const MiniGameDef* miniGame = minecraft->GetMiniGame();
            if (miniGame->GetHostOption(eCustomGameModeHostOption_TimeTrial) == 0)
            {
                const int currentRound = scoreboard->mCurrentRound;
                if (currentRound == static_cast<int>(miniGame->GetFinalRoundIndex()))
                    ShowAnimatedSplash(L10N::GetString(0x3f46d5d3), true, 0);
                else
                {
                    std::wstring splash = L10N::GetString(0x91a1400c);
                    splash = replaceAll(splash, L"%d", formatwstr(L"%d", currentRound + 1));
                    ShowAnimatedSplash(splash, true, 0);
                }
            }
            else
            {
                ShowAnimatedSplash(L10N::GetString(0xab5ed6f8), true, 0);
            }
        }
        m_resultSplashShown = false;
    }
    else if (clientGameMode->IsRoundEnding())
    {
        if (!Minecraft::InMiniGame(eMiniGameId_Glide, true))
        {
            if (scoreboard == 0 || !scoreboard->HasWinner(-1))
                ShowAnimatedSplash(L10N::GetString(0xd2622bfa), true, 0);
            else
            {
                std::wstring splash = L10N::GetString(IDS_MG_PLAYER_WON);
                splash = replaceAll(splash, L"{*PLAYER*}", scoreboard->GetWinnerName(-1));
                ShowAnimatedSplash(splash, true, 0);
            }
        }
        else if (scoreboard != 0)
        {
            StatsUID uid = player->getSUID();
            const MiniGameDef* miniGame = minecraft->GetMiniGame();
            if (miniGame->GetHostOption(eCustomGameModeHostOption_TimeTrial) == 0)
            {
                if (scoreboard->mCurrentRound != static_cast<int>(miniGame->GetFinalRoundIndex()))
                {
                    if (scoreboard->GetLifeSpan(uid, -1) == 0)
                        ShowAnimatedSplash(L10N::GetString(IDS_DNF), true, 1);
                    else
                        showTimedSplash(formatwstr(L"%02d:%02d.%03d",
                                                   scoreboard->GetLifeSpan(uid, -1) / 60000,
                                                   (scoreboard->GetLifeSpan(uid, -1) / 1000) % 0x3c,
                                                   scoreboard->GetLifeSpan(uid, -1) % 1000),
                                        0x3c, 2);
                }
                else if (!scoreboard->GlideHasOverallWinner())
                    ShowAnimatedSplash(L10N::GetString(0xd2622bfa), true, 1);
                else
                {
                    std::wstring splash = L10N::GetString(IDS_MG_PLAYER_WON);
                    splash = replaceAll(splash, L"{*PLAYER*}", scoreboard->GetGlideOverallWinner(miniGame->ArePointsActive()));
                    ShowAnimatedSplash(splash, true, 0);
                }
            }
            else if (miniGame->ArePointsActive())
            {
                showTimedSplash(formatwstr(L"%d", scoreboard->GetPoints(uid, -1)), 0x3c, 2);
            }
            else if (scoreboard->GetLifeSpan(uid, -1) == 0)
            {
                ShowAnimatedSplash(L10N::GetString(IDS_DNF), true, 1);
            }
            else
            {
                const int lifeSpan = scoreboard->GetLifeSpan(uid, -1);
                showTimedSplash(formatwstr(L"%02d:%02d.%03d", lifeSpan / 60000, (lifeSpan / 1000) % 0x3c, lifeSpan % 1000),
                                0x3c, 2);
            }
        }
        m_resultSplashShown = true;
    }
    else
    {
        GhostController* ghostController = minecraft->GetGhostController();
        if (Minecraft::InMiniGame(eMiniGameId_Glide, true) && scoreboard != 0 && !player->isSpectator() &&
            scoreboard->GetLifeSpan(player->getSUID(), -1) != 0 &&
            (ghostController == 0 || !ghostController->mRecordingActive[pad]))
        {
            StatsUID uid = player->getSUID();
            const MiniGameDef* miniGame = minecraft->GetMiniGame();
            if (!miniGame->ArePointsActive())
            {
                if (!scoreboard->IsPersonalBest(uid, -1))
                {
                    const int lifeSpan = scoreboard->GetLifeSpan(uid, -1);
                    showTimedSplash(formatwstr(L"%02d:%02d.%03d", lifeSpan / 60000, (lifeSpan / 1000) % 0x3c, lifeSpan % 1000),
                                    0x3c, 2);
                    m_resultSplashShown = true;
                    m_displayFlag = true;
                }
                else if (m_personalBestTime == 0)
                {
                    m_personalBestTime = System::processTimeInMilliSecs();
                }
                else if (System::processTimeInMilliSecs() >= m_personalBestTime + 3000)
                {
                    const int lifeSpan = scoreboard->GetLifeSpan(uid, -1);
                    showTimedSplash(formatwstr(L"%02d:%02d.%03d", lifeSpan / 60000, (lifeSpan / 1000) % 0x3c, lifeSpan % 1000),
                                    0x3c, 2);
                    m_resultSplashShown = true;
                    m_displayFlag = true;
                }
                else
                {
                    showTimedSplash(L10N::GetString(0x58973d2d), 0x3c, 2);
                    m_resultSplashShown = true;
                    m_displayFlag = true;
                }
            }
            else if (!scoreboard->IsPersonalBest(uid, -1))
            {
                showTimedSplash(formatwstr(L"%d", scoreboard->GetPoints(uid, -1)), 0x3c, 2);
                m_resultSplashShown = true;
                m_displayFlag = true;
            }
            else if (m_personalBestTime == 0)
            {
                m_personalBestTime = System::processTimeInMilliSecs();
            }
            else if (System::processTimeInMilliSecs() >= m_personalBestTime + 3000)
            {
                showTimedSplash(formatwstr(L"%d", scoreboard->GetPoints(uid, -1)), 0x3c, 2);
                m_resultSplashShown = true;
                m_displayFlag = true;
            }
            else
            {
                showTimedSplash(L10N::GetString(0x58973d2d), 0x3c, 2);
                m_resultSplashShown = true;
                m_displayFlag = true;
            }
        }
        else
        {
            m_comparisonScore = 0;
            m_personalBestTime = 0;
            if (m_timedSplashTicks == 0)
                ShowAnimatedSplash(L"", false, -1);
            else
            {
                ShowAnimatedSplash(m_timedSplashText, true, m_animatedSplashIcon);
                --m_timedSplashTicks;
            }
        }
    }
}
void UIScene_HUD::handleGameTick()
{
    if (m_movie == 0 || g_pApp->GetGameStarted() == 0)
        return;

    Minecraft* minecraft = Minecraft::GetInstance();
    const int playerId = m_playerId;
    if (minecraft->mLocalPlayers[playerId] && minecraft->mGameModes[playerId])
    {
        m_layer->showComponent(playerId, EUIScene_HUD, true);
        updateFrameTick();
        GetHudLoading();
    }
    else
    {
        m_layer->showComponent(playerId, EUIScene_HUD, false);
    }
}
void UIScene_HUD::render(int width, int height, C4JRender::eViewportType viewport)
{
    if (!m_splitScreen)
    {
        UIScene::render(width, height, viewport);
        return;
    }

    int screenWidth;
    int screenHeight;
    gConsoleUIController.getRenderDimensions(C4JRender::eViewportType_FullScreen,
                                             screenWidth, screenHeight);

    int renderWidth = width;
    int renderHeight = height;
    int movieY = 0;
    switch (viewport)
    {
    case C4JRender::eViewportType_TopHalf:
        gConsoleUIController.setupRenderPosition(0, 0);
        renderWidth = screenWidth;
        movieY = m_stageHeight / 2;
        break;
    case C4JRender::eViewportType_BottomHalf:
        gConsoleUIController.setupRenderPosition(0, screenHeight / 2);
        renderWidth = screenWidth;
        movieY = m_stageHeight / 2;
        break;
    case C4JRender::eViewportType_LeftHalf:
        gConsoleUIController.setupRenderPosition(0, 0);
        renderHeight = screenHeight;
        break;
    case C4JRender::eViewportType_RightHalf:
        gConsoleUIController.setupRenderPosition(screenWidth / 2, 0);
        renderHeight = screenHeight;
        break;
    case C4JRender::eViewportType_TopLeftQuarter:
        gConsoleUIController.setupRenderPosition(0, 0);
        movieY = m_stageHeight / 2;
        break;
    case C4JRender::eViewportType_TopRightQuarter:
        gConsoleUIController.setupRenderPosition(screenWidth / 2, 0);
        movieY = m_stageHeight / 2;
        break;
    case C4JRender::eViewportType_BottomLeftQuarter:
        gConsoleUIController.setupRenderPosition(0, screenHeight / 2);
        movieY = m_stageHeight / 2;
        break;
    case C4JRender::eViewportType_BottomRightQuarter:
        gConsoleUIController.setupRenderPosition(screenWidth / 2, screenHeight / 2);
        movieY = m_stageHeight / 2;
        break;
    default:
        gConsoleUIController.setupRenderPosition(0, 0);
        break;
    }

    gFui->render(m_movie, 0.0f, static_cast<float>(movieY),
                 static_cast<float>(renderWidth), static_cast<float>(renderHeight));
}
void UIScene_HUD::updateViewportTouchOffset()
{
    const float half = 0.5f;
    switch (m_layer->getViewport())
    {
    case 1:
    case 5:
        gConsoleUIController.updateViewportTouchOffset(0, -(int)(gConsoleUIController.m_screenHeight * half));
        break;
    case 6:
        gConsoleUIController.updateViewportTouchOffset((int)(gConsoleUIController.m_screenWidth * half),
                                                       -(int)(gConsoleUIController.m_screenHeight * half));
        break;
    case 4:
    case 8:
        gConsoleUIController.updateViewportTouchOffset((int)(gConsoleUIController.m_screenWidth * half), 0);
        break;
    default:
        gConsoleUIController.updateViewportTouchOffset(0, 0);
        break;
    }
}
void UIScene_HUD::handleTimerComplete(int)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    const int playerId = m_playerId;
    Gui* gui = minecraft->mGui;
    if (minecraft->mLocalPlayers[playerId] == 0 || gui == 0)
    {
        for (int i = 0; i < 10; ++i)
        {
            m_messageControls[i].setOpacity(0.0f);
            m_messageLabels[i].setOpacity(0.0f);
            m_messageLabels[i].setLabel(UIString(L""), false, false);
        }
        m_selectedLabel.setOpacity(0.0f);
        return;
    }

    for (int i = 0; i < 10; ++i)
    {
        const float opacity = static_cast<float>(gui->getOpacity(playerId, static_cast<unsigned int>(i)));
        m_messageControls[i].setOpacity(opacity);
        m_messageLabels[i].setOpacity(opacity);
        if (opacity > 0.0f && static_cast<size_t>(i) < gui->mMessages[playerId].size())
            m_messageLabels[i].setLabel(UIString(escapeXML(gui->mMessages[playerId][i].message)), false, false);
        else
            m_messageLabels[i].setLabel(UIString(L""), false, false);
    }

    m_selectedLabel.setOpacity(static_cast<float>(gui->getJukeboxOpacity(playerId)));
    m_selectedLabel.setLabel(UIString(gui->mSingleMessages[playerId]), false, false);
}
int UIScene_HUD::getSceneType() { return EUIScene_HUD; }
bool UIScene_HUD::stealsFocus() { return false; }
bool UIScene_HUD::hasFocus(int) { return false; }
bool UIScene_HUD::hidesLowerScenes() { return false; }
