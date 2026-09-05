#include "game/ui/FJ_Hud.hpp"

#include <algorithm>

FJ_Hud::FJ_Hud(fuiRenderNode* pRenderNode)
    : FJ_FuiNode(pRenderNode)
{
    std::fill(m_layout, m_layout + sizeof(m_layout), 0);
}

FJ_Hud::~FJ_Hud()
{
    m_checkpointNodes.clear();
}

FJ_Hud* FJ_Hud::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_Hud(pRenderNode);
}

unsigned int FJ_Hud::getType() { return 89; }

void FJ_Hud::SetPlayerLevel(int level) { SetPosition(level, std::wstring(), level != 0); }
void FJ_Hud::SetActiveSlot(int slot) { m_iTabIndex = slot; }
void FJ_Hud::SetHealth(int health, bool highlight, bool hardcore, bool hide) { m_iTabIndex = health; ShowHealth(!hide); ShowArmorBar(highlight); ShowArmour(hardcore); }
void FJ_Hud::SetFood(int food, bool highlight) { m_iTabIndex = food; ShowFood(true); ShowArmorBar(highlight); }
void FJ_Hud::SetAir(int air) { m_iTabIndex = air; ShowAir(air >= 0); }
void FJ_Hud::SetArmour(int armour) { m_iTabIndex = armour; ShowArmour(armour > 0); }
void FJ_Hud::SetRegenerationEffect(bool enabled) { ShowHealth(enabled); }
void FJ_Hud::SetJukeBoxLabelPos() { }
void FJ_Hud::SetSelectedLabel(const std::wstring&, bool visible) { FJ_FuiNode* node = getChildByName("selectedLabel"); if (node != NULL) { node->setVisible(visible); } }
void FJ_Hud::SetHealthAbsorb(bool visible, int health) { SetHealth(health, visible, false, !visible); }
void FJ_Hud::SetFoodSaturationLevel(int level) { m_iTabIndex = level; }
void FJ_Hud::ShowExpBar(bool visible) { FJ_FuiNode* node = getChildByName("expBar"); if (node != NULL) node->setVisible(visible); }
void FJ_Hud::SetRidingHorse(bool riding, bool showJump, int health) { ShowHorseHealth(riding); SetHorseHealth(health, showJump); }
void FJ_Hud::SetHorseHealth(int health, bool highlight) { m_iTabIndex = health; ShowHorseHealth(health >= 0); ShowArmorBar(highlight); }
void FJ_Hud::SetExpBarProgress(float progress) { FJ_FuiNode* node = getChildByName("expBar"); if (node != NULL) node->setAlpha(progress); }
void FJ_Hud::SetHorseJumpBarProgress(float progress) { FJ_FuiNode* node = getChildByName("horseJumpBar"); if (node != NULL) node->setAlpha(progress); }
void FJ_Hud::ShowQuickSelect(bool visible) { FJ_FuiNode* node = getChildByName("quickSelect"); if (node != NULL) node->setVisible(visible); }
void FJ_Hud::ShowCrosshair(bool visible) { FJ_FuiNode* node = getChildByName("crosshair"); if (node != NULL) node->setVisible(visible); }
void FJ_Hud::UpdateBossBarPositions() { }
void FJ_Hud::SetGamertag(const std::wstring&) { }
void FJ_Hud::PositionGameModeComponents() { }
void FJ_Hud::PositionIggySplash() { }
void FJ_Hud::RepositionHud(int width, int height) { setX((float)width); setY((float)height); }
void FJ_Hud::SetAlpha(float alpha) { FJ_FuiNode::setAlpha(alpha); }
void FJ_Hud::InitDamageIndicators() { }
int FJ_Hud::GetHotBarX() { return (int)getX(); }
int FJ_Hud::GetHotBarWidth() { return (int)getWidth(); }
int FJ_Hud::GetHotBarY() { return (int)getY(); }
void FJ_Hud::SetupOffhandSlot() { }
void FJ_Hud::ShowPulser(bool visible) { FJ_FuiNode* node = getChildByName("pulser"); if (node != NULL) node->setVisible(visible); }
void FJ_Hud::ShowAnimatedLogoText(FJ_Event* event) { _ShowAnimatedLogoText(this, event); }
void FJ_Hud::_ShowAnimatedLogoText(FJ_FuiNode*, FJ_Event*) { }
void FJ_Hud::ShowAnimatedSplash(const std::wstring&, bool visible) { ShowQuickSelect(visible); }
void FJ_Hud::UpdateOffsets(FJ_Event* event) { _UpdateOffsets(this, event); }
void FJ_Hud::_UpdateOffsets(FJ_FuiNode*, FJ_Event*) { }
void FJ_Hud::InitHud(FJ_Event* event) { _InitHud(this, event); }
void FJ_Hud::_InitHud(FJ_FuiNode*, FJ_Event*) { }
void FJ_Hud::LoadHud(int hudSize) { m_iTabIndex = hudSize; }
void FJ_Hud::init(FJ_Event* event) { InitHud(event); }
void FJ_Hud::_init(FJ_FuiNode*, FJ_Event*) { }
int FJ_Hud::GetHotBarHeight() { return (int)getHeight(); }
unsigned char FJ_Hud::GetHudLoading() { return 0; }
void FJ_Hud::SetSafeZone(int left, int top, int right, int bottom) { setX((float)left); setY((float)top); setHeight((float)(bottom - top)); setWidth((float)(right - left)); }
void FJ_Hud::SetAnimatedSplashTexture(const std::wstring&) { }
void FJ_Hud::SetPulserText(const std::wstring&) { }
void FJ_Hud::SetTooltipsEnabled(bool) { }
void FJ_Hud::ShowOffhandSlot(bool visible) { FJ_FuiNode* node = getChildByName("offhandSlot"); if (node != NULL) node->setVisible(visible); }
void FJ_Hud::SetIsRightHanded(bool) { }
void FJ_Hud::ShowDirectionalDamage(int, int, float alpha) { SetAlpha(alpha); }
void FJ_Hud::SetHudGameMode(int gameMode) { m_iTabIndex = gameMode; }
void FJ_Hud::SetPosition(int, const std::wstring&, bool visible) { setVisible(visible); }
void FJ_Hud::SetSpeedometer(int value) { m_iTabIndex = value; }
void FJ_Hud::SetTimerIcon(int value, int time) { m_iTabIndex = value + time; }
void FJ_Hud::SetScoreIcon(int value) { m_iTabIndex = value; }
void FJ_Hud::ShowScore(bool visible) { ShowQuickSelect(visible); }
void FJ_Hud::ShowArmorBar(bool visible) { FJ_FuiNode* node = getChildByName("armorBar"); if (node != NULL) node->setVisible(visible); }
void FJ_Hud::ShowArmorIcons(int armour, bool visible) { SetArmour(armour); ShowArmorBar(visible); }
void FJ_Hud::SetControlAlpha(const std::string& control, float alpha) { FJ_FuiNode* node = getChildByName(control); if (node != NULL) node->setAlpha(alpha); }
void FJ_Hud::SetWrongWayLabel(const std::wstring&) { }
unsigned int FJ_Hud::GetStatusEffectsYOffset() { return 0; }
void FJ_Hud::HideSelectedLabel() { SetSelectedLabel(std::wstring(), false); }
void FJ_Hud::UpdateStatusEffectTile(int, int, int, bool, float) { }
void FJ_Hud::SetNumBossBars(int count) { m_iTabIndex = count; }
void FJ_Hud::SetBossBar(int, float progress, const std::wstring&) { SetExpBarProgress(progress); }
void FJ_Hud::ShowGameModePlayers(bool visible) { ShowQuickSelect(visible); }
void FJ_Hud::SetGameModeNumPlayers(int count, bool visible) { m_iTabIndex = count; ShowGameModePlayers(visible); }
void FJ_Hud::SetGameModePlayerState(int player, int state, bool visible) { m_iTabIndex = player + state; ShowGameModePlayers(visible); }
void FJ_Hud::SetPercentageMapTraversed(int percentage, float progress) { m_iTabIndex = percentage; SetExpBarProgress(progress); }
void FJ_Hud::AddCheckpointAtPercentage(float) { }
void FJ_Hud::ClearCheckpointPositions() { m_checkpointNodes.clear(); }
void FJ_Hud::AnimateCheckpoint(int index) { if (index >= 0 && index < (int)m_checkpointNodes.size() && m_checkpointNodes[index] != NULL) m_checkpointNodes[index]->gotoAndPlay(2); }
void FJ_Hud::SetGlideHealth(int health) { SetHealth(health, false, false, false); }
void FJ_Hud::ShowHealth(bool visible) { FJ_FuiNode* node = getChildByName("health"); if (node != NULL) node->setVisible(visible); }
void FJ_Hud::ShowHorseHealth(bool visible) { FJ_FuiNode* node = getChildByName("horseHealth"); if (node != NULL) node->setVisible(visible); }
void FJ_Hud::ShowFood(bool visible) { FJ_FuiNode* node = getChildByName("food"); if (node != NULL) node->setVisible(visible); }
void FJ_Hud::ShowAir(bool visible) { FJ_FuiNode* node = getChildByName("air"); if (node != NULL) node->setVisible(visible); }
void FJ_Hud::ShowArmour(bool visible) { FJ_FuiNode* node = getChildByName("armour"); if (node != NULL) node->setVisible(visible); }
