#include "game/item/SpawnEggItem.hpp"

#include "game/core/GenericStats.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/L10N.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/EntityIO.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"

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

double SpawnEggItem::getYOffset(Level*, const BlockPos&)
{
    return 0.125;
}

bool SpawnEggItem::canSpawn(const ResourceLocation& entityType, Level* level, int* error)
{
    if (error != NULL)
    {
        *error = 0;
    }
    if (level == NULL || !EntityIO::isValidEntityType(entityType))
    {
        if (error != NULL)
        {
            *error = 1;
        }
        return false;
    }
    if (!level->canCreateMore(EntityIO::byString(entityType.toString()), 0))
    {
        if (error != NULL)
        {
            *error = 2;
        }
        return false;
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
    if (entity)
    {
        entity->setPos(x, y, z);
    }
    return boost::shared_ptr<Mob>();
}

void SpawnEggItem::spawnMobAt(Level* level, const ResourceLocation& entityType,
                              double x, double y, double z, int* error,
                              const std::wstring& customName)
{
    boost::shared_ptr<Mob> mob = getMob(level, entityType, x, y, z, error, customName);
    if (mob && level != NULL)
    {
        level->addEntity(boost::shared_ptr<Entity>());
    }
}

void SpawnEggItem::updateCustomEntityTag(Level*, boost::shared_ptr<Player>,
                                         not_null_ptr<ItemInstance>,
                                         boost::shared_ptr<Entity>)
{
}

void SpawnEggItem::DisplaySpawnError(boost::shared_ptr<Player>, int)
{
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
