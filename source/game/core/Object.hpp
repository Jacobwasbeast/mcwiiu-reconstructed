#ifndef GAME_CORE_OBJECT_HPP
#define GAME_CORE_OBJECT_HPP

struct Object
{
    enum EType
    {
        eType_Item = 0x4000202,
        eType_ItemInstance = 0x4000203,
        eType_ArmorItem = 0x4000205,
        eType_Block = 0x4000206
    };

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
