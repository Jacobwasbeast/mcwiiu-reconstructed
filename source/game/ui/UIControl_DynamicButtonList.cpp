#include "game/ui/UIControl_DynamicButtonList.hpp"

void UIControl_DynamicButtonList::tick()
{
    UIControl_List::tick();

    std::vector<UIString>::iterator label = m_labels.begin();
    int index = 0;
    while (label != m_labels.end()) {
        if (label->needsUpdating() != 0) {
            const std::wstring& value = label->getString();
            setButtonLabel(index, value);
            label->setUpdated();
        }
        ++label;
        ++index;
    }
}

void UIControl_DynamicButtonList::addItem(UIString label, int index)
{
    if (index < 0) {
        index = m_itemIndex;
    }
    m_labels.push_back(label);
    const std::wstring& value = label.getString();
    UIControl_ButtonList::addItem(value, index);
}

void UIControl_DynamicButtonList::removeItem(int index, bool removeNow)
{
    m_labels.erase(m_labels.begin() + index);
    UIControl_List::removeItem(index, removeNow);
}
