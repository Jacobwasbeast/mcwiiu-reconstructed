#ifndef GAME_UI_FJ_LIST_LISTBUTTONICONLEFT_HPP
#define GAME_UI_FJ_LIST_LISTBUTTONICONLEFT_HPP

#include <stdint.h>
#include <string>

#include "game/ui/FJ_List.hpp"

class fuiRenderNode;

class FJ_List_ListButtonIconLeft : public FJ_List
{
public:
    explicit FJ_List_ListButtonIconLeft(fuiRenderNode* renderNode);
    virtual ~FJ_List_ListButtonIconLeft();

    void UseIggyIcons();
    void addNewItem(const std::wstring& label, int id,
                    const std::wstring& textureName,
                    const std::wstring& textureName1);
    void SetTextureName(int index, const std::wstring& textureName);
    static FJ_List* Create(fuiRenderNode* renderNode);
    virtual uint32_t getType();

private:
    bool m_useLayerIcon;
    bool m_useIggyIcons;
};

#endif
