#include "game/ui/FJ_HtmlText_WithPanel.hpp"

#include <string>

#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fuiRenderNodeEditText.hpp"
#include "game/ui/fuiRenderNodeTimeline.hpp"

FJ_HtmlText_WithPanel::FJ_HtmlText_WithPanel(fuiRenderNode* renderNode)
    : FJ_HtmlText(renderNode)
{
    m_panelNode = NULL;
}

FJ_HtmlText_WithPanel::~FJ_HtmlText_WithPanel()
{
    if (m_panelNode != NULL)
    {
        FJ_FuiNode* parent = getParent();
        if (parent != NULL)
            parent = m_panelNode->getParent();
        if (parent != NULL)
            parent->removeChild(m_panelNode, true);
    }
    m_panelOffset = 20;
}

unsigned int FJ_HtmlText_WithPanel::getType()
{
    return 0x80a6;
}

void FJ_HtmlText_WithPanel::setVisible(bool visible)
{
    FJ_HtmlText::setVisible(visible);
    if (m_panelNode != NULL)
        m_panelNode->setVisible(visible);
}

void FJ_HtmlText_WithPanel::setY(float y)
{
    float oldY = getY();
    FJ_HtmlText::setY(y);
    if (m_panelNode != NULL)
        m_panelNode->setY(m_panelNode->getY() + (y - oldY));
}

void FJ_HtmlText_WithPanel::Init(const std::wstring& label)
{
    float width = getWidth();
    float height = getHeight();
    FJ_FuiNode* panel = getChildByName(std::string("HtmlTextPanel"));
    FJ_FuiNode* panelNode = panel->getChildAt(0);
    m_panelNode = panelNode;
    panel->removeChild(panelNode, false);
    removeChild(panel, true);

    getParent()->addChild(m_panelNode);
    m_panelNode->setX(getX());
    m_panelNode->setY(getY());
    m_panelNode->setWidth(width);
    m_panelNode->setHeight(height);

    fuiRenderNodeTimeline* renderParent = static_cast<fuiRenderNodeTimeline*>(m_pText->asTimeline());
    renderParent->removeChild(m_pText, false);
    fuiRenderNodeTimeline* parentTimeline = static_cast<fuiRenderNodeTimeline*>(getParent()->m_pRenderNode->asTimeline());
    parentTimeline->addChild(m_pText);

    m_pText->setX(getX() + static_cast<float>(m_panelOffset - 176));
    m_pText->setY(getY() + static_cast<float>(m_panelOffset - 176));
    m_pText->setWidth(width - static_cast<float>(m_panelOffset * 2 - 176));
    float arrowHeight = m_pUpArrow->getHeight();
    m_pText->setHeight(height - arrowHeight - static_cast<float>(m_panelOffset - 176));
    static_cast<fuiRenderNodeEditText*>(m_pText->asEditText())->setAutoSize(static_cast<TextFieldAutoSize>(1));
    m_pText->setVisibility(true);

    m_iWidth = 0;
    m_fWidth = width - static_cast<float>(m_panelOffset * 2 - 176);
    m_fScrollTop = 0.0f;
    m_fScrollBottom = height - arrowHeight - static_cast<float>(m_panelOffset - 176);
    FJ_HtmlText::SetLabel(label);
    HandleScrollListArrows(true);
}
