#include "game/entity/mob/EntityIO.hpp"

#include <vector>

#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/ItemEntity.hpp"
#include "game/core/ExperienceOrb.hpp"
#include "game/item/AreaEffectCloud.hpp"
#include "game/core/ElderGuardian.hpp"
#include "game/entity/mob/WitherSkeleton.hpp"
#include "game/core/Stray.hpp"
#include "game/core/ThrownEgg.hpp"
#include "game/entity/mob/LeashFenceKnotEntity.hpp"
#include "game/core/Painting.hpp"
#include "game/entity/projectile/TippableArrow.hpp"
#include "game/core/Snowball.hpp"
#include "game/entity/projectile/LargeFireball.hpp"
#include "game/entity/projectile/SmallFireball.hpp"
#include "game/core/ThrownEnderpearl.hpp"
#include "game/core/EyeOfEnderSignal.hpp"
#include "game/item/ThrownPotion.hpp"
#include "game/core/ThrownExpBottle.hpp"
#include "game/item/ItemFrame.hpp"
#include "game/core/WitherSkull.hpp"
#include "game/core/PrimedTnt.hpp"
#include "game/world/block/FallingBlockEntity.hpp"
#include "game/entity/mob/FireworksRocketEntity.hpp"
#include "game/core/Husk.hpp"
#include "game/entity/projectile/SpectralArrow.hpp"
#include "game/core/ShulkerBullet.hpp"
#include "game/entity/projectile/DragonFireball.hpp"
#include "game/world/generation/ZombieVillager.hpp"
#include "game/entity/mob/SkeletonHorse.hpp"
#include "game/entity/mob/ZombieHorse.hpp"
#include "game/core/ArmorStand.hpp"
#include "game/core/Donkey.hpp"
#include "game/core/Mule.hpp"
#include "game/core/EvocationFangs.hpp"
#include "game/core/EvocationIllager.hpp"
#include "game/core/Vex.hpp"
#include "game/core/VindicationIllager.hpp"
#include "game/core/IllusionIllager.hpp"
#include "game/core/ThrownTrident.hpp"
#include "game/core/Dolphin.hpp"
#include "game/core/Boat.hpp"
#include "game/core/MinecartRideable.hpp"
#include "game/core/MinecartChest.hpp"
#include "game/core/MinecartFurnace.hpp"
#include "game/core/MinecartTNT.hpp"
#include "game/core/MinecartHopper.hpp"
#include "game/core/MinecartSpawner.hpp"
#include "game/core/Creeper.hpp"
#include "game/entity/mob/Skeleton.hpp"
#include "game/core/Spider.hpp"
#include "game/core/Giant.hpp"
#include "game/entity/mob/Zombie.hpp"
#include "game/core/Slime.hpp"
#include "game/core/Ghast.hpp"
#include "game/entity/mob/PigZombie.hpp"
#include "game/core/EnderMan.hpp"
#include "game/core/CaveSpider.hpp"
#include "game/core/Silverfish.hpp"
#include "game/core/Blaze.hpp"
#include "game/core/MagmaCube.hpp"
#include "game/entity/mob/EnderDragon.hpp"
#include "game/core/WitherBoss.hpp"
#include "game/core/Bat.hpp"
#include "game/core/Witch.hpp"
#include "game/core/Endermite.hpp"
#include "game/core/Guardian.hpp"
#include "game/core/Shulker.hpp"
#include "game/core/Pig.hpp"
#include "game/core/Sheep.hpp"
#include "game/core/Cow.hpp"
#include "game/core/Chicken.hpp"
#include "game/core/Squid.hpp"
#include "game/core/Wolf.hpp"
#include "game/core/MushroomCow.hpp"
#include "game/core/SnowMan.hpp"
#include "game/core/Ocelot.hpp"
#include "game/world/generation/VillagerGolem.hpp"
#include "game/entity/mob/Horse.hpp"
#include "game/util/Rabbit.hpp"
#include "game/core/PolarBear.hpp"
#include "game/core/Llama.hpp"
#include "game/core/LlamaSpit.hpp"
#include "game/core/Parrot.hpp"
#include "game/core/TropicalFish.hpp"
#include "game/core/Fish.hpp"
#include "game/core/Pufferfish.hpp"
#include "game/core/Salmon.hpp"
#include "game/core/Turtle.hpp"
#include "game/core/Drowned.hpp"
#include "game/core/Phantom.hpp"
#include "game/world/generation/Villager.hpp"
#include "game/core/EnderCrystal.hpp"

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
    template <typename T>
    Entity* createEntity(Level* level)
    {
        return new T(level);
    }

    template <>
    Entity* createEntity<Guardian>(Level* level)
    {
        return new Guardian(level, true);
    }

    template <>
    Entity* createEntity<Cow>(Level* level)
    {
        return new Cow(level, true);
    }

    template <>
    Entity* createEntity<Fish>(Level* level)
    {
        return new Fish(level, false);
    }
}

const unsigned long long entityTypeItem = 0x000000000800000AULL;
const unsigned long long entityTypeExperienceOrb = 0x0000000008000003ULL;

EntityIO::SpawnableMobInfo::SpawnableMobInfo(int infoId, const ResourceLocation& infoLocation,
                                              eMinecraftColour infoColour,
                                              eMinecraftColour infoSecondaryColour,
                                              eMinecraftIconOverride infoIcon)
    : id(infoId), location(infoLocation), colour(infoColour), secondaryColour(infoSecondaryColour),
      icon(infoIcon), type(0), flags(0)
{
}

EntityIO::SpawnableMobInfo::SpawnableMobInfo(const SpawnableMobInfo& other)
    : id(other.id), location(other.location), colour(other.colour), secondaryColour(other.secondaryColour),
      icon(other.icon), type(other.type), flags(other.flags)
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
        ? ResourceLocation(std::wstring(L"Player")).equals(&location)
        : (entity->GetType() == 0x000000000B000001ULL
           && ResourceLocation(std::wstring(L"LightningBolt")).equals(&location));
}

int EntityIO::getNameDisplayId(const std::wstring& name)
{
    ResourceLocation location(name);
    SpawnInfo::const_iterator it = sSpawnInfo.find(location);
    if (it == sSpawnInfo.end() || it->second == NULL)
    {
        return -1;
    }
    return it->second->id;
}

int EntityIO::eTypeToIoid(unsigned long long entityType)
{
    TypeIds::const_iterator it = sTypeIds.find(entityType);
    if (it == sTypeIds.end())
    {
        return -1;
    }
    return it->second;
}

EntityIO::SpawnableMobInfo EntityIO::addSpawnInfo(int id, std::wstring name,
                                                  eMinecraftColour colour,
                                                  eMinecraftColour secondaryColour,
                                                  eMinecraftIconOverride icon)
{
    ResourceLocation location(name);
    SpawnableMobInfo* info = new SpawnableMobInfo(id, location, colour, secondaryColour, icon);
    SpawnInfo::iterator old = sSpawnInfo.find(location);
    if (old != sSpawnInfo.end())
    {
        delete old->second;
    }
    sSpawnInfo[location] = info;
    return *info;
}

void EntityIO::Register(Creator creator, int id, const std::wstring& name,
                        unsigned long long entityType,
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
    Register(createEntity<ItemEntity>, 1, L"item", 0x800000AULL, L"Item");
    Register(createEntity<ExperienceOrb>, 2, L"xp_orb", 0x8000003ULL, L"XPOrb");
    Register(createEntity<AreaEffectCloud>, 3, L"area_effect_cloud", 0x800000FULL, L"AreaEffectCloud");
    Register(createEntity<ElderGuardian>, 4, L"elder_guardian", 0x5D544001ULL, L"ElderGuardian");
    Register(createEntity<WitherSkeleton>, 5, L"wither_skeleton", 0x5D540442ULL, L"WitherSkeleton");
    Register(createEntity<Stray>, 6, L"stray", 0x5D540443ULL, L"Stray");
    Register(createEntity<ThrownEgg>, 7, L"egg", 0x8A200002ULL, L"ThrownEgg");
    Register(createEntity<LeashFenceKnotEntity>, 8, L"leash_knot", 0xA080003ULL, L"LeashKnot");
    Register(createEntity<Painting>, 9, L"painting", 0xA080001ULL, L"Painting");
    Register(createEntity<TippableArrow>, 10, L"arrow", 0x8A020002ULL, L"Arrow");
    Register(createEntity<Snowball>, 11, L"snowball", 0x8A200001ULL, L"Snowball");
    Register(createEntity<LargeFireball>, 12, L"fireball", 0x8A400003ULL, L"Fireball");
    Register(createEntity<SmallFireball>, 13, L"small_fireball", 0x8A400004ULL, L"SmallFireball");
    Register(createEntity<ThrownEnderpearl>, 14, L"ender_pearl", 0x8A200003ULL, L"ThrownEnderpearl");
    Register(createEntity<EyeOfEnderSignal>, 15, L"eye_of_ender_signal", 0x88000004ULL, L"EyeOfEnderSignal");
    Register(createEntity<ThrownPotion>, 16, L"potion", 0x8A200004ULL, L"ThrownPotion");
    Register(createEntity<ThrownExpBottle>, 17, L"xp_bottle", 0x8A200005ULL, L"ThrownExpBottle");
    Register(createEntity<ItemFrame>, 18, L"item_frame", 0xA080002ULL, L"ItemFrame");
    Register(createEntity<WitherSkull>, 19, L"wither_skull", 0x8A400002ULL, L"WitherSkull");
    Register(createEntity<PrimedTnt>, 20, L"tnt", 0x800000BULL, L"PrimedTnt");
    Register(createEntity<FallingBlockEntity>, 21, L"falling_block", 0x8000009ULL, L"FallingSand");
    Register(createEntity<FireworksRocketEntity>, 22, L"fireworks_rocket", 0x88000005ULL, L"FireworksRocketEntity");
    Register(createEntity<Husk>, 23, L"husk", 0x5D550043ULL, L"Husk");
    Register(createEntity<SpectralArrow>, 24, L"spectral_arrow", 0x8A020001ULL, L"SpectralArrow");
    Register(createEntity<ShulkerBullet>, 25, L"shulker_bullet", 0x8000010ULL, L"ShulkerBullet");
    Register(createEntity<DragonFireball>, 26, L"dragon_fireball", 0x8A400001ULL, L"DragonFireball");
    Register(createEntity<ZombieVillager>, 27, L"zombie_villager", 0x5D550042ULL, L"ZombieVillager");
    Register(createEntity<SkeletonHorse>, 28, L"skeleton_horse", 0x2D690002ULL, L"SkeletonHorse");
    Register(createEntity<ZombieHorse>, 29, L"zombie_horse", 0x2D690003ULL, L"ZombieHorse");
    Register(createEntity<ArmorStand>, 30, L"armor_stand", 0xC000001ULL, L"ArmorStand");
    Register(createEntity<Donkey>, 31, L"donkey", 0x2D698001ULL, L"Donkey");
    Register(createEntity<Mule>, 32, L"mule", 0x2D698002ULL, L"Mule");
    Register(createEntity<EvocationFangs>, 33, L"evocation_fangs", 0x8000011ULL, L"EvocationFangs");
    Register(createEntity<EvocationIllager>, 34, L"evocation_illager", 0x5D541841ULL, L"EvocationIllager");
    Register(createEntity<Vex>, 35, L"vex", 0x5D540048ULL, L"Vex");
    Register(createEntity<VindicationIllager>, 36, L"vindication_illager", 0x5D541041ULL, L"VindicationIllager");
    Register(createEntity<IllusionIllager>, 37, L"illusion_illager", 0x5D541842ULL, L"IllusionIllager");
    Register(createEntity<ThrownTrident>, 38, L"trident", 0x8A200006ULL, L"Trident");
    Register(createEntity<Dolphin>, 39, L"dolphin", 0xD602001ULL, L"Dolphin");
    Register(createEntity<Boat>, 41, L"boat", 0x8000008ULL, L"Boat");
    Register(createEntity<MinecartRideable>, 42, L"minecart", 0xA800001ULL, Minecart::Type::RIDEABLE->getName());
    Register(createEntity<MinecartChest>, 43, L"chest_minecart", 0xA840002ULL, Minecart::Type::CHEST->getName());
    Register(createEntity<MinecartFurnace>, 44, L"furnace_minecart", 0xA800003ULL, Minecart::Type::FURNACE->getName());
    Register(createEntity<MinecartTNT>, 45, L"tnt_minecart", 0xA800004ULL, Minecart::Type::TNT->getName());
    Register(createEntity<MinecartHopper>, 46, L"hopper_minecart", 0xA840005ULL, Minecart::Type::HOPPER->getName());
    Register(createEntity<MinecartSpawner>, 47, L"spawner_minecart", 0xA800006ULL, Minecart::Type::SPAWNER->getName());
    Register(createEntity<Creeper>, 50, L"creeper", 0x5D540041ULL, L"Creeper");
    Register(createEntity<Skeleton>, 51, L"skeleton", 0x5D540441ULL, L"Skeleton");
    Register(createEntity<Spider>, 52, L"spider", 0x5D560040ULL, L"Spider");
    Register(createEntity<Giant>, 53, L"giant", 0x5D540042ULL, L"Giant");
    Register(createEntity<Zombie>, 54, L"zombie", 0x5D550040ULL, L"Zombie");
    Register(createEntity<Slime>, 55, L"slime", 0x5D108000ULL, L"Slime");
    Register(createEntity<Ghast>, 56, L"ghast", 0x5D0000C1ULL, L"Ghast");
    Register(createEntity<PigZombie>, 57, L"zombie_pigman", 0x5D550041ULL, L"PigZombie");
    Register(createEntity<EnderMan>, 58, L"enderman", 0x5D540043ULL, L"Enderman");
    Register(createEntity<CaveSpider>, 59, L"cave_spider", 0x5D560041ULL, L"CaveSpider");
    Register(createEntity<Silverfish>, 60, L"silverfish", 0x5D540044ULL, L"Silverfish");
    Register(createEntity<Blaze>, 61, L"blaze", 0x5D540045ULL, L"Blaze");
    Register(createEntity<MagmaCube>, 62, L"magma_cube", 0x5D108001ULL, L"LavaSlime");
    Register(createEntity<EnderDragon>, 63, L"ender_dragon", 0xD000005ULL, L"EnderDragon");
    Register(createEntity<WitherBoss>, 64, L"wither", 0x5D540047ULL, L"WitherBoss");
    Register(createEntity<Bat>, 65, L"bat", 0x1D000101ULL, L"Bat");
    Register(createEntity<Witch>, 66, L"witch", 0x5D540046ULL, L"Witch");
    Register(createEntity<Endermite>, 67, L"endermite", 0x4D54004BULL, L"Endermite");
    Register(createEntity<Guardian>, 68, L"guardian", 0x5D544000ULL, L"Guardian");
    Register(createEntity<Shulker>, 69, L"shulker", 0x4D580043ULL, L"Shulker");
    Register(createEntity<Pig>, 90, L"pig", 0x2D680003ULL, L"Pig");
    Register(createEntity<Sheep>, 91, L"sheep", 0x2D680002ULL, L"Sheep");
    Register(createEntity<Cow>, 92, L"cow", 0x2D6A0000ULL, L"Cow");
    Register(createEntity<Chicken>, 93, L"chicken", 0xD680004ULL, L"Chicken");
    Register(createEntity<Squid>, 94, L"squid", 0xD502001ULL, L"Squid");
    Register(createEntity<Wolf>, 95, L"wolf", 0xD6C0002ULL, L"Wolf");
    Register(createEntity<MushroomCow>, 96, L"mooshroom", 0x2D6A0001ULL, L"MushroomCow");
    Register(createEntity<SnowMan>, 97, L"snowman", 0x2D580001ULL, L"SnowMan");
    Register(createEntity<Ocelot>, 98, L"ocelot", 0x2D6C0001ULL, L"Ozelot");
    Register(createEntity<VillagerGolem>, 99, L"villager_golem", 0xD580002ULL, L"VillagerGolem");
    Register(createEntity<Horse>, 100, L"horse", 0x2D690001ULL, L"Horse");
    Register(createEntity<Rabbit>, 101, L"rabbit", 0x2D680005ULL, L"Rabbit");
    Register(createEntity<PolarBear>, 102, L"polar_bear", 0x2D680006ULL, L"PolarBear");
    Register(createEntity<Llama>, 103, L"llama", 0x2D698003ULL, L"Llama");
    Register(createEntity<LlamaSpit>, 104, L"llama_spit", 0x88000012ULL, L"LlamaSpit");
    Register(createEntity<Parrot>, 105, L"parrot", 0x2D6C0003ULL, L"Parrot");
    Register(createEntity<TropicalFish>, 106, L"tropicalfish", 0xD502005ULL, L"TropicalFish");
    Register(createEntity<Fish>, 107, L"fish", 0xD502002ULL, L"Fish");
    Register(createEntity<Pufferfish>, 108, L"pufferfish", 0xD502003ULL, L"Pufferfish");
    Register(createEntity<Salmon>, 109, L"salmon", 0xD502004ULL, L"Salmon");
    Register(createEntity<Turtle>, 110, L"turtle", 0x2D680007ULL, L"Turtle");
    Register(createEntity<Drowned>, 111, L"drowned", 0x5D550044ULL, L"Drowned");
    Register(createEntity<Phantom>, 112, L"phantom", 0x5D540049ULL, L"Phantom");
    Register(createEntity<Villager>, 120, L"villager", 0xD600001ULL, L"Villager");
    Register(createEntity<EnderCrystal>, 200, L"ender_crystal", 0x800000EULL, L"EnderCrystal");

    addSpawnInfo(static_cast<int>(0xD3E3ACFBU), L"bat",
                 static_cast<eMinecraftColour>(326), static_cast<eMinecraftColour>(327),
                 static_cast<eMinecraftIconOverride>(0));
    addSpawnInfo(static_cast<int>(0xE6EDF96CU), L"blaze",
                 static_cast<eMinecraftColour>(304), static_cast<eMinecraftColour>(305),
                 static_cast<eMinecraftIconOverride>(0));
    addSpawnInfo(static_cast<int>(0x11C0F98BU), L"cave_spider",
                 static_cast<eMinecraftColour>(300), static_cast<eMinecraftColour>(301),
                 static_cast<eMinecraftIconOverride>(0));
    addSpawnInfo(static_cast<int>(0xBF2E68C5U), L"chicken",
                 static_cast<eMinecraftColour>(314), static_cast<eMinecraftColour>(315),
                 static_cast<eMinecraftIconOverride>(0));
    addSpawnInfo(static_cast<int>(0x32A43497U), L"cow",
                 static_cast<eMinecraftColour>(312), static_cast<eMinecraftColour>(313),
                 static_cast<eMinecraftIconOverride>(0));
    addSpawnInfo(static_cast<int>(0x2F0DAD1AU), L"creeper",
                 static_cast<eMinecraftColour>(284), static_cast<eMinecraftColour>(285),
                 static_cast<eMinecraftIconOverride>(0));
    addSpawnInfo(static_cast<int>(0x47859436U), L"donkey",
                 static_cast<eMinecraftColour>(348), static_cast<eMinecraftColour>(349),
                 static_cast<eMinecraftIconOverride>(0));
    addSpawnInfo(static_cast<int>(0xF5FCCE7AU), L"elder_guardian",
                 static_cast<eMinecraftColour>(342), static_cast<eMinecraftColour>(343),
                 static_cast<eMinecraftIconOverride>(0));
    addSpawnInfo(static_cast<int>(0x801001D6U), L"enderman",
                 static_cast<eMinecraftColour>(298), static_cast<eMinecraftColour>(299),
                 static_cast<eMinecraftIconOverride>(0));
    addSpawnInfo(static_cast<int>(0xE774D679U), L"endermite",
                 static_cast<eMinecraftColour>(332), static_cast<eMinecraftColour>(333),
                 static_cast<eMinecraftIconOverride>(0));
    addSpawnInfo(static_cast<int>(0xD9AA9869U), L"evocation_illager",
                 static_cast<eMinecraftColour>(344), static_cast<eMinecraftColour>(345),
                 static_cast<eMinecraftIconOverride>(0));
    addSpawnInfo(static_cast<int>(0x71DF0C85U), L"ghast",
                 static_cast<eMinecraftColour>(294), static_cast<eMinecraftColour>(295),
                 static_cast<eMinecraftIconOverride>(0));
    addSpawnInfo(static_cast<int>(0x7FAA806FU), L"guardian",
                 static_cast<eMinecraftColour>(334), static_cast<eMinecraftColour>(335),
                 static_cast<eMinecraftIconOverride>(0));
    addSpawnInfo(static_cast<int>(0xEE23D9B7U), L"horse",
                 static_cast<eMinecraftColour>(330), static_cast<eMinecraftColour>(331),
                 static_cast<eMinecraftIconOverride>(0));
    addSpawnInfo(static_cast<int>(0xD84778B9U), L"husk",
                 static_cast<eMinecraftColour>(358), static_cast<eMinecraftColour>(359),
                 static_cast<eMinecraftIconOverride>(0));
    addSpawnInfo(static_cast<int>(0xC5D9DA21U), L"llama",
                 static_cast<eMinecraftColour>(346), static_cast<eMinecraftColour>(347),
                 static_cast<eMinecraftIconOverride>(0));
    addSpawnInfo(static_cast<int>(0x9E8D900FU), L"magma_cube",
                 static_cast<eMinecraftColour>(306), static_cast<eMinecraftColour>(307),
                 static_cast<eMinecraftIconOverride>(0));
    addSpawnInfo(static_cast<int>(0x2775807CU), L"mooshroom",
                 static_cast<eMinecraftColour>(320), static_cast<eMinecraftColour>(321),
                 static_cast<eMinecraftIconOverride>(0));
    addSpawnInfo(static_cast<int>(0x98B4F19DU), L"mule",
                 static_cast<eMinecraftColour>(354), static_cast<eMinecraftColour>(355),
                 static_cast<eMinecraftIconOverride>(0));
    addSpawnInfo(static_cast<int>(0xAA44AD8BU), L"ocelot",
                 static_cast<eMinecraftColour>(322), static_cast<eMinecraftColour>(323),
                 static_cast<eMinecraftIconOverride>(0));
    addSpawnInfo(static_cast<int>(0x92734C7EU), L"parrot",
                 static_cast<eMinecraftColour>(366), static_cast<eMinecraftColour>(367),
                 static_cast<eMinecraftIconOverride>(0));
    addSpawnInfo(static_cast<int>(0x8F3FDC8AU), L"pig",
                 static_cast<eMinecraftColour>(308), static_cast<eMinecraftColour>(309),
                 static_cast<eMinecraftIconOverride>(0));
    addSpawnInfo(static_cast<int>(0xE4B7D800U), L"polar_bear",
                 static_cast<eMinecraftColour>(338), static_cast<eMinecraftColour>(339),
                 static_cast<eMinecraftIconOverride>(0));
    addSpawnInfo(static_cast<int>(0x5F31CCFAU), L"rabbit",
                 static_cast<eMinecraftColour>(336), static_cast<eMinecraftColour>(337),
                 static_cast<eMinecraftIconOverride>(0));
    addSpawnInfo(static_cast<int>(0x38EEFD87U), L"sheep",
                 static_cast<eMinecraftColour>(310), static_cast<eMinecraftColour>(311),
                 static_cast<eMinecraftIconOverride>(0));
    addSpawnInfo(static_cast<int>(0xD69F89AAU), L"shulker",
                 static_cast<eMinecraftColour>(340), static_cast<eMinecraftColour>(341),
                 static_cast<eMinecraftIconOverride>(0));
    addSpawnInfo(static_cast<int>(0xAEEE3B2FU), L"silverfish",
                 static_cast<eMinecraftColour>(302), static_cast<eMinecraftColour>(303),
                 static_cast<eMinecraftIconOverride>(0));
    addSpawnInfo(static_cast<int>(0x3F15B435U), L"skeleton",
                 static_cast<eMinecraftColour>(286), static_cast<eMinecraftColour>(287),
                 static_cast<eMinecraftIconOverride>(0));
    addSpawnInfo(static_cast<int>(0x7E8FE011U), L"skeleton_horse",
                 static_cast<eMinecraftColour>(350), static_cast<eMinecraftColour>(351),
                 static_cast<eMinecraftIconOverride>(0));
    addSpawnInfo(static_cast<int>(0x75D1B12AU), L"slime",
                 static_cast<eMinecraftColour>(292), static_cast<eMinecraftColour>(293),
                 static_cast<eMinecraftIconOverride>(0));
    addSpawnInfo(static_cast<int>(0x89E834A5U), L"spider",
                 static_cast<eMinecraftColour>(288), static_cast<eMinecraftColour>(289),
                 static_cast<eMinecraftIconOverride>(0));
    addSpawnInfo(static_cast<int>(0xB7460FF6U), L"squid",
                 static_cast<eMinecraftColour>(316), static_cast<eMinecraftColour>(317),
                 static_cast<eMinecraftIconOverride>(0));
    addSpawnInfo(static_cast<int>(0xC9DE5707U), L"stray",
                 static_cast<eMinecraftColour>(356), static_cast<eMinecraftColour>(357),
                 static_cast<eMinecraftIconOverride>(0));
    addSpawnInfo(static_cast<int>(0x62CBA372U), L"turtle",
                 static_cast<eMinecraftColour>(370), static_cast<eMinecraftColour>(371),
                 static_cast<eMinecraftIconOverride>(0));
    addSpawnInfo(static_cast<int>(0xC0A08C84U), L"dolphin",
                 static_cast<eMinecraftColour>(372), static_cast<eMinecraftColour>(373),
                 static_cast<eMinecraftIconOverride>(0));
    addSpawnInfo(static_cast<int>(0x710952F7U), L"vex",
                 static_cast<eMinecraftColour>(360), static_cast<eMinecraftColour>(361),
                 static_cast<eMinecraftIconOverride>(0));
    addSpawnInfo(static_cast<int>(0x6FA8F932U), L"villager",
                 static_cast<eMinecraftColour>(324), static_cast<eMinecraftColour>(325),
                 static_cast<eMinecraftIconOverride>(0));
    addSpawnInfo(static_cast<int>(0xF3D4F291U), L"vindication_illager",
                 static_cast<eMinecraftColour>(362), static_cast<eMinecraftColour>(363),
                 static_cast<eMinecraftIconOverride>(0));
    addSpawnInfo(static_cast<int>(0x8AAD2185U), L"witch",
                 static_cast<eMinecraftColour>(328), static_cast<eMinecraftColour>(329),
                 static_cast<eMinecraftIconOverride>(0));
    addSpawnInfo(static_cast<int>(0x1CFE3457U), L"wither_skeleton",
                 static_cast<eMinecraftColour>(368), static_cast<eMinecraftColour>(369),
                 static_cast<eMinecraftIconOverride>(0));
    addSpawnInfo(static_cast<int>(0x8A3B6226U), L"wolf",
                 static_cast<eMinecraftColour>(318), static_cast<eMinecraftColour>(319),
                 static_cast<eMinecraftIconOverride>(0));
    addSpawnInfo(static_cast<int>(0xDDD48F6AU), L"zombie",
                 static_cast<eMinecraftColour>(290), static_cast<eMinecraftColour>(291),
                 static_cast<eMinecraftIconOverride>(0));
    addSpawnInfo(static_cast<int>(0x2E0607AEU), L"zombie_horse",
                 static_cast<eMinecraftColour>(352), static_cast<eMinecraftColour>(353),
                 static_cast<eMinecraftIconOverride>(0));
    addSpawnInfo(static_cast<int>(0xF566D3CCU), L"zombie_pigman",
                 static_cast<eMinecraftColour>(296), static_cast<eMinecraftColour>(297),
                 static_cast<eMinecraftIconOverride>(0));
    addSpawnInfo(static_cast<int>(0x33791323U), L"zombie_villager",
                 static_cast<eMinecraftColour>(364), static_cast<eMinecraftColour>(365),
                 static_cast<eMinecraftIconOverride>(0));
    addSpawnInfo(static_cast<int>(0xD8197146U), L"tropicalfish",
                 static_cast<eMinecraftColour>(374), static_cast<eMinecraftColour>(375),
                 static_cast<eMinecraftIconOverride>(0));
    addSpawnInfo(static_cast<int>(0x60751ECCU), L"fish",
                 static_cast<eMinecraftColour>(376), static_cast<eMinecraftColour>(377),
                 static_cast<eMinecraftIconOverride>(0));
    addSpawnInfo(static_cast<int>(0xC761AE7AU), L"pufferfish",
                 static_cast<eMinecraftColour>(378), static_cast<eMinecraftColour>(379),
                 static_cast<eMinecraftIconOverride>(0));
    addSpawnInfo(static_cast<int>(0xAAFBA9FEU), L"salmon",
                 static_cast<eMinecraftColour>(380), static_cast<eMinecraftColour>(381),
                 static_cast<eMinecraftIconOverride>(0));
    addSpawnInfo(static_cast<int>(0x830F5655U), L"drowned",
                 static_cast<eMinecraftColour>(382), static_cast<eMinecraftColour>(383),
                 static_cast<eMinecraftIconOverride>(0));
    addSpawnInfo(static_cast<int>(0xF845C93DU), L"phantom",
                 static_cast<eMinecraftColour>(384), static_cast<eMinecraftColour>(385),
                 static_cast<eMinecraftIconOverride>(0));
    sEntityTypes.insert(ResourceLocation(std::wstring(L"LightningBolt")));
}
