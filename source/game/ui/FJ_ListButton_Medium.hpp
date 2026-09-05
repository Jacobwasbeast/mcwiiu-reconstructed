#ifndef GAME_UI_FJ_LISTBUTTON_MEDIUM_HPP
#define GAME_UI_FJ_LISTBUTTON_MEDIUM_HPP

#include <stdint.h>
#include <string>
#include "game/ui/FJ_ListButton.hpp"

class fuiRenderNode;

class FJ_ListButton_Medium : public FJ_ListButton
{
public:
    explicit FJ_ListButton_Medium(fuiRenderNode* renderNode);
    virtual ~FJ_ListButton_Medium();
    static FJ_ListButton* Create(fuiRenderNode* renderNode);
    virtual uint32_t getType();

private:
    std::wstring m_text;
};

#endif
