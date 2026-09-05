#include "game/ui/FJ_BookAndQuillMenu.hpp"

#include "game/ui/FJ_Base.hpp"
#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/fuiRenderNodeEditText.hpp"
#include "game/ui/fuiRenderNode.hpp"

FJ_BookAndQuillMenu::FJ_BookAndQuillMenu(fuiRenderNode* renderNode)
    : FJ_Document(renderNode), m_updateCounter(0), m_caretVisible(false), m_cursorAtLineEnd(false),
      m_unknown5c(0), m_currentPage(0), m_bookIsSigned(false), m_unusedFlag(false)
{
}

FJ_BookAndQuillMenu::~FJ_BookAndQuillMenu()
{
}

FJ_Document* FJ_BookAndQuillMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_BookAndQuillMenu(renderNode);
}

unsigned int FJ_BookAndQuillMenu::getType()
{
    return 21;
}

void FJ_BookAndQuillMenu::SetBookFocus(bool focus)
{
    if (m_bookIsSigned)
        return;
    if (m_caret != NULL)
        m_caret->setVisible(focus);
}

void FJ_BookAndQuillMenu::ChangePage(int page, int pageCount)
{
    FJ_FuiNode* left = getChildByName("BookArrowLeft");
    FJ_FuiNode* right = getChildByName("BookArrowRight");
    if (page == 0 && pageCount == 0) {
        left->gotoAndStop("ArrowNone");
        right->gotoAndStop("ArrowNone");
    } else if (page < m_currentPage) {
        if (page < 1)
            left->gotoAndPlay("ArrowAnimEnd");
        else
            right->gotoAndPlay("ArrowAnim");
        right->gotoAndStop("ArrowNormal");
        m_currentPage = page;
    } else if (m_currentPage < page) {
        if (page < pageCount)
            right->gotoAndPlay("ArrowAnim");
        else
            right->gotoAndPlay("ArrowAnimEnd");
        left->gotoAndStop("ArrowNormal");
        m_currentPage = page;
    } else {
        if (page == 0)
            left->gotoAndStop("ArrowNone");
        else
            left->gotoAndStop("ArrowNormal");
        if (pageCount < 1 || pageCount <= page)
            right->gotoAndStop("ArrowNone");
        else
            right->gotoAndStop("ArrowNormal");
        m_currentPage = page;
    }
}

int FJ_BookAndQuillMenu::GetCaretOffset(int position)
{
    int offset = 0;
    std::wstring* labelText = m_label == NULL ? NULL : (std::wstring*)m_label->GetLabel();
    if (labelText == NULL || labelText->empty())
        return offset;
    const std::wstring& label = *labelText;
    for (int i = 0; i < position && i < static_cast<int>(label.size()); ++i) {
        if (label[i] == L'\n')
            ++offset;
    }
    return offset;
}

void FJ_BookAndQuillMenu::SetCaretPosition(int position, bool atEnd, bool force)
{
    if (m_caret == NULL)
        return;
    m_cursorAtLineEnd = atEnd;
    if (!m_caretVisible) {
        m_caret->SetTextOnly(L"");
    } else {
        const std::wstring& label = *(std::wstring*)m_label->GetLabel();
        int index = position + (force ? 0 : GetCaretOffset(position));
        if (index >= static_cast<int>(label.size()))
            index = static_cast<int>(label.size()) - 1;
        if (index < 0)
            index = 0;
        m_caret->SetTextOnly(label.substr(0, index) + L"#" + label.substr(index));
        m_label->SetTextOnly(label);
    }
    m_caretVisible = true;
}

void FJ_BookAndQuillMenu::UpdateBookAndQuill()
{
    if (m_updateCounter % 15 == 1) {
        m_caretVisible = !m_caretVisible;
        if (m_caret != NULL) {
            if (!m_caretVisible)
                m_caret->SetTextOnly(L"");
            else
                m_caret->SetTextOnly(m_cursorAtLineEnd ? L"" : L" ");
        }
    }
    ++m_updateCounter;
}

void FJ_BookAndQuillMenu::_UpdateBookAndQuill(FJ_FuiNode* node, FJ_Event*)
{
    static_cast<FJ_BookAndQuillMenu*>(node)->UpdateBookAndQuill();
}

void FJ_BookAndQuillMenu::ASConstructor()
{
    m_label = getChildByName("BookAndQuillLabel")->asFJ_Base();
    m_caret = getChildByName("Caret")->asFJ_Base();
    m_caretVisible = false;
    m_cursorAtLineEnd = false;
    m_updateCounter = 0;
    m_bookIsSigned = false;
    m_pageText = L"";
    m_currentPage = 0;
    ChangePage(0, 0);
    m_label->SetTextOnly(L"");
    m_label->ResizingComplete();
    m_caret->SetTextOnly(L"");
    SetCaretPosition(0, false, false);
    addEventListener(eFJEventType_EnterFrame, this, _UpdateBookAndQuill, false);
}

void FJ_BookAndQuillMenu::SetSafeZone(int top, int bottom, int left, int right)
{
    FJ_Document::SetSafeZone(top, bottom, left, right);
    double height = getStage()->getStageHeight();
    if (height == 360.0 || height == 540.0)
        setY(-static_cast<float>(top) - 176.0f);
}

void FJ_BookAndQuillMenu::SetBookIsSigned()
{
    FJ_FuiNode* close = getChildByName("ButtonClose");
    FJ_FuiNode* sign = getChildByName("ButtonSign");
    if (close != NULL && sign != NULL) {
        removeEventListener(eFJEventType_EnterFrame, this, _UpdateBookAndQuill);
        removeChild(sign, true);
        removeChild(close, true);
        removeChild(getChildByName("Caret"), true);
        m_caret = NULL;
    }
    m_bookIsSigned = true;
}

void FJ_BookAndQuillMenu::SetFontSizeOverride(int)
{
}

std::wstring FJ_BookAndQuillMenu::GetPageText(bool secondPage)
{
    if (m_label == NULL || m_label->GetTextField() == NULL)
        return m_pageText;
    m_label->GetTextField()->getNumLines();
    return m_pageText;
}

bool FJ_BookAndQuillMenu::SetPageText(const std::wstring& text, int mode)
{
    std::wstring display = text + L" ";
    if (m_label != NULL)
        m_label->SetTextOnly(display);
    if (m_label != NULL && m_label->GetTextField() != NULL && m_label->GetTextField()->getNumLines() > 13) {
        if (mode == 0)
            m_label->SetTextOnly(GetPageText(false));
        else if (mode == 1)
            m_label->SetTextOnly(text);
        else if (mode == 2) {
            m_label->SetTextOnly(GetPageText(false));
            m_pageText = GetPageText(false);
        }
        return false;
    }
    m_pageText = display;
    return true;
}

uint64_t FJ_BookAndQuillMenu::SetFocusToBook(bool focus)
{
    if (!m_bookIsSigned) {
        if (!focus)
            SetBookFocus(false);
        else {
            getStage()->setFocus(m_label);
            FJ_Document::OnFocusChange(true);
            SetBookFocus(true);
        }
    }
    return (uint64_t)(unsigned long)this;
}

bool FJ_BookAndQuillMenu::GetBookHasFocus()
{
    return getStage()->getFocus() != NULL && getStage()->getFocus()->isFJ_Label_Book();
}

void FJ_BookAndQuillMenu::OnFocusChange(bool focused)
{
    FJ_Document::OnFocusChange(focused);
    SetBookFocus(getStage()->getFocus() != NULL && getStage()->getFocus()->isFJ_Label_Book());
}

void FJ_BookAndQuillMenu::DoHorizontalResizeCheck()
{
    FJ_FuiNode* root = getChildAt(0);
    for (int i = 0; root != NULL && i < root->getNumChildren(); ++i) {
        FJ_Base* base = root->getChildAt(i)->asFJ_Base();
        if (base != NULL && base->isFJ_Button() && base->getNewWidth() > 0)
            base->ResizingComplete();
    }
}
