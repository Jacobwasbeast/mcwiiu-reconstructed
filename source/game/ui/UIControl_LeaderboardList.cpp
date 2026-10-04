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
    init(m_controlId);
}

void UIControl_LeaderboardList::setupTitles(const std::wstring& title,
                                            const std::wstring& subtitle)
{
    m_fuiRenderNode->getFuiNode()->asFJ_LeaderboardList()->SetupTitles(title, subtitle);
}

void UIControl_LeaderboardList::initLeaderboard(int focusRank, int totalEntries, int numColumns)
{
    FJ_LeaderboardList* list = m_fuiRenderNode->getFuiNode()->asFJ_LeaderboardList();
    list->InitLeaderboard(focusRank, totalEntries, numColumns);
    if (!m_touchBoxAdded) {
        int layerId = m_scene->m_layer->m_layerId;
        if (layerId == 4 || layerId == 6)
            gConsoleUIController.TouchBoxAdd(this, m_scene);
        m_touchBoxAdded = true;
    }
}

void UIControl_LeaderboardList::setColumnIcon(int column, int icon)
{
    int offsetIcon = 0;
    if (icon > 32000)
        offsetIcon = icon - 32000;
    m_fuiRenderNode->getFuiNode()->asFJ_LeaderboardList()->SetColumnIcon(column, offsetIcon);
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
    const std::wstring* firstColumn = 0;
    if (!firstColumnText.empty())
        firstColumn = &firstColumnText;
    const std::wstring* secondColumn = 0;
    if (!secondColumnText.empty())
        secondColumn = &secondColumnText;
    const std::wstring* thirdColumn = 0;
    if (!thirdColumnText.empty())
        thirdColumn = &thirdColumnText;
    const std::wstring* fourthColumn = 0;
    if (!fourthColumnText.empty())
        fourthColumn = &fourthColumnText;
    const std::wstring* fifthColumn = 0;
    if (!fifthColumnText.empty())
        fifthColumn = &fifthColumnText;
    const std::wstring* sixthColumn = 0;
    if (!sixthColumnText.empty())
        sixthColumn = &sixthColumnText;
    const std::wstring* seventhColumn = 0;
    if (!seventhColumnText.empty())
        seventhColumn = &seventhColumnText;
    m_fuiRenderNode->getFuiNode()->asFJ_LeaderboardList()->AddDataSet(loading, rank, value, gamertag,
        complete, firstColumn, secondColumn, thirdColumn, fourthColumn, fifthColumn,
        sixthColumn, seventhColumn);
}

void UIControl_LeaderboardList::SetTouchFocus(int x, int y, bool focus)
{
    m_fuiRenderNode->getFuiNode()->asFJ_LeaderboardList()->SetTouchFocus(x, y, focus);
}
