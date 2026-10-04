#include "game/item/Potion.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/L10N.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/item/Items.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/block/DefaultedMappedRegistry.hpp"

static ResourceLocation* DEFAULT_POTION_KEY = new ResourceLocation(L"water");
DefaultedMappedRegistry<ResourceLocation, Potion*>* Potion::potionRegistry =
    new DefaultedMappedRegistry<ResourceLocation, Potion*>(*DEFAULT_POTION_KEY);
static int gNextPotionId = 0;

Potion* Potion::byId(int id)
{
    return potionRegistry->byId(id);
}

int Potion::getId(Potion* potion)
{
    return potionRegistry->getId(potion);
}

Potion* Potion::byName(const std::wstring& name)
{
    return potionRegistry->get(ResourceLocation(name));
}

Potion::Potion(int nameId, int useDescriptionId, int splashNameId, int lingeringNameId, int arrowNameId, int arrowUseDescriptionId)
{
    mNameId = nameId;
    mUseDescriptionId = useDescriptionId;
    mArrowUseDescriptionId = arrowUseDescriptionId;
    mArrowNameId = arrowNameId;
    mLingeringNameId = lingeringNameId;
    mSplashNameId = splashNameId;
}

Potion::Potion(MobEffectInstance* effect, int nameId, int useDescriptionId, int splashNameId, int lingeringNameId, int arrowNameId, int arrowUseDescriptionId)
{
    mNameId = nameId;
    mUseDescriptionId = useDescriptionId;
    mArrowUseDescriptionId = arrowUseDescriptionId;
    mArrowNameId = arrowNameId;
    mLingeringNameId = lingeringNameId;
    mSplashNameId = splashNameId;
    mEffects.push_back(effect);
}

Potion::Potion(const std::vector<MobEffectInstance*>& effects, int nameId, int useDescriptionId, int splashNameId, int lingeringNameId, int arrowNameId, int arrowUseDescriptionId)
    : mNameId(nameId), mSplashNameId(splashNameId), mLingeringNameId(lingeringNameId), mArrowNameId(arrowNameId), mUseDescriptionId(useDescriptionId), mArrowUseDescriptionId(arrowUseDescriptionId), mEffects(effects)
{
}

std::wstring Potion::getName(EType type)
{
    int id;
    switch (type)
    {
    case NORMAL:
        id = mNameId;
        break;
    case SPLASH:
        id = mSplashNameId;
        break;
    case LINGERING:
        id = mLingeringNameId;
        break;
    case ARROW:
        id = mArrowNameId;
        break;
    default:
        id = mNameId;
        break;
    }
    return L10N::GetString(id);
}

std::vector<MobEffectInstance*>& Potion::getEffects()
{
    return mEffects;
}

int Potion::getUseDescriptionId(EType type)
{
    return type == ARROW ? mArrowUseDescriptionId : mUseDescriptionId;
}

void Potion::addPotion(const std::wstring& name, Potion* potion)
{
    potionRegistry->registerMapping(gNextPotionId++, ResourceLocation(name), potion);
}

void Potion::StaticCtor()
{
    MobEffectInstance* effect;
    addPotion(L"empty", new Potion(IDS_POTION_EFFECT_EMPTY, IDS_POTION_DESC_EMPTY, IDS_SPLASH_POTION_EFFECT_EMPTY, IDS_LINGERING_POTION_EFFECT_EMPTY, IDS_TIPPED_ARROW_EFFECT_EMPTY, IDS_DESC_ITEM_TIPPED_ARROW_GENERIC));
    addPotion(L"water", new Potion(IDS_POTION_EFFECT_WATER, IDS_POTION_DESC_WATER_BOTTLE, IDS_SPLASH_POTION_EFFECT_WATER, IDS_LINGERING_POTION_EFFECT_WATER, IDS_TIPPED_ARROW_EFFECT_WATER, IDS_DESC_ITEM_TIPPED_ARROW_GENERIC));
    addPotion(L"mundane", new Potion(IDS_POTION_EFFECT_MUNDANE, IDS_POTION_DESC_WATER_BOTTLE, IDS_SPLASH_POTION_EFFECT_MUNDANE, IDS_LINGERING_POTION_EFFECT_MUNDANE, IDS_TIPPED_ARROW_EFFECT_MUNDANE, IDS_DESC_ITEM_TIPPED_ARROW_GENERIC));
    addPotion(L"thick", new Potion(IDS_POTION_EFFECT_THICK, IDS_POTION_DESC_WATER_BOTTLE, IDS_SPLASH_POTION_EFFECT_THICK, IDS_LINGERING_POTION_EFFECT_THICK, IDS_TIPPED_ARROW_EFFECT_THICK, IDS_DESC_ITEM_TIPPED_ARROW_GENERIC));
    addPotion(L"awkward", new Potion(IDS_POTION_EFFECT_AWKWARD, IDS_POTION_DESC_WATER_BOTTLE, IDS_SPLASH_POTION_EFFECT_AWKWARD, IDS_LINGERING_POTION_EFFECT_AWKWARD, IDS_TIPPED_ARROW_EFFECT_AWKWARD, IDS_DESC_ITEM_TIPPED_ARROW_GENERIC));

    effect = new MobEffectInstance(MobEffects::NIGHT_VISION, 3600);
    effect->durationEffectId = 0;
    addPotion(L"night_vision", new Potion(effect, IDS_POTION_EFFECT_NIGHT_VISION, IDS_POTION_DESC_NIGHTVISION, IDS_SPLASH_POTION_EFFECT_NIGHT_VISION, IDS_LINGERING_POTION_EFFECT_NIGHT_VISION, IDS_TIPPED_ARROW_EFFECT_NIGHT_VISION, IDS_DESC_ITEM_TIPPED_ARROW_NIGHTVISION));
    effect = new MobEffectInstance(MobEffects::NIGHT_VISION, 9600);
    effect->durationEffectId = 1;
    addPotion(L"long_night_vision", new Potion(effect, IDS_POTION_EFFECT_NIGHT_VISION, IDS_POTION_DESC_NIGHTVISION, IDS_SPLASH_POTION_EFFECT_NIGHT_VISION, IDS_LINGERING_POTION_EFFECT_NIGHT_VISION, IDS_TIPPED_ARROW_EFFECT_NIGHT_VISION, IDS_DESC_ITEM_TIPPED_ARROW_NIGHTVISION));
    effect = new MobEffectInstance(MobEffects::INVISIBILITY, 3600);
    effect->durationEffectId = 2;
    addPotion(L"invisibility", new Potion(effect, IDS_POTION_EFFECT_INVISIBILITY, IDS_POTION_DESC_INVISIBILITY, IDS_ITEM_DRIED_POTION_TITLE, IDS_LINGERING_POTION_EFFECT_INVISIBILITY, IDS_TIPPED_ARROW_EFFECT_INVISIBILITY, IDS_DESC_ITEM_TIPPED_ARROW_INVISIBILITY));
    effect = new MobEffectInstance(MobEffects::INVISIBILITY, 9600);
    effect->durationEffectId = 3;
    addPotion(L"long_invisibility", new Potion(effect, IDS_POTION_EFFECT_INVISIBILITY, IDS_POTION_DESC_INVISIBILITY, IDS_ITEM_DRIED_POTION_TITLE, IDS_LINGERING_POTION_EFFECT_INVISIBILITY, IDS_TIPPED_ARROW_EFFECT_INVISIBILITY, IDS_DESC_ITEM_TIPPED_ARROW_INVISIBILITY));
    effect = new MobEffectInstance(MobEffects::JUMP, 3600);
    effect->durationEffectId = 4;
    addPotion(L"leaping", new Potion(effect, IDS_POTION_EFFECT_LEAPING, 0x552a6dc4, IDS_SPLASH_POTION_EFFECT_LEAPING, IDS_LINGERING_POTION_EFFECT_LEAPING, IDS_TIPPED_ARROW_EFFECT_LEAPING, 0x569cc9e7));
    effect = new MobEffectInstance(MobEffects::JUMP, 9600);
    effect->durationEffectId = 5;
    addPotion(L"long_leaping", new Potion(effect, IDS_POTION_EFFECT_LEAPING, 0x552a6dc4, IDS_SPLASH_POTION_EFFECT_LEAPING, IDS_LINGERING_POTION_EFFECT_LEAPING, IDS_TIPPED_ARROW_EFFECT_LEAPING, 0x569cc9e7));
    effect = new MobEffectInstance(MobEffects::JUMP, 1800, 1);
    effect->durationEffectId = 6;
    addPotion(L"strong_leaping", new Potion(effect, IDS_POTION_EFFECT_LEAPING, 0x552a6dc4, IDS_SPLASH_POTION_EFFECT_LEAPING, IDS_LINGERING_POTION_EFFECT_LEAPING, IDS_TIPPED_ARROW_EFFECT_LEAPING, 0x569cc9e7));
    effect = new MobEffectInstance(MobEffects::FIRE_RESISTANCE, 3600);
    effect->durationEffectId = 7;
    addPotion(L"fire_resistance", new Potion(effect, IDS_POTION_EFFECT_FIRE_RESISTANCE, IDS_POTION_DESC_FIRERESISTANCE, IDS_SPLASH_POTION_EFFECT_FIRE_RESISTANCE, IDS_LINGERING_POTION_EFFECT_FIRE_RESISTANCE, IDS_TIPPED_ARROW_EFFECT_FIRE_RESISTANCE, IDS_DESC_ITEM_TIPPED_ARROW_FIRERESISTANCE));
    effect = new MobEffectInstance(MobEffects::FIRE_RESISTANCE, 9600);
    effect->durationEffectId = 8;
    addPotion(L"long_fire_resistance", new Potion(effect, IDS_POTION_EFFECT_FIRE_RESISTANCE, IDS_POTION_DESC_FIRERESISTANCE, IDS_SPLASH_POTION_EFFECT_FIRE_RESISTANCE, IDS_LINGERING_POTION_EFFECT_FIRE_RESISTANCE, IDS_TIPPED_ARROW_EFFECT_FIRE_RESISTANCE, IDS_DESC_ITEM_TIPPED_ARROW_FIRERESISTANCE));
    effect = new MobEffectInstance(MobEffects::MOVEMENT_SPEED, 3600);
    effect->durationEffectId = 9;
    addPotion(L"swiftness", new Potion(effect, IDS_POTION_EFFECT_SWIFTNESS, IDS_POTION_DESC_MOVESPEED, IDS_SPLASH_POTION_EFFECT_SWIFTNESS, IDS_LINGERING_POTION_EFFECT_SWIFTNESS, IDS_TIPPED_ARROW_EFFECT_SWIFTNESS, IDS_DESC_ITEM_TIPPED_ARROW_MOVESPEED));
    effect = new MobEffectInstance(MobEffects::MOVEMENT_SPEED, 9600);
    effect->durationEffectId = 10;
    addPotion(L"long_swiftness", new Potion(effect, IDS_POTION_EFFECT_SWIFTNESS, IDS_POTION_DESC_MOVESPEED, IDS_SPLASH_POTION_EFFECT_SWIFTNESS, IDS_LINGERING_POTION_EFFECT_SWIFTNESS, IDS_TIPPED_ARROW_EFFECT_SWIFTNESS, IDS_DESC_ITEM_TIPPED_ARROW_MOVESPEED));
    effect = new MobEffectInstance(MobEffects::MOVEMENT_SPEED, 1800, 1);
    effect->durationEffectId = 11;
    addPotion(L"strong_swiftness", new Potion(effect, IDS_POTION_EFFECT_SWIFTNESS, IDS_POTION_DESC_MOVESPEED, IDS_SPLASH_POTION_EFFECT_SWIFTNESS, IDS_LINGERING_POTION_EFFECT_SWIFTNESS, IDS_TIPPED_ARROW_EFFECT_SWIFTNESS, IDS_DESC_ITEM_TIPPED_ARROW_MOVESPEED));
    effect = new MobEffectInstance(MobEffects::MOVEMENT_SLOWDOWN, 1800);
    effect->durationEffectId = 12;
    addPotion(L"slowness", new Potion(effect, IDS_POTION_EFFECT_SLOWNESS, IDS_POTION_DESC_MOVESLOWDOWN, IDS_SPLASH_POTION_EFFECT_SLOWNESS, IDS_LINGERING_POTION_EFFECT_SLOWNESS, IDS_TIPPED_ARROW_EFFECT_SLOWNESS, IDS_DESC_ITEM_TIPPED_ARROW_MOVESLOWDOWN));
    effect = new MobEffectInstance(MobEffects::MOVEMENT_SLOWDOWN, 4800);
    effect->durationEffectId = 13;
    addPotion(L"long_slowness", new Potion(effect, IDS_POTION_EFFECT_SLOWNESS, IDS_POTION_DESC_MOVESLOWDOWN, IDS_SPLASH_POTION_EFFECT_SLOWNESS, IDS_LINGERING_POTION_EFFECT_SLOWNESS, IDS_TIPPED_ARROW_EFFECT_SLOWNESS, IDS_DESC_ITEM_TIPPED_ARROW_MOVESLOWDOWN));
    effect = new MobEffectInstance(MobEffects::WATER_BREATHING, 3600);
    effect->durationEffectId = 14;
    addPotion(L"water_breathing", new Potion(effect, IDS_POTION_EFFECT_WATER_BREATHING, IDS_POTION_DESC_WATERBREATHING, IDS_SPLASH_POTION_EFFECT_WATER_BREATHING, IDS_LINGERING_POTION_EFFECT_WATER_BREATHING, IDS_TIPPED_ARROW_EFFECT_WATER_BREATHING, IDS_DESC_ITEM_TIPPED_ARROW_WATERBREATHING));
    effect = new MobEffectInstance(MobEffects::WATER_BREATHING, 9600);
    effect->durationEffectId = 15;
    addPotion(L"long_water_breathing", new Potion(effect, IDS_POTION_EFFECT_WATER_BREATHING, IDS_POTION_DESC_WATERBREATHING, IDS_SPLASH_POTION_EFFECT_WATER_BREATHING, IDS_LINGERING_POTION_EFFECT_WATER_BREATHING, IDS_TIPPED_ARROW_EFFECT_WATER_BREATHING, IDS_DESC_ITEM_TIPPED_ARROW_WATERBREATHING));
    effect = new MobEffectInstance(MobEffects::HEAL, 1);
    effect->durationEffectId = 16;
    addPotion(L"healing", new Potion(effect, IDS_POTION_EFFECT_HEALING, IDS_POTION_DESC_HEAL, IDS_SPLASH_POTION_EFFECT_HEALING, IDS_LINGERING_POTION_EFFECT_HEALING, IDS_TIPPED_ARROW_EFFECT_HEALING, IDS_DESC_ITEM_TIPPED_ARROW_HEAL));
    effect = new MobEffectInstance(MobEffects::HEAL, 1, 1);
    effect->durationEffectId = 17;
    addPotion(L"strong_healing", new Potion(effect, IDS_POTION_EFFECT_HEALING, IDS_POTION_DESC_HEAL, IDS_SPLASH_POTION_EFFECT_HEALING, IDS_LINGERING_POTION_EFFECT_HEALING, IDS_TIPPED_ARROW_EFFECT_HEALING, IDS_DESC_ITEM_TIPPED_ARROW_HEAL));
    effect = new MobEffectInstance(MobEffects::HARM, 1);
    effect->durationEffectId = 18;
    addPotion(L"harming", new Potion(effect, IDS_POTION_EFFECT_HARMING, IDS_POTION_DESC_HARM, IDS_SPLASH_POTION_EFFECT_HARMING, IDS_LINGERING_POTION_EFFECT_HARMING, IDS_TIPPED_ARROW_EFFECT_HARMING, IDS_DESC_ITEM_TIPPED_ARROW_HARM));
    effect = new MobEffectInstance(MobEffects::HARM, 1, 1);
    effect->durationEffectId = 19;
    addPotion(L"strong_harming", new Potion(effect, IDS_POTION_EFFECT_HARMING, IDS_POTION_DESC_HARM, IDS_SPLASH_POTION_EFFECT_HARMING, IDS_LINGERING_POTION_EFFECT_HARMING, IDS_TIPPED_ARROW_EFFECT_HARMING, IDS_DESC_ITEM_TIPPED_ARROW_HARM));
    effect = new MobEffectInstance(MobEffects::POISON, 900);
    effect->durationEffectId = 20;
    addPotion(L"poison", new Potion(effect, IDS_POTION_EFFECT_POISON, IDS_POTION_DESC_POISON, IDS_SPLASH_POTION_EFFECT_POISON, IDS_LINGERING_POTION_EFFECT_POISON, IDS_TIPPED_ARROW_EFFECT_POISON, IDS_DESC_ITEM_TIPPED_ARROW_POISON));
    effect = new MobEffectInstance(MobEffects::POISON, 2400);
    effect->durationEffectId = 21;
    addPotion(L"long_poison", new Potion(effect, IDS_POTION_EFFECT_POISON, IDS_POTION_DESC_POISON, IDS_SPLASH_POTION_EFFECT_POISON, IDS_LINGERING_POTION_EFFECT_POISON, IDS_TIPPED_ARROW_EFFECT_POISON, IDS_DESC_ITEM_TIPPED_ARROW_POISON));
    effect = new MobEffectInstance(MobEffects::POISON, 450, 1);
    effect->durationEffectId = 22;
    addPotion(L"strong_poison", new Potion(effect, IDS_POTION_EFFECT_POISON, IDS_POTION_DESC_POISON, IDS_SPLASH_POTION_EFFECT_POISON, IDS_LINGERING_POTION_EFFECT_POISON, IDS_TIPPED_ARROW_EFFECT_POISON, IDS_DESC_ITEM_TIPPED_ARROW_POISON));
    effect = new MobEffectInstance(MobEffects::REGENERATION, 900);
    effect->durationEffectId = 23;
    addPotion(L"regeneration", new Potion(effect, IDS_POTION_EFFECT_REGENERATION, IDS_POTION_DESC_REGENERATION, IDS_SPLASH_POTION_EFFECT_REGENERATION, IDS_LINGERING_POTION_EFFECT_REGENERATION, IDS_TIPPED_ARROW_EFFECT_REGENERATION, IDS_DESC_ITEM_TIPPED_ARROW_REGENERATION));
    effect = new MobEffectInstance(MobEffects::REGENERATION, 2400);
    effect->durationEffectId = 24;
    addPotion(L"long_regeneration", new Potion(effect, IDS_POTION_EFFECT_REGENERATION, IDS_POTION_DESC_REGENERATION, IDS_SPLASH_POTION_EFFECT_REGENERATION, IDS_LINGERING_POTION_EFFECT_REGENERATION, IDS_TIPPED_ARROW_EFFECT_REGENERATION, IDS_DESC_ITEM_TIPPED_ARROW_REGENERATION));
    effect = new MobEffectInstance(MobEffects::REGENERATION, 450, 1);
    effect->durationEffectId = 25;
    addPotion(L"strong_regeneration", new Potion(effect, IDS_POTION_EFFECT_REGENERATION, IDS_POTION_DESC_REGENERATION, IDS_SPLASH_POTION_EFFECT_REGENERATION, IDS_LINGERING_POTION_EFFECT_REGENERATION, IDS_TIPPED_ARROW_EFFECT_REGENERATION, IDS_DESC_ITEM_TIPPED_ARROW_REGENERATION));
    effect = new MobEffectInstance(MobEffects::DAMAGE_BOOST, 3600);
    effect->durationEffectId = 26;
    addPotion(L"strength", new Potion(effect, IDS_POTION_EFFECT_STRENGTH, IDS_POTION_DESC_DAMAGEBOOST, IDS_SPLASH_POTION_EFFECT_STRENGTH, IDS_LINGERING_POTION_EFFECT_STRENGTH, IDS_TIPPED_ARROW_EFFECT_STRENGTH, IDS_DESC_ITEM_TIPPED_ARROW_DAMAGEBOOST));
    effect = new MobEffectInstance(MobEffects::DAMAGE_BOOST, 9600);
    effect->durationEffectId = 27;
    addPotion(L"long_strength", new Potion(effect, IDS_POTION_EFFECT_STRENGTH, IDS_POTION_DESC_DAMAGEBOOST, IDS_SPLASH_POTION_EFFECT_STRENGTH, IDS_LINGERING_POTION_EFFECT_STRENGTH, IDS_TIPPED_ARROW_EFFECT_STRENGTH, IDS_DESC_ITEM_TIPPED_ARROW_DAMAGEBOOST));
    effect = new MobEffectInstance(MobEffects::DAMAGE_BOOST, 1800, 1);
    effect->durationEffectId = 28;
    addPotion(L"strong_strength", new Potion(effect, IDS_POTION_EFFECT_STRENGTH, IDS_POTION_DESC_DAMAGEBOOST, IDS_SPLASH_POTION_EFFECT_STRENGTH, IDS_LINGERING_POTION_EFFECT_STRENGTH, IDS_TIPPED_ARROW_EFFECT_STRENGTH, IDS_DESC_ITEM_TIPPED_ARROW_DAMAGEBOOST));
    effect = new MobEffectInstance(MobEffects::WEAKNESS, 1800);
    effect->durationEffectId = 29;
    addPotion(L"weakness", new Potion(effect, IDS_POTION_EFFECT_WEAKNESS, IDS_POTION_DESC_WEAKNESS, IDS_SPLASH_POTION_EFFECT_WEAKNESS, IDS_LINGERING_POTION_EFFECT_WEAKNESS, IDS_TIPPED_ARROW_EFFECT_WEAKNESS, IDS_DESC_ITEM_TIPPED_ARROW_WEAKNESS));
    effect = new MobEffectInstance(MobEffects::WEAKNESS, 4800);
    effect->durationEffectId = 30;
    addPotion(L"long_weakness", new Potion(effect, IDS_POTION_EFFECT_WEAKNESS, IDS_POTION_DESC_WEAKNESS, IDS_SPLASH_POTION_EFFECT_WEAKNESS, IDS_LINGERING_POTION_EFFECT_WEAKNESS, IDS_TIPPED_ARROW_EFFECT_WEAKNESS, IDS_DESC_ITEM_TIPPED_ARROW_WEAKNESS));
    effect = new MobEffectInstance(MobEffects::LUCK, 6000);
    effect->durationEffectId = 31;
    addPotion(L"luck", new Potion(effect, IDS_POTION_EFFECT_LUCK, IDS_POTION_DESC_LUCK, IDS_SPLASH_POTION_EFFECT_LUCK, IDS_LINGERING_POTION_EFFECT_LUCK, IDS_TIPPED_ARROW_EFFECT_LUCK, IDS_DESC_ITEM_TIPPED_ARROW_LUCK));

    std::vector<MobEffectInstance*> turtleMasterEffects;
    effect = new MobEffectInstance(MobEffects::MOVEMENT_SLOWDOWN, 400, 3);
    effect->durationEffectId = 33;
    turtleMasterEffects.push_back(effect);
    effect = new MobEffectInstance(MobEffects::DAMAGE_RESISTANCE, 400, 2);
    effect->durationEffectId = 33;
    turtleMasterEffects.push_back(effect);
    addPotion(L"turtle_master", new Potion(turtleMasterEffects, IDS_POTION_EFFECT_TURTLE_MASTER, IDS_POTION_DESC_TURTLE_MASTER, IDS_SPLASH_POTION_EFFECT_TURTLE_MASTER, IDS_LINGERING_POTION_EFFECT_TURTLE_MASTER, IDS_TIPPED_ARROW_EFFECT_TURTLE_MASTER, IDS_DESC_ITEM_TIPPED_ARROW_TURTLE_MASTER));

    std::vector<MobEffectInstance*> longTurtleMasterEffects;
    effect = new MobEffectInstance(MobEffects::MOVEMENT_SLOWDOWN, 800, 3);
    effect->durationEffectId = 34;
    longTurtleMasterEffects.push_back(effect);
    effect = new MobEffectInstance(MobEffects::DAMAGE_RESISTANCE, 800, 2);
    effect->durationEffectId = 34;
    longTurtleMasterEffects.push_back(effect);
    addPotion(L"long_turtle_master", new Potion(longTurtleMasterEffects, IDS_POTION_EFFECT_TURTLE_MASTER, IDS_POTION_DESC_TURTLE_MASTER, IDS_SPLASH_POTION_EFFECT_TURTLE_MASTER, IDS_LINGERING_POTION_EFFECT_TURTLE_MASTER, IDS_TIPPED_ARROW_EFFECT_TURTLE_MASTER, IDS_DESC_ITEM_TIPPED_ARROW_TURTLE_MASTER));

    std::vector<MobEffectInstance*> strongTurtleMasterEffects;
    effect = new MobEffectInstance(MobEffects::MOVEMENT_SLOWDOWN, 400, 5);
    effect->durationEffectId = 35;
    strongTurtleMasterEffects.push_back(effect);
    effect = new MobEffectInstance(MobEffects::DAMAGE_RESISTANCE, 400, 3);
    effect->durationEffectId = 35;
    strongTurtleMasterEffects.push_back(effect);
    addPotion(L"strong_turtle_master", new Potion(strongTurtleMasterEffects, IDS_POTION_EFFECT_TURTLE_MASTER, IDS_POTION_DESC_TURTLE_MASTER, IDS_SPLASH_POTION_EFFECT_TURTLE_MASTER, IDS_LINGERING_POTION_EFFECT_TURTLE_MASTER, IDS_TIPPED_ARROW_EFFECT_TURTLE_MASTER, IDS_DESC_ITEM_TIPPED_ARROW_TURTLE_MASTER));

    effect = new MobEffectInstance(MobEffects::WITHER, 800, 1);
    effect->durationEffectId = 32;
    addPotion(L"wither", new Potion(effect, IDS_POTION_EFFECT_WITHER, IDS_POTION_DESC_WITHER, IDS_SPLASH_POTION_EFFECT_WITHER, IDS_LINGERING_POTION_EFFECT_WITHER, IDS_TIPPED_ARROW_EFFECT_WITHER, IDS_DESC_ITEM_TIPPED_ARROW_WITHER));
    effect = new MobEffectInstance(MobEffects::SLOW_FALLING, 1800);
    effect->durationEffectId = 36;
    addPotion(L"slow_falling", new Potion(effect, IDS_POTION_EFFECT_SLOW_FALLING, IDS_POTION_DESC_SLOW_FALLING, IDS_SPLASH_POTION_EFFECT_SLOW_FALLING, IDS_LINGERING_POTION_EFFECT_SLOW_FALLING, IDS_TIPPED_ARROW_EFFECT_SLOW_FALLING, IDS_DESC_ITEM_TIPPED_ARROW_SLOW_FALLING));
    effect = new MobEffectInstance(MobEffects::SLOW_FALLING, 4800);
    effect->durationEffectId = 37;
    addPotion(L"long_slow_falling", new Potion(effect, IDS_POTION_EFFECT_SLOW_FALLING, IDS_POTION_DESC_SLOW_FALLING, IDS_SPLASH_POTION_EFFECT_SLOW_FALLING, IDS_LINGERING_POTION_EFFECT_SLOW_FALLING, IDS_TIPPED_ARROW_EFFECT_SLOW_FALLING, IDS_DESC_ITEM_TIPPED_ARROW_SLOW_FALLING));
}

Item* Potion::getBasePotion(EType type)
{
    if (type == SPLASH)
        return Items::splashPotion;
    if (type == LINGERING)
        return Items::lingeringPotion;
    return Items::potion;
}

bool Potion::hasInstantEffects()
{
    if (mEffects.empty()) return false;
    std::vector<MobEffectInstance*>::iterator it;
    for (it = mEffects.begin(); it != mEffects.end(); ++it)
        if ((*it)->getEffect()->isInstantenous()) return true;
    return false;
}
