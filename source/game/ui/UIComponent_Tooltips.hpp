#ifndef GAME_UI_UICOMPONENT_TOOLTIPS_HPP
#define GAME_UI_UICOMPONENT_TOOLTIPS_HPP

#include <string>
#include "game/ui/UIScene.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/UIControl_Touch.hpp"

class UILayer;
enum EControllerActions : int;

class UIComponent_Tooltips : public UIScene
{
public:
    class _TooltipValues
    {
    public:
        bool m_enabled;
        int m_textId;
        UIString m_text;
        int m_type;
        int m_subtype;
        bool m_dynamic;
        int m_button;

        _TooltipValues();
        ~_TooltipValues();
    };

    UIComponent_Tooltips(int sceneType, void* data, UILayer* layer);
    virtual ~UIComponent_Tooltips();

    void overrideSFX(int first, int second, bool enabled);
    virtual unsigned int mapElementsAndNames();
    virtual std::wstring getMoviePath();
    virtual double getSafeZoneHalfWidth();
    virtual void updateSafeZone();
    void _SetTooltip(int index, int type, int subtype, unsigned int button,
                     const UIString& text, bool enabled);
    void _Relayout();
    virtual void tick();
    virtual void render(int width, int height, C4JRender::eViewportType viewport);
    virtual void updateViewportTouchOffset();
    unsigned int _SetTooltip(unsigned int player, int index);
    void SetTooltipText(unsigned int player, int index);
    void SetEnableTooltips(bool enabled);
    void ShowTooltip(unsigned int player, bool visible);
    unsigned int GetToolTipButton(unsigned int button);
    unsigned int _SetTooltip(unsigned int player, int type, int subtype, int textId,
                             bool& changed);
    unsigned int HandleToolTipButton(unsigned int player, EControllerActions action,
                                     unsigned int& button, int type);
    void UpdateControllerType(int controllerType);
    void SetTooltips(int player, int* tooltips, bool visible);
    void SetTooltips(int player, int first, int second, int third, int fourth,
                     int fifth, int sixth, int seventh, int eighth, int ninth,
                     int tenth, int eleventh, int twelfth, int thirteenth,
                     int fourteenth, int fifteenth, int sixteenth, bool visible);
    void EnableTooltip(unsigned int player, bool enabled);
    bool IsToolTipDynamic(unsigned int player, EControllerActions& action);
    virtual void handleReload();
    virtual void handleInput(int player, int action, bool repeat, bool released,
                             bool pressed, bool& handled);
    virtual void handleTouchInput(unsigned int player, int action, int x, int y,
                                  bool repeat, bool released, bool pressed);
    virtual int getSceneType();
    virtual bool stealsFocus();
    virtual bool hasFocus(int sceneType);
    virtual bool hidesLowerScenes();

private:
    bool m_splitTooltips;
    bool m_tooltipsEnabled;
    int m_controllerType;
    UILayer* m_tooltipLayer;
    int m_safeZoneWidth;
    int m_safeZoneHeight;
    bool m_touchVisible;
    int m_activeTooltip;
    _TooltipValues m_tooltipValues[16];
    UIControl_Touch m_touchControls[16];
    unsigned char m_touchState[212];
};

#endif
