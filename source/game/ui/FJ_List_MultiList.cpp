#define private public
#include "game/ui/FJ_List.hpp"
#include "game/ui/FJ_ListButton.hpp"
#include "game/ui/FJ_MenuButton.hpp"
#include "game/ui/FJ_Label.hpp"
#include "game/ui/FJ_CheckBox.hpp"
#include "game/ui/FJ_Slider.hpp"
#include "game/ui/FJ_TextInput.hpp"
#undef private

#include "game/ui/FJ_List_MultiList.hpp"
#include "game/ui/FJ_Factory.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_ListButtonController.hpp"
#include "game/ui/fuiRenderNode.hpp"


void __CPR268__SetPossibleLabels__17FJJ_List_MultiListFiQ2_3std218vector__tm__203_Q2_3std78basic_string__tm__58_wQ2_3std20char_traits__tm__2_wQ2_3std18allocator__tm__2_wQ2_3std105allocator__tm__88_Q2_3stdJ73J(
    FJ_List_MultiList* list, int index, std::vector<std::wstring> labels)
{
    if (list->m_labelLists.size() <= static_cast<unsigned int>(index))
        list->m_labelLists.resize(static_cast<unsigned int>(index) + 1);
    list->m_labelLists[static_cast<unsigned int>(index)] = labels;
}

FJ_List_MultiList::FJ_List_MultiList(fuiRenderNode* renderNode)
    : FJ_List(renderNode)
{
}

FJ_List_MultiList::~FJ_List_MultiList()
{
}

FJ_List* FJ_List_MultiList::Create(fuiRenderNode* renderNode)
{
    return new FJ_List_MultiList(renderNode);
}

uint32_t FJ_List_MultiList::getType()
{
    return 0x8031;
}

void FJ_List_MultiList::addNewItem_Label(const std::wstring& label)
{
    FJ_Label* item = m_checkboxTick
        ? FJ_Factory::createObject("fourj.Labels.FJ_ListLabel_SmallBlack", m_pRenderNode, true, NULL)->asFJ_Label()
        : FJ_Factory::createObject("fourj.Labels.FJ_ListLabel_Black", m_pRenderNode, true, NULL)->asFJ_Label();
    addNewListItem(item, label, -1);
    item->Init(label);
}

void FJ_List_MultiList::addNewItem_Button(const std::wstring& label, int id, bool useEllipsis)
{
    FJ_ListButton* item = m_checkboxTick
        ? FJ_Factory::createObject("fourj.Buttons.FJ_ListButton_Small", m_pRenderNode, true, NULL)->asFJ_ListButton()
        : FJ_Factory::createObject("fourj.Buttons.FJ_ListButton_Medium", m_pRenderNode, true, NULL)->asFJ_ListButton();
    item->m_listButtonFlag = true;
    item->UseEllipsis(useEllipsis);
    if (id >= 0 && static_cast<unsigned int>(id) < m_labelLists.size() && !m_labelLists[static_cast<unsigned int>(id)].empty())
    {
        item->CheckLabelWidths(m_labelLists[static_cast<unsigned int>(id)]);
        m_labelLists[static_cast<unsigned int>(id)].clear();
    }
    addNewListItem(item, label, id);
    item->Init(label, id);
}

void FJ_List_MultiList::addNewItem_MenuButton(const std::wstring& label, int id)
{
    FJ_FuiNode* node = m_checkboxTick
        ? FJ_Factory::createObject("fourj.Buttons.FJ_MenuButton_Small", m_pRenderNode, true, NULL)
        : FJ_Factory::createObject("fourj.Buttons.FJ_MenuButton_Normal", m_pRenderNode, true, NULL);
    if (id >= 0 && static_cast<unsigned int>(id) < m_labelLists.size() && !m_labelLists[static_cast<unsigned int>(id)].empty())
    {
        node->asFJ_Base()->CheckLabelWidths(m_labelLists[static_cast<unsigned int>(id)]);
        m_labelLists[static_cast<unsigned int>(id)].clear();
    }
    addNewListItem(node->asFJ_Base(), label, id);
    node->asFJ_MenuButton()->Init(label, id);
}

void FJ_List_MultiList::addNewItem_ControllerButton(const std::wstring& label, const std::wstring& secondaryLabel, bool html, int id)
{
    FJ_ListButtonController* item = m_checkboxTick
        ? FJ_Factory::createObject("fourj.Buttons.FJ_ListButtonController_Small", m_pRenderNode, true, NULL)->asFJ_ListButtonController()
        : FJ_Factory::createObject("fourj.Buttons.FJ_ListButtonController_Normal", m_pRenderNode, true, NULL)->asFJ_ListButtonController();
    if (id >= 0 && static_cast<unsigned int>(id) < m_labelLists.size() && !m_labelLists[static_cast<unsigned int>(id)].empty())
    {
        item->CheckLabelWidths(m_labelLists[static_cast<unsigned int>(id)]);
        m_labelLists[static_cast<unsigned int>(id)].clear();
    }
    addNewListItem(item, label, id);
    item->Init(label, id);
    item->SetSecondaryText(secondaryLabel, true, html);
}

void FJ_List_MultiList::addNewItem_CheckBox(const std::wstring& label, int id, bool checked)
{
    FJ_CheckBox* item = m_checkboxTick
        ? FJ_Factory::createObject("fourj.Checkbox.FJ_CheckBox_Small", m_pRenderNode, true, NULL)->asFJ_CheckBox()
        : FJ_Factory::createObject("fourj.Checkbox.FJ_CheckBox_Normal", m_pRenderNode, true, NULL)->asFJ_CheckBox();
    if (id >= 0 && static_cast<unsigned int>(id) < m_labelLists.size() && !m_labelLists[static_cast<unsigned int>(id)].empty())
    {
        item->CheckLabelWidths(m_labelLists[static_cast<unsigned int>(id)]);
        m_labelLists[static_cast<unsigned int>(id)].clear();
    }
    addNewListItem(item, label, id);
    item->Init(label, id, checked);
}

void FJ_List_MultiList::addNewItem_Slider(const std::wstring& label, int id, int minValue, int maxValue, int value, bool repeat, int repeatDelay, int repeatStep)
{
    FJ_Slider* item = m_checkboxTick
        ? FJ_Factory::createObject("fourj.FJ_Slider_Small", m_pRenderNode, true, NULL)->asFJ_Slider()
        : FJ_Factory::createObject("fourj.FJ_Slider_Normal", m_pRenderNode, true, NULL)->asFJ_Slider();
    if (id >= 0 && static_cast<unsigned int>(id) < m_labelLists.size() && !m_labelLists[static_cast<unsigned int>(id)].empty())
    {
        item->CheckLabelWidths(m_labelLists[static_cast<unsigned int>(id)]);
        m_labelLists[static_cast<unsigned int>(id)].clear();
    }
    addNewListItem(item, label, id);
    item->Init(label, id, minValue, maxValue, value, repeat, repeatDelay, repeatStep);
}

void FJ_List_MultiList::addNewItem_TextInput(const std::wstring& label, int id, int charLimit)
{
    FJ_TextInput* item = m_checkboxTick
        ? FJ_Factory::createObject("fourj.TextInput.FJ_TextInput_Small", m_pRenderNode, true, NULL)->asFJ_TextInput()
        : FJ_Factory::createObject("fourj.TextInput.FJ_TextInput_Normal", m_pRenderNode, true, NULL)->asFJ_TextInput();
    addNewListItem(item, label, id);
    item->Init(label, id);
    item->SetCharLimit(charLimit);
}

void FJ_List_MultiList::HandleCharInput(unsigned int character)
{
    FJ_FuiNode* item = ReturnItem(GetActiveItem());
    if (item != NULL && item->asFJ_TextInput() != NULL)
        item->asFJ_TextInput()->HandleDirectInput(character);
}

void FJ_List_MultiList::SetCheckBox(int index, bool checked)
{
    FJ_FuiNode* item = ReturnItem(index);
    if (item != NULL && item->asFJ_CheckBox() != NULL)
        item->asFJ_CheckBox()->SetCheckBox(checked);
}

uint8_t FJ_List_MultiList::GetCheckBox(int index)
{
    FJ_FuiNode* item = ReturnItem(index);
    return item != NULL && item->asFJ_CheckBox() != NULL ? item->asFJ_CheckBox()->getChecked() : 0;
}

void FJ_List_MultiList::SetSliderValue(int index, int value)
{
    FJ_FuiNode* item = ReturnItem(index);
    if (item != NULL && item->asFJ_Slider() != NULL)
        item->asFJ_Slider()->SetSliderValue(value);
}

uint32_t FJ_List_MultiList::GetSliderValue(int index)
{
    FJ_FuiNode* item = ReturnItem(index);
    return item != NULL && item->asFJ_Slider() != NULL ? item->asFJ_Slider()->getSliderCurrent() : 0;
}

void FJ_List_MultiList::SetSecondaryLabel(int index, const std::wstring& label, bool html, bool autoSize)
{
    FJ_FuiNode* item = ReturnItem(index);
    if (item != NULL && item->asFJ_ListButtonController() != NULL)
        item->asFJ_ListButtonController()->SetSecondaryText(label, html, autoSize);
}

void FJ_List_MultiList::ToggleButtonOutline(int index, bool show)
{
    FJ_FuiNode* item = ReturnItem(index);
    if (item != NULL && item->asFJ_Button() != NULL)
        item->asFJ_Button()->ToggleOutline(show);
}
