#include "game/ui/UIScene_HUD.hpp"

#include <algorithm>
#include <cmath>
#include <cwchar>
#include <list>
#include <utility>
#include <vector>

#include "game/core/ClientMasterGameMode.hpp"
#include "game/core/ClientScoreboard.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/FoodData.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/GhostController.hpp"
#include "game/core/HumanoidArm.hpp"
#include "game/core/L10N.hpp"
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
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/generation/GhostPlayer.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"
#include "platform/wiiu/CProfile.hpp"

namespace {

int GetScoreboardCurrentRound(ClientScoreboard* scoreboard)
{
    return *reinterpret_cast<int*>(reinterpret_cast<char*>(scoreboard) + 0x30);
}

GhostController::GhostRecord* GetGhostLoadedRecord(GhostController* controller, int pad)
{
    return *reinterpret_cast<GhostController::GhostRecord**>(
        reinterpret_cast<char*>(controller) + 0xd4 + pad * 4);
}

bool GetGhostRecordingSlotFlag1(GhostController* controller, int pad)
{
    return *reinterpret_cast<bool*>(reinterpret_cast<char*>(controller) + 0x81 + pad);
}

bool GetGhostRecordingSlotFlag2(GhostController* controller, int pad)
{
    return *reinterpret_cast<bool*>(reinterpret_cast<char*>(controller) + 0x78 + pad);
}

bool GetDirectionalDamageActive(LocalPlayer* player, int index)
{
    return *reinterpret_cast<bool*>(reinterpret_cast<char*>(player) + 0x9ac + index);
}

bool GetSplitScreenDamageFlag(LocalPlayer* player)
{
    return *reinterpret_cast<bool*>(reinterpret_cast<char*>(player) + 0x9b4);
}

}

UIScene_HUD::UIScene_HUD(int sceneType, void*, UILayer* layer)
    : UIScene(sceneType, layer), m_splitScreen(false), m_animatedSplashIcon(-1),
      m_hudSize(-1), m_air(10), m_hudGameMode(-1), m_selectedLabelTicks(0),
      m_timedSplashTicks(0), m_ridingHorse(true), m_displayNameVisible(true),
      m_scoreIcon(0), m_displayFlag(true), m_animatedSplashVisible(true),
      m_resultSplashShown(false), m_timedSplashLocked(false), m_checkpointsInitialised(false),
      m_wrongWayTicks(0), m_wrongWayVisible(false), m_levelGenerationId(-1),
      m_previousHealth(0), m_comparisonScore(0), m_personalBestTime(0)
{
    for (int i = 0; i < 4; ++i)
        m_previousArmorIcons[i] = m_armorIcons[i] = !HideMainHUD();
    m_healthVisible = !HideMainHUD();
    m_foodVisible = !HideMainHUD();
    m_airVisible = !HideMainHUD();
    m_armourVisible = !HideMainHUD();
    m_quickSelectVisible = !HideMainHUD();
    for (int i = 0; i < 4; ++i)
        m_lastCheckpointIds[i] = 0;
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
    return 0;
}

void UIScene_HUD::ShowArmorIcons(int armour, bool visible)
{
    if (getFJ_Hud() != 0) getFJ_Hud()->ShowArmorIcons(armour, visible);
}

bool UIScene_HUD::SORTBYTIMELEFT(MobEffectInstance* first, MobEffectInstance* second)
{
    return second->getDuration() > first->getDuration();
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
    UIScene::tick();
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
    return std::wstring(L"hud");
}

void UIScene_HUD::customDraw(const char* name, fuiRect* rect)
{
    UIScene::customDraw(name, rect);
}

void UIScene_HUD::SetHudSize(int size)
{
    if (getFJ_Hud() != 0) getFJ_Hud()->LoadHud(size);
}

void UIScene_HUD::SetDisplayName(const std::wstring& name)
{
    if (getFJ_Hud() != 0) getFJ_Hud()->SetGamertag(name);
}

void UIScene_HUD::repositionHud()
{
    if (getFJ_Hud() != 0) getFJ_Hud()->RepositionHud(0, 0);
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
    m_armorIcons[0] = visible;
    m_armorIcons[1] = visible;
    m_armorIcons[2] = visible;
    m_armorIcons[3] = visible;
    m_previousArmorIcons[0] = visible;
    m_previousArmorIcons[1] = visible;
    m_previousArmorIcons[2] = visible;
    m_previousArmorIcons[3] = visible;
}
void UIScene_HUD::SetTooltipsEnabled(bool enabled) { if (getFJ_Hud() != 0) getFJ_Hud()->SetTooltipsEnabled(enabled); }
void UIScene_HUD::handleReload()
{
    m_quickSelectVisible = true;
    UIScene::handleReload();
}

unsigned int UIScene_HUD::getPad() { return static_cast<unsigned int>(m_playerId); }
void UIScene_HUD::SetOpacity(float opacity) { if (getFJ_Hud() != 0) getFJ_Hud()->SetAlpha(opacity); }
void UIScene_HUD::SetVisible(bool visible) { if (getFJ_Hud() != 0) getFJ_Hud()->ShowHealth(visible); }
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
void UIScene_HUD::SetAir(int air, int) { if (getFJ_Hud() != 0) getFJ_Hud()->SetAir(air); }
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
void UIScene_HUD::ShowAnimatedSplash(const std::wstring& label, bool visible, int icon) { if (icon >= 0) SetAnimatedSplashIcon(icon); if (getFJ_Hud() != 0) getFJ_Hud()->ShowAnimatedSplash(label, visible); }
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

    const int health = (int)std::ceil(player->getHealth());
    const int armour = player->getArmorValue();
    SetHealth(health, m_previousHealth, health < m_previousHealth, false, armour == 0 && health == 0);
    m_previousHealth = health;
    SetHealthAbsorb((int)player->getAbsorptionAmount());
    ShowArmour(armour > 0);
    if (armour > 0)
        SetArmour(armour);

    boost::shared_ptr<Entity> vehicle = player->getVehicle();
    if (vehicle == 0 || (vehicle->GetType() & ENTITY_TYPE_LIVING_ENTITY) != ENTITY_TYPE_LIVING_ENTITY)
    {
        SetRidingHorse(false, false, 0);
        ShowFood(true);
        ShowHorseHealth(false);
        FoodData* food = player->getFoodData();
        SetFood(food->getFoodLevel(), food->getLastFoodLevel(), false);
        SetFoodSaturationLevel((int)food->getSaturationLevel());
    }
}
void UIScene_HUD::renderVehicleHealth()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    boost::shared_ptr<MultiplayerLocalPlayer> player = minecraft->mLocalPlayers[getPad()];
    if (player == 0)
        return;
    boost::shared_ptr<Entity> vehicle = player->getVehicle();
    if (vehicle == 0 || (vehicle->GetType() & ENTITY_TYPE_LIVING_ENTITY) != ENTITY_TYPE_LIVING_ENTITY)
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
    if (localPlayer != 0)
    {
        LevelGenerationOptions* options = g_pApp->getLevelGenerationOptions();
        if (options != 0 && options->getUniqueId() != 0)
            showNumPlayers = false;
    }

    bool glideLayout = false;
    if (Minecraft::InMiniGame(eMiniGameId_Glide, true) && showNumPlayers)
        glideLayout = true;

    bool showGhost = false;
    const MiniGameDef* miniGame = minecraft->GetMiniGame();
    if (miniGame->GetHostOption(eCustomGameModeHostOption_TimeTrial) != 0)
    {
        GhostController* ghostController = minecraft->GetGhostController();
        if (ghostController != 0 && !showNumPlayers && GetGhostLoadedRecord(ghostController, pad) != 0)
        {
            if (GameSettings::GetGameSettings(pad, static_cast<eGameSetting>(0x2a)) != 0)
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
        SetHudSize(static_cast<int>(GameSettings::GetGameSettings(pad, static_cast<eGameSetting>(0x1b))));
    else
        SetHudSize(static_cast<int>(GameSettings::GetGameSettings(pad, static_cast<eGameSetting>(0x1c))));

    if (gProfile != 0)
        SetDisplayName(gProfile->GetGamertag(pad));

    bool tooltipsEnabled = true;
    if (gConsoleUIController.GetMenuDisplayed(pad) == 0 &&
        GameSettings::GetGameSettings(pad, static_cast<eGameSetting>(0x11)) == 0)
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
            if (g_pApp->GetGameHostOption(static_cast<eGameHostOption>(0x1f)) == 0)
                ShowExpBar(true);
            SetExpBarProgress(player->mExperienceProgress, player->getXpNeededForNextLevel());
            SetExpLevel(player->mExperienceLevel);
        }
    }
    else
    {
        SetHorseJumpBarProgress(static_cast<float>(player->getJumpRidingScale()));
    }

    if (Minecraft::InMiniGame(eMiniGameId_Any, true) && !GetSplitScreenDamageFlag(player.get()))
    {
        const MiniGameDef* miniGame = minecraft->GetMiniGame();
        const int delay = static_cast<int>(miniGame->GetDamageIndicatorDelay());
        for (int index = 0; index < 4; ++index)
        {
            boost::shared_ptr<MultiplayerLocalPlayer> local = minecraft->mLocalPlayers[index];
            if (local == 0)
                continue;
            if (GetDirectionalDamageActive(local.get(), index * 2))
                ShowDirectionalDamage(index * 2, delay, static_cast<float>(local->getDamageTaken()));
            if (GetDirectionalDamageActive(local.get(), index * 2 + 1))
                ShowDirectionalDamage(index * 2 + 1, delay, static_cast<float>(local->getDamageTaken()));
        }
    }
    else if (Minecraft::InMiniGame(eMiniGameId_Any, true) && GetSplitScreenDamageFlag(player.get()))
    {
        const MiniGameDef* miniGame = minecraft->GetMiniGame();
        const int delay = static_cast<int>(miniGame->GetDamageIndicatorDelay());
        ShowDirectionalDamage(8, delay, static_cast<float>(player->getDamageTaken()));
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

    unsigned int hudOpacity = GameSettings::GetGameSettings(pad, static_cast<eGameSetting>(0x10));
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
        gConsoleUIController.IsSceneInStack(pad, static_cast<EUIScene>(0x1f)) != 0 ||
        g_pApp->GetXuiAction(pad) == 0x1f ||
        GameSettings::GetGameSettings(pad, static_cast<eGameSetting>(0x17)) == 0)
    {
        SetVisible(false);
    }
    else
    {
        SetVisible(true);
    }

    if (g_pApp->GetGameHostOption(static_cast<eGameHostOption>(0x1f)) == 0)
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
                if (camera != 0 && (camera->GetType() & 0x0c800000ULL) == 0x0c800000ULL)
                    glidePlayer = boost::static_pointer_cast<Player>(camera);
            }

            SetHudGameMode(2);
            const int currentRound = scoreboard != 0 ? GetScoreboardCurrentRound(scoreboard) : 0;
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
                SetTime(L10N::GetString(0xf62114b0));
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
                    SetTime(L10N::GetString(0xf62114b0));
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
                    GhostController::GhostRecord* ghostRecord = GetGhostLoadedRecord(ghostController, pad);
                    const bool hasLifeSpan = scoreboard != 0 && scoreboard->GetLifeSpan(uid, -1) != 0;
                    if (ghostRecord != 0 &&
                        (ghostPlayer != 0 || hasLifeSpan) &&
                        (GetGhostRecordingSlotFlag1(ghostController, pad) ||
                         ghostController->HasFinished(ghostRecord)))
                    {
                        int comparisonDelta = 0;
                        if (sortedPlayers.size() <= 1 || !hasLifeSpan)
                        {
                            comparisonDelta = static_cast<int>(elapsedMs) - ghostRecord->time;
                            m_comparisonScore = static_cast<unsigned int>(ghostRecord->time);
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
                const int currentRound = GetScoreboardCurrentRound(scoreboard);
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
                std::wstring splash = L10N::GetString(0xa2a6c763);
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
                if (GetScoreboardCurrentRound(scoreboard) != static_cast<int>(miniGame->GetFinalRoundIndex()))
                {
                    if (scoreboard->GetLifeSpan(uid, -1) == 0)
                        ShowAnimatedSplash(L10N::GetString(0xf62114b0), true, 1);
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
                    std::wstring splash = L10N::GetString(0xa2a6c763);
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
                ShowAnimatedSplash(L10N::GetString(0xf62114b0), true, 1);
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
            (ghostController == 0 || !GetGhostRecordingSlotFlag2(ghostController, pad)))
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
        m_layer->showComponent(playerId, static_cast<EUIScene>(0x31), true);
        updateFrameTick();
        GetHudLoading();
    }
    else
    {
        m_layer->showComponent(playerId, static_cast<EUIScene>(0x31), false);
    }
}
void UIScene_HUD::render(int width, int height, C4JRender::eViewportType viewport) { UIScene::render(width, height, viewport); }
void UIScene_HUD::updateViewportTouchOffset() { UIScene::updateViewportTouchOffset(); }
void UIScene_HUD::handleTimerComplete(int timerId) { UIScene::handleTimerComplete(timerId); }
int UIScene_HUD::getSceneType() { return 0x31; }
bool UIScene_HUD::stealsFocus() { return false; }
bool UIScene_HUD::hasFocus(int) { return false; }
bool UIScene_HUD::hidesLowerScenes() { return false; }
