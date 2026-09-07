#ifndef GAME_UI_UISCENE_HUD_HPP
#define GAME_UI_UISCENE_HUD_HPP

#include <string>

#include "game/ui/UIScene.hpp"
#include "game/ui/UIControl.hpp"
#include "game/ui/UIControl_Label.hpp"

class FJ_Hud;
class MobEffectInstance;

class UIScene_HUD : public UIScene
{
public:
    enum eGameModePlayerState
    {
        eGameModePlayerState_Alive = 0,
        eGameModePlayerState_Dead = 1
    };

    UIScene_HUD(int sceneType, void* data, UILayer* layer);
    virtual ~UIScene_HUD();
    FJ_Hud* getFJ_Hud();
    void SetSelectedLabel(const std::wstring& label);
    unsigned int HideMainHUD();
    virtual unsigned int mapElementsAndNames();
    void ShowArmorIcons(int armour, bool visible);
    static bool SORTBYTIMELEFT(MobEffectInstance* first, MobEffectInstance* second);
    void updateEffectsDisplay();
    void SetNumBossBars(int count);
    void SetBossBar(int index, float progress, const std::wstring& name);
    void HideSelectedLabel();
    virtual void tick();
    virtual void updateSafeZone();
    virtual std::wstring getMoviePath();
    virtual void customDraw(const char* name, fuiRect* rect);
    void SetHudSize(int size);
    void SetDisplayName(const std::wstring& name);
    void repositionHud();
    void ShowGameModePlayers(bool visible);
    int ShowHudComponent(bool visible, bool immediate);
    void ShowHealth(bool visible);
    void ShowFood(bool visible);
    void ShowArmour(bool visible);
    void ShowArmorBar(bool visible);
    void ShowQuickSelect(bool visible);
    void ShowExpBar(bool visible);
    void ShowHorseHealth(bool visible);
    void ShowAir(bool visible);
    void SetRidingHorse(bool riding, bool showJump, int health);
    void SetRegenerationEffect(bool enabled);
    void SetHealthAbsorb(int health);
    void InitMainHUD();
    void SetTooltipsEnabled(bool enabled);
    virtual void handleReload();
    unsigned int getPad();
    void SetOpacity(float opacity);
    void SetVisible(bool visible);
    unsigned char GetHudLoading();
    void SetExpBarProgress(float progress, int unused);
    void SetExpLevel(int level);
    void SetActiveSlot(int slot);
    void SetGlideHealth(int health);
    void SetHealth(int health, int unused, bool highlight, bool hardcore, bool hide);
    void SetFood(int food, int unused, bool highlight);
    void SetAir(int air, int unused);
    void SetArmour(int armour);
    void SetRoundNumber(unsigned int round, unsigned int total, bool visible);
    void ShowCrosshair(bool visible);
    void SetFoodSaturationLevel(int level);
    void SetHorseHealth(int health, bool highlight);
    void SetHorseJumpBarProgress(float progress);
    void ShowDisplayName(bool visible);
    void SetHudGameMode(int gameMode);
    void SetGameModePlayerState(int player, eGameModePlayerState state, bool visible);
    void SetGameModeNumPlayers(int count, bool visible);
    int GetHotBarX();
    int GetHotBarY();
    int GetHotBarWidth();
    int GetHotBarHeight();
    void ShowDirectionalDamage(int direction, int ticks, float alpha);
    void SetAnimatedSplashTexture(const std::wstring& texture);
    void SetAnimatedSplashIcon(int icon);
    void ShowAnimatedSplash(const std::wstring& label, bool visible, int icon);
    void SetIsRightHanded(bool rightHanded);
    void ShowOffhandSlot(bool visible);
    void SetScore(const std::wstring& score);
    void ShowScore(bool visible);
    void SetSpeed(const std::wstring& speed);
    void SetTime(const std::wstring& time);
    void SetTimerIcon(int icon, int time);
    void SetWrongWay(const std::wstring& label);
    void SetPercentageMapTraversed(int position, double percentage);
    void AddCheckpointAtPercentage(double percentage);
    void ClearCheckpointPositions();
    void AnimateCheckpoint(unsigned int index);
    void SetPositions(unsigned int position, const std::wstring& label, bool visible);
    void SetSpeedometer(unsigned int value);
    void SetScoreIcon(unsigned int value);
    unsigned int GetStatusEffectsYOffset();
    void renderPlayerHealth();
    void renderVehicleHealth();
    static std::wstring formatTime(long long time, bool showHours);
    void updateGameModePlayers();
    void ClearGlideHUD(int value);
    void showTimedSplash(const std::wstring& label, unsigned int duration, int icon);
    void updateFrameTick();
    void handleGameTick();
    virtual void render(int width, int height, C4JRender::eViewportType viewport);
    virtual void updateViewportTouchOffset();
    virtual void handleTimerComplete(int timerId);
    virtual int getSceneType();
    virtual bool stealsFocus();
    virtual bool hasFocus(int sceneType);
    virtual bool hidesLowerScenes();
    void ShowPulser(bool visible);
    void SetPulserText(const std::wstring& text);
    void ShowAttackIndicator(bool visible, float alpha, int ticks);
    void SetAltitude(int altitude);

private:
    struct HudState
    {
        unsigned char m_prefix[932];
        FJ_Hud* m_hud;
        unsigned char m_suffix[304];
    } m_state;
    UIControl_Label m_selectedLabel;
    unsigned char m_controlGap1[800];
    UIControl_Label m_displayName;
    UIControl m_hudComponent;
    UIControl_Label m_healthLabel;
    UIControl_Label m_foodLabel;
    UIControl_Label m_armourLabel;
    unsigned char m_controlGap2[528];
    UIControl_Label m_effectLabel;
    UIControl_Label m_splashLabel;
};

#endif
