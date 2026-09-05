#ifndef GAME_UI_UICONTROL_GRIDLIST_HPP
#define GAME_UI_UICONTROL_GRIDLIST_HPP

#include <boost/unordered_map.hpp>
#include <string>
#include <vector>

#include "game/ui/UIControl_Base.hpp"

class UIControl_GridList : public UIControl_Base
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

    class PendingSetCurrentSelection : public PendingUpdate<UIControl_GridList>
    {
    public:
        PendingSetCurrentSelection(UIControl_GridList* list, int selection)
            : PendingUpdate<UIControl_GridList>(0), m_list(list), m_selection(selection) {}
        virtual ~PendingSetCurrentSelection();
        virtual unsigned int OnTick();
    private:
        UIControl_GridList* m_list;
        int m_selection;
    };

    class PendingSetEnable : public PendingUpdate<UIControl_GridList>
    {
    public:
        PendingSetEnable(UIControl_GridList* list, int item, bool enabled)
            : PendingUpdate<UIControl_GridList>(0), m_list(list), m_item(item), m_enabled(enabled) {}
        virtual ~PendingSetEnable();
        virtual unsigned int OnTick();
    private:
        UIControl_GridList* m_list;
        int m_item;
        bool m_enabled;
    };

    UIControl_GridList();
    virtual ~UIControl_GridList();
    bool CanTouchFocus(int x, int y);
    void SetTouchFocus(int x, int y, bool focus);
    bool CanTouchTrigger(int x, int y);
    void addElement(UIControl::eUIControlType type, int index);
    void clearElements();
    int getListIndex(int index);
    virtual void setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name);
    void clearList();
    void init(int controlId);
    virtual void ReInit();
    virtual void tick();
    void setCurrentSelection(int selection, bool immediate);
    void EnableItem(int item, bool enabled, bool immediate);
    void remElement(int index);
    bool istype(UIControl::eUIControlType type, int index);
    int getCurrentSelection() const;
    int getItemCount();
    void updateChildFocus(int index);
    bool IsEnabled(int index);

private:
    std::vector<GenericPendingUpdate*> m_pendingUpdates;
    std::vector<int> m_elements;
    int m_currentSelection;
    int m_itemIndex;
    boost::unordered_map<unsigned int, UIControl::eUIControlType> m_elementTypes;
    boost::unordered_map<unsigned int, bool> m_elementEnabled;
};

template <class T>
UIControl_GridList::PendingUpdate<T>::~PendingUpdate()
{
}

#endif
