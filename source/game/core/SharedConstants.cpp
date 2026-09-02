#include "game/core/SharedConstants.hpp"

#include "game/core/Calendar.hpp"

std::wstring ALLOWED_CHARACTERS;

bool IS_HALLOWEEN = false;
bool IS_HALLOWEEN_SEASON = false;
bool IS_CHRISTMAS = false;
bool IS_CHRISTMAS_SEASON = false;

void SharedConstants::staticCtor()
{
    ALLOWED_CHARACTERS = L" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_'abcdefghijklmnopqrstuvwxyz{|}~\u00A0\u00C7\u00FC\u00E9\u00E2\u00E4\u00E0\u00E5\u00E7\u00EA\u00EB\u00E8\u00EF\u00EE\u00EC\u00C4\u00C5\u00C9\u00E6\u00C6\u00F4\u00F6\u00F2\u00FB\u00F9\u00FF\u00D6\u00DC\u00F8\u00A3\u00D8\u00D7\u0192\u00E1\u00ED\u00F3\u00FA\u00F1\u00D1\u00AA\u00BA\u00BF\u00AE\u00AC\u00BD\u00BC\u00A1\u00AB\u00BB\u00E3";

    IS_HALLOWEEN = Calendar::GetMonth() == 10 && Calendar::GetDayOfMonth() == 31;

    IS_HALLOWEEN_SEASON = (Calendar::GetMonth() == 10 && Calendar::GetDayOfMonth() >= 20)
        || (Calendar::GetMonth() == 11 && Calendar::GetDayOfMonth() <= 3);

    IS_CHRISTMAS = (Calendar::GetMonth() == 12 && Calendar::GetDayOfMonth() >= 24)
        || (Calendar::GetMonth() == 1 && Calendar::GetDayOfMonth() <= 1);

    IS_CHRISTMAS_SEASON = (Calendar::GetMonth() == 12 && Calendar::GetDayOfMonth() >= 17)
        && (Calendar::GetMonth() == 12 && Calendar::GetDayOfMonth() <= 23);
}
