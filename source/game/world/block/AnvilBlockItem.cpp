#include "game/world/block/AnvilBlockItem.hpp"

#include <typeinfo>

#include "game/core/Boxed.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/block/AbstractProperty.hpp"
#include "game/world/block/AnvilBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"

static const int ANVIL_BLOCK_NAME_IDS[] = {
    (int)0x99A9E601,
    (int)0x872533DB,
    (int)0x872533DB
};

AnvilBlockItem::AnvilBlockItem(Block* block)
    : MultiBlockItem(block, block, AnvilBlockNameResolver, NULL)
{
}

AnvilBlockItem::~AnvilBlockItem()
{
}

int AnvilBlockItem::AnvilBlockNameResolver(int damage)
{
    Boxed* boxed = AnvilBlock::DAMAGE->getValueAtIndex((unsigned int)damage);
    TypedBoxed<int>* value = boxed->isA(typeid(int)) ? static_cast<TypedBoxed<int>*>(boxed) : NULL;
    return ANVIL_BLOCK_NAME_IDS[value->getValue()];
}

int AnvilBlockItem::AnvilBlockNameResolver(not_null_ptr<ItemInstance> itemInstance)
{
    return AnvilBlockNameResolver(itemInstance->getAuxValue());
}

int AnvilBlockItem::getLevelDataForAuxValue(int auxValue)
{
    return auxValue;
}

int AnvilBlockItem::getDescriptionId(int auxValue)
{
    return AnvilBlockNameResolver(auxValue >> 2);
}
