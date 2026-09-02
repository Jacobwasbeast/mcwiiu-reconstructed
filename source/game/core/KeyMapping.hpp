#ifndef GAME_CORE_KEYMAPPING_HPP
#define GAME_CORE_KEYMAPPING_HPP

#include <string>

class KeyMapping
{
public:
    KeyMapping(const std::wstring& name, int key);

private:
    std::wstring mName;
    int mKey;
};

#endif
