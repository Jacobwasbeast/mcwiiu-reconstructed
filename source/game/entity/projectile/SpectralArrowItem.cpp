#include "game/entity/projectile/SpectralArrowItem.hpp"

#include "game/entity/projectile/SpectralArrow.hpp"

SpectralArrowItem::SpectralArrowItem()
    : ArrowItem()
{
}

SpectralArrowItem::~SpectralArrowItem()
{
}

boost::shared_ptr<Arrow> SpectralArrowItem::createArrow(Level* level,
                                                        not_null_ptr<ItemInstance> itemInstance,
                                                        boost::shared_ptr<LivingEntity> entity)
{
    boost::shared_ptr<SpectralArrow> arrow(new SpectralArrow(level, entity));
    return arrow;
}
