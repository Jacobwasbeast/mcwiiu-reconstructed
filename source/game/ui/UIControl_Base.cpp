#include "game/ui/UIControl_Base.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"

#include <string>
#include <vector>

UIControl_Base::UIControl_Base()
    : UIControl()
    , m_labelChanged(false)
    , m_label()
{
}

UIControl_Base::~UIControl_Base()
{
}

bool UIControl_Base::setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name)
{
    return UIControl::setupControl(scene, node, name);
}

void UIControl_Base::setLabel(UIString label, bool update, bool force)
{
    if (force || (m_label.empty() == false && label.empty() == false && m_label.compare(label)) || (m_label.empty() != label.empty())) {
        m_labelChanged = true;
    }
    m_label = label;
    if (!m_labelChanged || !update || m_fuiRenderNode == 0) {
        return;
    }
    m_labelChanged = false;
}

void UIControl_Base::tick()
{
    if (m_label.needsUpdating() || m_labelChanged) {
        setLabel(m_label, true, true);
        m_label.setUpdated();
    }
}

const wchar_t* UIControl_Base::getLabel()
{
    if (m_fuiRenderNode == 0) {
        return L"buckets_";
    }
    return (const wchar_t*)(long)m_label.getString();
}

void UIControl_Base::setAllPossibleLabels(int firstLabel, unsigned int labelCount, wchar_t (*labels)[256])
{
    std::vector<std::wstring> possibleLabels;
    unsigned int groups = labelCount / 4;
    unsigned int remainder = labelCount & 3;
    unsigned int i;
    (void)firstLabel;
    for (i = 0; i < groups; ++i) {
        wchar_t* group = labels[i * 4];
        possibleLabels.push_back(std::wstring(group - 256));
        possibleLabels.push_back(std::wstring(group));
        possibleLabels.push_back(std::wstring(group + 256));
        possibleLabels.push_back(std::wstring(group + 512));
    }
    for (i = 0; i < remainder; ++i) {
        possibleLabels.push_back(std::wstring(labels[groups * 4 + i]));
    }
    if (!possibleLabels.empty()) {
        UIString label(possibleLabels[0]);
        setLabel(label, true, true);
    }
}

bool UIControl_Base::hasFocus()
{
    if (m_scene != 0) {
        m_scene->controlHasFocus(this);
    }
    return false;
}
