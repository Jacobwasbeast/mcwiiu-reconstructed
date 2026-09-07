#include "game/ui/UIScene_HUD.hpp"

#include <algorithm>
#include <cwchar>

#include "game/core/ClientMasterGameMode.hpp"
#include "game/core/Minecraft.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/ui/FJ_Hud.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"

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
    return first < second;
}

void UIScene_HUD::updateEffectsDisplay()
{
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
void UIScene_HUD::InitMainHUD() { }
void UIScene_HUD::SetTooltipsEnabled(bool enabled) { if (getFJ_Hud() != 0) getFJ_Hud()->SetTooltipsEnabled(enabled); }
void UIScene_HUD::handleReload()
{
    m_quickSelectVisible = true;
    UIScene::handleReload();
}

unsigned int UIScene_HUD::getPad() { return 0; }
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
void UIScene_HUD::renderPlayerHealth() { }
void UIScene_HUD::renderVehicleHealth() { }
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
void UIScene_HUD::updateGameModePlayers() { }
void UIScene_HUD::ClearGlideHUD(int) { ShowScore(false); ShowExpBar(false); SetHudGameMode(1); ShowGameModePlayers(true); updateGameModePlayers(); SetRoundNumber(0, 0, false); }
void UIScene_HUD::showTimedSplash(const std::wstring& label, unsigned int duration, int icon) { ShowAnimatedSplash(label, true, (int)duration); SetAnimatedSplashIcon(icon); }
void UIScene_HUD::updateFrameTick() { }
void UIScene_HUD::handleGameTick() { }
void UIScene_HUD::render(int width, int height, C4JRender::eViewportType viewport) { UIScene::render(width, height, viewport); }
void UIScene_HUD::updateViewportTouchOffset() { UIScene::updateViewportTouchOffset(); }
void UIScene_HUD::handleTimerComplete(int timerId) { UIScene::handleTimerComplete(timerId); }
int UIScene_HUD::getSceneType() { return 0x31; }
bool UIScene_HUD::stealsFocus() { return false; }
bool UIScene_HUD::hasFocus(int) { return false; }
bool UIScene_HUD::hidesLowerScenes() { return false; }
