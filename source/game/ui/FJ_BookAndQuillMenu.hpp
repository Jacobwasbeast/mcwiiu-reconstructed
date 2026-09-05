#ifndef GAME_UI_FJ_BOOKANDQUILLMENU_HPP
#define GAME_UI_FJ_BOOKANDQUILLMENU_HPP

#include <string>
#include <stdint.h>

#include "game/ui/FJ_Document.hpp"

class FJ_Base;
class FJ_Event;
class FJ_FuiNode;
class fuiRenderNode;

class FJ_BookAndQuillMenu : public FJ_Document
{
private:
    unsigned int m_unknown18[6];
    FJ_FuiNode* m_pageNode;
    FJ_Base* m_label;
    FJ_Base* m_caret;
    unsigned int m_unknown4c;
    int m_updateCounter;
    bool m_caretVisible;
    bool m_cursorAtLineEnd;
    unsigned short m_unknown5c;
    std::wstring m_pageText;
    unsigned int m_unknown68[4];
    int m_currentPage;
    bool m_bookIsSigned;
    bool m_unusedFlag;

public:
    FJ_BookAndQuillMenu(fuiRenderNode* renderNode);
    virtual ~FJ_BookAndQuillMenu();

    static FJ_Document* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
    void SetBookFocus(bool focus);
    void ChangePage(int page, int pageCount);
    int GetCaretOffset(int position);
    void SetCaretPosition(int position, bool atEnd, bool force);
    void UpdateBookAndQuill();
    static void _UpdateBookAndQuill(FJ_FuiNode* node, FJ_Event* event);
    virtual void ASConstructor();
    virtual void SetSafeZone(int top, int bottom, int left, int right);
    void SetBookIsSigned();
    void SetFontSizeOverride(int size);
    std::wstring GetPageText(bool secondPage);
    bool SetPageText(const std::wstring& text, int mode);
    uint64_t SetFocusToBook(bool focus);
    bool GetBookHasFocus();
    virtual void OnFocusChange(bool focused);
    virtual void DoHorizontalResizeCheck();
};

#endif
