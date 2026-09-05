#include "game/item/CompassItem.hpp"

#include <cmath>

#include <boost/shared_ptr.hpp>

#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/Math.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/Level.hpp"

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
    if (minecraft != NULL && minecraft->mLevel != NULL && auxValue == 0)
    {
        icon = mIcons[0];
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
    Entity* entity = (Entity*)frame.get();
    const Direction* direction = entity->getDirection();
    int rotation = Mth::wrapDegrees(direction->get2DDataValue() * 90 + 180);
    return (double)rotation;
}

double CompassItem::CompassItem_ItemPropertyFunction::getSpawnToAngle(Level* level,
                                                                        boost::shared_ptr<Entity> entity)
{
    BlockPos spawn = level->getSharedSpawnPos();
    return std::atan2((double)spawn.z - entity->mZ,
                      (double)spawn.x - entity->mX);
}

double CompassItem::CompassItem_ItemPropertyFunction::wobble(Level* level, double value)
{
    long long gameTime = level->getGameTime();
    if (gameTime == mLastUpdate)
        return mRotation;

    mLastUpdate = level->getGameTime();
    double delta = std::fmod(value - mRotation, 6.2831854820251465);
    delta = Mth::clamp(delta, -1.0, 1.0);
    mWobble = (delta * 0.1 + mWobble) * 0.8;
    mRotation += mWobble;
    return mRotation;
}

double CompassItem::CompassItem_ItemPropertyFunction::call(not_null_ptr<ItemInstance> itemInstance,
                                                            Level* level,
                                                            boost::shared_ptr<LivingEntity> entity)
{
    if (entity == NULL && !itemInstance->isFramed())
        return 0.0f;

    Entity* source = (Entity*)entity.get();
    boost::shared_ptr<ItemFrame> frame;
    bool framed = false;
    if (source == NULL)
    {
        frame = itemInstance->getFrame();
        source = (Entity*)frame.get();
        framed = true;
    }

    if (level == NULL)
        return 0.0f;

    double angle;
    bool natural = level->isDay();
    if (!natural)
    {
        angle = Math::random() * 6.2831854820251465;
    }
    else
    {
        if (framed)
            angle = getFrameRotation(frame);
        else
            angle = (double)source->mYRot;
        angle = std::fmod(angle, 360.0);
        double spawnAngle;
        if (framed)
        {
            spawnAngle = std::atan2((double)level->getSharedSpawnPos().z - source->mZ,
                                    (double)level->getSharedSpawnPos().x - source->mX);
        }
        else
        {
            spawnAngle = getSpawnToAngle(level, boost::static_pointer_cast<Entity>(entity));
        }
        angle = 3.1415927410125732 - ((angle - 90.0) * 0.01745329238474369 - spawnAngle);
    }

    if (!framed)
        angle = wobble(level, angle);

    return Mth::positiveModulo((float)(angle / 6.2831854820251465), 1.0f);
}

float CompassItem::CompassItem_ItemPropertyFunction::call(not_null_ptr<ItemInstance> itemInstance,
                                                           Level* level,
                                                           boost::shared_ptr<LivingEntity> entity) const
{
    return const_cast<CompassItem_ItemPropertyFunction*>(this)->call(itemInstance, level, entity);
}
