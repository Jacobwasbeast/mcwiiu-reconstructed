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
    if (!m_caretVisible)
        m_caret->SetTextOnly(L" ");

    const std::wstring& label = *(std::wstring*)m_label->GetLabel();
    if (label.empty())
    {
        m_caret->setX(m_label->getX());
        m_caret->setY(m_label->getY());
    }
    else
    {
        const int index = position + (force ? 0 : GetCaretOffset(position));
        int clampedIndex = index;
        bool wasClamped = false;
        if (clampedIndex >= static_cast<int>(label.size()))
        {
            clampedIndex = static_cast<int>(label.size()) - 1;
            wasClamped = true;
        }

        if (label.substr(clampedIndex, 1).empty())
        {
            if (!force)
            {
                const std::wstring marked = label.substr(0, index) + L"#" + label.substr(index);
                m_caret->SetTextOnly(marked);
                SetCaretPosition(index, atEnd, true);
                m_label->SetTextOnly(label);
            }
        }
        else
        {
            const std::pair<float, float> boundsAtIndex = m_label->GetTextField()->getCharBoundaries(clampedIndex);
            const std::pair<float, float> boundsAtStart = m_label->GetTextField()->getCharBoundaries(0);
            const float widthAdjust = m_cursorAtLineEnd ? 0.0f : 1.0f;
            const float extraWidth = wasClamped ? (boundsAtIndex.second - boundsAtIndex.first) : 0.0f;
            m_caret->setX(m_label->getX() + boundsAtIndex.first - boundsAtStart.first - widthAdjust + extraWidth);
            m_caret->setY(m_label->getY());
        }
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
        setY(top);
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

void FJ_BookAndQuillMenu::SetFocusToBook(bool focus)
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
    FJ_FuiNode* signNode = getChildByName("ButtonSign");
    FJ_FuiNode* closeNode = getChildByName("ButtonClose");

    int signWidth = 0;
    int signX = 0;
    int closeX = 0;
    if (signNode != NULL && closeNode != NULL)
    {
        signWidth = static_cast<int>(signNode->getWidth());
        signX = static_cast<int>(signNode->getX());
        closeX = static_cast<int>(closeNode->getX());
    }

    bool anyLabelTooWide = false;
    int maxNewWidth = 0;
    for (int i = 0; i < m_root->getNumChildren(); ++i)
    {
        FJ_FuiNode* child = m_root->getChildAt(i);
        FJ_Base* base = child != NULL ? child->asFJ_Base() : NULL;
        if (base != NULL && base->isFJ_Button())
        {
            if (maxNewWidth < static_cast<int>(base->getNewWidth()))
                maxNewWidth = static_cast<int>(base->getNewWidth());
            if (base->getLabelTooWide())
                anyLabelTooWide = true;
        }
    }

    if (anyLabelTooWide)
    {
        for (int i = 0; i < m_root->getNumChildren(); ++i)
        {
            FJ_FuiNode* child = m_root->getChildAt(i);
            FJ_Base* base = child != NULL ? child->asFJ_Base() : NULL;
            if (base != NULL && base->isFJ_Button())
                base->SetNewObjectWidth(maxNewWidth);
        }
    }

    for (int i = 0; i < m_root->getNumChildren(); ++i)
    {
        FJ_FuiNode* child = m_root->getChildAt(i);
        if (child != NULL && child->isFJ_Base())
            child->asFJ_Base()->ResizingComplete();
    }

    const bool closeFound = (getChildByName("ButtonClose") != NULL);
    bool signFound = false;
    if (closeFound)
        signFound = (getChildByName("ButtonSign") != NULL);

    if (closeFound && signFound)
    {
        signNode->setX(static_cast<float>(signX + signWidth) - signNode->getWidth());
        closeNode->setX(static_cast<float>(closeX));
    }
}
