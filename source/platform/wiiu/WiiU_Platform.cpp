#include "platform/wiiu/WiiU_Platform.hpp"

#include "game/core/L10N.hpp"

wchar_t* GetRichPresenceStringFromId(int id)
{
    return const_cast<wchar_t*>(L10N::GetString(id));
}

unsigned int get_system_speaker_configuration(unsigned int* configuration)
{
    *configuration = 2;
    return 1;
}
