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
        GenericPendingUpdate(int value) : m_value(value) {}
        virtual ~GenericPendingUpdate();
        virtual unsigned int OnTick() = 0;
    protected:
        int m_value;
    };

    template <class T>
    class PendingUpdate : public GenericPendingUpdate
    {
    public:
        PendingUpdate(int value) : GenericPendingUpdate(value) {}
        virtual ~PendingUpdate() {}
    };

    class PendingSetCurrentSelection : public PendingUpdate<UIControl_GridList>
    {
    public:
        PendingSetCurrentSelection(UIControl_GridList* list, int selection)
            : PendingUpdate<UIControl_GridList>(selection), m_list(list) {}
        virtual ~PendingSetCurrentSelection();
        virtual unsigned int OnTick();
    private:
        UIControl_GridList* m_list;
    };

    class PendingSetEnable : public PendingUpdate<UIControl_GridList>
    {
    public:
        PendingSetEnable(UIControl_GridList* list, int item, bool enabled)
            : PendingUpdate<UIControl_GridList>(item), m_list(list), m_enabled(enabled) {}
        virtual ~PendingSetEnable();
        virtual unsigned int OnTick();
    private:
        UIControl_GridList* m_list;
        bool m_enabled;
    };

    UIControl_GridList();
    virtual ~UIControl_GridList();
    virtual void setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name);
    virtual void tick();
    virtual void ReInit();
    virtual void clearList();

    bool CanTouchFocus(int x, int y);
    void SetTouchFocus(int x, int y, bool focus);
    bool CanTouchTrigger(int x, int y);
    void addElement(UIControl::eUIControlType type, int index);
    void clearElements();
    int getListIndex(int index);
    void init(int controlId);
    void setCurrentSelection(int selection, bool immediate);
    void EnableItem(int item, bool enabled, bool immediate);
    void remElement(int index);
    bool istype(UIControl::eUIControlType type, int index);
    unsigned int getCurrentSelection() const;
    unsigned int getItemCount();
    void updateChildFocus(int index);
    unsigned char IsEnabled(int index);

private:
    std::vector<GenericPendingUpdate*> m_pendingUpdates;
    std::vector<int> m_elements;
    int m_currentSelection;
    int m_itemIndex;
    boost::unordered_map<unsigned int, UIControl::eUIControlType> m_elementTypes;
    boost::unordered_map<unsigned int, bool> m_elementEnabled;
};

#endif
