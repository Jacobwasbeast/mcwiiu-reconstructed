#include "game/ui/FJ_DLCMainMenu.hpp"

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNode.hpp"

FJ_DLCMainMenu::FJ_DLCMainMenu(fuiRenderNode* renderNode)
    : FJ_Document(renderNode)
{
}

FJ_DLCMainMenu::~FJ_DLCMainMenu()
{
}

FJ_Document* FJ_DLCMainMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_DLCMainMenu(renderNode);
}

unsigned int FJ_DLCMainMenu::getType()
{
    return 59;
}

void FJ_DLCMainMenu::handlePress(FJ_Event* event)
{
    if (event == NULL || event->m_pTarget == NULL)
    {
        return;
    }
    FJ_Document::handlePress(event);
}
