#ifndef GAME_UI_FJ_HUD_HPP
#define GAME_UI_FJ_HUD_HPP

#include <string>
#include <vector>

#include "game/ui/FJ_FuiNode.hpp"

class FJ_Event;
class fuiRenderNode;

class FJ_Hud : public FJ_FuiNode
{
public:
    FJ_Hud(fuiRenderNode* pRenderNode);
    virtual ~FJ_Hud();
    static FJ_Hud* Create(fuiRenderNode* pRenderNode);
    virtual unsigned int getType();

    void SetPlayerLevel(int level);
    void SetActiveSlot(int slot);
    void SetHealth(int health, bool highlight, bool hardcore, bool hide);
    void SetFood(int food, bool highlight);
    void SetAir(int air);
    void SetArmour(int armour);
    void SetRegenerationEffect(bool enabled);
    void SetJukeBoxLabelPos();
    void SetSelectedLabel(const std::wstring& label, bool visible);
    void SetHealthAbsorb(bool visible, int health);
    void SetFoodSaturationLevel(int level);
    void ShowExpBar(bool visible);
    void SetRidingHorse(bool riding, bool showJump, int health);
    void SetHorseHealth(int health, bool highlight);
    void SetExpBarProgress(float progress);
    void SetHorseJumpBarProgress(float progress);
    void ShowQuickSelect(bool visible);
    void ShowCrosshair(bool visible);
    void UpdateBossBarPositions();
    void SetGamertag(const std::wstring& gamertag);
    void PositionGameModeComponents();
    void PositionIggySplash();
    void RepositionHud(int width, int height);
    void SetAlpha(float alpha);
    void InitDamageIndicators();
    int GetHotBarX();
    int GetHotBarWidth();
    int GetHotBarY();
    void SetupOffhandSlot();
    void ShowPulser(bool visible);
    void ShowAnimatedLogoText(FJ_Event* event);
    static void _ShowAnimatedLogoText(FJ_FuiNode* node, FJ_Event* event);
    void ShowAnimatedSplash(const std::wstring& label, bool visible);
    void UpdateOffsets(FJ_Event* event);
    static void _UpdateOffsets(FJ_FuiNode* node, FJ_Event* event);
    void InitHud(FJ_Event* event);
    static void _InitHud(FJ_FuiNode* node, FJ_Event* event);
    void LoadHud(int hudSize);
    void init(FJ_Event* event);
    static void _init(FJ_FuiNode* node, FJ_Event* event);
    int GetHotBarHeight();
    unsigned char GetHudLoading();
    void SetSafeZone(int left, int top, int right, int bottom);
    void SetAnimatedSplashTexture(const std::wstring& texture);
    void SetPulserText(const std::wstring& text);
    void SetTooltipsEnabled(bool enabled);
    void ShowOffhandSlot(bool visible);
    void SetIsRightHanded(bool rightHanded);
    void ShowDirectionalDamage(int direction, int ticks, float alpha);
    void SetHudGameMode(int gameMode);
    void SetPosition(int position, const std::wstring& label, bool visible);
    void SetSpeedometer(int value);
    void SetTimerIcon(int value, int time);
    void SetScoreIcon(int value);
    void ShowScore(bool visible);
    void ShowArmorBar(bool visible);
    void ShowArmorIcons(int armour, bool visible);
    void SetControlAlpha(const std::string& control, float alpha);
    void SetWrongWayLabel(const std::wstring& label);
    unsigned int GetStatusEffectsYOffset();
    void HideSelectedLabel();
    void UpdateStatusEffectTile(int index, int effect, int duration, bool ambient, float alpha);
    void SetNumBossBars(int count);
    void SetBossBar(int index, float progress, const std::wstring& name);
    void ShowGameModePlayers(bool visible);
    void SetGameModeNumPlayers(int count, bool visible);
    void SetGameModePlayerState(int player, int state, bool visible);
    void SetPercentageMapTraversed(int percentage, float progress);
    void AddCheckpointAtPercentage(float percentage);
    void ClearCheckpointPositions();
    void AnimateCheckpoint(int index);
    void SetGlideHealth(int health);
    void ShowHealth(bool visible);
    void ShowHorseHealth(bool visible);
    void ShowFood(bool visible);
    void ShowAir(bool visible);
    void ShowArmour(bool visible);

private:
    unsigned char m_layout[800];
    std::vector<FJ_FuiNode*> m_checkpointNodes;
    unsigned char m_tail[428];
};

#endif
