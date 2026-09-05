#ifndef GAME_UI_FJ_BUTTON_HPP
#define GAME_UI_FJ_BUTTON_HPP

#include <stdint.h>
#include <string>
#include "game/ui/FJ_Base.hpp"

class FJ_Event;
class FJ_FuiNode;
class fuiRenderNode;
class fuiRenderNodeEditText;

class FJ_Button : public FJ_Base
{
public:
    explicit FJ_Button(fuiRenderNode* renderNode);
    virtual ~FJ_Button();

    void setButtonPressed(bool pressed);
    void ChangeState(int state);
    void ToggleOutline(bool show);
    void focusInHandler(FJ_Event* event);
    static void _focusInHandler(FJ_FuiNode* node, FJ_Event* event);
    void focusOutHandler(FJ_Event* event);
    static void _focusOutHandler(FJ_FuiNode* node, FJ_Event* event);
    void keyDownHandler(FJ_Event* event);
    static void _keyDownHandler(FJ_FuiNode* node, FJ_Event* event);
    void keyUpHandler(FJ_Event* event);
    static void _keyUpHandler(FJ_FuiNode* node, FJ_Event* event);
    void HideUntilInit();
    void HandleUpdateLabel(FJ_Event* event);
    static void _HandleUpdateLabel(FJ_FuiNode* node, FJ_Event* event);
    void Init(const std::wstring& name, int id);
    void SetKeyDownDelegate(void (*delegate)(FJ_FuiNode*, int), FJ_FuiNode* node);
    fuiRenderNodeEditText* GetSecondaryTextField();
    void SetSecondaryText(const std::wstring& text, bool html, bool autoSize);
    void UpdateLabel();
    virtual void SetLabel(const std::wstring& label);
    void AdjustDisabledTint();
    void EnableButton(bool enabled);
    virtual void GetLabelTooWide(fuiRenderNodeEditText* textField);

    bool getButtonPressed();
    bool getShowOutline();
    bool getEnabled();
    int getCurrentState();

private:
    bool m_buttonPressed;
    bool m_showOutline;
    bool m_enabled;
    int m_currentState;
    void (*m_keyDownDelegate)(FJ_FuiNode*, int);
    FJ_FuiNode* m_keyDownDelegateNode;
    bool m_focused;
    bool m_hiddenUntilInit;
    bool m_hasSecondaryText;
    std::wstring m_secondaryText;
    float m_secondaryWidth;
    float m_secondaryX;
    void (*m_updateLabelDelegate)(FJ_FuiNode*);
    bool m_secondaryHtml;
    bool m_secondaryAutoSize;
};

#endif
