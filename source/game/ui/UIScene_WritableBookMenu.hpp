#ifndef GAME_UI_UISCENE_WRITABLEBOOKMENU_HPP
#define GAME_UI_UISCENE_WRITABLEBOOKMENU_HPP

#include <string>

#include "game/ui/UIScene.hpp"
#include "game/ui/UIControl_Button.hpp"
#include "game/ui/UIControl_Label.hpp"
#include "game/ui/UIControl_Touch.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/core/CMinecraftApp.hpp"

class ItemInstance;
class ListTag;
class UILayer;

class UIScene_WritableBookMenu : public UIScene
{
public:
    enum EFormatTextType
    {
        EFormatTextType_Display = 0,
        EFormatTextType_Sign = 1,
        EFormatTextType_Keyboard = 2
    };

    UIScene_WritableBookMenu(int type, void* data, UILayer* layer);
    virtual ~UIScene_WritableBookMenu();
    virtual std::wstring getMoviePath();
    virtual unsigned int mapElementsAndNames();
    virtual int getSceneType();
    virtual void tick();
    virtual void SetFocusToElement(int focus);
    virtual void handleTimerComplete(int timerId);
    virtual void updateTooltips();
    virtual void handleInput(int player, int action, bool repeat, bool released, bool pressed, bool& handled);
    virtual void handleDestroy();
    virtual void handlePreReload();
    virtual void handleReload();
    virtual void handleTouchInput(unsigned int player, int action, int x, int y, bool repeat, bool released, bool pressed);

    void HandleFontSizeOverride();
    void SetBookIsSigned();
    void SetPageLabel(std::wstring& text);
    void ChangePage(int page, int pageCount);
    void UpdatePageNumber();
    void Init();
    void RetrieveBookText();
    void SetCaretPosition(int position);
    int FormatAndDisplayText(EFormatTextType type);
    void SetFocusToBook(bool focus);
    void OnStopSigning();
    static int ExitBookDialogReturned(void* context, int button, C4JStorage::EMessageResult result);
    unsigned int HandleCancel();
    bool GetBookHasFocus();
    static int KeyboardCompleteCallback(void* context, bool success);
    void StoreBookText();
    void OnStartSigning();
    bool SaveChanges(bool signing);
    void Sign();
    bool OnSignButtonPress();
    bool OnDoneButtonPress();
    void ScrollPageLeft();
    bool appendPageToBook();
    void ScrollPageRight();
    void GetPageText();
    void setCurrentPageText(std::wstring& text);
    std::wstring getCurrentPageText();
    void RetrieveTitleText();

private:
    not_null_ptr<ItemInstance> m_book;
    ListTag* m_pages;
    int m_caretPosition;
    int m_currentPage;
    int m_pageCount;
    std::wstring m_pageText;
    std::wstring m_secondPageText;
    std::wstring m_titleText;
    bool m_bookHasFocus;
    bool m_signing;
    unsigned short m_unusedWord;
    int m_textLength;
    bool m_canEdit;
    bool m_textChanged;
    bool m_titleChanged;
    unsigned char m_reserved[0x1fd];
    int m_controlState;
    bool m_bookIsSigned;
    bool m_unused;
    unsigned char m_unusedControlByte;
    UIControl_Label m_pageLabel;
    UIControl_Button m_signButton;
    UIControl_Button m_doneButton;
    UIControl_Touch m_bookTouch;
    UIControl_Touch m_leftTouch;
    UIControl_Touch m_rightTouch;
};

#endif
