#include "game/ui/FJ_HowToPlayMenu.hpp"

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/fuiRenderNode.hpp"

FJ_HowToPlayMenu::FJ_HowToPlayMenu(fuiRenderNode* renderNode)
    : FJ_Document(renderNode)
{
}

FJ_HowToPlayMenu::~FJ_HowToPlayMenu()
{
}

void FJ_HowToPlayMenu::handlePress(FJ_Event* event)
{
    FJ_Document::handlePress(event);
    if (event != 0 && event->m_pTarget != 0)
    {
        event->m_pTarget->setVisible(true);
    }
}

FJ_Document* FJ_HowToPlayMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_HowToPlayMenu(renderNode);
}

unsigned int FJ_HowToPlayMenu::getType()
{
    return 9;
}
