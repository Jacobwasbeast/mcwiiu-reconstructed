#ifndef GAME_CORE_OBJECT_HPP
#define GAME_CORE_OBJECT_HPP

struct Object
{
    Object(void* object, int auxValue, int flags)
        : object(object), auxValue(auxValue), flags(flags)
    {
    }

    void* object;
    int value;
    int auxValue;
    int flags;
};

#endif
