#include "game/ui/FJ_Hud.hpp"

#include <algorithm>

#include "game/ui/FJ_BitmapIcon.hpp"
#include "game/ui/FJ_Base.hpp"
#include "game/ui/FJ_MobEffect_Tile.hpp"
#include "game/ui/FJ_MobEffect_Tile_Normal.hpp"
#include "game/ui/FJ_MobEffect_Tile_Small.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/util/Math.hpp"

FJ_Hud::FJ_Hud(fuiRenderNode* pRenderNode)
    : FJ_FuiNode(pRenderNode)
    , m_gameMode(0)
{
    std::fill(m_layout, m_layout + sizeof(m_layout), 0);
    std::fill(m_statusEffectTiles, m_statusEffectTiles + 27, static_cast<FJ_MobEffect_Tile*>(NULL));
    std::fill(m_statusEffectAmbient, m_statusEffectAmbient + 27, false);
    std::fill(m_damageIndicators, m_damageIndicators + 4, static_cast<FJ_FuiNode*>(NULL));
    std::fill(m_damageIndicatorAlpha, m_damageIndicatorAlpha + 4, 0.0f);
    m_iggySplash = NULL;
    m_offhandSlot = NULL;
    m_hotbarAnchor = NULL;
    m_splashX = -1.0f;
    m_splashY = -1.0f;
    m_offhandEnabled = false;
    m_rightHanded = false;
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
void FJ_Hud::SetJukeBoxLabelPos()
{
    FJ_FuiNode* label = getChildByName("Jukebox");
    FJ_FuiNodeStage* stage = getStage();
    if (label == NULL || stage == NULL)
        return;

    const float y = label->getY() +
        static_cast<float>(Math::round(label->getHeight() * 1.25f));
    label->setX(static_cast<float>(Math::round(
        stage->getStageWidth() - label->getWidth())));
    label->setY(static_cast<float>(Math::round(y)));
}
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
void FJ_Hud::UpdateBossBarPositions()
{
    FJ_FuiNodeStage* stage = getStage();
    if (stage == NULL)
        return;
    FJ_FuiNode* bars = getChildByName("BossBars");
    if (bars == NULL)
        return;
    const float width = bars->getWidth();
    const float x = static_cast<float>(Math::round((stage->getStageWidth() - width) * 0.5f));
    bars->setX(x);
}
void FJ_Hud::SetGamertag(const std::wstring& gamertag)
{
    FJ_FuiNode* node = getChildByName("Gamertag");
    if (node != NULL && node->asFJ_Base() != NULL)
        node->asFJ_Base()->SetTextOnly(gamertag);
    UpdateBossBarPositions();
}
void FJ_Hud::PositionGameModeComponents()
{
    if (m_gameMode == 2)
        SetJukeBoxLabelPos();
}
void FJ_Hud::PositionIggySplash()
{
    if (m_iggySplash == NULL)
        m_iggySplash = getChildByName("IggySplash");
    if (m_iggySplash == NULL)
        return;
    FJ_FuiNodeStage* stage = getStage();
    if (stage == NULL)
        return;
    float x = m_splashX;
    float y = m_splashY;
    if (x == -1.0f || y == -1.0f)
    {
        x = (stage->getStageWidth() - m_iggySplash->getWidth() * 0.5f) * 0.5f;
        y = stage->getStageHeight() * 0.25f;
    }
    m_iggySplash->setX(static_cast<float>(Math::round(x)));
    m_iggySplash->setY(static_cast<float>(Math::round(y)));
}
void FJ_Hud::RepositionHud(int width, int height) { setX((float)width); setY((float)height); }
void FJ_Hud::SetAlpha(float alpha) { FJ_FuiNode::setAlpha(alpha); }
void FJ_Hud::InitDamageIndicators()
{
    FJ_FuiNode* root = getChildAt(0);
    FJ_FuiNode* indicators = root == NULL ? NULL : root->getChildByName("DamageIndicators");
    if (indicators == NULL)
        return;
    for (int i = 0; i < 4; ++i)
    {
        std::string name = std::string("Damage_") + static_cast<char>('0' + i);
        m_damageIndicators[i] = indicators->getChildByName(name);
        m_damageIndicatorAlpha[i] = 0.0f;
        if (m_damageIndicators[i] != NULL)
            m_damageIndicators[i]->setAlpha(0.0f);
    }
}
int FJ_Hud::GetHotBarX() { return (int)getX(); }
int FJ_Hud::GetHotBarWidth() { return (int)getWidth(); }
int FJ_Hud::GetHotBarY() { return (int)getY(); }
void FJ_Hud::SetupOffhandSlot()
{
    if (m_offhandSlot == NULL)
        m_offhandSlot = getChildByName("offhandSlot");
    if (m_hotbarAnchor == NULL)
        m_hotbarAnchor = getChildByName("hotbar");
    if (m_offhandSlot == NULL)
        return;
    bool visible = m_offhandEnabled && (m_hotbarAnchor == NULL || m_hotbarAnchor->isVisible());
    m_offhandSlot->setVisible(visible);
    float width = static_cast<float>(Math::round(m_offhandSlot->getWidth() * 0.25f));
    float x = m_rightHanded ? GetHotBarX() - m_offhandSlot->getWidth() - width
                            : GetHotBarX() + GetHotBarWidth() + width;
    m_offhandSlot->setX(x);
    m_offhandSlot->setY(static_cast<float>(GetHotBarY()));
}
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
void FJ_Hud::SetAnimatedSplashTexture(const std::wstring& texture)
{
    FJ_FuiNode* node = getChildByName("AnimatedLogo.AnimatedLogoTexture");
    if (node != NULL)
    {
        FJ_BitmapIcon* icon = node->asFJ_BitmapIcon();
        if (icon != NULL)
        {
            icon->SetTextureName(texture);
        }
    }
}
void FJ_Hud::SetPulserText(const std::wstring& text)
{
    FJ_FuiNode* node = getChildByName("pulser");
    if (node != NULL && node->asFJ_Base() != NULL)
        node->asFJ_Base()->SetTextOnly(text);
    PositionIggySplash();
}
void FJ_Hud::SetTooltipsEnabled(bool enabled)
{
    FJ_FuiNode* node = getChildByName("tooltips");
    if (node != NULL)
        node->setVisible(enabled);
    SetupOffhandSlot();
    RepositionHud(static_cast<int>(getWidth()), static_cast<int>(getHeight()));
}
void FJ_Hud::ShowOffhandSlot(bool visible) { FJ_FuiNode* node = getChildByName("offhandSlot"); if (node != NULL) node->setVisible(visible); }
void FJ_Hud::SetIsRightHanded(bool) { }
void FJ_Hud::ShowDirectionalDamage(int, int, float alpha) { SetAlpha(alpha); }
void FJ_Hud::SetHudGameMode(int gameMode) { m_gameMode = gameMode; }
void FJ_Hud::SetPosition(int, const std::wstring&, bool visible) { setVisible(visible); }
void FJ_Hud::SetSpeedometer(int value) { m_iTabIndex = value; }
void FJ_Hud::SetTimerIcon(int value, int time) { m_iTabIndex = value + time; }
void FJ_Hud::SetScoreIcon(int value) { m_iTabIndex = value; }
void FJ_Hud::ShowScore(bool visible) { ShowQuickSelect(visible); }
void FJ_Hud::ShowArmorBar(bool visible) { FJ_FuiNode* node = getChildByName("armorBar"); if (node != NULL) node->setVisible(visible); }
void FJ_Hud::ShowArmorIcons(int armour, bool visible) { SetArmour(armour); ShowArmorBar(visible); }
void FJ_Hud::SetControlAlpha(const std::string& control, float alpha) { FJ_FuiNode* node = getChildByName(control); if (node != NULL) node->setAlpha(alpha); }
void FJ_Hud::SetWrongWayLabel(const std::wstring& label)
{
    FJ_FuiNode* node = getChildByName("WrongWay");
    if (node != NULL && node->asFJ_Base() != NULL)
        node->asFJ_Base()->SetTextOnly(label);
}
unsigned int FJ_Hud::GetStatusEffectsYOffset() { return 0; }
void FJ_Hud::HideSelectedLabel() { SetSelectedLabel(std::wstring(), false); }
void FJ_Hud::UpdateStatusEffectTile(int activeCount, int slot, int effect, bool ambient, float alpha)
{
    if (slot < 0 || slot >= 27)
        return;
    for (int i = activeCount; i < 27; ++i)
    {
        if (m_statusEffectTiles[i] != NULL)
        {
            removeChild(m_statusEffectTiles[i], true);
            m_statusEffectTiles[i] = NULL;
        }
    }
    FJ_MobEffect_Tile* tile = m_statusEffectTiles[slot];
    if (tile == NULL)
    {
        if (getStage() != NULL && getStage()->getStageHeight() == 1080.0f)
            tile = FJ_MobEffect_Tile_Normal::Create(m_pRenderNode);
        else
            tile = FJ_MobEffect_Tile_Small::Create(m_pRenderNode);
        if (tile == NULL)
            return;
        tile->Init();
        m_statusEffectTiles[slot] = tile;
        m_statusEffectAmbient[slot] = ambient;
        addChild(tile);
    }
    tile->SetupEffectType(effect + 2);
    tile->SetEffectAlpha(alpha);
}
void FJ_Hud::SetNumBossBars(int count) { m_iTabIndex = count; }
void FJ_Hud::SetBossBar(int, float progress, const std::wstring&) { SetExpBarProgress(progress); }
void FJ_Hud::ShowGameModePlayers(bool visible) { ShowQuickSelect(visible); }
void FJ_Hud::SetGameModeNumPlayers(int count, bool visible) { m_iTabIndex = count; ShowGameModePlayers(visible); }
void FJ_Hud::SetGameModePlayerState(int player, int state, bool visible) { m_iTabIndex = player + state; ShowGameModePlayers(visible); }
void FJ_Hud::SetPercentageMapTraversed(int percentage, float progress) { m_iTabIndex = percentage; SetExpBarProgress(progress); }
void FJ_Hud::AddCheckpointAtPercentage(float percentage)
{
    FJ_FuiNode* node = getChildByName("Checkpoint");
    if (node == NULL)
        return;
    node->setX(static_cast<float>(Math::round(getWidth() * percentage)));
    addChild(node);
    m_checkpointNodes.push_back(node);
}
void FJ_Hud::ClearCheckpointPositions() { m_checkpointNodes.clear(); }
void FJ_Hud::AnimateCheckpoint(int index) { if (index >= 0 && index < (int)m_checkpointNodes.size() && m_checkpointNodes[index] != NULL) m_checkpointNodes[index]->gotoAndPlay(2); }
void FJ_Hud::SetGlideHealth(int health) { SetHealth(health, false, false, false); }
void FJ_Hud::ShowHealth(bool visible) { FJ_FuiNode* node = getChildByName("health"); if (node != NULL) node->setVisible(visible); }
void FJ_Hud::ShowHorseHealth(bool visible) { FJ_FuiNode* node = getChildByName("horseHealth"); if (node != NULL) node->setVisible(visible); }
void FJ_Hud::ShowFood(bool visible) { FJ_FuiNode* node = getChildByName("food"); if (node != NULL) node->setVisible(visible); }
void FJ_Hud::ShowAir(bool visible) { FJ_FuiNode* node = getChildByName("air"); if (node != NULL) node->setVisible(visible); }
void FJ_Hud::ShowArmour(bool visible) { FJ_FuiNode* node = getChildByName("armour"); if (node != NULL) node->setVisible(visible); }
