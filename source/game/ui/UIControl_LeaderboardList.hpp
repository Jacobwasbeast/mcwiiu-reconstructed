#ifndef GAME_UI_UICONTROL_LEADERBOARDLIST_HPP
#define GAME_UI_UICONTROL_LEADERBOARDLIST_HPP

#include "game/ui/UIControl_Base.hpp"

class UIControl_LeaderboardList : public UIControl_Base
{
public:
    UIControl_LeaderboardList();
    virtual ~UIControl_LeaderboardList();
    virtual void setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name);
    virtual void ReInit();
    void clearList();
    void init(int controlId);
    void setupTitles(const std::wstring& title, const std::wstring& subtitle);
    void initLeaderboard(int firstValue, int spacing, int offset);
    void setColumnIcon(int column, int icon);
    void addDataSet(bool loading, int rank, int value, const std::wstring& gamertag,
                    bool complete, const std::wstring& value0, const std::wstring& value1,
                    const std::wstring& value2, const std::wstring& value3,
                    const std::wstring& value4, const std::wstring& value5,
                    const std::wstring& value6);
    void SetTouchFocus(int x, int y, bool focus);

private:
    bool m_touchBoxAdded;
};

#endif
