#include "game/ui/UIControl_HTMLLabel.hpp"

#include "game/ui/FJ_DynamicHtmlText.hpp"
#include "game/ui/FJ_HtmlText.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"

extern ConsoleUIController gConsoleUIController;

UIControl_HTMLLabel::UIControl_HTMLLabel()
    : UIControl_Label()
{
}

int UIControl_HTMLLabel::GetRealWidth()
{
    FJ_FuiNode* node = m_fuiRenderNode->getFuiNode();
    if (node->isFJ_HtmlText())
        return (int)node->asFJ_HtmlText()->GetRealWidth();

    if (node->isFJ_DynamicHtmlText())
        return (int)node->asFJ_DynamicHtmlText()->GetRealWidth();

    return 0;
}

int UIControl_HTMLLabel::GetRealHeight()
{
    FJ_FuiNode* node = m_fuiRenderNode->getFuiNode();
    if (node->isFJ_HtmlText())
        return (int)node->asFJ_HtmlText()->GetRealHeight();

    if (node->isFJ_DynamicHtmlText())
        return (int)node->asFJ_DynamicHtmlText()->GetRealHeight();

    return 0;
}

void UIControl_HTMLLabel::setupControl(UIScene* scene, fuiRenderNode* node,
                                       const std::string& name)
{
    m_controlType = UIControl::eUIControlType_HTMLLabel;
    UIControl_Base::setupControl(scene, node, name);
}

void UIControl_HTMLLabel::ReInit()
{
    UIControl::ReInit();
    init(UIString(L"px != 0"));
}

void UIControl_HTMLLabel::SetupTouch()
{
    int sceneType = m_scene->getSceneType();
    if (sceneType == 4 || sceneType == 6 || sceneType == 7)
        gConsoleUIController.TouchBoxAdd(this, m_scene);
}

void UIControl_HTMLLabel::TouchScroll(int offset, bool active)
{
    FJ_FuiNode* node = m_fuiRenderNode->getFuiNode();
    if (node->isFJ_HtmlText())
    {
        node->asFJ_HtmlText()->TouchScroll(offset, active);
        return;
    }

    if (node->isFJ_DynamicHtmlText())
        node->asFJ_DynamicHtmlText()->TouchScroll(offset, active);
}
