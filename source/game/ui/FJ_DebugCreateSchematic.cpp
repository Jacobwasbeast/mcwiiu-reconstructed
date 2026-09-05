#include "game/ui/FJ_DebugCreateSchematic.hpp"

#include <string>

#include "game/ui/FJ_Base.hpp"
#include "game/ui/FJ_CheckBox.hpp"
#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"

FJ_DebugCreateSchematic::FJ_DebugCreateSchematic(fuiRenderNode* renderNode)
    : FJ_Document(renderNode)
{
    addEventListener(eFJEventType_CheckboxToggled, this, _handleCheckboxToggled, false);

    FJ_FuiNode* name = getChildByName(std::string("Name"));
    name->setVisible(true);
}

FJ_DebugCreateSchematic::~FJ_DebugCreateSchematic()
{
}

uint64_t FJ_DebugCreateSchematic::handleCheckboxToggled(FJ_Event* event)
{
    FJ_CheckBox* checkBox = event->m_pTarget->asFJ_CheckBox();
    if (checkBox != NULL)
    {
        m_pCallbackScene->handleCheckboxToggled(checkBox->getId(), checkBox->getChecked());
    }
    return 0;
}

void FJ_DebugCreateSchematic::_handleCheckboxToggled(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_DebugCreateSchematic*>(node)->handleCheckboxToggled(event);
}

FJ_Document* FJ_DebugCreateSchematic::Create(fuiRenderNode* renderNode)
{
    return new FJ_DebugCreateSchematic(renderNode);
}

unsigned int FJ_DebugCreateSchematic::getType()
{
    return 52;
}
