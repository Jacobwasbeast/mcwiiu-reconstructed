#include "game/ui/FJ_LabelOutline.hpp"

#include <cstdio>

#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fuiRenderNodeEditText.hpp"

FJ_LabelOutline::FJ_LabelOutline(fuiRenderNode* renderNode)
    : FJ_Base(renderNode)
{
    m_centreLabel = true;
    FJ_FuiNode* labelContainer = getChildAt(0)->asMovieClip();
    labelContainer->setX(0.0f);
    labelContainer->setY(0.0f);

    m_labelText = static_cast<fuiRenderNodeEditText*>(labelContainer->getRenderNodeChildByName(std::string("FJ_Label"))->asEditText());
    m_labelText->setAutoSize(TextFieldAutoSizeLeft);

    int i;
    for (i = 0; i < 4; ++i)
    {
        char name[68];
        std::sprintf(name, "FJ_Outline_%d", i * 2);
        fuiRenderNode* outline = labelContainer->getRenderNodeChildByName(std::string(name));
        if (outline != 0)
        {
            fuiRenderNodeEditText* text = static_cast<fuiRenderNodeEditText*>(outline->asEditText());
            m_outlineText.push_back(text);
            text->setAutoSize(TextFieldAutoSizeLeft);
            m_outlineX.push_back(text->getX());
        }

        std::sprintf(name, "FJ_Outline_%d", i * 2 + 1);
        outline = labelContainer->getRenderNodeChildByName(std::string(name));
        if (outline != 0)
        {
            fuiRenderNodeEditText* text = static_cast<fuiRenderNodeEditText*>(outline->asEditText());
            m_outlineText.push_back(text);
            text->setAutoSize(TextFieldAutoSizeLeft);
            m_outlineX.push_back(text->getX());
        }
    }

    m_halfWidth = static_cast<int>(getWidth() * 0.5f);
    m_labelX = static_cast<int>(m_labelText->getX());
    m_centreLabel = true;
}

FJ_LabelOutline::~FJ_LabelOutline()
{
}

void FJ_LabelOutline::Init(const std::wstring& label)
{
    SetLabel(label);
}

void FJ_LabelOutline::SetLabel(const std::wstring& label)
{
    m_label.assign(label, 0, static_cast<std::wstring::size_type>(-1));
    m_labelText->setX(static_cast<float>(m_labelX));
    m_labelText->setText(label);

    float offset = 0.0f;
    if (m_centreLabel)
    {
        offset = static_cast<float>(static_cast<int>(m_labelText->m_textWidth * 0.5f - static_cast<float>(m_halfWidth)));
        m_labelText->setX(m_labelText->getX() - offset);
    }

    std::size_t i;
    for (i = 0; i < m_outlineText.size(); ++i)
    {
        m_outlineText[i]->setText(label);
        m_outlineText[i]->setX(m_outlineX[i]);
        if (m_centreLabel)
            m_outlineText[i]->setX(m_outlineText[i]->getX() - offset);
    }
}
