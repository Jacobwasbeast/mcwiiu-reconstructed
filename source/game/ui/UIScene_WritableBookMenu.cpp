#include "game/ui/UIScene_WritableBookMenu.hpp"

#include <cstring>
#include <cwchar>

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/render/Renderer.hpp"
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
#include "platform/wiiu/CInput.hpp"

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
    if (XGetLanguage() == 0x11 && !gRenderer->IsWidescreen())
    {
        FJ_BookAndQuillMenu* menu = static_cast<FJ_BookAndQuillMenu*>(m_movie->getRootNode()->getFuiNode());
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
    wchar_t signButtonLabels[2][256];
    wchar_t doneButtonLabels[2][256];

    swprintf(signButtonLabels[0], 256, L"%ls", L10N::GetString(0xdb6de516));
    swprintf(signButtonLabels[1], 256, L"%ls", L10N::GetString(0xa16c845c));
    m_signButton.setAllPossibleLabels(0, 2, signButtonLabels);

    swprintf(doneButtonLabels[0], 256, L"%ls", L10N::GetString(0x6d2a309c));
    swprintf(doneButtonLabels[1], 256, L"%ls", L10N::GetString(0x4fcbc7e7));
    m_doneButton.setAllPossibleLabels(0, 2, doneButtonLabels);

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
    int slot0 = 0x8d6323ed;
    int left = -1;
    int right = -1;
    if (!m_unused)
    {
        if (m_currentPage > 0)
            left = 0x14b3660;
        if (!m_canEdit)
        {
            if (m_currentPage < m_pageCount - 1)
                right = 0x1269f5be;
            slot0 = -1;
        }
        else
        {
            if (m_currentPage < m_pageCount - 1)
            {
                int tooltips[16] = {slot0, 0x4f6bcf5e, -1, -1, -1, -1, left, static_cast<int>(0x1269f5be),
                                    -1, -1, -1, -1, -1, -1, -1, -1};
                gConsoleUIController.SetTooltips(m_playerId, tooltips, false, false);
                return;
            }
            if (m_currentPage < 0x31)
            {
                int tooltips[16] = {slot0, 0x4f6bcf5e, -1, -1, -1, -1, left, static_cast<int>(0xe14c8f20),
                                    -1, -1, -1, -1, -1, -1, -1, -1};
                gConsoleUIController.SetTooltips(m_playerId, tooltips, false, false);
                return;
            }
        }
    }
    int tooltips[16] = {slot0, 0x4f6bcf5e, -1, -1, -1, -1, left, right,
                        -1, -1, -1, -1, -1, -1, -1, -1};
    gConsoleUIController.SetTooltips(m_playerId, tooltips, false, false);
}

void UIScene_WritableBookMenu::OnStopSigning()
{
    m_signing = false;
    m_pageLabel.setVisible(true);
    m_signButton.setLabel(UIString(L10N::GetString(0xdb6de516)), false, false);
    m_doneButton.setLabel(UIString(L10N::GetString(0x6d2a309c)), false, false);
    doHorizontalResizeCheck();
    m_caretPosition = static_cast<int>(m_pageText.size());
    FormatAndDisplayText(EFormatTextType_Display);
    SetCaretPosition(m_caretPosition);
    UpdatePageNumber();
    updateTooltips();
    SetFocusToBook(true);
    gConsoleUIController.TouchBoxRebuild(this);
}

int UIScene_WritableBookMenu::ExitBookDialogReturned(void* context, int, C4JStorage::EMessageResult result)
{
    if (context != 0 && result == 5)
        static_cast<UIScene_WritableBookMenu*>(context)->m_bookIsSigned = true;
    return 0;
}

unsigned int UIScene_WritableBookMenu::HandleCancel()
{
    if (m_unused)
    {
        m_unused = false;
        OnStopSigning();
        return 0;
    }
    if (m_textChanged)
    {
        int parameters[2] = { static_cast<int>(0x4fcbc7e7), static_cast<int>(0xb9a1bf4d) };
        gConsoleUIController.RequestAlertMessage(0xa95e7f9a, 0x5bdca79f, parameters, 2, m_playerId,
            ExitBookDialogReturned, this, NULL, true);
        return 0;
    }
    navigateBack();
    gConsoleUIController.PlayUISFX(SoundEvents::UI_BACK);
    return 0;
}

bool UIScene_WritableBookMenu::GetBookHasFocus()
{
    FJ_BookAndQuillMenu* menu = static_cast<FJ_BookAndQuillMenu*>(m_movie->getRootNode()->getFuiNode());
    return menu != 0 && menu->GetBookHasFocus();
}

int UIScene_WritableBookMenu::KeyboardCompleteCallback(void* context, bool success)
{
    unsigned int callbackId = static_cast<unsigned int>(reinterpret_cast<unsigned long>(context));
    UIScene_WritableBookMenu* scene =
        static_cast<UIScene_WritableBookMenu*>(gConsoleUIController.GetSceneFromCallbackId(callbackId));
    if (scene == 0)
        return 0;

    scene->m_bookIsSigned = false;
    if (!success)
        return 0;

    wchar_t text[260];
    std::memset(text, 0, sizeof(wchar_t) * 257);
    if (!scene->m_unused)
    {
        g_pInput->GetText(text, 0x100);
        scene->m_pageText.assign(text, wcslen(text));
        scene->setCurrentPageText(scene->m_pageText);
        scene->m_caretPosition = static_cast<int>(scene->m_pageText.size());
    }
    else
    {
        g_pInput->GetText(text, 0x10);
        scene->m_secondPageText.assign(text, wcslen(text));
        scene->m_caretPosition = static_cast<int>(scene->m_secondPageText.size());
    }
    scene->addTimer(10, 50);
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
    m_pageLabel.setVisible(false);
    m_signButton.setLabel(UIString(L10N::GetString(0xa16c845c)), false, false);
    m_doneButton.setLabel(UIString(L10N::GetString(0x4fcbc7e7)), false, false);
    doHorizontalResizeCheck();
    m_caretPosition = 0;
    FormatAndDisplayText(EFormatTextType_Display);
    SetCaretPosition(m_caretPosition);
    UpdatePageNumber();
    updateTooltips();
    SetFocusToBook(true);
    gConsoleUIController.TouchBoxRebuild(this);
}

bool UIScene_WritableBookMenu::SaveChanges(bool signing)
{
    if (!m_canEdit)
        return false;
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
    if (action < 10)
    {
        if (action < 4)
            return;
        if (action > 5)
        {
            if (action == 6)
            {
                if (GetBookHasFocus())
                {
                    if (!released && !repeat)
                        return;
                    int caret = m_caretPosition;
                    if (!m_unused)
                    {
                        if (caret < static_cast<int>(m_pageText.size()))
                            ++caret;
                        if (caret > 0x100)
                            caret = 0x100;
                    }
                    else
                    {
                        if (caret < static_cast<int>(m_secondPageText.size()))
                            ++caret;
                        if (caret > 0x10)
                            caret = 0x10;
                    }
                    m_caretPosition = caret;
                    SetCaretPosition(m_caretPosition);
                    handled = true;
                    return;
                }
            }
            else
            {
                if (action != 7)
                    return;
                if (GetBookHasFocus())
                {
                    if (!released && !repeat)
                        return;
                    int caret = m_caretPosition - 1;
                    if (caret > -1)
                        m_caretPosition = caret;
                    else
                        m_caretPosition = 0;
                    SetCaretPosition(m_caretPosition);
                    handled = true;
                    return;
                }
            }
        }
        UIScene::sendInputToMovie(action, repeat, released, pressed);
    }
    else if (action < 0xb)
    {
        if (released)
        {
            ScrollPageRight();
            handled = true;
            return;
        }
    }
    else if (action == 0xb)
    {
        if (released)
        {
            ScrollPageLeft();
            handled = true;
            return;
        }
    }
    else if (action != 0x14)
    {
        if (action != 0x15)
            return;
        if (!released)
            return;
        HandleCancel();
        handled = true;
        return;
    }
    else
    {
        if (!released)
            return;
        if (!GetBookHasFocus())
        {
            if (m_signButton.hasFocus())
            {
                OnSignButtonPress();
                handled = true;
                return;
            }
            if (m_doneButton.hasFocus())
            {
                OnDoneButtonPress();
                handled = true;
                return;
            }
        }
        else if (m_canEdit)
        {
            m_bookIsSigned = true;
            unsigned int maxLen = m_unused ? 0x10 : 0x100;
            if (m_unusedControlByte == 0)
            {
                if (!m_unused)
                {
                    g_pInput->RequestKeyboard(L10N::GetString((int)0x671ea8b2u), m_pageText.c_str(),
                                              m_playerId, maxLen, KeyboardCompleteCallback,
                                              reinterpret_cast<void*>(GetCallbackUniqueId()),
                                              static_cast<C_4JInput::EKeyboardMode>(3));
                }
                else
                {
                    g_pInput->RequestKeyboard(L10N::GetString((int)0xc5848e87u), m_secondPageText.c_str(),
                                              m_playerId, maxLen, KeyboardCompleteCallback,
                                              reinterpret_cast<void*>(GetCallbackUniqueId()),
                                              static_cast<C_4JInput::EKeyboardMode>(3));
                }
                handled = true;
                return;
            }
        }
    }
    handled = true;
}

void UIScene_WritableBookMenu::handleTouchInput(unsigned int, int action, int, int, bool, bool, bool pressed)
{
    if (!pressed) return;
    if (action == 2)
    {
        if (m_canEdit)
        {
            if (GetBookHasFocus())
            {
                bool handled = false;
                handleInput(m_playerId, 0x14, false, true, false, handled);
                return;
            }
            SetFocusToBook(true);
            m_doneButton.setFocus(false);
            m_signButton.setFocus(false);
        }
        return;
    }
    if (action == 3) ScrollPageLeft();
    else if (action == 4) ScrollPageRight();
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
