#include "game/ui/UIControl_Base.hpp"
#include "game/ui/FJ_Base.hpp"
#include "game/ui/FJ_Cursor.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/core/StringConversion.hpp"

#include <string>
#include <vector>


UIControl_Base::UIControl_Base()
{
    m_labelChanged = false;
}

UIControl_Base::~UIControl_Base()
{
}

void UIControl_Base::setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name)
{
    UIControl::setupControl(scene, node, name);
}

void UIControl_Base::setLabel(UIString label, bool update, bool force)
{
    if (force || ((!m_label.empty() || !label.empty()) && m_label.compare(label)))
        m_labelChanged = true;

    m_label = label;
    if (!m_labelChanged || !update || m_fuiRenderNode == 0)
        return;

    m_labelChanged = false;

    std::wstring text;
    if (label.isCharString() == 0)
        text.assign(m_label.getString());
    else
        text = convStringToWstring(std::string(label.getCharString().c_str()));

    FJ_FuiNode* node = m_fuiRenderNode->getFuiNode();
    if (node->isFJ_Base())
        node->asFJ_Base()->SetLabel(text);
    else if (node->isFJ_Cursor())
        node->asFJ_Cursor()->SetLabel(text);
}

void UIControl_Base::tick()
{
    if (m_label.needsUpdating() != 0 || m_labelChanged)
    {
        setLabel(m_label, true, true);
        m_label.setUpdated();
    }
}

const wchar_t* UIControl_Base::getLabel()
{
    FJ_FuiNode* node = m_fuiRenderNode->getFuiNode();
    if (!node->isFJ_Base())
        return L"buckets_";
    return ((std::wstring*)(long)node->asFJ_Base()->GetLabel())->c_str();
}

void UIControl_Base::setAllPossibleLabels(int firstLabel, unsigned int labelCount, wchar_t (*labels)[256])
{
    std::vector<std::wstring> possibleLabels;
    unsigned int groupCount = labelCount / 4;
    unsigned int remainder = labelCount & 3;
    unsigned int i;

    for (i = 0; i < groupCount; ++i)
    {
        wchar_t* group = labels[i * 4 + 1];
        possibleLabels.push_back(std::wstring(group - 256));
        possibleLabels.push_back(std::wstring(group));
        possibleLabels.push_back(std::wstring(group + 256));
        possibleLabels.push_back(std::wstring(group + 512));
    }

    for (i = 0; i < remainder; ++i)
        possibleLabels.push_back(std::wstring(labels[groupCount * 4 + i]));

    FJ_FuiNode* node = m_fuiRenderNode->getFuiNode();
    node->asFJ_Base()->CheckLabelWidths(possibleLabels);
}

bool UIControl_Base::hasFocus()
{
    return m_scene->controlHasFocus(this);
}
