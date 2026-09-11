#include "game/item/ComplexItem.hpp"

ComplexItem::ComplexItem() : Item()
{
}

ComplexItem::~ComplexItem()
{
}

bool ComplexItem::isComplex()
{
    return true;
}

boost::shared_ptr<Packet> ComplexItem::getUpdatePacket(not_null_ptr<ItemInstance> itemInstance,
                                                       Level* level,
                                                       boost::shared_ptr<Player> player)
{
    boost::shared_ptr<Packet> result;
    unsigned int* storage = reinterpret_cast<unsigned int*>(&result);
    if (storage != NULL || (storage = static_cast<unsigned int*>(operator new(8)), storage != NULL))
    {
        storage[0] = 0;
        storage[1] = 0;
    }
    return result;
}
