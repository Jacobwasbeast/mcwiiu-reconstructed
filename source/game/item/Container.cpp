#include "game/item/Container.hpp"

#include "game/item/ItemInstance.hpp"

#include <new>

Container::Container()
{
    Container* obj = this;
#if defined(__POWERPC__) || defined(__powerpc__) || defined(__PPC__)
    // GHS passes the virtual-base pointer in r4. Reading it back is how the
    // original recovers it, and only PowerPC has that register.
    register Nameable* vbaseParam asm("r4");
#else
    // No such register off PowerPC. NULL takes the branch below that builds
    // the base in place, which is what a standalone construction needs; the
    // PPC path is unchanged.
    Nameable* vbaseParam = NULL;
#endif
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
