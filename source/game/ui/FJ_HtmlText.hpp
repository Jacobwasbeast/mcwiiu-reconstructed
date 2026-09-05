#ifndef GAME_UI_FJ_HTMLTEXT_HPP
#define GAME_UI_FJ_HTMLTEXT_HPP

#include <string>

#include "game/ui/FJ_Base.hpp"

class FJ_Event;
class FJ_FuiNode;
class FJ_ScrollArrow;
class fuiRenderNode;

class FJ_HtmlText : public FJ_Base
{
public:
    bool m_bUseScrollRect;
    bool m_bNoBackground;
    int m_iFocusDirection;
    float m_fScrollSpeed;
    fuiRenderNode* m_pText;
    int m_iWidth;
    float m_fWidth;
    float m_fScrollTop;
    float m_fScrollBottom;
    FJ_ScrollArrow* m_pUpArrow;
    FJ_ScrollArrow* m_pDownArrow;
    unsigned char m_scrollDirection;
    bool m_bScrollingUp;
    bool m_bScrollingDown;
    int m_iTargetScroll;
    int m_iCharacterHeight;
    int m_iScrollDelay;
    int m_iFlickDelay;
    bool m_bScrollingEnabled;
    bool m_bInitialized;
    int m_iRealWidth;
    int m_iRealHeight;
    void (*m_pScrollingCompleteDelegate)();
    bool m_bTouchScrolling;
    int m_iTouchStart;
    int m_iTouchPosition;
    int m_iTouchScroll;
    int m_iTouchTarget;

    FJ_HtmlText(fuiRenderNode* renderNode);
    virtual ~FJ_HtmlText();
    static FJ_FuiNode* Create(fuiRenderNode* renderNode);
    unsigned int CanScroll(bool up);
    void HandleScrollListArrows(bool positionArrows);
    void UpdateScrolling(FJ_Event* event);
    static void _UpdateScrolling(FJ_FuiNode* node, FJ_Event* event);
    void keyDownHandler(FJ_Event* event);
    static void _keyDownHandler(FJ_FuiNode* node, FJ_Event* event);
    void GetScrollTarget(bool up);
    void keyUpHandler(FJ_Event* event);
    static void _keyUpHandler(FJ_FuiNode* node, FJ_Event* event);
    int getTextHeight();
    virtual unsigned int getType();
    virtual void setVisible(bool visible);
    void UpdateScrollListArrows();
    virtual void setX(float x);
    virtual void setY(float y);
    virtual void setHeight(float height);
    void Init(const std::wstring& label);
    void GetScrollDistance();
    virtual void SetLabel(const std::wstring& label);
    int GetNumberOfLines();
    unsigned int GetOptimalHeight(int lines);
    double GetRealWidth();
    double GetRealHeight();
    void TouchScroll(int position, bool active);
    void SetScrollingCompleteDelegate(void (*delegate)());
    std::wstring GetLabel();
    void SetScrollingEnabled(bool enabled);
};

#endif
