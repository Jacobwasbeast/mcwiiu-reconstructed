#ifndef GAME_CORE_L10N_HPP
#define GAME_CORE_L10N_HPP

class L10N
{
public:
    static const wchar_t* GetString(int id);
    static void loadStringTable();
};

#endif
