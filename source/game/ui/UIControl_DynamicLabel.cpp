#include "game/ui/UIControl_DynamicLabel.hpp"

#include "game/ui/FJ_DynamicHtmlText.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"

extern ConsoleUIController gConsoleUIController;

UIControl_DynamicLabel::UIControl_DynamicLabel()
    : UIControl_Label()
{
}

int UIControl_DynamicLabel::GetRealWidth()
{
    return (int)static_cast<FJ_DynamicHtmlText*>(m_fuiRenderNode->getFuiNode())->GetRealWidth();
}

int UIControl_DynamicLabel::GetRealHeight()
{
    return (int)static_cast<FJ_DynamicHtmlText*>(m_fuiRenderNode->getFuiNode())->GetRealHeight();
}

void UIControl_DynamicLabel::setupControl(UIScene* scene, fuiRenderNode* node,
                                          const std::string& name)
{
    m_controlType = UIControl::eUIControlType_DynamicLabel;
    UIControl_Base::setupControl(scene, node, name);
}

void UIControl_DynamicLabel::addText(const std::wstring& text, bool setup)
{
    static_cast<FJ_DynamicHtmlText*>(m_fuiRenderNode->getFuiNode())->AddText(text, setup);
}

void UIControl_DynamicLabel::ReInit()
{
    UIControl::ReInit();
}

void UIControl_DynamicLabel::SetupTouch()
{
    unsigned int layerId = m_scene->m_layer->m_layerId;
    if (layerId != 4)
    {
        if (layerId < 6)
            return;
        if (layerId > 7)
            return;
    }
    gConsoleUIController.TouchBoxAdd(this, m_scene);
}

void UIControl_DynamicLabel::TouchScroll(int offset, bool active)
{
    static_cast<FJ_DynamicHtmlText*>(m_fuiRenderNode->getFuiNode())->TouchScroll(offset, active);
}
