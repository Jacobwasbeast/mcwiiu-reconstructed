#ifndef GAME_UI_FJ_LIST_MULTILIST_HPP
#define GAME_UI_FJ_LIST_MULTILIST_HPP

#include <stdint.h>
#include <string>
#include <vector>

#include "game/ui/FJ_List.hpp"

class fuiRenderNode;

class FJ_List_MultiList : public FJ_List
{
public:
    explicit FJ_List_MultiList(fuiRenderNode* renderNode);
    virtual ~FJ_List_MultiList();

    static FJ_List* Create(fuiRenderNode* renderNode);
    virtual uint32_t getType();
    void addNewItem_Label(const std::wstring& label);
    void addNewItem_Button(const std::wstring& label, int id, bool useEllipsis);
    void addNewItem_MenuButton(const std::wstring& label, int id);
    void addNewItem_ControllerButton(const std::wstring& label, const std::wstring& secondaryLabel, bool html, int id);
    void addNewItem_CheckBox(const std::wstring& label, int id, bool checked);
    void addNewItem_Slider(const std::wstring& label, int id, int minValue, int maxValue, int value, bool repeat, int repeatDelay, int repeatStep);
    void addNewItem_TextInput(const std::wstring& label, int id, int charLimit);
    virtual void HandleCharInput(unsigned int character);
    void SetCheckBox(int index, bool checked);
    uint8_t GetCheckBox(int index);
    void SetSliderValue(int index, int value);
    uint32_t GetSliderValue(int index);
    void SetSecondaryLabel(int index, const std::wstring& label, bool html, bool autoSize);
    void ToggleButtonOutline(int index, bool show);

private:
    friend void __CPR268__SetPossibleLabels__17FJJ_List_MultiListFiQ2_3std218vector__tm__203_Q2_3std78basic_string__tm__58_wQ2_3std20char_traits__tm__2_wQ2_3std18allocator__tm__2_wQ2_3std105allocator__tm__88_Q2_3stdJ73J(
        FJ_List_MultiList*, int, std::vector<std::wstring>);
    std::vector<std::vector<std::wstring> > m_labelLists;
};

#endif
