#include "game/item/Potion.hpp"
#include "game/core/L10N.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/block/SimpleRegistry.hpp"

#include <map>

struct ResourceLocationHash
{
    std::size_t operator()(const ResourceLocation& value) const { return value.hashCode(); }
};

struct ResourceLocationEqual
{
    bool operator()(const ResourceLocation& a, const ResourceLocation& b) const { return a == b; }
};

typedef SimpleRegistry<ResourceLocation, Potion*, ResourceLocationHash, ResourceLocationEqual> PotionRegistry;
static PotionRegistry gPotionRegistry;
SimpleRegistry<ResourceLocation, Potion*, ResourceLocationHash, ResourceLocationEqual>* Potion::potionRegistry = &gPotionRegistry;
static std::map<int, Potion*> gPotionsById;
static std::map<std::wstring, Potion*> gPotionsByName;
static int gNextPotionId = 0;

Potion* Potion::byId(int id)
{
    std::map<int, Potion*>::const_iterator it = gPotionsById.find(id);
    return it == gPotionsById.end() ? NULL : it->second;
}

int Potion::getId(Potion* potion)
{
    std::map<int, Potion*>::const_iterator it;
    for (it = gPotionsById.begin(); it != gPotionsById.end(); ++it)
        if (it->second == potion) return it->first;
    return -1;
}

Potion* Potion::byName(const std::wstring& name)
{
    std::map<std::wstring, Potion*>::const_iterator it = gPotionsByName.find(name);
    return it == gPotionsByName.end() ? NULL : it->second;
}

Potion::Potion(int id, int nameId, int extendedNameId, int amplifiedNameId, int useDescriptionId, int splashUseDescriptionId)
    : mId(id), mNameId(extendedNameId), mExtendedNameId(amplifiedNameId), mAmplifiedNameId(useDescriptionId), mUseDescriptionId(nameId), mSplashUseDescriptionId(splashUseDescriptionId)
{
}

Potion::Potion(MobEffectInstance* effect, int id, int nameId, int extendedNameId, int amplifiedNameId, int useDescriptionId, int splashUseDescriptionId)
    : mId(id), mNameId(extendedNameId), mExtendedNameId(amplifiedNameId), mAmplifiedNameId(useDescriptionId), mUseDescriptionId(nameId), mSplashUseDescriptionId(splashUseDescriptionId)
{
    mEffects.push_back(effect);
}

Potion::Potion(const std::vector<MobEffectInstance*>& effects, int id, int nameId, int extendedNameId, int amplifiedNameId, int useDescriptionId, int splashUseDescriptionId)
    : mId(id), mNameId(extendedNameId), mExtendedNameId(amplifiedNameId), mAmplifiedNameId(useDescriptionId), mUseDescriptionId(nameId), mSplashUseDescriptionId(splashUseDescriptionId), mEffects(effects)
{
}

std::wstring Potion::getName(EType type)
{
    int id = mId;
    if (type == EXTENDED) id = mNameId;
    else if (type == AMPLIFIED) id = mExtendedNameId;
    else if (type == SPLASH) id = mAmplifiedNameId;
    return L10N::GetString(id);
}

std::vector<MobEffectInstance*>& Potion::getEffects()
{
    return mEffects;
}

int Potion::getUseDescriptionId(EType type)
{
    return type == SPLASH ? mSplashUseDescriptionId : mUseDescriptionId;
}

void Potion::addPotion(const std::wstring& name, Potion* potion)
{
    gPotionsByName[name] = potion;
    gPotionsById[gNextPotionId] = potion;
    potionRegistry->registerKey(ResourceLocation(name), potion);
    ++gNextPotionId;
}

void Potion::StaticCtor()
{
    static const wchar_t* names[] = { L"empty", L"water", L"mundane", L"thick", L"awkward", L"night_vision", L"long_night_vision", L"invisibility", L"long_invisibility", L"leaping", L"long_leaping", L"strong_leaping", L"fire_resistance", L"long_fire_resistance", L"swiftness", L"long_swiftness", L"strong_swiftness", L"slowness", L"long_slowness", L"water_breathing", L"long_water_breathing", L"healing", L"strong_healing", L"harming", L"strong_harming", L"poison", L"long_poison", L"strong_poison", L"regeneration", L"long_regeneration", L"strong_regeneration", L"strength", L"long_strength", L"strong_strength", L"weakness", L"long_weakness", L"luck", L"turtle_master", L"long_turtle_master", L"strong_turtle_master", L"wither", L"slow_falling", L"long_slow_falling" };
    const int count = sizeof(names) / sizeof(names[0]);
    int i;
    for (i = 0; i < count; ++i) addPotion(names[i], new Potion(0, 0, 0, 0, 0, 0));
}

Potion* Potion::getBasePotion(EType type)
{
    if (type == EXTENDED) return byId(1);
    if (type == AMPLIFIED) return byId(2);
    return byId(0);
}

bool Potion::hasInstantEffects()
{
    std::vector<MobEffectInstance*>::const_iterator it;
    for (it = mEffects.begin(); it != mEffects.end(); ++it)
        if (*it != NULL && (*it)->getEffect() != NULL && (*it)->getEffect()->isInstantenous()) return true;
    return false;
}
