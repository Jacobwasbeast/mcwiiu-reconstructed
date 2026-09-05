#include "game/ui/FJ_DebugFillArea.hpp"

#include <string>

#include "game/ui/FJ_Base.hpp"
#include "game/ui/FJ_CheckBox.hpp"
#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"

FJ_DebugFillArea::FJ_DebugFillArea(fuiRenderNode* renderNode)
    : FJ_Document(renderNode)
{
    addEventListener(eFJEventType_CheckboxToggled, this, _handleCheckboxToggled, false);

    FJ_FuiNode* node = getChildByName(std::string("EdgeTile"));
    node->setVisible(true);
    node = getChildByName(std::string("EdgeData"));
    node->setVisible(true);
    node = getChildByName(std::string("FillTile"));
    node->setVisible(true);
    node = getChildByName(std::string("FillData"));
    node->setVisible(true);
}

FJ_DebugFillArea::~FJ_DebugFillArea()
{
}

void FJ_DebugFillArea::handleCheckboxToggled(FJ_Event* event)
{
    FJ_CheckBox* checkBox = event->m_pTarget->asFJ_CheckBox();
    if (checkBox != NULL)
        m_pCallbackScene->handleCheckboxToggled(checkBox->getId(), checkBox->getChecked());
}

void FJ_DebugFillArea::_handleCheckboxToggled(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_DebugFillArea*>(node)->handleCheckboxToggled(event);
}

FJ_Document* FJ_DebugFillArea::Create(fuiRenderNode* renderNode)
{
    return new FJ_DebugFillArea(renderNode);
}

unsigned int FJ_DebugFillArea::getType()
{
    return 53;
}
