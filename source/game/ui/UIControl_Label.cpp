#include "game/ui/UIControl_Label.hpp"

#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_HtmlText.hpp"
#include "game/ui/FJ_Label.hpp"
#include "game/ui/FJ_LabelOutline.hpp"
#include "game/ui/FJ_Label_Blink.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/fuiRenderNode.hpp"

#include <string>

UIControl_Label::UIControl_Label()
    : UIControl_Base()
    , m_reinit(true)
{
}

UIControl_Label::~UIControl_Label()
{
}

void UIControl_Label::setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name)
{
    m_controlType = UIControl::eUIControlType_Label;
    UIControl_Base::setupControl(scene, node, name);
}

void UIControl_Label::init(UIString label)
{
    m_label = label;

    if (m_fuiRenderNode == 0)
        return;

    FJ_FuiNode* node = m_fuiRenderNode->getFuiNode();
    if (node->isFJ_Label())
    {
        node->asFJ_Label()->SetLabel(label.getString());
        return;
    }

    if (node->isFJ_Label_Blink())
    {
        node->asFJ_Label_Blink()->Init(label.getString());
        return;
    }

    if (node->isFJ_HtmlText())
    {
        node->asFJ_HtmlText()->SetLabel(label.getString());
        return;
    }

    if (node->isFJ_LabelOutline())
        node->asFJ_LabelOutline()->Init(label.getString());
}

void UIControl_Label::ReInit()
{
    UIControl::ReInit();
    if (m_reinit)
    {
        UIString label = m_label;
        init(label);
    }
}
