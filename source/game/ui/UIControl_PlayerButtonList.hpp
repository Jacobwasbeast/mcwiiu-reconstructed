#ifndef GAME_UI_UICONTROL_PLAYERBUTTONLIST_HPP
#define GAME_UI_UICONTROL_PLAYERBUTTONLIST_HPP

#include <string>

#include "game/ui/UIControl_ButtonList.hpp"

class UIScene;
class fuiRenderNode;

class UIControl_PlayerButtonList : public UIControl_ButtonList
{
public:
    class PendingAddItem : public UIControl_List::GenericPendingUpdate
    {
    public:
        PendingAddItem(UIControl_PlayerButtonList* list, int type, std::wstring label,
                       int playerIcon, int voipIcon);
        virtual ~PendingAddItem();
        virtual unsigned int OnTick();

    private:
        UIControl_PlayerButtonList* m_list;
        std::wstring m_label;
        int m_playerIcon;
        int m_voipIcon;
    };

    virtual ~UIControl_PlayerButtonList();
    virtual void setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name);
    void setVOIPIcon(int item, int icon);
    void addItem(const std::wstring& label, int playerIcon, int voipIcon, bool immediate);
    void setPlayerIcon(int item, int icon);
};

#endif
