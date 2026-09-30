#ifndef GAME_UI_FJ_LABEL_BOOK_HPP
#define GAME_UI_FJ_LABEL_BOOK_HPP

#include <string>
#include "game/ui/FJ_Label.hpp"

class FJ_Label_Book : public FJ_Label
{
public:

    virtual void Init(const std::wstring& label);
    virtual void SetLabel(const std::wstring& label);
};

#endif
