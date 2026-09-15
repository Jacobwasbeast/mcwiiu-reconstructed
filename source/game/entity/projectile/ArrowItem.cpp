#include "game/entity/projectile/ArrowItem.hpp"

#include "game/entity/projectile/TippableArrow.hpp"

ArrowItem::ArrowItem()
    : Item()
{
}

ArrowItem::~ArrowItem() = default;

boost::shared_ptr<Arrow> ArrowItem::createArrow(Level* level,
                                                not_null_ptr<ItemInstance> itemInstance,
                                                boost::shared_ptr<LivingEntity> entity)
{
    boost::shared_ptr<TippableArrow> arrow(new TippableArrow(level, entity));
    arrow->setEffectsFromItem(itemInstance);
    return arrow;
}
