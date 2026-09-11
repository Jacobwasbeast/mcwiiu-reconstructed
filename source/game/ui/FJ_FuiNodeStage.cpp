#include "game/ui/FJ_FuiNodeStage.hpp"

#include "game/ui/FJ_FocusEvent.hpp"
#include "game/ui/fuiRenderNodeStage.hpp"

FJ_FuiNodeStage::FJ_FuiNodeStage(fuiRenderNode* pRenderNode)
    : FJ_FuiNode(pRenderNode)
    , m_pFocus(NULL)
{
}

FJ_FuiNodeStage::~FJ_FuiNodeStage()
{
}

void FJ_FuiNodeStage::removeFocus(FJ_FuiNode* pNode)
{
    if (m_pFocus != pNode)
        return;
    m_pFocus = NULL;
}

void FJ_FuiNodeStage::setFocus(FJ_FuiNode* pNode)
{
    if (m_pFocus != pNode && m_pFocus != NULL)
    {
        FJ_FocusEvent event(false, false, false);
        m_pFocus->dispatchEvent(&event);
    }

    m_pFocus = pNode;
    if (pNode != NULL)
    {
        FJ_FocusEvent event(false, true, false);
        pNode->dispatchEvent(&event);
    }
}

FJ_FuiNode* FJ_FuiNodeStage::getFocus()
{
    if (m_pFocus == NULL)
        return this;
    return m_pFocus;
}

double FJ_FuiNodeStage::getStageHeight()
{
    return static_cast<fuiRenderNodeStage*>(m_pRenderNode)->getStageHeight();
}

double FJ_FuiNodeStage::getStageWidth()
{
    return static_cast<fuiRenderNodeStage*>(m_pRenderNode)->getStageWidth();
}
