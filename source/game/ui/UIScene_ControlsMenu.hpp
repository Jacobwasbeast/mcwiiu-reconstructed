#ifndef GAME_UI_UISCENE_CONTROLSMENU_HPP
#define GAME_UI_UISCENE_CONTROLSMENU_HPP

#include <stdint.h>
#include <string>

#include "game/core/GameSettings.hpp"
#include "game/ui/UIControl_Label.hpp"
#include "game/ui/UIControl_MultiList.hpp"
#include "game/ui/UIScene.hpp"

class UIScene_ControlsMenu : public UIScene
{
public:
    enum EControl
    {
        EControl_ResetToDefaults,
        EControl_InvertLook,
        EControl_SouthPaw,
        EControl_Look,
        EControl_Move,
        EControl_Use,
        EControl_Action,
        EControl_Jump,
        EControl_Drop,
        EControl_Inventory,
        EControl_Crafting,
        EControl_CycleHeldItemLeft,
        EControl_CycleHeldItemRight,
        EControl_Sneak,
        EControl_Sprint,
        EControl_PickBlock,
        EControl_ChangeCameraMode,
        EControl_FlyLeft,
        EControl_FlyRight,
        EControl_FlyUp,
        EControl_FlyDown,
        EControl_SpectatePrevious,
        EControl_SpectateNext,
        EControl_SpectateNoise,
        EControl_SpectateCancel,
        EControl_ConfirmReady,
        EControl_Vote,
        EControl_RestartSection,
        EControl_RestartRace,
        EControl_LookBehind,
        EControl_Label
    };

    UIScene_ControlsMenu(int player, void* data, UILayer* layer);
    virtual ~UIScene_ControlsMenu();
    std::wstring GetControlsString(int player, EControllerActions action, bool& mapped);
    void SetControlsHelp(int player, int control, std::wstring help);
    void Init(int player);
    virtual unsigned int mapElementsAndNames();
    virtual void updateTooltips();
    virtual void updateComponents();
    unsigned char GetControlsAction(EControl control);
    std::wstring GetControlsString(EControl control, bool& mapped);
    void UpdateControls(int player);
    static int ResetControlsWarningDialogReturned(void* context, int pad,
                                                            C4JStorage::EMessageResult result);
    unsigned int CheckMapping(int player);
    virtual void tick();
    virtual std::wstring getMoviePath();
    virtual void handleCheckboxElementToggled(int first, int second, bool checked);
    static int ResetDefaultsDialogReturned(void* context, int pad,
                                                     C4JStorage::EMessageResult result);
    static int ControlsWarningDialogReturned(void* context, int pad,
                                                      C4JStorage::EMessageResult result);
    virtual void handleReload();
    unsigned int CheckControls(int player, EControl control);
    void HandleButtonEdit(int player, int action, bool repeat, bool released, bool pressed);
    unsigned int CheckControlsOnExit(int player);
    virtual void handleInput(int player, int action, bool repeat, bool released, bool pressed,
                             bool& handled);
    virtual void handlePress(int controlId, int childId);
    virtual void handleFocusChange(int focus, int childFocus);
    virtual int getSceneType();

private:
    unsigned char m_flags[10];
    int m_unknownD4;
    int m_unknownD8;
    int m_unknownDC;
    UIControl_Label m_version;
    UIControl_MultiList m_controlsList;
};

#endif
