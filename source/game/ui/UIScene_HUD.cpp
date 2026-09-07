#include "game/ui/UIScene_HUD.hpp"

#include <algorithm>
#include <cwchar>

#include "game/core/ClientMasterGameMode.hpp"
#include "game/core/Minecraft.hpp"
#include "game/ui/FJ_Hud.hpp"

UIScene_HUD::UIScene_HUD(int sceneType, void*, UILayer* layer)
    : UIScene(sceneType, layer), m_state(), m_selectedLabel(), m_controlGap1(), m_displayName(),
      m_hudComponent(), m_healthLabel(), m_foodLabel(), m_armourLabel(), m_controlGap2(),
      m_effectLabel(), m_splashLabel()
{
    m_state.m_hud = 0;
}

UIScene_HUD::~UIScene_HUD()
{
}

FJ_Hud* UIScene_HUD::getFJ_Hud()
{
    return m_state.m_hud;
}

void UIScene_HUD::SetSelectedLabel(const std::wstring& label)
{
    if (m_state.m_hud != 0) m_state.m_hud->SetSelectedLabel(label, true);
}

unsigned int UIScene_HUD::HideMainHUD()
{
    if (m_state.m_hud != 0) m_state.m_hud->ShowHealth(false);
    return 0;
}

unsigned int UIScene_HUD::mapElementsAndNames()
{
    return 0;
}

void UIScene_HUD::ShowArmorIcons(int armour, bool visible)
{
    if (m_state.m_hud != 0) m_state.m_hud->ShowArmorIcons(armour, visible);
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
    if (m_state.m_hud != 0) m_state.m_hud->SetNumBossBars(count);
}

void UIScene_HUD::SetBossBar(int index, float progress, const std::wstring& name)
{
    if (m_state.m_hud != 0) m_state.m_hud->SetBossBar(index, progress, name);
}

void UIScene_HUD::HideSelectedLabel()
{
    if (m_state.m_hud != 0) m_state.m_hud->HideSelectedLabel();
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
    if (m_state.m_hud != 0) m_state.m_hud->LoadHud(size);
}

void UIScene_HUD::SetDisplayName(const std::wstring& name)
{
    if (m_state.m_hud != 0) m_state.m_hud->SetGamertag(name);
}

void UIScene_HUD::repositionHud()
{
    if (m_state.m_hud != 0) m_state.m_hud->RepositionHud(0, 0);
}

void UIScene_HUD::ShowGameModePlayers(bool visible)
{
    if (m_state.m_hud != 0) m_state.m_hud->ShowGameModePlayers(visible);
}

int UIScene_HUD::ShowHudComponent(bool hidden, bool visible)
{
    if (!hidden && visible && Minecraft::InMiniGame(eMiniGameId_Lobby, true) &&
        Minecraft::GetInstance()->GetClientMasterGameMode()->IsRoundEnding())
        visible = false;
    return visible;
}

void UIScene_HUD::ShowHealth(bool visible) { if (m_state.m_hud != 0) m_state.m_hud->ShowHealth(visible); }
void UIScene_HUD::ShowFood(bool visible) { if (m_state.m_hud != 0) m_state.m_hud->ShowFood(visible); }
void UIScene_HUD::ShowArmour(bool visible) { if (m_state.m_hud != 0) m_state.m_hud->ShowArmour(visible); }
void UIScene_HUD::ShowArmorBar(bool visible) { if (m_state.m_hud != 0) m_state.m_hud->ShowArmorBar(visible); }
void UIScene_HUD::ShowQuickSelect(bool visible) { if (m_state.m_hud != 0) m_state.m_hud->ShowQuickSelect(visible); }
void UIScene_HUD::ShowExpBar(bool visible) { if (m_state.m_hud != 0) m_state.m_hud->ShowExpBar(visible); }
void UIScene_HUD::ShowHorseHealth(bool visible) { if (m_state.m_hud != 0) m_state.m_hud->ShowHorseHealth(visible); }
void UIScene_HUD::ShowAir(bool visible) { if (m_state.m_hud != 0) m_state.m_hud->ShowAir(visible); }
void UIScene_HUD::SetRidingHorse(bool riding, bool showJump, int health) { if (m_state.m_hud != 0) m_state.m_hud->SetRidingHorse(riding, showJump, health); }
void UIScene_HUD::SetRegenerationEffect(bool enabled) { if (m_state.m_hud != 0) m_state.m_hud->SetRegenerationEffect(enabled); }
void UIScene_HUD::SetHealthAbsorb(int health) { if (m_state.m_hud != 0) m_state.m_hud->SetHealthAbsorb(true, health); }
void UIScene_HUD::InitMainHUD() { }
void UIScene_HUD::SetTooltipsEnabled(bool enabled) { if (m_state.m_hud != 0) m_state.m_hud->SetTooltipsEnabled(enabled); }
void UIScene_HUD::handleReload() { UIScene::handleReload(); }
unsigned int UIScene_HUD::getPad() { return 0; }
void UIScene_HUD::SetOpacity(float opacity) { if (m_state.m_hud != 0) m_state.m_hud->SetAlpha(opacity); }
void UIScene_HUD::SetVisible(bool visible) { if (m_state.m_hud != 0) m_state.m_hud->ShowHealth(visible); }
unsigned char UIScene_HUD::GetHudLoading() { return m_state.m_hud == 0 ? 0 : m_state.m_hud->GetHudLoading(); }
void UIScene_HUD::SetExpBarProgress(float progress, int) { if (m_state.m_hud != 0) m_state.m_hud->SetExpBarProgress(progress); }
void UIScene_HUD::SetExpLevel(int level) { if (m_state.m_hud != 0) m_state.m_hud->SetPlayerLevel(level); }
void UIScene_HUD::SetActiveSlot(int slot) { if (m_state.m_hud != 0) m_state.m_hud->SetActiveSlot(slot); }
void UIScene_HUD::SetGlideHealth(int health) { if (m_state.m_hud != 0) m_state.m_hud->SetGlideHealth(health); }
void UIScene_HUD::SetHealth(int health, int minimumHealth, bool highlight, bool hardcore, bool hide)
{
    if (health < minimumHealth)
        health = minimumHealth;

    if (m_state.m_hud != 0)
        m_state.m_hud->SetHealth(health, highlight, hardcore, hide);
}
void UIScene_HUD::SetFood(int food, int, bool highlight) { if (m_state.m_hud != 0) m_state.m_hud->SetFood(food, highlight); }
void UIScene_HUD::SetAir(int air, int) { if (m_state.m_hud != 0) m_state.m_hud->SetAir(air); }
void UIScene_HUD::SetArmour(int armour) { if (m_state.m_hud != 0) m_state.m_hud->SetArmour(armour); }
void UIScene_HUD::SetRoundNumber(unsigned int round, unsigned int total, bool visible)
{
    wchar_t text[32];

    if (visible)
        std::swprintf(text, 32, L"%u/%u", round, total);
    else
        text[0] = 0;

    m_splashLabel.setLabel(UIString(text), false, false);
}
void UIScene_HUD::ShowCrosshair(bool visible) { if (m_state.m_hud != 0) m_state.m_hud->ShowCrosshair(visible); }
void UIScene_HUD::SetFoodSaturationLevel(int level) { if (m_state.m_hud != 0) m_state.m_hud->SetFoodSaturationLevel(level); }
void UIScene_HUD::SetHorseHealth(int health, bool highlight) { if (m_state.m_hud != 0) m_state.m_hud->SetHorseHealth(health, highlight); }
void UIScene_HUD::SetHorseJumpBarProgress(float progress) { if (m_state.m_hud != 0) m_state.m_hud->SetHorseJumpBarProgress(progress); }
void UIScene_HUD::ShowDisplayName(bool visible) { if (m_state.m_hud != 0) m_state.m_hud->ShowGameModePlayers(visible); }
void UIScene_HUD::SetHudGameMode(int gameMode) { if (m_state.m_hud != 0) m_state.m_hud->SetHudGameMode(gameMode); }
void UIScene_HUD::SetGameModePlayerState(int player, eGameModePlayerState state, bool visible) { if (m_state.m_hud != 0) m_state.m_hud->SetGameModePlayerState(player, (int)state, visible); }
void UIScene_HUD::SetGameModeNumPlayers(int count, bool visible) { if (m_state.m_hud != 0) m_state.m_hud->SetGameModeNumPlayers(count, visible); }
int UIScene_HUD::GetHotBarX() { return m_state.m_hud == 0 ? 0 : m_state.m_hud->GetHotBarX(); }
int UIScene_HUD::GetHotBarY() { return m_state.m_hud == 0 ? 0 : m_state.m_hud->GetHotBarY(); }
int UIScene_HUD::GetHotBarWidth() { return m_state.m_hud == 0 ? 0 : m_state.m_hud->GetHotBarWidth(); }
int UIScene_HUD::GetHotBarHeight() { return m_state.m_hud == 0 ? 0 : m_state.m_hud->GetHotBarHeight(); }
void UIScene_HUD::ShowDirectionalDamage(int direction, int ticks, float alpha) { if (m_state.m_hud != 0) m_state.m_hud->ShowDirectionalDamage(direction, ticks, alpha); }
void UIScene_HUD::SetAnimatedSplashTexture(const std::wstring& texture) { if (m_state.m_hud != 0) m_state.m_hud->SetAnimatedSplashTexture(texture); }
void UIScene_HUD::SetAnimatedSplashIcon(int icon) { std::wstring texture; if (icon == 2) texture = L"Graphics/GameModeGraphics/AwardIcon.png"; SetAnimatedSplashTexture(texture); }
void UIScene_HUD::ShowAnimatedSplash(const std::wstring& label, bool visible, int icon) { if (icon >= 0) SetAnimatedSplashIcon(icon); if (m_state.m_hud != 0) m_state.m_hud->ShowAnimatedSplash(label, visible); }
void UIScene_HUD::SetIsRightHanded(bool rightHanded) { if (m_state.m_hud != 0) m_state.m_hud->SetIsRightHanded(rightHanded); }
void UIScene_HUD::ShowOffhandSlot(bool visible) { if (m_state.m_hud != 0) m_state.m_hud->ShowOffhandSlot(visible); }
void UIScene_HUD::SetScore(const std::wstring& score) { m_healthLabel.setLabel(UIString(score), false, false); }
void UIScene_HUD::ShowScore(bool visible) { if (m_state.m_hud != 0) m_state.m_hud->ShowScore(visible); }
void UIScene_HUD::SetSpeed(const std::wstring& speed) { m_foodLabel.setLabel(UIString(speed), false, false); }
void UIScene_HUD::SetTime(const std::wstring& time) { m_armourLabel.setLabel(UIString(time), false, false); }
void UIScene_HUD::SetTimerIcon(int icon, int time) { if (m_state.m_hud != 0) m_state.m_hud->SetTimerIcon(icon, time); }
void UIScene_HUD::SetWrongWay(const std::wstring& label) { if (m_state.m_hud != 0) m_state.m_hud->SetWrongWayLabel(label); }
void UIScene_HUD::SetPercentageMapTraversed(int position, double percentage) { percentage = std::max(0.0, std::min(100.0, percentage)); if (m_state.m_hud != 0) m_state.m_hud->SetPercentageMapTraversed(position, (float)percentage); }
void UIScene_HUD::AddCheckpointAtPercentage(double percentage) { if (m_state.m_hud != 0) m_state.m_hud->AddCheckpointAtPercentage((float)percentage); }
void UIScene_HUD::ClearCheckpointPositions() { if (m_state.m_hud != 0) m_state.m_hud->ClearCheckpointPositions(); }
void UIScene_HUD::AnimateCheckpoint(unsigned int index) { if (m_state.m_hud != 0) m_state.m_hud->AnimateCheckpoint((int)index); }
void UIScene_HUD::SetPositions(unsigned int position, const std::wstring& label, bool visible) { if (m_state.m_hud != 0) m_state.m_hud->SetPosition((int)position, label, visible); }
void UIScene_HUD::SetSpeedometer(unsigned int value) { if (m_state.m_hud != 0) m_state.m_hud->SetSpeedometer((int)value); }
void UIScene_HUD::SetScoreIcon(unsigned int value) { if (m_state.m_hud != 0) m_state.m_hud->SetScoreIcon((int)value); }
unsigned int UIScene_HUD::GetStatusEffectsYOffset() { return m_state.m_hud == 0 ? 0 : m_state.m_hud->GetStatusEffectsYOffset(); }
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
