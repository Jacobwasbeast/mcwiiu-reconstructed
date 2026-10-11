#ifndef GAME_CORE_SHAREDCONSTANTS_HPP
#define GAME_CORE_SHAREDCONSTANTS_HPP

#include <string>

class SharedConstants
{
public:
    static void staticCtor();
    static bool isAllowedChatCharacter(char character);

    static const std::wstring VERSION_STRING;
    static const std::wstring DATA_VERSION_TAG;

    static std::wstring ALLOWED_CHARACTERS;

    static const float CONTAINER_INTERACT_DISTANCE_SQR;

    static bool IS_HALLOWEEN;
    static bool IS_HALLOWEEN_SEASON;
    static bool IS_CHRISTMAS;
    static bool IS_CHRISTMAS_SEASON;
};

#endif
