#include "game/entity/ai/EntityByTypeSelector.hpp"

#include "game/entity/mob/Entity.hpp"

EntityByTypeSelector::EntityByTypeSelector(unsigned long long entityType)
    : mEntityType(entityType)
{
}

bool EntityByTypeSelector::apply(boost::shared_ptr<Entity> entity) const
{
    unsigned long long type = entity->GetType();

    if ((mEntityType & 0x3F) != 0)
    {
        return type == mEntityType;
    }

    return (type & mEntityType) == mEntityType;
}

EntityByTypeSelector::~EntityByTypeSelector()
{
}
