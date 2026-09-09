#include "game/item/CompassItem.hpp"

#include <cmath>

#include <boost/shared_ptr.hpp>

#include "game/core/IconRegister.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/ItemFrame.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/Math.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"

CompassItem::CompassItem()
    : Item()
{
    mIcons[0] = NULL;
    addProperty(ResourceLocation(L"angle"), new CompassItem_ItemPropertyFunction());
}

CompassItem::~CompassItem()
{
}

TextureAtlasSprite* CompassItem::getIcon(int auxValue)
{
    TextureAtlasSprite* icon = Item::getIcon(auxValue);
    Minecraft* minecraft = Minecraft::GetInstance();

    if (minecraft->mPlayer && auxValue == 0)
    {
        icon = mIcons[static_cast<LocalPlayer*>(minecraft->mPlayer.get())->mUserIndex];
    }

    return icon;
}

void CompassItem::registerIcons(IconRegister* iconRegister)
{
    Item::registerIcons(iconRegister);
    mIcons[0] = iconRegister->registerIcon(L"compass_0");
    mIcons[1] = iconRegister->registerIcon(L"compass_1");
    mIcons[2] = iconRegister->registerIcon(L"compass_2");
    mIcons[3] = iconRegister->registerIcon(L"compass_3");
}

double CompassItem::CompassItem_ItemPropertyFunction::getFrameRotation(boost::shared_ptr<ItemFrame> frame)
{
    return (double)Mth::wrapDegrees(frame->getDirection()->get2DDataValue() * 90 + 180);
}

double CompassItem::CompassItem_ItemPropertyFunction::getSpawnToAngle(Level* level,
                                                                     boost::shared_ptr<Entity> entity)
{
    BlockPos spawn = level->getSharedSpawnPos();

    return std::atan2((double)spawn.z - entity->mZ, (double)spawn.x - entity->mX);
}

double CompassItem::CompassItem_ItemPropertyFunction::wobble(Level* level, double value)
{
    if (level->getGameTime() == mLastUpdate)
    {
        return mRotation;
    }

    mLastUpdate = level->getGameTime();

    double delta = std::fmod(value - mRotation, 6.2831854820251465);
    delta = Mth::clamp(delta, -1.0, 1.0);
    mWobble = (delta * 0.1 + mWobble) * 0.8;
    mRotation += mWobble;

    return mRotation;
}

float CompassItem::CompassItem_ItemPropertyFunction::call(not_null_ptr<ItemInstance> itemInstance,
                                                          Level* level,
                                                          boost::shared_ptr<LivingEntity> entity)
{
    boost::shared_ptr<Entity> source;

    if (entity == NULL)
    {
        if (!itemInstance->isFramed())
        {
            return 0.0f;
        }

        source = boost::dynamic_pointer_cast<Entity>(itemInstance->getFrame());
    }
    else
    {
        source = boost::dynamic_pointer_cast<Entity>(entity);
    }

    if (level == NULL)
    {
        level = source->mLevel;
    }

    double angle;

    if (!level->mDimension->isNaturalDimension())
    {
        angle = Math::random() * 6.2831854820251465;
    }
    else
    {
        double rotation;

        if (entity != NULL)
        {
            rotation = (double)source->mYRot;
        }
        else
        {
            rotation = getFrameRotation(boost::dynamic_pointer_cast<ItemFrame>(source));
        }

        rotation = std::fmod(rotation, 360.0);
        angle = 3.1415927410125732 -
            ((rotation - 90.0) * 0.01745329238474369 - getSpawnToAngle(level, source));
    }

    if (entity != NULL)
    {
        angle = wobble(level, angle);
    }

    return Mth::positiveModulo((float)(angle / 6.2831854820251465), 1.0f);
}
