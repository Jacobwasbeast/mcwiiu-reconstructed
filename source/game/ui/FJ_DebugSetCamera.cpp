#include "game/ui/FJ_DebugSetCamera.hpp"

#include <string>

#include "game/ui/FJ_Base.hpp"
#include "game/ui/FJ_CheckBox.hpp"
#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"

FJ_DebugSetCamera::FJ_DebugSetCamera(fuiRenderNode* renderNode)
    : FJ_Document(renderNode)
{
    addEventListener(eFJEventType_CheckboxToggled, this, _handleCheckboxToggled, false);

    FJ_FuiNode* node = getChildByName(std::string("CamX"));
    node->setVisible(true);
    node = getChildByName(std::string("CamY"));
    node->setVisible(true);
    node = getChildByName(std::string("CamZ"));
    node->setVisible(true);
    node = getChildByName(std::string("YRot"));
    node->setVisible(true);
    node = getChildByName(std::string("Elevation"));
    node->setVisible(true);
}

void FJ_DebugSetCamera::handleCheckboxToggled(FJ_Event* event)
{
    FJ_CheckBox* checkBox = event->m_pTarget->asFJ_CheckBox();
    if (checkBox != NULL)
        m_pCallbackScene->handleCheckboxToggled(checkBox->getId(), checkBox->getChecked());
}

void FJ_DebugSetCamera::_handleCheckboxToggled(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_DebugSetCamera*>(node)->handleCheckboxToggled(event);
}

FJ_Document* FJ_DebugSetCamera::Create(fuiRenderNode* renderNode)
{
    return new FJ_DebugSetCamera(renderNode);
}

unsigned int FJ_DebugSetCamera::getType()
{
    return 58;
}
