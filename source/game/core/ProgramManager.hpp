#ifndef GAME_CORE_PROGRAMMANAGER_HPP
#define GAME_CORE_PROGRAMMANAGER_HPP

#include <stdint.h>

class Effect;

class ProgramManager
{
public:
    static void createInstance();
    static uint32_t getInstance();
    void releaseProgram(Effect* effect);
    void createProgram();
    void linkProgram(Effect* effect);
};

#endif
