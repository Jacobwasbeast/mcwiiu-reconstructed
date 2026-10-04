#include "game/ui/FJ_LeaderboardMenu.hpp"

#include <string>

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_Label.hpp"
#include "game/ui/FJ_LeaderboardList.hpp"
#include "game/ui/fuiRenderNode.hpp"

FJ_LeaderboardMenu::FJ_LeaderboardMenu(fuiRenderNode* renderNode)
    : FJ_Document(renderNode)
{
}

FJ_Document* FJ_LeaderboardMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_LeaderboardMenu(renderNode);
}

unsigned int FJ_LeaderboardMenu::getType()
{
    return 0x19;
}

void FJ_LeaderboardMenu::ASConstructor()
{
    FJ_Label* infoLabel = getChildByName("Info")->asFJ_Label();
    infoLabel->SetLabel(L"");

    FJ_LeaderboardList* leaderboardList = static_cast<FJ_LeaderboardList*>(getChildByName("Gamers"));
    leaderboardList->SetupPleaseWait(infoLabel);
}

void FJ_LeaderboardMenu::_keyDownHandler(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_Document*>(node)->keyDownHandler(event);
}

void FJ_LeaderboardMenu::keyDownHandler(FJ_Event* event)
{
    FJ_FuiNode* gamersNode = getChildByName("Gamers");
    FJ_LeaderboardList* leaderboardList = static_cast<FJ_LeaderboardList*>(gamersNode);

    if (event->m_eType == eFJEventType_FocusOut)
    {
        leaderboardList->NavigateLeaderboard(true, false);
    }
    else if (event->m_eType == eFJEventType_FocusChangeInit)
    {
        leaderboardList->NavigateLeaderboard(false, false);
    }
    else if (event->m_eType == eFJEventType_KeyDown)
    {
        leaderboardList->NavigateLeaderboard(true, true);
    }
    else if (event->m_eType == eFJEventType_FocusIn)
    {
        leaderboardList->NavigateLeaderboard(false, true);
    }
}
