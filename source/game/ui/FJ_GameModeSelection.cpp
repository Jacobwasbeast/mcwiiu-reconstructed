#include "game/ui/FJ_GameModeSelection.hpp"

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/fuiRenderNode.hpp"

FJ_GameModeSelection::FJ_GameModeSelection(fuiRenderNode* renderNode)
    : FJ_Document(renderNode)
{
}

FJ_GameModeSelection::~FJ_GameModeSelection()
{
}

FJ_Document* FJ_GameModeSelection::Create(fuiRenderNode* renderNode)
{
    return new FJ_GameModeSelection(renderNode);
}

unsigned int FJ_GameModeSelection::getType()
{
    return 0x3d;
}

void FJ_GameModeSelection::handlePress(FJ_Event* event)
{
    FJ_Document::handlePress(event);
    if (event != 0 && event->m_pTarget != 0)
    {
        event->m_pTarget->setVisible(true);
    }
}
