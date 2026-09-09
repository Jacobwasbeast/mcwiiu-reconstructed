#include "game/ui/UIControl_LeaderboardList.hpp"
#include "game/core/ConsoleUIController.hpp"

#include <string>

#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_LeaderboardList.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"

extern ConsoleUIController gConsoleUIController;

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
    m_controlType = UIControl::eUIControlType_LeaderboardList;
    UIControl_Base::setupControl(scene, node, name);
    m_touchBoxAdded = false;
}

void UIControl_LeaderboardList::init(int controlId)
{
    m_controlId = controlId;
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
            gConsoleUIController.TouchBoxAdd(this, m_scene);
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
                                            const std::wstring& firstColumnText,
                                            const std::wstring& secondColumnText,
                                            const std::wstring& thirdColumnText,
                                            const std::wstring& fourthColumnText,
                                            const std::wstring& fifthColumnText,
                                            const std::wstring& sixthColumnText,
                                            const std::wstring& seventhColumnText)
{
    const std::wstring* firstColumn = firstColumnText.empty() ? 0 : &firstColumnText;
    const std::wstring* secondColumn = secondColumnText.empty() ? 0 : &secondColumnText;
    const std::wstring* thirdColumn = thirdColumnText.empty() ? 0 : &thirdColumnText;
    const std::wstring* fourthColumn = fourthColumnText.empty() ? 0 : &fourthColumnText;
    const std::wstring* fifthColumn = fifthColumnText.empty() ? 0 : &fifthColumnText;
    const std::wstring* sixthColumn = sixthColumnText.empty() ? 0 : &sixthColumnText;
    const std::wstring* seventhColumn = seventhColumnText.empty() ? 0 : &seventhColumnText;
    __CPR220__AddDataSet__18FJJ_LeaderboardListFbiT2RCQ2_3std78basic_string__tm__58_wQ2_3std20char_traits__tm__2_wQ2_3std18allocator__tm__2_wT1PCQ2_3stdJ46JN66(
        m_fuiRenderNode->getFuiNode()->asFJ_LeaderboardList(), loading, rank, value, gamertag,
        complete, firstColumn, secondColumn, thirdColumn, fourthColumn, fifthColumn,
        sixthColumn, seventhColumn);
}

void UIControl_LeaderboardList::SetTouchFocus(int x, int y, bool focus)
{
    m_fuiRenderNode->getFuiNode()->asFJ_LeaderboardList()->SetTouchFocus(x, y, focus);
}
