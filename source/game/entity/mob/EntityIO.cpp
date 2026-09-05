#include "game/entity/mob/EntityIO.hpp"

#include <vector>

#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

#include "game/data/CompoundTag.hpp"

namespace
{
    typedef boost::unordered_map<unsigned long long, ResourceLocation> TypeKeys;
    typedef boost::unordered_map<ResourceLocation, unsigned long long> KeysByType;
    typedef boost::unordered_map<unsigned long long, EntityIO::Creator> Creators;
    typedef boost::unordered_map<ResourceLocation, EntityIO::SpawnableMobInfo*> SpawnInfo;
    typedef boost::unordered_map<unsigned long long, int> TypeIds;

    TypeKeys sTypeKeys;
    KeysByType sKeysByType;
    Creators sCreators;
    SpawnInfo sSpawnInfo;
    boost::unordered_set<ResourceLocation> sEntityTypes;
    std::vector<std::wstring> sTranslationNames;
    std::vector<unsigned long long> sTypesById;
    TypeIds sTypeIds;
    EntityIO sEntityIO;
}

const unsigned long long entityTypeItem = 0x000000000800000AULL;
const unsigned long long entityTypeExperienceOrb = 0x0000000008000003ULL;

EntityIO::SpawnableMobInfo::SpawnableMobInfo(int infoId, const ResourceLocation& infoLocation,
                                              eMinecraftColour infoColour,
                                              eMinecraftIconOverride infoIcon, int infoType)
    : id(infoId), location(infoLocation), colour(infoColour), icon(infoIcon), type(infoType),
      nameDisplayId(0), flags(0)
{
}

EntityIO::SpawnableMobInfo::SpawnableMobInfo(const SpawnableMobInfo& other)
    : id(other.id), location(other.location), colour(other.colour), icon(other.icon),
      type(other.type), nameDisplayId(other.nameDisplayId), flags(other.flags)
{
}

ResourceLocation EntityIO::getKey(unsigned long long entityType)
{
    TypeKeys::const_iterator it = sTypeKeys.find(entityType);
    if (it == sTypeKeys.end())
    {
        return ResourceLocation();
    }
    return it->second;
}

boost::shared_ptr<Entity> EntityIO::newEntity(unsigned long long entityType, Level* level)
{
    if ((entityType & ENTITY_TYPE_ENTITY) != ENTITY_TYPE_ENTITY)
    {
        return boost::shared_ptr<Entity>();
    }

    Creators::const_iterator it = sCreators.find(entityType);
    if (it == sCreators.end() || it->second == NULL)
    {
        return boost::shared_ptr<Entity>();
    }

    return boost::shared_ptr<Entity>((it->second)(level));
}

ResourceLocation EntityIO::getKey(const boost::shared_ptr<Entity>& entity)
{
    if (!entity)
    {
        return ResourceLocation();
    }
    return getKey(entity->GetType());
}

std::wstring EntityIO::getTranslationName(boost::shared_ptr<Entity> entity)
{
    unsigned long long type = entity->GetType();
    TypeIds::const_iterator found = sTypeIds.find(type);
    if (found != sTypeIds.end() && found->second >= 0 && found->second < (int)sTranslationNames.size())
    {
        return sTranslationNames[found->second];
    }
    return std::wstring(L"boost::detail");
}

unsigned long long EntityIO::byId(int id)
{
    if (id >= 0 && id < (int)sTypesById.size())
    {
        return sTypesById[id];
    }
    return 0;
}

unsigned long long EntityIO::byString(const std::wstring& name)
{
    ResourceLocation location(name);
    return sKeysByType.at(location);
}

boost::shared_ptr<Entity> EntityIO::newEntity(int id, Level* level)
{
    return newEntity(byId(id), level);
}

boost::shared_ptr<Entity> EntityIO::newEntity(const ResourceLocation& location, Level* level)
{
    return newEntity(sKeysByType.at(location), level);
}

boost::unordered_set<ResourceLocation>* EntityIO::getAllEntityTypes()
{
    return &sEntityTypes;
}

bool EntityIO::isValidEntityType(const ResourceLocation& location)
{
    ResourceLocation defaultLocation;
    return location.equals(&defaultLocation) || sEntityTypes.find(location) != sEntityTypes.end();
}

boost::shared_ptr<Entity> EntityIO::newEntity(CompoundTag* tag, Level* level)
{
    if (tag == NULL)
    {
        return boost::shared_ptr<Entity>();
    }

    std::wstring name = tag->getString(L"id");
    ResourceLocation location(name);
    if (!isValidEntityType(location))
    {
        return boost::shared_ptr<Entity>();
    }

    if (location.getPath() == L"Minecart" && tag->contains(L"Type"))
    {
        static const wchar_t* names[] = { L"Minecart", L"minecraft:chest_minecart", L"minecraft:furnace_minecart", L"minecraft:tnt_minecart", L"minecraft:hopper_minecart", L"minecraft:spawner_minecart" };
        int type = tag->getInt(L"Type");
        if (type >= 0 && type < 6)
        {
            tag->putString(L"id", names[type]);
            tag->remove(L"Type");
            location = ResourceLocation(tag->getString(L"id"));
        }
    }

    boost::shared_ptr<Entity> entity = newEntity(location, level);
    if (entity)
    {
        entity->load(tag);
    }
    return entity;
}

bool EntityIO::isEntityOfType(boost::shared_ptr<Entity> entity, const ResourceLocation& location)
{
    if (!entity)
    {
        return false;
    }

    ResourceLocation key = getKey(entity->GetType());
    if (!key.isDefault())
    {
        return key.equals(&location);
    }
    return (entity->GetType() & ENTITY_TYPE_PLAYER) == ENTITY_TYPE_PLAYER
        ? ResourceLocation(std::wstring(L"minecraft:player")).equals(&location)
        : (entity->GetType() == 0x000000000B000001ULL
           && ResourceLocation(std::wstring(L"minecraft:item")).equals(&location));
}

int EntityIO::getNameDisplayId(const std::wstring& name)
{
    ResourceLocation location(name);
    SpawnInfo::const_iterator it = sSpawnInfo.find(location);
    if (it == sSpawnInfo.end() || it->second == NULL)
    {
        return -1;
    }
    return it->second->nameDisplayId;
}

void EntityIO::addSpawnInfo(int id, std::wstring name, eMinecraftColour colour,
                            eMinecraftIconOverride icon)
{
    ResourceLocation location(name);
    SpawnableMobInfo* info = new SpawnableMobInfo(id, location, colour, icon);
    SpawnInfo::iterator old = sSpawnInfo.find(location);
    if (old != sSpawnInfo.end())
    {
        delete old->second;
    }
    sSpawnInfo[location] = info;
}

void EntityIO::Register(Creator creator, int id, const std::wstring& name,
                        unsigned long long entityType,
                        unsigned long long parentType,
                        const std::wstring& translationName)
{
    ResourceLocation location(name);
    sKeysByType[location] = entityType;
    sTypeKeys[entityType] = location;
    sCreators[entityType] = creator;
    sEntityTypes.insert(location);
    while ((int)sTypesById.size() <= id)
    {
        sTypesById.push_back(0);
        sTranslationNames.push_back(std::wstring());
    }
    sTypesById[id] = entityType;
    sTranslationNames[id] = translationName;
    sTypeIds[entityType] = id;
}

void EntityIO::bootStrap()
{
    if (!sEntityTypes.empty())
    {
        return;
    }
    sEntityIO.Register(NULL, 0, L"Item", entityTypeItem, ENTITY_TYPE_ENTITY, L"Item");
    sEntityIO.Register(NULL, 1, L"XPOrb", entityTypeExperienceOrb, ENTITY_TYPE_ENTITY, L"ExperienceOrb");
    sEntityIO.Register(NULL, 2, L"AreaEffectCloud", 0x000000000C000002ULL, ENTITY_TYPE_ENTITY, L"AreaEffectCloud");
    sEntityIO.Register(NULL, 3, L"ElderGuardian", 0x000000005D540044ULL, ENTITY_TYPE_MONSTER, L"ElderGuardian");
    sEntityIO.Register(NULL, 4, L"WitherSkeleton", 0x000000005D540045ULL, ENTITY_TYPE_MONSTER, L"WitherSkeleton");
    sEntityIO.Register(NULL, 5, L"Stray", 0x000000005D540043ULL, ENTITY_TYPE_MONSTER, L"Stray");
}
