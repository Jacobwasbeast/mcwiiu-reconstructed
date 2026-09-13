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
    node->asFJ_Document()->keyDownHandler(event);
}

void FJ_ToolTips::init(FJ_Event*)
{
    static const char* const names[16] = {
        "ToolTip_0", "ToolTip_1", "ToolTip_2", "ToolTip_3",
        "ToolTip_4", "ToolTip_5", "ToolTip_6", "ToolTip_7",
        "ToolTip_8", "ToolTip_9", "ToolTip_10", "ToolTip_11",
        "ToolTip_12", "ToolTip_13", "ToolTip_14", "ToolTip_15"
    };
    static const char* const touchNames[16] = {
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
        static_cast<FJ_ToolTip*>(first)->Init(std::wstring(), i, false, -1, -1);
        FJ_FuiNode* second = getChildByName(std::string(names[i + 1]));
        m_toolTipNodes.push_back(second);
        static_cast<FJ_ToolTip*>(second)->Init(std::wstring(), i + 1, false, -1, -1);
    }
    getStage()->addEventListener(eFJEventType_KeyDown, this, _keyDownHandler, false);
    m_touchControlsAvailable = false;
    m_touchNodes.clear();
    FJ_FuiNode* firstTouchNode = getChildByName(std::string(touchNames[0]));
    if (firstTouchNode != 0)
    {
        for (int i = 0; i < 16; ++i)
        {
            m_touchNodes.push_back(getChildByName(std::string(touchNames[i])));
        }
        m_touchControlsAvailable = true;
    }
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
    int index;
    switch (event->m_eType)
    {
    case 0: index = 0; break;
    case 1: index = 4; break;
    case 2: index = 5; break;
    case 3:
    case 4:
    case 5:
    case 6: index = 15; break;
    case 7: index = 1; break;
    case 8: index = 2; break;
    case 9: index = 3; break;
    case 10: index = 7; break;
    case 11: index = 6; break;
    case 12: index = 9; break;
    case 13: index = 10; break;
    default: return;
    }

    FJ_ToolTip* toolTip = static_cast<FJ_ToolTip*>(m_toolTipNodes[index]);
    if (toolTip->IsSingleButtonToolTip())
    {
        toolTip->Animate();
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
    m_toolTipIndex = index;
}

void FJ_ToolTips::UpdateLayout()
{
    int bottom = getISafeZoneBottom();
    if (bottom == 0)
    {
        bottom = 2;
    }
    int left = getISafeZoneLeft();
    if (left == 0)
    {
        left = 2;
    }

    FJ_FuiNode* firstNode = m_toolTipNodes[0];
    fuiRect firstRect = firstNode->getRect(firstNode);
    const float stageHeight = static_cast<float>(getStage()->getStageHeight());
    const float y = (stageHeight - firstNode->getHeight()) - static_cast<float>(bottom) - firstRect.yMin;

    for (int i = 0; i < 16; ++i)
    {
        FJ_FuiNode* node = m_toolTipNodes[i];
        if (!node->isVisible())
        {
            if (m_touchControlsAvailable)
            {
                FJ_FuiNode* touchNode = m_touchNodes[i];
                touchNode->setX(-1.0f);
                touchNode->setY(-1.0f);
                touchNode->setWidth(1.0f);
                touchNode->setHeight(1.0f);
            }
            continue;
        }
        node->setX(static_cast<float>(left));
        node->setY(y);
        if (m_touchControlsAvailable)
        {
            FJ_FuiNode* touchNode = m_touchNodes[i];
            touchNode->setVisible(true);
            touchNode->setX(static_cast<float>(left));
            touchNode->setY(y - node->getHeight());
            touchNode->setWidth(node->getWidth());
            fuiRect touchRect = node->getRect(touchNode);
            touchNode->setHeight(node->getHeight() * 2.0f + touchRect.yMax);
        }
        left += static_cast<int>(node->getWidth());
    }
}
