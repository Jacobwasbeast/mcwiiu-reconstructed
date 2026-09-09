#include "game/world/block/TheEndPortalBlockEntity.hpp"

#include "game/core/Direction.hpp"

TheEndPortalBlockEntity::~TheEndPortalBlockEntity()
{
}

BlockEntity* TheEndPortalBlockEntity::create()
{
    return new TheEndPortalBlockEntity();
}

boost::shared_ptr<BlockEntity> TheEndPortalBlockEntity::clone()
{
    boost::shared_ptr<BlockEntity> result(new TheEndPortalBlockEntity());
    BlockEntity::clone(result);
    return result;
}

bool TheEndPortalBlockEntity::shouldRenderFace(const Direction* direction)
{
    return direction == Direction::DOWN;
}

unsigned long long TheEndPortalBlockEntity::GetType()
{
    return 0x01300200ULL;
}
