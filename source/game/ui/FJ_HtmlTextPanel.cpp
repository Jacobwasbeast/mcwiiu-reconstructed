#include "game/ui/FJ_HtmlTextPanel.hpp"

#include <string>
#include <utility>

#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fuiRenderNodeEditText.hpp"
#include "game/util/Math.hpp"

FJ_HtmlTextPanel::FJ_HtmlTextPanel(fuiRenderNode* pRenderNode)
    : FJ_FuiNode(pRenderNode)
{
    FJ_FuiNode* textContainer = getChildByName(std::string("FJ_TextContainer"))->asMovieClip();
    FJ_FuiNode* textFieldContainer = textContainer->getChildAt(0)->asMovieClip();
    m_textField = static_cast<fuiRenderNodeEditText*>(textFieldContainer->getRenderNodeChildByName(std::string("FJ_TextField")));
    addChild(m_textField);
    m_textField->setY(textFieldContainer->getY());
    removeChild(textContainer, true);

    m_pointerTextPanel = getChildByName(std::string("PointerTextPanel"))->asMovieClip();
    m_fixedWidthEnabled = false;
    m_fixedWidth = 0;
    SetLabel(std::wstring(L""));
}

FJ_HtmlTextPanel::~FJ_HtmlTextPanel()
{
}

void FJ_HtmlTextPanel::SetLabel(const std::wstring& label)
{
    m_textField->setHTMLText(label);
    if (!m_fixedWidthEnabled)
    {
        m_textField->setAutoSize(TextFieldAutoSizeLeft);
    }
    else
    {
        m_textField->setAutoSize(TextFieldAutoSizeCenter);
        m_textField->setWidth((float)m_fixedWidth);
        m_textField->setHeight(m_textField->getHeight() + 5.0f);
    }

    m_pointerTextPanel->setVisible(!label.empty());
    if (m_pointerTextPanel->isVisible())
    {
        std::pair<float, float> boundaries = m_textField->getCharBoundaries(0);
        float textWidth = boundaries.second - boundaries.first;
        m_textField->setX(m_pointerTextPanel->getX() + 4.0f);

        FJ_FuiNode* panelContents = m_pointerTextPanel->getChildAt(0);
        panelContents->setWidth((float)Math::round(textWidth * 0.8f + m_textField->getWidth()));
        panelContents->setHeight(m_textField->getHeight() + textWidth);
        m_textField->setX((float)Math::round(textWidth * 0.35f + m_pointerTextPanel->getX()));
        m_textField->setY((float)Math::round(m_pointerTextPanel->getY() + textWidth * 0.5f));
    }
}

FJ_HtmlTextPanel* FJ_HtmlTextPanel::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_HtmlTextPanel(pRenderNode);
}

unsigned int FJ_HtmlTextPanel::getType()
{
    return 0x6f;
}

void FJ_HtmlTextPanel::SetFixedWidth(int width)
{
    m_fixedWidth = width;
    m_fixedWidthEnabled = true;
}
