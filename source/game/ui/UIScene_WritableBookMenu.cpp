#include "game/ui/UIScene_WritableBookMenu.hpp"

#include <cwchar>

#include "game/core/ByteArrayOutputStream.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/data/StringTag.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/WrittenBookItem.hpp"
#include "game/network/ClientPacketListener.hpp"
#include "game/network/ServerboundCustomPayloadPacket.hpp"
#include "game/ui/FJ_BookAndQuillMenu.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"

extern unsigned int XGetLanguage();
extern std::wstring _toString(int value);
extern std::wstring trimString(const std::wstring& text);
extern ConsoleUIController gConsoleUIController;

UIScene_WritableBookMenu::UIScene_WritableBookMenu(int type, void* data, UILayer* layer)
    : UIScene(type, layer), m_book(static_cast<ItemInstance*>(data)), m_pages(0), m_caretPosition(0),
      m_currentPage(0), m_pageCount(1), m_pageText(), m_secondPageText(), m_titleText(),
      m_bookHasFocus(false), m_signing(false), m_unusedWord(0), m_textLength(0), m_canEdit(true),
      m_textChanged(false), m_titleChanged(false), m_controlState(0), m_bookIsSigned(false), m_unused(false),
      m_unusedControlByte(0)
{
    Init();
}

UIScene_WritableBookMenu::~UIScene_WritableBookMenu()
{
    if (m_layer != 0)
        m_layer->removeComponent((EUIScene)0x48);
}

std::wstring UIScene_WritableBookMenu::getMoviePath()
{
    return g_pApp->GetLocalPlayerCount() < 2 ? L"BookAndQuillMenu" : L"BookAndQuillMenuSplit";
}

unsigned int UIScene_WritableBookMenu::mapElementsAndNames()
{
    UIScene::mapElementsAndNames();
    fuiRenderNode* root = m_movie->getRootNode();
    m_pageLabel.setupControl(this, root, "PageLabel");
    m_signButton.setupControl(this, root, "ButtonSign");
    m_doneButton.setupControl(this, root, "ButtonClose");
    m_bookTouch.setupControl(this, root, "TouchControlBook");
    m_leftTouch.setupControl(this, root, "TouchControlScrollLeft");
    m_rightTouch.setupControl(this, root, "TouchControlScrollRight");
    return 1;
}

void UIScene_WritableBookMenu::HandleFontSizeOverride()
{
    if (XGetLanguage() == 0x11 && !g_pApp->IsLocaleKorea())
    {
        FJ_BookAndQuillMenu* menu = static_cast<FJ_BookAndQuillMenu*>(m_movie->getRootNode()->getFuiNode());
        if (menu != 0)
            menu->SetFontSizeOverride(11);
    }
}

void UIScene_WritableBookMenu::SetBookIsSigned()
{
    FJ_BookAndQuillMenu* menu = static_cast<FJ_BookAndQuillMenu*>(m_movie->getRootNode()->getFuiNode());
    if (menu != 0)
        menu->SetBookIsSigned();
    m_signButton.removeUINodeRef();
    m_doneButton.removeUINodeRef();
}

void UIScene_WritableBookMenu::SetPageLabel(std::wstring& text)
{
    m_pageLabel.setLabel(UIString(text), false, false);
}

void UIScene_WritableBookMenu::ChangePage(int page, int pageCount)
{
    FJ_BookAndQuillMenu* menu = static_cast<FJ_BookAndQuillMenu*>(m_movie->getRootNode()->getFuiNode());
    if (menu != 0)
        menu->ChangePage(page, pageCount);
}

void UIScene_WritableBookMenu::UpdatePageNumber()
{
    std::wstring label = L"Page %d of %d";
    std::wstring first = _toString(m_currentPage + 1);
    std::wstring second = _toString(m_pageCount);
    std::wstring::size_type pos = label.find(L"%d");
    if (pos != std::wstring::npos)
        label.replace(pos, 2, first);
    pos = label.find(L"%d");
    if (pos != std::wstring::npos)
        label.replace(pos, 2, second);
    SetPageLabel(label);
    if (!m_bookIsSigned)
        ChangePage(m_currentPage, m_pageCount - 1);
    else if (m_signing)
        ChangePage(0, 0);
    else
        ChangePage(m_currentPage, 0x31);
}

void UIScene_WritableBookMenu::Init()
{
    m_canEdit = WrittenBookItem::HasBookTags(m_book);
    if (m_book != 0 && m_canEdit)
    {
        CompoundTag* tag = m_book->getTag();
        if (tag != 0)
        {
            m_titleText = tag->getString(L"title");
            if (!m_titleText.empty())
                m_titleChanged = true;
            if (!m_bookIsSigned)
            {
                ListTag* pages = tag->getList(L"pages", 8);
                if (pages != 0 && pages->size() > 0)
                {
                    m_pages = pages;
                    m_pageCount = pages->size();
                }
            }
        }
    }
    if (m_pages == 0 && m_canEdit)
    {
        m_pages = new ListTag(8);
        m_pages->add(new StringTag(L""));
        m_pageCount = 1;
    }
    UpdatePageNumber();
}

void UIScene_WritableBookMenu::RetrieveBookText()
{
    GetPageText();
    m_caretPosition = static_cast<int>(m_pageText.size());
}

void UIScene_WritableBookMenu::SetCaretPosition(int position)
{
    FJ_BookAndQuillMenu* menu = static_cast<FJ_BookAndQuillMenu*>(m_movie->getRootNode()->getFuiNode());
    if (menu != 0)
        menu->SetCaretPosition(position + m_textLength, !m_bookIsSigned, false);
}

int UIScene_WritableBookMenu::FormatAndDisplayText(EFormatTextType type)
{
    FJ_BookAndQuillMenu* menu = static_cast<FJ_BookAndQuillMenu*>(m_movie->getRootNode()->getFuiNode());
    if (menu == 0)
        return 0;
    if (type == EFormatTextType_Keyboard)
        return menu->SetPageText(m_pageText, 2) ? 1 : 0;
    return menu->SetPageText(m_pageText, type == EFormatTextType_Sign ? 1 : 0) ? 1 : 0;
}

void UIScene_WritableBookMenu::tick()
{
    UIScene::tick();
    if (m_bookIsSigned)
        navigateBack();
}

void UIScene_WritableBookMenu::handlePreReload()
{
    HandleFontSizeOverride();
}

void UIScene_WritableBookMenu::SetFocusToBook(bool focus)
{
    FJ_BookAndQuillMenu* menu = static_cast<FJ_BookAndQuillMenu*>(m_movie->getRootNode()->getFuiNode());
    if (menu != 0)
        menu->SetFocusToBook(focus);
    m_bookHasFocus = focus;
}

void UIScene_WritableBookMenu::handleReload()
{
    UpdatePageNumber();
    if (!m_bookIsSigned)
        SetBookIsSigned();
    doHorizontalResizeCheck();
    FormatAndDisplayText(EFormatTextType_Display);
    if (m_bookIsSigned)
        SetCaretPosition(m_caretPosition);
    if (m_doneButton.hasFocus() || m_signButton.hasFocus())
        SetFocusToBook(false);
    gConsoleUIController.TouchBoxRebuild(this);
}

void UIScene_WritableBookMenu::updateTooltips()
{
    int left = m_currentPage > 0 ? 0x14b3660 : -1;
    int right = -1;
    if (!m_bookIsSigned && m_currentPage < m_pageCount - 1)
        right = 0x1269f5be;
    int tooltips[16] = {m_bookIsSigned ? 0x8d6323ed : -1, 0x4f6bcf5e, -1, -1, -1, -1,
                        left, right, -1, -1, -1, -1, -1, -1, -1, -1};
    gConsoleUIController.SetTooltips(m_playerId, tooltips, false, false);
}

void UIScene_WritableBookMenu::OnStopSigning()
{
    m_signing = false;
    UpdatePageNumber();
    SetFocusToBook(true);
}

unsigned int UIScene_WritableBookMenu::ExitBookDialogReturned(void* context, int, C4JStorage::EMessageResult result)
{
    if (context != 0 && result == 5)
        static_cast<UIScene_WritableBookMenu*>(context)->m_bookIsSigned = true;
    return 0;
}

unsigned int UIScene_WritableBookMenu::HandleCancel()
{
    if (m_signing)
        OnStopSigning();
    else
        navigateBack();
    return 0;
}

bool UIScene_WritableBookMenu::GetBookHasFocus()
{
    FJ_BookAndQuillMenu* menu = static_cast<FJ_BookAndQuillMenu*>(m_movie->getRootNode()->getFuiNode());
    return menu != 0 && menu->GetBookHasFocus();
}

unsigned int UIScene_WritableBookMenu::KeyboardCompleteCallback(void* context, bool success)
{
    UIScene_WritableBookMenu* scene = static_cast<UIScene_WritableBookMenu*>(context);
    if (scene != 0 && success)
    {
        scene->m_textChanged = true;
        scene->addTimer(10, 50);
    }
    return 0;
}

void UIScene_WritableBookMenu::StoreBookText()
{
    m_textChanged = true;
    setCurrentPageText(m_pageText);
}

void UIScene_WritableBookMenu::OnStartSigning()
{
    m_signing = true;
    SetFocusToBook(false);
    UpdatePageNumber();
}

bool UIScene_WritableBookMenu::SaveChanges(bool signing)
{
    if (!m_bookIsSigned || signing)
    {
        if (m_book == 0)
            return false;
        if (!m_book->hasTag())
            m_book->setTag(new CompoundTag());
        CompoundTag* tag = m_book->getTag();
        tag->putString(L"author", m_titleText);
        if (m_pages != 0)
            tag->put(L"pages", m_pages);
        tag->putByte(L"resolved", signing ? 1 : 0);
        m_textChanged = false;
    }
    return true;
}

void UIScene_WritableBookMenu::Sign()
{
    SaveChanges(true);
    navigateBack();
    gConsoleUIController.PlayUISFX(0);
}

bool UIScene_WritableBookMenu::OnSignButtonPress()
{
    if (!m_signing)
    {
        if (m_textChanged)
            StoreBookText();
        OnStartSigning();
        return true;
    }
    if (!trimString(m_titleText).empty())
        Sign();
    return true;
}

bool UIScene_WritableBookMenu::OnDoneButtonPress()
{
    if (!m_bookIsSigned)
        return false;
    if (m_signing)
    {
        OnStopSigning();
        return true;
    }
    if (m_textChanged)
    {
        StoreBookText();
        SaveChanges(false);
    }
    navigateBack();
    gConsoleUIController.PlayUISFX(0);
    return true;
}

void UIScene_WritableBookMenu::ScrollPageLeft()
{
    if (m_currentPage > 0 && !m_signing)
    {
        if (m_textChanged) StoreBookText();
        --m_currentPage;
        RetrieveBookText();
        UpdatePageNumber();
        updateTooltips();
        FormatAndDisplayText(EFormatTextType_Display);
        SetCaretPosition(m_caretPosition);
    }
}

bool UIScene_WritableBookMenu::appendPageToBook()
{
    if (m_pages != 0 && m_pages->size() < 50)
    {
        m_pages->add(new StringTag(L""));
        ++m_pageCount;
        m_textChanged = true;
        return true;
    }
    return false;
}

void UIScene_WritableBookMenu::ScrollPageRight()
{
    if (m_signing) return;
    if (m_textChanged) StoreBookText();
    if (m_currentPage >= m_pageCount - 1 && !appendPageToBook()) return;
    ++m_currentPage;
    RetrieveBookText();
    FormatAndDisplayText(EFormatTextType_Display);
    SetCaretPosition(m_caretPosition);
    UpdatePageNumber();
    updateTooltips();
}

void UIScene_WritableBookMenu::handleInput(int player, int action, bool repeat, bool released, bool pressed, bool& handled)
{
    gConsoleUIController.AnimateKeyPress(player, action, repeat, released, pressed);
    if (action == 10 && pressed) { ScrollPageRight(); handled = true; return; }
    if (action == 11 && pressed) { ScrollPageLeft(); handled = true; return; }
    if (action == 0x15 && pressed) { HandleCancel(); handled = true; return; }
    if (action == 0x14 && pressed)
    {
        if (GetBookHasFocus()) SetCaretPosition(m_caretPosition + 1);
        else if (m_signButton.hasFocus()) OnSignButtonPress();
        else if (m_doneButton.hasFocus()) OnDoneButtonPress();
        handled = true;
        return;
    }
    UIScene::handleInput(player, action, repeat, released, pressed, handled);
}

void UIScene_WritableBookMenu::handleTouchInput(unsigned int, int action, int, int, bool, bool, bool pressed)
{
    if (!pressed) return;
    if (action == 3) ScrollPageLeft();
    else if (action == 4) ScrollPageRight();
    else if (action == 2) SetFocusToBook(true);
}

void UIScene_WritableBookMenu::SetFocusToElement(int focus)
{
    if (focus < 2) SetFocusToBook(false);
    UIScene::SetFocusToElement(focus);
}

void UIScene_WritableBookMenu::handleDestroy()
{
    gConsoleUIController.TouchBoxesClear(this);
}

void UIScene_WritableBookMenu::GetPageText()
{
    if (m_pages != 0 && m_currentPage >= 0 && m_currentPage < m_pages->size())
        m_pageText = m_pages->getString(m_currentPage);
    else
        m_pageText.clear();
}

void UIScene_WritableBookMenu::handleTimerComplete(int timerId)
{
    if (timerId == 10)
    {
        killTimer(10);
        if (!FormatAndDisplayText(EFormatTextType_Keyboard))
        {
            GetPageText();
            m_caretPosition = static_cast<int>(m_pageText.size());
            FormatAndDisplayText(EFormatTextType_Display);
        }
        SetCaretPosition(m_caretPosition);
    }
}

void UIScene_WritableBookMenu::setCurrentPageText(std::wstring& text)
{
    if (m_pages != 0 && m_currentPage >= 0 && m_currentPage < m_pages->size())
    {
        if (m_pages->get(m_currentPage) != 0)
        {
            m_pages->set(m_currentPage, new StringTag(text));
            m_textChanged = true;
        }
    }
}

std::wstring UIScene_WritableBookMenu::getCurrentPageText()
{
    if (m_pages != 0 && m_currentPage >= 0 && m_currentPage < m_pages->size())
        return m_pages->getString(m_currentPage);
    return L"map/set<T> root";
}

int UIScene_WritableBookMenu::getSceneType()
{
    return 0x3d;
}
