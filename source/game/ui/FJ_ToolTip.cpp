#include "game/ui/FJ_ToolTip.hpp"

#include <cstdio>
#include <string>

#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fuiRenderNodeEditText.hpp"
#include "game/ui/fuiRenderNodeTimeline.hpp"
#include "game/util/Math.hpp"

extern std::string g_toolTipLabels[][2];
std::string g_toolTipLabels[16][2] = {
    {"Button_A", "Button_A_Animated"},
    {"Button_B", "Button_B_Animated"},
    {"Button_X", "Button_X_Animated"},
    {"Button_Y", "Button_Y_Animated"},
    {"Button_LeftTrigger", "Button_LeftTrigger_Animated"},
    {"Button_RightTrigger", "Button_RightTrigger_Animated"},
    {"Button_LeftBumper", "Button_LeftBumper_Animated"},
    {"Button_RightBumper", "Button_RightBumper_Animated"},
    {"Button_LStick", "Button_LStick_Animated"},
    {"Button_RStick", "Button_RStick_Animated"},
    {"Button_SystemSelect", "Button_SystemSelect_Animated"},
    {"Button_DpadLeft", "Button_Select_Animated"},
    {"Button_DpadRight", "Button_Select_Animated"},
    {"Button_DpadUp", "Button_Select_Animated"},
    {"Button_DpadDown", "Button_Select_Animated"},
    {"Button_LeftStick", "Button_LeftStick_Animated"}
};
std::string g_toolTipControllerSuffix("_CTRL_");

FJ_ToolTip::FJ_ToolTip(fuiRenderNode* renderNode)
    : FJ_Base(renderNode), m_controllerType(0), m_primaryType(-1), m_secondaryType(-1)
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
    fuiRenderNodeEditText* field = NULL;
    FJ_FuiNode* child;

    if (!isFJ_Label())
    {
        FJ_FuiNode* container = getChildByName(std::string("FJ_TextContainer_2"));
        if (container == NULL)
            return NULL;
        if (container->getNumChildren() == 0)
            return NULL;
        child = container->getChildAt(0);
    }
    else
    {
        child = getChildAt(0)->asMovieClip();
    }

    if (child != NULL)
    {
        fuiRenderNodeTimeline* timeline =
            static_cast<fuiRenderNodeTimeline*>(child->m_pRenderNode->asTimeline());
        if (timeline != NULL)
            field = timeline->getChildAt(0)->asEditText();
    }
    return field;
}

void FJ_ToolTip::SetSecondaryText(const std::wstring& text)
{
    FJ_FuiNode* container = getChildByName(std::string("FJ_TextContainer_2"))->asMovieClip();
    fuiRenderNodeEditText* field = GetSecondaryTextField();
    field->setText(text);

    if (container != NULL)
    {
        container->setScaleX(1.0f);
        container->setScaleY(1.0f);
    }
    field->setScaleX(1.0f / getScaleX());
    field->setScaleY(1.0f / getScaleY());
    field->setAutoSize(TextFieldAutoSizeLeft);
    if (container != NULL)
        container->setY(0.0f);

    int x = Math::round((container->getWidth() / getScaleX() - field->getWidth()) * 0.5f
                        + m_centerBiasX);
    field->setX(static_cast<float>(x));
    int y = Math::round((m_objectHeight / getScaleY() - field->getHeight()) * 0.5f);
    field->setY(static_cast<float>(y));
}

void FJ_ToolTip::SetType(int type, int subtype)
{
    char label[64];
    if (m_controllerType < 1)
        label[0] = '\0';
    else
        std::sprintf(label, "%s%d", g_toolTipControllerSuffix.c_str(), m_controllerType);

    if (type < 0)
    {
        m_primaryText->setVisible(false);
    }
    else
    {
        if (m_primaryType != type)
            m_primaryText->gotoAndStop(g_toolTipLabels[type][0] + label);
        m_primaryText->setVisible(true);
    }

    if (subtype < 0)
    {
        m_secondaryPanel->setVisible(false);
        SetSecondaryText(std::wstring(L""));
        m_secondaryText->setVisible(false);
    }
    else
    {
        if (static_cast<int>(m_primaryText->getWidth()) < 1)
        {
            m_secondaryPanel->setX(m_primaryText->getX()
                                   + static_cast<float>(m_textContainerX) + 1.0f);
        }
        else
        {
            m_secondaryPanel->setX(m_primaryText->getX()
                                   + static_cast<float>(static_cast<int>(m_primaryText->getWidth()))
                                   + 1.0f);
        }
        m_secondaryPanel->setVisible(true);

        SetSecondaryText(std::wstring(L"+"));

        if (m_secondaryType != subtype)
            m_secondaryText->gotoAndStop(g_toolTipLabels[subtype][0] + label);

        m_secondaryText->setX(m_secondaryPanel->getX() + m_secondaryPanel->getWidth() + 1.0f);
        m_secondaryText->setVisible(true);
    }

    m_primaryType = type;
    m_secondaryType = subtype;
}

void FJ_ToolTip::Init(const std::wstring& label, int id, bool visible, int type, int subtype)
{
    InitBase(label, id);
    m_controllerType = 0;
    SetType(type, subtype);
    setVisible(false);
}

void FJ_ToolTip::UpdateControllerType(int controllerType)
{
#if defined(RECON_NATIVE)
    const int primaryType = m_primaryType;
    const int secondaryType = m_secondaryType;
    if (m_controllerType != controllerType)
    {
        m_primaryType = -1;
        m_secondaryType = -1;
    }
    m_controllerType = controllerType;
    SetType(primaryType, secondaryType);
#else
    m_controllerType = controllerType;
    SetType(m_primaryType, m_secondaryType);
#endif
}

uint8_t FJ_ToolTip::IsSingleButtonToolTip()
{
    return static_cast<uint8_t>(m_secondaryText->isVisible() ^ 1);
}

void FJ_ToolTip::Animate()
{
    char label[64];
    if (m_controllerType < 1)
        label[0] = '\0';
    else
        std::sprintf(label, "%s%d", g_toolTipControllerSuffix.c_str(), m_controllerType);

    if (m_primaryText->isVisible())
        m_primaryText->gotoAndPlay(g_toolTipLabels[m_primaryType][1] + label);

    if (m_secondaryText->isVisible())
        m_secondaryText->gotoAndPlay(g_toolTipLabels[m_secondaryType][1] + label);
}

void FJ_ToolTip::SetLabel(const std::wstring& label)
{
    FJ_Base::SetLabel(label);

    if (m_primaryText->isVisible() && !m_secondaryText->isVisible())
    {
        int width = m_textContainerX;
        if (m_secondaryText->getWidth() > 0.0f)
            width = static_cast<int>(m_secondaryText->getWidth());

        fuiRect rect = m_primaryText->getRect(m_primaryText);
        m_textContainer->setX(rect.xMin + m_primaryText->getX()
                              + static_cast<float>(width + 1));
        return;
    }

    if (m_primaryText->isVisible() && m_secondaryText->isVisible())
    {
        int width = m_textContainerX;
        if (m_secondaryText->getWidth() > 0.0f)
            width = static_cast<int>(m_secondaryText->getWidth());

        fuiRect rect = m_secondaryText->getRect(m_secondaryText);
        m_textContainer->setX(rect.xMin + m_secondaryText->getX()
                              + static_cast<float>(width + 1));
        return;
    }

    m_textContainer->setX(0.0f);
}
