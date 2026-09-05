#include "game/ui/FJ_ToolTips.hpp"

#include <string>

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_ToolTip.hpp"

FJ_ToolTips::FJ_ToolTips(fuiRenderNode* renderNode)
    : FJ_Document(renderNode), m_useTouchControls(0),
      m_touchControlsAvailable(false)
{
}

FJ_ToolTips::~FJ_ToolTips()
{
}

FJ_Document* FJ_ToolTips::Create(fuiRenderNode* renderNode)
{
    return new FJ_ToolTips(renderNode);
}

unsigned int FJ_ToolTips::getType()
{
    return 6;
}

void FJ_ToolTips::_keyDownHandler(FJ_FuiNode* node, FJ_Event* event)
{
    node->keyDownHandler(event);
}

void FJ_ToolTips::init(FJ_Event*)
{
    static const char* const names[16] = {
        "Touch_0", "Touch_1", "Touch_2", "Touch_3",
        "Touch_4", "Touch_5", "Touch_6", "Touch_7",
        "Touch_8", "Touch_9", "Touch_10", "Touch_11",
        "Touch_12", "Touch_13", "Touch_14", "Touch_15"
    };
    m_toolTipSuffix.assign(L"  ");
    m_toolTipNodes.clear();
    for (int i = 0; i < 16; i += 2)
    {
        FJ_FuiNode* first = getChildByName(std::string(names[i]));
        m_toolTipNodes.push_back(first);
        if (first != 0)
        {
            static_cast<FJ_ToolTip*>(first)->Init(std::wstring(), i, false, -1, -1);
        }
        FJ_FuiNode* second = getChildByName(std::string(names[i + 1]));
        m_toolTipNodes.push_back(second);
        if (second != 0)
        {
            static_cast<FJ_ToolTip*>(second)->Init(std::wstring(), i + 1, false, -1, -1);
        }
    }
    getStage()->addEventListener(eFJEventType_KeyDown, this, _keyDownHandler, false);
    m_touchControlsAvailable = false;
    m_touchNodes.clear();
    for (int i = 0; i < 16; ++i)
    {
        FJ_FuiNode* node = getChildByName(std::string(names[i]));
        if (node != 0)
        {
            m_touchNodes.push_back(node);
            m_touchControlsAvailable = true;
        }
    }
    m_useTouchControls = 1;
}

void FJ_ToolTips::UpdateControllerType(int controllerType)
{
    for (std::vector<FJ_FuiNode*>::iterator it = m_toolTipNodes.begin(); it != m_toolTipNodes.end(); ++it)
    {
        static_cast<FJ_ToolTip*>(*it)->UpdateControllerType(controllerType);
    }
}

void FJ_ToolTips::keyDownHandler(FJ_Event* event)
{
    int index = static_cast<int>(event->m_eType);
    if (index >= 0 && index < static_cast<int>(m_toolTipNodes.size()))
    {
        FJ_ToolTip* toolTip = static_cast<FJ_ToolTip*>(m_toolTipNodes[index]);
        if (toolTip->IsSingleButtonToolTip())
        {
            toolTip->Animate();
        }
    }
}

void FJ_ToolTips::SetToolTip(int index, int type, int subtype, const std::wstring& text, bool visible)
{
    if (index < 16)
    {
        FJ_ToolTip* toolTip = static_cast<FJ_ToolTip*>(m_toolTipNodes[index]);
        toolTip->setVisible(visible);
        toolTip->SetType(type, subtype);
        toolTip->SetLabel(text + m_toolTipSuffix);
    }
}

void FJ_ToolTips::UpdateLayout()
{
    int bottom = getISafeZoneBottom();
    int left = getISafeZoneLeft();
    if (bottom == 0)
    {
        bottom = 2;
    }
    if (left == 0)
    {
        left = 2;
    }
    for (int i = 0; i < 16; ++i)
    {
        FJ_FuiNode* node = m_toolTipNodes[i];
        if (node == 0 || !node->isVisible())
        {
            continue;
        }
        node->setX(static_cast<float>(left));
        node->setY(getStage()->getStageHeight() - node->getHeight() - static_cast<float>(bottom));
        left += static_cast<int>(node->getWidth());
    }
}
