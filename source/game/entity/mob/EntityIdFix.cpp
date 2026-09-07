#include "game/entity/mob/EntityIdFix.hpp"

#include <string>

#include <boost/unordered_map.hpp>

#include "game/data/CompoundTag.hpp"

static boost::unordered_map<std::wstring, std::wstring> sEntityIdMap;

void makeIdMap__Fv()
{
    sEntityIdMap[L"AreaEffectCloud"] = L"minecraft_area_effect_cloud";
    sEntityIdMap[L"ArmorStand"] = L"minecraft_armor_stand";
    sEntityIdMap[L"Arrow"] = L"minecraft_arrow";
    sEntityIdMap[L"Bat"] = L"minecraft_bat";
    sEntityIdMap[L"Blaze"] = L"minecraft_blaze";
    sEntityIdMap[L"Boat"] = L"minecraft_boat";
    sEntityIdMap[L"CaveSpider"] = L"minecraft_cave_spider";
    sEntityIdMap[L"Chicken"] = L"minecraft_chicken";
    sEntityIdMap[L"Cow"] = L"minecraft_cow";
    sEntityIdMap[L"Creeper"] = L"minecraft_creeper";
    sEntityIdMap[L"Donkey"] = L"minecraft_donkey";
    sEntityIdMap[L"DragonFireball"] = L"minecraft_dragon_fireball";
    sEntityIdMap[L"ElderGuardian"] = L"minecraft_elder_guardian";
    sEntityIdMap[L"EnderCrystal"] = L"minecraft_ender_crystal";
    sEntityIdMap[L"EnderDragon"] = L"minecraft_ender_dragon";
    sEntityIdMap[L"Enderman"] = L"minecraft_enderman";
    sEntityIdMap[L"Endermite"] = L"minecraft_endermite";
    sEntityIdMap[L"EyeOfEnderSignal"] = L"minecraft_eye_of_ender_signal";
    sEntityIdMap[L"FallingSand"] = L"minecraft_falling_block";
    sEntityIdMap[L"Fireball"] = L"minecraft_fireball";
    sEntityIdMap[L"FireworksRocketEntity"] = L"minecraft_fireworks_rocket";
    sEntityIdMap[L"Ghast"] = L"minecraft_ghast";
    sEntityIdMap[L"Giant"] = L"minecraft_giant";
    sEntityIdMap[L"Guardian"] = L"minecraft_guardian";
    sEntityIdMap[L"Horse"] = L"minecraft_horse";
    sEntityIdMap[L"Husk"] = L"minecraft_husk";
    sEntityIdMap[L"Item"] = L"minecraft_item";
    sEntityIdMap[L"ItemFrame"] = L"minecraft_item_frame";
    sEntityIdMap[L"LavaSlime"] = L"minecraft_magma_cube";
    sEntityIdMap[L"LeashKnot"] = L"minecraft_leash_knot";
    sEntityIdMap[L"MinecartChest"] = L"minecraft_chest_minecart";
    sEntityIdMap[L"MinecartCommandBlock"] = L"minecraft_commandblock_minecart";
    sEntityIdMap[L"MinecartFurnace"] = L"minecraft_furnace_minecart";
    sEntityIdMap[L"MinecartHopper"] = L"minecraft_hopper_minecart";
    sEntityIdMap[L"MinecartRideable"] = L"minecraft_minecart";
    sEntityIdMap[L"MinecartSpawner"] = L"minecraft_spawner_minecart";
    sEntityIdMap[L"MinecartTNT"] = L"minecraft_tnt_minecart";
    sEntityIdMap[L"Mule"] = L"minecraft_mule";
    sEntityIdMap[L"MushroomCow"] = L"minecraft_mooshroom";
    sEntityIdMap[L"Ozelot"] = L"minecraft_ocelot";
    sEntityIdMap[L"Painting"] = L"minecraft_painting";
    sEntityIdMap[L"Pig"] = L"minecraft_pig";
    sEntityIdMap[L"PigZombie"] = L"minecraft_zombie_pigman";
    sEntityIdMap[L"PolarBear"] = L"minecraft_polar_bear";
    sEntityIdMap[L"PrimedTnt"] = L"minecraft_tnt";
    sEntityIdMap[L"Rabbit"] = L"minecraft_rabbit";
    sEntityIdMap[L"Sheep"] = L"minecraft_sheep";
    sEntityIdMap[L"Shulker"] = L"minecraft_shulker";
    sEntityIdMap[L"ShulkerBullet"] = L"minecraft_shulker_bullet";
    sEntityIdMap[L"Silverfish"] = L"minecraft_silverfish";
    sEntityIdMap[L"Skeleton"] = L"minecraft_skeleton";
    sEntityIdMap[L"SkeletonHorse"] = L"minecraft_skeleton_horse";
    sEntityIdMap[L"Slime"] = L"minecraft_slime";
    sEntityIdMap[L"SmallFireball"] = L"minecraft_small_fireball";
    sEntityIdMap[L"SnowMan"] = L"minecraft_snowman";
    sEntityIdMap[L"Snowball"] = L"minecraft_snowball";
    sEntityIdMap[L"SpectralArrow"] = L"minecraft_spectral_arrow";
    sEntityIdMap[L"Spider"] = L"minecraft_spider";
    sEntityIdMap[L"Squid"] = L"minecraft_squid";
    sEntityIdMap[L"Stray"] = L"minecraft_stray";
    sEntityIdMap[L"ThrownEgg"] = L"minecraft_egg";
    sEntityIdMap[L"ThrownEnderpearl"] = L"minecraft_ender_pearl";
    sEntityIdMap[L"ThrownExpBottle"] = L"minecraft_xp_bottle";
    sEntityIdMap[L"ThrownPotion"] = L"minecraft_potion";
    sEntityIdMap[L"Villager"] = L"minecraft_villager";
    sEntityIdMap[L"VillagerGolem"] = L"minecraft_villager_golem";
    sEntityIdMap[L"Witch"] = L"minecraft_witch";
    sEntityIdMap[L"WitherBoss"] = L"minecraft_wither";
    sEntityIdMap[L"WitherSkeleton"] = L"minecraft_wither_skeleton";
    sEntityIdMap[L"WitherSkull"] = L"minecraft_wither_skull";
    sEntityIdMap[L"Wolf"] = L"minecraft_wolf";
    sEntityIdMap[L"XPOrb"] = L"minecraft_xp_orb";
    sEntityIdMap[L"Zombie"] = L"minecraft_zombie";
    sEntityIdMap[L"ZombieHorse"] = L"minecraft_zombie_horse";
    sEntityIdMap[L"ZombieVillager"] = L"minecraft_zombie_villager";
}

EntityIdFix::~EntityIdFix()
{
}

int EntityIdFix::getVersion()
{
    return 704;
}

CompoundTag* EntityIdFix::fix(CompoundTag* tag)
{
    boost::unordered_map<std::wstring, std::wstring>::const_iterator entry = sEntityIdMap.find(tag->getString(L"id"));

    if (entry != sEntityIdMap.end())
    {
        tag->putString(L"id", entry->second);
    }

    return tag;
}
