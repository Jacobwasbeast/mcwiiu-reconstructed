#include "game/item/SpawnEggItem.hpp"

#include "game/core/Difficulty.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/L10N.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/EntityIO.hpp"
#include "game/util/AABB.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/Player.hpp"

SpawnEggItem::SpawnEggItem()
    : Item(), mOverlayIcon(NULL)
{
    mMaxStackSize = 64;
    mStackedByData = true;
}

SpawnEggItem::~SpawnEggItem()
{
}

ResourceLocation SpawnEggItem::getNamedIdFrom(not_null_ptr<ItemInstance> itemInstance)
{
    CompoundTag* tag = itemInstance->getTag();
    if (tag == NULL || !tag->contains(L"EntityTag", 10))
    {
        return ResourceLocation();
    }

    CompoundTag* entityTag = tag->getCompound(L"EntityTag");
    if (entityTag != NULL && entityTag->contains(L"id", 8))
    {
        ResourceLocation result(entityTag->getString(L"id"));
        if (result.toString().find(L':') == std::wstring::npos)
        {
            entityTag->putString(L"id", result.toString());
        }
        return result;
    }
    return ResourceLocation();
}

std::wstring SpawnEggItem::getName(not_null_ptr<ItemInstance> itemInstance)
{
    ResourceLocation entityType = getNamedIdFrom(itemInstance);
    int displayId = EntityIO::getNameDisplayId(entityType.toString());
    std::wstring name = getDescription();
    if (displayId == -1 || displayId == -2)
    {
        return L"{*CREATURE*}";
    }
    std::wstring creature = L"{*CREATURE*}";
    std::wstring::size_type position = name.find(creature);
    if (position != std::wstring::npos)
    {
        name.replace(position, creature.size(), L10N::GetString(displayId));
    }
    return name;
}

double SpawnEggItem::getYOffset(Level* level, const BlockPos& pos)
{
    double yOffset = 0.0;
    AABB* box = AABB::newPermanent(pos)->expand(0.0, -1.0, 0.0);
    boost::shared_ptr<Entity> emptyEntity;
    std::vector<AABB*>* collisions =
        level->getCollisionAABBs(emptyEntity, box, false, false, false);
    if (collisions != NULL && !collisions->empty())
    {
        double maxY = box->mMinY;
        for (std::vector<AABB*>::iterator it = collisions->begin(); it != collisions->end(); ++it)
        {
            AABB* collision = *it;
            if (collision->mMaxY > maxY)
            {
                maxY = collision->mMaxY;
            }
        }
        yOffset = maxY - (double)pos.y;
    }
    return yOffset;
}

bool SpawnEggItem::canSpawn(const ResourceLocation& entityType, Level* level, int* error)
{
    if (error != NULL)
    {
        *error = 0;
    }
    if (level == NULL)
    {
        return false;
    }

    boost::shared_ptr<Entity> entity = EntityIO::newEntity(entityType, level);
    if (!entity)
    {
        if (error != NULL)
        {
            *error = 1;
        }
        return false;
    }

    const unsigned long long type = entity->GetType();
    const bool peaceful = level->getDifficulty() == Difficulty::PEACEFUL;

    if (type == 0x000000005D540049ULL)
    {
        if (peaceful)
        {
            if (error != NULL)
            {
                *error = 12;
            }
            return false;
        }
        if (!level->canCreateMore(type, 0))
        {
            if (error != NULL)
            {
                *error = 10;
            }
            return false;
        }
        return true;
    }
    if (type == ENTITY_TYPE_DOLPHIN)
    {
        if (!level->canCreateMore(type, 0))
        {
            if (error != NULL)
            {
                *error = 9;
            }
            return false;
        }
        return true;
    }
    if (type == ENTITY_TYPE_SHULKER)
    {
        if (!level->canCreateMore(type, 0))
        {
            if (error != NULL)
            {
                *error = 7;
            }
            return false;
        }
        return true;
    }
    if (type == 0x000000001D000101ULL)
    {
        if (!level->canCreateMore(type, 0))
        {
            if (error != NULL)
            {
                *error = 3;
            }
            return false;
        }
        return true;
    }
    if (type == 0x000000002D6A0001ULL)
    {
        if (!level->canCreateMore(type, 0))
        {
            if (error != NULL)
            {
                *error = 5;
            }
            return false;
        }
        return true;
    }
    if (type == ENTITY_TYPE_VILLAGER)
    {
        if (!level->canCreateMore(type, 0))
        {
            if (error != NULL)
            {
                *error = 8;
            }
            return false;
        }
        return true;
    }
    if (type == 0x000000000D6C0002ULL)
    {
        if (!level->canCreateMore(type, 0))
        {
            if (error != NULL)
            {
                *error = 4;
            }
            return false;
        }
        return true;
    }
    if (type == 0x000000000D680004ULL)
    {
        if (!level->canCreateMore(type, 0))
        {
            if (error != NULL)
            {
                *error = 2;
            }
            return false;
        }
        return true;
    }
    if ((type & 0x000000000D502000ULL) == 0x000000000D502000ULL)
    {
        if ((type & 0x40000000ULL) != 0 && type != ENTITY_TYPE_SHULKER && peaceful)
        {
            if (error != NULL)
            {
                *error = 12;
            }
            return false;
        }
        if (!level->canCreateMore(type, 0))
        {
            if (error != NULL)
            {
                *error = 11;
            }
            return false;
        }
        return true;
    }
    if ((type & 0x20000000ULL) == 0x20000000ULL)
    {
        if ((type & 0x40000000ULL) != 0 && type != ENTITY_TYPE_SHULKER && peaceful)
        {
            if (error != NULL)
            {
                *error = 12;
            }
            return false;
        }
        if (!level->canCreateMore(type, 0))
        {
            if (error != NULL)
            {
                *error = 1;
            }
            return false;
        }
        return true;
    }
    if ((type & 0x40000000ULL) == 0x40000000ULL)
    {
        if (peaceful)
        {
            if (error != NULL)
            {
                *error = 12;
            }
            return false;
        }
        if (!level->canCreateMore(type, 0))
        {
            if (error != NULL)
            {
                *error = 2;
            }
            return false;
        }
        return true;
    }

    return true;
}

boost::shared_ptr<Mob> SpawnEggItem::getMob(Level* level, const ResourceLocation& entityType,
                                             double x, double y, double z, int* error,
                                             const std::wstring& customName)
{
    if (!canSpawn(entityType, level, error))
    {
        return boost::shared_ptr<Mob>();
    }
    boost::shared_ptr<Entity> entity = EntityIO::newEntity(entityType, level);
    boost::shared_ptr<Mob> mob = boost::dynamic_pointer_cast<Mob>(entity);
    if (mob)
    {
        mob->setPos(x, y, z);
    }
    return mob;
}

boost::shared_ptr<Mob> SpawnEggItem::spawnMobAt(Level* level, const ResourceLocation& entityType,
                                                double x, double y, double z, int* error,
                                                const std::wstring& customName)
{
    boost::shared_ptr<Mob> mob = getMob(level, entityType, x, y, z, error, customName);
    if (mob && (mob->GetType() & 0x0D000000ULL) == 0x0D000000ULL)
    {
        level->addEntity(mob);
        mob->playAmbientSound();
    }
    return mob;
}

void SpawnEggItem::updateCustomEntityTag(Level*, boost::shared_ptr<Player>,
                                         not_null_ptr<ItemInstance>,
                                         boost::shared_ptr<Entity>)
{
}

void SpawnEggItem::DisplaySpawnError(boost::shared_ptr<Player> player, int error)
{
    if (!player)
        return;

    uint32_t message = 0;
    switch (error)
    {
    case 1: message = 0x9f8bffae; break;
    case 2: message = 0x82460728; break;
    case 3: message = 0xfab1deb6; break;
    case 4: message = 0x70edc806; break;
    case 5: message = 0x3e35b6e8; break;
    case 7: message = 0xa82b5a66; break;
    case 8: message = 0x16972fbf; break;
    case 9: message = 0xb714e0e2; break;
    case 10: message = 0x3dcd926b; break;
    case 11: message = 0x60a3835d; break;
    case 12: message = 0xd97fdba2; break;
    default: return;
    }

    player->sendMessage(message, NULL, 0, NULL, 0);
}

InteractionResult::EInteractionResult SpawnEggItem::useOn(boost::shared_ptr<Player> player,
                       Level* level, const BlockPos& pos,
                       InteractionHand::EInteractionHand hand, const Direction* direction,
                       float hitX, float hitY, float hitZ, bool simulate)
{
    return Item::useOn(player, level, pos, hand, direction, hitX, hitY, hitZ, simulate);
}

InteractionResultHolder SpawnEggItem::use(Level* level, boost::shared_ptr<Player> player,
                                           InteractionHand::EInteractionHand hand)
{
    return Item::use(level, player, hand);
}

void SpawnEggItem::addSpawnEntity(not_null_ptr<ItemInstance> itemInstance,
                                  const ResourceLocation& entityType)
{
    CompoundTag* tag = itemInstance->getTag();
    if (tag == NULL)
    {
        tag = new CompoundTag();
        itemInstance->setTag(tag);
    }
    CompoundTag* entityTag = new CompoundTag();
    entityTag->putString(L"id", entityType.toString());
    tag->put(L"EntityTag", entityTag);
}

bool SpawnEggItem::hasMultipleSpriteLayers()
{
    return true;
}

TextureAtlasSprite* SpawnEggItem::getLayerIcon(int auxValue, int layer,
                                               not_null_ptr<ItemInstance>)
{
    if (layer < 1)
    {
        return Item::getLayerIcon(auxValue, 0, ItemInstance::EMPTY);
    }
    return mOverlayIcon;
}

void SpawnEggItem::registerIcons(IconRegister* iconRegister)
{
    Item::registerIcons(iconRegister);
    mOverlayIcon = iconRegister->registerIcon(getIconName() + L"_overlay");
}

unsigned long long SpawnEggItem::fjGetType(const not_null_ptr<ItemInstance>& itemInstance)
{
    return EntityIO::byString(getNamedIdFrom(itemInstance).toString());
}

bool SpawnEggItem::TestUse(Level* level, boost::shared_ptr<Player> player,
                           InteractionHand::EInteractionHand hand)
{
    return Item::TestUse(level, player, hand);
}

unsigned int SpawnEggItem::GetUseTooltip(const ItemTooltipDataHolder& tooltipData)
{
    return TestUse(NULL, boost::shared_ptr<Player>(), InteractionHand::eInteractionHand_MainHand)
        ? 0x4c36cd96u : 0xffffffffu;
}
