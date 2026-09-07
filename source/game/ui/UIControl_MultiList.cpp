#define private public
#include "game/ui/UIControl_MultiList.hpp"
#undef private

#include "game/ui/FJ_FuiNode.hpp"
#define private public
#include "game/ui/FJ_List_MultiList.hpp"
#undef private
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"

UIControl_MultiList::UIControl_MultiList()
{
}

UIControl_MultiList::~UIControl_MultiList()
{
}

void UIControl_MultiList::AddNewButton(const std::wstring& label, int id, bool useEllipsis)
{
    m_fuiRenderNode->getFuiNode()->asFJ_List_MultiList()->addNewItem_Button(label, id, useEllipsis);
    addElement((UIControl::eUIControlType)1, id);
}

void UIControl_MultiList::AddNewLabel(const std::wstring& label, int id)
{
    m_fuiRenderNode->getFuiNode()->asFJ_List_MultiList()->addNewItem_Label(label);
    addElement((UIControl::eUIControlType)8, id);
}

void UIControl_MultiList::AddNewCheckbox(const std::wstring& label, int id, bool checked)
{
    m_fuiRenderNode->getFuiNode()->asFJ_List_MultiList()->addNewItem_CheckBox(label, id, checked);
    addElement((UIControl::eUIControlType)2, id);
}

void UIControl_MultiList::AddNewControllerButton(const std::wstring& label,
                                                  const std::wstring& secondaryLabel,
                                                  bool html, int id)
{
    m_fuiRenderNode->getFuiNode()->asFJ_List_MultiList()->addNewItem_ControllerButton(
        label, secondaryLabel, html, id);
    addElement((UIControl::eUIControlType)1, id);
}

void UIControl_MultiList::AddNewSlider(const std::wstring& label, int id, int minValue,
                                       int maxValue, int value, bool repeat, int repeatDelay,
                                       int repeatStep)
{
    m_fuiRenderNode->getFuiNode()->asFJ_List_MultiList()->addNewItem_Slider(
        label, id, minValue, maxValue, value, repeat, repeatDelay, repeatStep);
    addElement((UIControl::eUIControlType)15, id);
}

void UIControl_MultiList::SetCheckboxValue(int id, bool checked, bool immediate)
{
    if (immediate) {
        m_fuiRenderNode->getFuiNode()->asFJ_List_MultiList()->SetCheckBox(
            getListIndex(id), checked);
        return;
    }
    m_pendingUpdates.push_back(new PendingSetChecked(this, id, checked));
}

bool UIControl_MultiList::IsCheckbox(int id)
{
    return istype((UIControl::eUIControlType)2, id);
}

uint64_t UIControl_MultiList::GetCheckboxValue(int id)
{
    return (uint64_t)m_fuiRenderNode->getFuiNode()->asFJ_List_MultiList()->GetCheckBox(
        getListIndex(id)) << 32;
}

void UIControl_MultiList::AddNewTextInput(const std::wstring& label, int id, int charLimit)
{
    m_fuiRenderNode->getFuiNode()->asFJ_List_MultiList()->addNewItem_TextInput(
        label, id, charLimit);
    addElement((UIControl::eUIControlType)17, id);
}

uint64_t UIControl_MultiList::GetSliderValue(int id)
{
    return (uint64_t)m_fuiRenderNode->getFuiNode()->asFJ_List_MultiList()->GetSliderValue(
        getListIndex(id)) << 32;
}

void UIControl_MultiList::setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name)
{
    m_controlType = 28;
    UIControl_List::setupControl(scene, node, name);
}

void UIControl_MultiList::setAllPossibleLabels(int firstLabel, unsigned int labelCount,
                                                wchar_t (*labels)[256])
{
    std::vector<std::wstring> possibleLabels;
    unsigned int groupCount = labelCount / 4;
    unsigned int remainder = labelCount & 3;
    unsigned int i;
    for (i = 0; i < groupCount; ++i) {
        wchar_t* group = labels[i * 4 + 1];
        possibleLabels.push_back(std::wstring(group - 256));
        possibleLabels.push_back(std::wstring(group));
        possibleLabels.push_back(std::wstring(group + 256));
        possibleLabels.push_back(std::wstring(group + 512));
    }
    for (i = 0; i < remainder; ++i)
        possibleLabels.push_back(std::wstring(labels[groupCount * 4 + i]));

    FJ_List_MultiList* list = m_fuiRenderNode->getFuiNode()->asFJ_List_MultiList();
    if (list->m_labelLists.size() <= (unsigned int)firstLabel)
        list->m_labelLists.resize((unsigned int)firstLabel + 1);
    list->m_labelLists[(unsigned int)firstLabel] = possibleLabels;
}

void UIControl_MultiList::AddNewMenuButton(const std::wstring& label, int id)
{
    m_fuiRenderNode->getFuiNode()->asFJ_List_MultiList()->addNewItem_MenuButton(label, id);
    addElement((UIControl::eUIControlType)1, id);
}

void UIControl_MultiList::IsSlider(int id)
{
    istype((UIControl::eUIControlType)15, id);
}

void UIControl_MultiList::HighlightItem(unsigned int id, bool update)
{
    setCurrentSelection(getListIndex((int)id), update, true);
}

void UIControl_MultiList::SetSliderValue(int id, int value, bool immediate)
{
    if (immediate) {
        m_fuiRenderNode->getFuiNode()->asFJ_List_MultiList()->SetSliderValue(
            getListIndex(id), value);
        return;
    }
    m_pendingUpdates.push_back(new PendingSetSlider(this, id, value));
}

void UIControl_MultiList::SetItemOpacity(int id, float opacity, bool immediate)
{
    if (!immediate) {
        m_pendingUpdates.push_back(new PendingSetOpacity(this, id, opacity));
        return;
    }
    if (m_fuiRenderNode != NULL) {
        FJ_List_MultiList* list = m_fuiRenderNode->getFuiNode()->asFJ_List_MultiList();
        if (list != NULL)
            list->SetItemAlpha(getListIndex(id), opacity);
    }
}

void UIControl_MultiList::SetSecondaryLabel(int id, UIString label, bool html, bool autoSize,
                                             bool immediate)
{
    (void)immediate;
    m_fuiRenderNode->getFuiNode()->asFJ_List_MultiList()->SetSecondaryLabel(
        getListIndex(id), label.getString(), html, autoSize);
}

void UIControl_MultiList::ToggleButtonOutline(int id, bool outline, bool immediate)
{
    if (immediate) {
        m_fuiRenderNode->getFuiNode()->asFJ_List_MultiList()->ToggleButtonOutline(
            getListIndex(id), outline);
        return;
    }
    m_pendingUpdates.push_back(new PendingSetToggleButtonOutline(this, id, outline));
}

UIControl_MultiList::PendingSetChecked::~PendingSetChecked()
{
}

unsigned int UIControl_MultiList::PendingSetChecked::OnTick()
{
    m_list->SetCheckboxValue(m_item, m_checked, true);
    return 1;
}

UIControl_MultiList::PendingSetOpacity::~PendingSetOpacity()
{
}

unsigned int UIControl_MultiList::PendingSetOpacity::OnTick()
{
    m_list->SetItemOpacity(m_item, m_opacity, true);
    return 1;
}

UIControl_MultiList::PendingSetSlider::~PendingSetSlider()
{
}

unsigned int UIControl_MultiList::PendingSetSlider::OnTick()
{
    m_list->SetSliderValue(m_item, m_value, true);
    return 1;
}

UIControl_MultiList::PendingSetToggleButtonOutline::~PendingSetToggleButtonOutline()
{
}

unsigned int UIControl_MultiList::PendingSetToggleButtonOutline::OnTick()
{
    m_list->ToggleButtonOutline(m_item, m_outline, true);
    return 1;
}
