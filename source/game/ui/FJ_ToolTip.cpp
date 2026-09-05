#include "game/ui/FJ_ToolTip.hpp"

#include <cstdio>
#include <string>

#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/util/Math.hpp"

class fuiRenderNodeEditText : public fuiRenderNode
{
public:
    void setText(const std::wstring& text);
    void setAutoSize(int autoSize);
};

extern std::string g_toolTipTypeLabels[];
extern std::string g_toolTipAnimationLabels[];

FJ_ToolTip::FJ_ToolTip(fuiRenderNode* renderNode)
    : FJ_Base(renderNode), m_controllerType(0), m_primaryText(0), m_secondaryText(0),
      m_textContainer(0), m_secondaryPanel(0), m_primaryType(-1), m_secondaryType(-1),
      m_textContainerX(0)
{
    m_primaryText = getChildByName(std::string("ToolTip_0"))->getChildAt(0)->asMovieClip();
    m_secondaryText = getChildByName(std::string("ToolTip_1"))->getChildAt(0)->asMovieClip();
    m_textContainer = getChildByName(std::string("FJ_TextContainer"))->asMovieClip();
    m_secondaryPanel = getChildByName(std::string("FJ_TextContainer_2"))->asMovieClip();
    m_textContainerX = static_cast<int>(m_textContainer->getX());
}

FJ_ToolTip::~FJ_ToolTip()
{
}

fuiRenderNodeEditText* FJ_ToolTip::GetSecondaryTextField()
{
    FJ_FuiNode* child = 0;
    if (!isFJ_Label())
    {
        FJ_FuiNode* container = getChildByName(std::string("FJ_TextContainer_2"));
        if (container == 0 || container->getNumChildren() == 0)
        {
            return 0;
        }
        child = container->getChildAt(0);
    }
    else
    {
        child = getChildAt(0)->asMovieClip();
    }
    if (child == 0 || child->m_pRenderNode == 0)
    {
        return 0;
    }
    return static_cast<fuiRenderNodeEditText*>(child->m_pRenderNode->asTimeline()->asEditText());
}

void FJ_ToolTip::SetSecondaryText(const std::wstring& text)
{
    FJ_FuiNode* container = m_secondaryPanel;
    fuiRenderNodeEditText* field = GetSecondaryTextField();
    field->setText(text);
    const float scale = 1.0f;
    if (container != 0)
    {
        container->setScaleX(scale);
        container->setScaleY(scale);
    }
    field->setScaleX(scale / getScaleX());
    field->setScaleY(scale / getScaleY());
    field->setAutoSize(1);
    if (container != 0)
    {
        container->setX(100.0f);
    }
    const float width = container == 0 ? 0.0f : container->getWidth();
    const int x = Math::round((width / getScaleX() - field->getWidth()) * 0.5f + m_objectWidth);
    field->setX(static_cast<float>(x));
    const int y = Math::round((m_objectHeight / getScaleY() - field->getHeight()) * 0.5f);
    field->setY(static_cast<float>(y));
}

void FJ_ToolTip::SetType(int type, int subtype)
{
    char label[64];
    if (m_controllerType < 1)
    {
        label[0] = 0;
    }
    else
    {
        std::sprintf(label, "%s%d", "", m_controllerType);
    }
    if (type < 0)
    {
        m_primaryText->setVisible(false);
    }
    else
    {
        if (m_primaryType != type)
        {
            m_primaryText->gotoAndStop(g_toolTipTypeLabels[type] + label);
        }
        m_primaryText->setVisible(true);
    }
    if (subtype < 0)
    {
        m_secondaryPanel->setVisible(false);
        SetSecondaryText(std::wstring(L""));
        m_primaryType = type;
        m_secondaryType = subtype;
        return;
    }
    const float offset = m_textContainerX;
    if (m_primaryText->getWidth() < 1.0f)
    {
        m_secondaryPanel->setX(m_primaryText->getX() + static_cast<float>(m_textContainerX) + offset);
    }
    else
    {
        m_secondaryPanel->setX(m_primaryText->getX() + m_primaryText->getWidth() + offset);
    }
    m_secondaryPanel->setVisible(true);
    SetSecondaryText(std::wstring(L"+"));
    if (m_secondaryType != subtype)
    {
        m_secondaryText->gotoAndStop(g_toolTipTypeLabels[subtype] + label);
    }
    m_primaryType = type;
    m_secondaryType = subtype;
}

uint64_t FJ_ToolTip::Init(const std::wstring& label, int id, bool visible, int type, int subtype)
{
    InitBase(label, id);
    m_controllerType = 0;
    SetType(type, subtype);
    SetNewObjectHeight(0);
    return 0;
}

void FJ_ToolTip::UpdateControllerType(int controllerType)
{
    m_controllerType = controllerType;
    SetType(m_primaryType, m_secondaryType);
}

uint8_t FJ_ToolTip::IsSingleButtonToolTip()
{
    return static_cast<uint8_t>(!m_secondaryText->isVisible());
}

uint64_t FJ_ToolTip::Animate()
{
    char label[64];
    if (m_controllerType < 1)
    {
        label[0] = 0;
    }
    else
    {
        std::sprintf(label, "%s%d", "", m_controllerType);
    }
    if (m_primaryText->isVisible())
    {
        m_primaryText->gotoAndPlay(g_toolTipAnimationLabels[m_primaryType] + label);
    }
    if (m_secondaryText->isVisible())
    {
        m_secondaryText->gotoAndPlay(g_toolTipAnimationLabels[m_secondaryType] + label);
    }
    return static_cast<uint64_t>((uintptr_t)this);
}

void FJ_ToolTip::SetLabel(const std::wstring& label)
{
    FJ_Base::SetLabel(label);
    float width = 100.0f;
    if (m_primaryText->isVisible() && !m_secondaryText->isVisible())
    {
        width = static_cast<float>(m_textContainerX);
        if (m_secondaryText->getWidth() > width)
        {
            width = m_secondaryText->getWidth();
        }
        fuiRect rect = m_primaryText->getRect(m_primaryText);
        m_textContainer->setX(rect.xMin + m_primaryText->getX() + static_cast<float>(static_cast<int>(width) + 1));
        return;
    }
    if (m_primaryText->isVisible() && m_secondaryText->isVisible())
    {
        width = static_cast<float>(m_textContainerX);
        if (m_secondaryText->getWidth() > width)
        {
            width = m_secondaryText->getWidth();
        }
        fuiRect rect = m_secondaryText->getRect(m_secondaryText);
        m_textContainer->setX(rect.xMin + m_secondaryText->getX() + static_cast<float>(static_cast<int>(width) + 1));
        return;
    }
    m_textContainer->setX(100.0f);
}
