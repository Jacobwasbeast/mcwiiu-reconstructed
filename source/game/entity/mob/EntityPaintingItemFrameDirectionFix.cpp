#include "game/entity/mob/EntityPaintingItemFrameDirectionFix.hpp"

#include <string>

#include "game/core/Direction.hpp"
#include "game/data/CompoundTag.hpp"

EntityPaintingItemFrameDirectionFix::~EntityPaintingItemFrameDirectionFix()
{
}

int EntityPaintingItemFrameDirectionFix::getVersion()
{
    return 111;
}

CompoundTag* EntityPaintingItemFrameDirectionFix::fix(CompoundTag* tag)
{
    bool isPainting = CheckID(tag, L"Painting");
    bool isItemFrame = CheckID(tag, L"ItemFrame");

    if ((isPainting || isItemFrame) && !tag->contains(L"Facing", 99))
    {
        const Direction* direction;

        if (tag->contains(L"Direction", 99))
        {
            direction = Direction::from2DDataValue(tag->getByte(L"Direction"));

            tag->putInt(L"TileX", tag->getInt(L"TileX") + direction->getStepX());
            tag->putInt(L"TileY", tag->getInt(L"TileY") + direction->getStepY());
            tag->putInt(L"TileZ", tag->getInt(L"TileZ") + direction->getStepZ());

            tag->remove(L"Direction");

            if (isItemFrame && tag->contains(L"ItemRotation", 99))
            {
                tag->putByte(L"ItemRotation", tag->getByte(L"ItemRotation") * 2);
            }
        }
        else
        {
            direction = Direction::from2DDataValue(tag->getByte(L"Dir"));

            tag->remove(L"Dir");
        }

        tag->putByte(L"Facing", direction->get2DDataValue());
    }

    return tag;
}
