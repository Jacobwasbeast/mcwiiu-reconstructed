#include "game/ui/UIControl_Touch.hpp"
#include "game/ui/UIControl.hpp"
#include "game/ui/UIControl_Base.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/core/ConsoleUIController.hpp"

extern ConsoleUIController gConsoleUIController;

UIControl_Touch::UIControl_Touch()
    : UIControl_Base()
{
}

bool UIControl_Touch::setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name)
{
    m_controlType = UIControl::eUIControlType_Touch;
    return UIControl_Base::setupControl(scene, node, name);
}

void UIControl_Touch::init(int controlId)
{
    m_controlId = controlId;
    int layerId = m_scene->m_layer->m_layerId;
    if (layerId == 2 || layerId == 4 || (layerId >= 6 && layerId <= 7))
        gConsoleUIController.TouchBoxAdd(this, m_scene);
}

void UIControl_Touch::ReInit()
{
    UIControl::ReInit();
    init(m_controlId);
}
