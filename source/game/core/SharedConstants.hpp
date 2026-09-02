#ifndef GAME_CORE_SHAREDCONSTANTS_HPP
#define GAME_CORE_SHAREDCONSTANTS_HPP

#include <string>

extern std::wstring ALLOWED_CHARACTERS;

extern bool IS_HALLOWEEN;
extern bool IS_HALLOWEEN_SEASON;
extern bool IS_CHRISTMAS;
extern bool IS_CHRISTMAS_SEASON;

class SharedConstants
{
public:
    static void staticCtor();
    static bool isAllowedChatCharacter(char character);
};

#endif
