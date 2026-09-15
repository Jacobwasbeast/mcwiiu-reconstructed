#ifndef GAME_UI_UISCENE_CREDITS_HPP
#define GAME_UI_UISCENE_CREDITS_HPP

#include <string>
#include <vector>

#include "game/ui/UIScene.hpp"

struct SCreditTextItemDef
{
    wchar_t* text;
    int primaryId;
    int secondaryId;
    int type;
};

class UILayer;

class UIScene_Credits : public UIScene
{
public:
    enum ECreditTextTypes
    {
        ECreditTextTypes_0 = 0
    };

    enum ECreditIcons
    {
        ECreditIcons_0 = 0
    };

    UIScene_Credits(int type, void* data, UILayer* layer);
    virtual ~UIScene_Credits();

    bool AlreadySeenCreditText(const std::wstring& text);
    void AddCreditText(const wchar_t* text);
    void addCreditLine(const std::wstring& text);
    int GetDLCCreditsCount();
    void setNextLabel(const std::wstring& text, ECreditTextTypes type);
    virtual void updateTooltips();
    virtual void updateComponents();
    virtual void handleReload();
    SCreditTextItemDef* GetDLCCredits(int index);
    void addImage(ECreditIcons icon);
    virtual void tick();
    virtual std::wstring getMoviePath();
    virtual void handleInput(int playerId, int action, bool repeat, bool released, bool pressed,
                             bool& handled);
    virtual void handleRequestMoreData(int id, bool forward);
    virtual int getSceneType();
    void setLogoLanguage(int language);

private:
    int m_currentCredit;
    int m_creditCount;
    bool m_requestMoreData;
    std::vector<SCreditTextItemDef*> m_dlcCredits;
    std::vector<std::wstring> m_seenCreditText;
    int m_unknownF0;
};

#endif
