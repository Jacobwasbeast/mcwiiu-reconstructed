#include "game/ui/UIControl_LeaderboardList.hpp"

#include <string>

#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_LeaderboardList.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"

class FJ_LeaderboardList;

class ConsoleUIController
{
public:
    void TouchBoxAdd(UIControl* control, UIScene* scene);
};

extern ConsoleUIController g_ConsoleUIController;

void __CPR220__AddDataSet__18FJJ_LeaderboardListFbiT2RCQ2_3std78basic_string__tm__58_wQ2_3std20char_traits__tm__2_wQ2_3std18allocator__tm__2_wT1PCQ2_3stdJ46JN66(
    FJ_LeaderboardList* list, bool loading, int rank, int value,
    const std::wstring& gamertag, bool complete, const std::wstring* value0,
    const std::wstring* value1, const std::wstring* value2,
    const std::wstring* value3, const std::wstring* value4,
    const std::wstring* value5, const std::wstring* value6);

UIControl_LeaderboardList::UIControl_LeaderboardList()
    : UIControl_Base()
{
}

UIControl_LeaderboardList::~UIControl_LeaderboardList()
{
}

void UIControl_LeaderboardList::clearList()
{
    m_fuiRenderNode->getFuiNode()->asFJ_LeaderboardList()->ResetLeaderboard();
}

void UIControl_LeaderboardList::setupControl(UIScene* scene, fuiRenderNode* node,
                                             const std::string& name)
{
    m_controlType = 9;
    UIControl_Base::setupControl(scene, node, name);
    m_touchBoxAdded = false;
}

void UIControl_LeaderboardList::init(int controlId)
{
    m_controlType = controlId;
}

void UIControl_LeaderboardList::ReInit()
{
    UIControl::ReInit();
    init(m_controlType);
}

void UIControl_LeaderboardList::setupTitles(const std::wstring& title,
                                            const std::wstring& subtitle)
{
    m_fuiRenderNode->getFuiNode()->asFJ_LeaderboardList()->SetupTitles(title, subtitle);
}

void UIControl_LeaderboardList::initLeaderboard(int firstValue, int spacing, int offset)
{
    FJ_LeaderboardList* list = m_fuiRenderNode->getFuiNode()->asFJ_LeaderboardList();
    list->InitLeaderboard(firstValue, spacing, offset);
    if (!m_touchBoxAdded) {
        if (m_scene->getSceneType() == 4 || m_scene->getSceneType() == 6)
            g_ConsoleUIController.TouchBoxAdd(this, m_scene);
        m_touchBoxAdded = true;
    }
}

void UIControl_LeaderboardList::setColumnIcon(int column, int icon)
{
    if (icon <= 32000)
        icon = 0;
    else
        icon -= 32000;
    m_fuiRenderNode->getFuiNode()->asFJ_LeaderboardList()->SetColumnIcon(column, icon);
}

void UIControl_LeaderboardList::addDataSet(bool loading, int rank, int value,
                                            const std::wstring& gamertag, bool complete,
                                            const std::wstring& value0,
                                            const std::wstring& value1,
                                            const std::wstring& value2,
                                            const std::wstring& value3,
                                            const std::wstring& value4,
                                            const std::wstring& value5,
                                            const std::wstring& value6)
{
    const std::wstring* p0 = value0.empty() ? 0 : &value0;
    const std::wstring* p1 = value1.empty() ? 0 : &value1;
    const std::wstring* p2 = value2.empty() ? 0 : &value2;
    const std::wstring* p3 = value3.empty() ? 0 : &value3;
    const std::wstring* p4 = value4.empty() ? 0 : &value4;
    const std::wstring* p5 = value5.empty() ? 0 : &value5;
    const std::wstring* p6 = value6.empty() ? 0 : &value6;
    __CPR220__AddDataSet__18FJJ_LeaderboardListFbiT2RCQ2_3std78basic_string__tm__58_wQ2_3std20char_traits__tm__2_wQ2_3std18allocator__tm__2_wT1PCQ2_3stdJ46JN66(
        m_fuiRenderNode->getFuiNode()->asFJ_LeaderboardList(), loading, rank, value, gamertag,
        complete, p0, p1, p2, p3, p4, p5, p6);
}

void UIControl_LeaderboardList::SetTouchFocus(int x, int y, bool focus)
{
    m_fuiRenderNode->getFuiNode()->asFJ_LeaderboardList()->SetTouchFocus(x, y, focus);
}
