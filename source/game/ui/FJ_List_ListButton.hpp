#ifndef GAME_UI_FJ_LIST_LISTBUTTON_HPP
#define GAME_UI_FJ_LIST_LISTBUTTON_HPP

#include <stdint.h>
#include <string>

#include "game/ui/FJ_List.hpp"

class fuiRenderNode;

class FJ_List_ListButton : public FJ_List
{
public:
    explicit FJ_List_ListButton(fuiRenderNode* renderNode);
    virtual ~FJ_List_ListButton();

    void addNewItem(const std::wstring& label, int id);
    static FJ_List* Create(fuiRenderNode* renderNode);
    virtual uint32_t getType();

private:
    int m_labelAlignment;
    bool m_mediumButtons;
};

#endif
