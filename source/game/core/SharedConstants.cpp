#include "game/core/SharedConstants.hpp"

#include "game/core/Calendar.hpp"

std::wstring ALLOWED_CHARACTERS;

bool IS_HALLOWEEN = false;
bool IS_HALLOWEEN_SEASON = false;
bool IS_CHRISTMAS = false;
bool IS_CHRISTMAS_SEASON = false;

void SharedConstants::staticCtor()
{
    ALLOWED_CHARACTERS = L" !\"#$%&'()*+,-./0123456789:;<=>?";

    IS_HALLOWEEN = Calendar::GetMonth() == 10 && Calendar::GetDayOfMonth() == 31;

    IS_HALLOWEEN_SEASON = (Calendar::GetMonth() == 10 && Calendar::GetDayOfMonth() >= 20)
        || (Calendar::GetMonth() == 11 && Calendar::GetDayOfMonth() <= 3);

    IS_CHRISTMAS = (Calendar::GetMonth() == 12 && Calendar::GetDayOfMonth() >= 24)
        || (Calendar::GetMonth() == 1 && Calendar::GetDayOfMonth() <= 1);

    IS_CHRISTMAS_SEASON = (Calendar::GetMonth() == 12 && Calendar::GetDayOfMonth() >= 17)
        && (Calendar::GetMonth() == 12 && Calendar::GetDayOfMonth() <= 23);
}

bool SharedConstants::isAllowedChatCharacter(char character)
{
    return character >= ' ' && character <= '?';
}
