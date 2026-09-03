#include "game/ui/UIControl_Touch.hpp"
#include "game/ui/UIControl.hpp"
#include "game/ui/UIControl_Base.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"

void ConsoleUIController_TouchBoxAdd(UIControl* control, UIScene* scene);

UIControl_Touch::UIControl_Touch()
    : UIControl_Base()
{
}

UIControl_Touch::~UIControl_Touch()
{
}

bool UIControl_Touch::setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name)
{
    m_unknown0 = 20;
    return UIControl_Base::setupControl(scene, node, name);
}

void UIControl_Touch::init(int controlType)
{
    m_unknown0 = (unsigned int)controlType;
    if (m_scene == 0) {
        return;
    }
    int sceneType = m_scene->getSceneType();
    if (sceneType == 2 || sceneType == 4 || sceneType == 6 || sceneType == 7) {
        ConsoleUIController_TouchBoxAdd(this, m_scene);
    }
}

void UIControl_Touch::ReInit()
{
    UIControl::ReInit();
    init((int)m_unknown0);
}
