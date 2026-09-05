#include "game/ui/UIControl_Label.hpp"

#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/fuiRenderNode.hpp"

#include <string>

class FJ_Label
{
public:
    virtual void SetLabel(const std::wstring& label);
};

class FJ_HtmlText
{
public:
    virtual void SetLabel(const std::wstring& label);
};

class FJ_LabelOutline
{
public:
    void Init(const std::wstring& label);
};

class FJ_Label_Blink
{
public:
    void Init(const std::wstring& label);
};

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
    m_controlType = 8;
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
        node->asFJ_Label()->SetLabel(*(const std::wstring*)(long)label.getString());
        return;
    }

    if (node->isFJ_Label_Blink())
    {
        node->asFJ_Label_Blink()->Init(*(const std::wstring*)(long)label.getString());
        return;
    }

    if (node->isFJ_HtmlText())
    {
        node->asFJ_HtmlText()->SetLabel(*(const std::wstring*)(long)label.getString());
        return;
    }

    if (node->isFJ_LabelOutline())
        node->asFJ_LabelOutline()->Init(*(const std::wstring*)(long)label.getString());
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
