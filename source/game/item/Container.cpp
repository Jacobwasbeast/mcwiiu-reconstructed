#include "game/item/Container.hpp"

#include "game/item/ItemInstance.hpp"

#include <new>

Container::Container()
{
    Container* obj = this;
    register Nameable* vbaseParam asm("r4");
    if (obj != NULL || (obj = static_cast<Container*>(operator new(0xc)), obj != NULL))
    {
        unsigned int* storage = reinterpret_cast<unsigned int*>(obj);
        Nameable* vbase = vbaseParam;
        if (vbase == NULL)
        {
            vbase = reinterpret_cast<Nameable*>(storage + 2);
            storage[2] = 0x100004C4u;
        }
        storage[1] = reinterpret_cast<unsigned int>(vbase);
        storage[0] = 0x10002174u;
        *reinterpret_cast<unsigned int*>(vbase) = 0x100004ECu;
    }
}

Container::~Container()
{
}

bool Container::isFull()
{
    for (unsigned int slot = 0; slot < getContainerSize(); ++slot)
    {
        if (getItem(slot)->isEmpty())
        {
            return false;
        }
    }

    return true;
}
