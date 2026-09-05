#ifndef GAME_UI_UICONTROL_LIST_HPP
#define GAME_UI_UICONTROL_LIST_HPP

#include <boost/unordered_map.hpp>
#include <string>
#include <vector>

#include "game/ui/UIControl_Base.hpp"
#include "game/ui/UIString.hpp"

class UIControl_List : public UIControl_Base
{
public:
    class GenericPendingUpdate
    {
    public:
        GenericPendingUpdate(int type) : m_type(type) {}
        virtual ~GenericPendingUpdate();
        virtual unsigned int OnTick() = 0;
    protected:
        int m_type;
    };

    template <class T>
    class PendingUpdate : public GenericPendingUpdate
    {
    public:
        PendingUpdate(int type) : GenericPendingUpdate(type) {}
        virtual ~PendingUpdate();
    };

    class PendingRemoveItem : public PendingUpdate<UIControl_List>
    {
    public:
        PendingRemoveItem(UIControl_List* list, int item)
            : PendingUpdate<UIControl_List>(0), m_list(list), m_item(item) {}
        virtual ~PendingRemoveItem();
        virtual unsigned int OnTick();
    private:
        UIControl_List* m_list;
        int m_item;
    };

    class PendingSetCurrentSelection : public PendingUpdate<UIControl_List>
    {
    public:
        PendingSetCurrentSelection(UIControl_List* list, int selection, bool immediate)
            : PendingUpdate<UIControl_List>(0), m_list(list), m_selection(selection), m_immediate(immediate) {}
        virtual ~PendingSetCurrentSelection();
        virtual unsigned int OnTick();
    private:
        UIControl_List* m_list;
        int m_selection;
        bool m_immediate;
    };

    class PendingSetEnable : public PendingUpdate<UIControl_List>
    {
    public:
        PendingSetEnable(UIControl_List* list, int item, bool enabled)
            : PendingUpdate<UIControl_List>(0), m_list(list), m_item(item), m_enabled(enabled) {}
        virtual ~PendingSetEnable();
        virtual unsigned int OnTick();
    private:
        UIControl_List* m_list;
        int m_item;
        bool m_enabled;
    };

    class PendingSetLabel : public PendingUpdate<UIControl_List>
    {
    public:
        PendingSetLabel(UIControl_List* list, int item, UIString label)
            : PendingUpdate<UIControl_List>(0), m_list(list), m_item(item), m_label(label) {}
        virtual ~PendingSetLabel();
        virtual unsigned int OnTick();
    private:
        UIControl_List* m_list;
        int m_item;
        UIString m_label;
    };

    UIControl_List();
    virtual ~UIControl_List();
    virtual void setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name);
    virtual void tick();
    virtual void ReInit();
    virtual void UpdateControl();
    void addElement(UIControl::eUIControlType type, int index);
    void GetTopItem();
    void setCurrentSelection(int selection, bool update, bool immediate);
    void updateChildFocus(int index);
    unsigned int getItemCount();
    unsigned int getCurrentSelection() const;
    void init(int controlId);
    int getListIndex(int index);
    void EnableItem(int item, bool enabled, bool immediate);
    bool istype(UIControl::eUIControlType type, int index);
    void clearElements();
    void clearList();
    void SetItemLabel(int item, UIString label, bool immediate);
    void remElement(int index);
    void removeItem(int item, bool immediate);
    unsigned char IsEnabled(int index);
    bool CheckElementExists(int index);
    void GetActiveItem();
    void SetTouchFocus(int x, int y, bool focus);
    bool CanTouchFocus(int x, int y);
    bool CanTouchTrigger(int x, int y);
    int getListDataValue(int index);
    UIString GetItemLabel(int index);
    void SetVerticalSpacing(int spacing);
    int GetVerticalSpacing();

private:
    std::vector<GenericPendingUpdate*> m_pendingUpdates;
    std::vector<int> m_elements;
    int m_currentSelection;
    int m_itemIndex;
    boost::unordered_map<unsigned int, UIControl::eUIControlType> m_elementTypes;
    boost::unordered_map<unsigned int, bool> m_elementEnabled;
};

template <class T>
UIControl_List::PendingUpdate<T>::~PendingUpdate()
{
}

#endif
