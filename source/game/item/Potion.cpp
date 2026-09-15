#include "game/item/Potion.hpp"
#include "game/core/L10N.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/MobEffects.hpp"
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
    addPotion(L"empty", new Potion(0xd1664335, 0x6e3a986b, 0xe6cc7b07, 0xc5127f79, 0x9c7f167c, 0x2453270c));
    addPotion(L"water", new Potion(0xf00211d, 0xe4c9f6a8, 0x6e0e150f, 0x28ff8661, 0xefb7cc44, 0x2453270c));
    addPotion(L"mundane", new Potion(0xa542dc40, 0xe4c9f6a8, 0x20b82cd2, 0x26aeacdc, 0x4694ed89, 0x2453270c));
    addPotion(L"thick", new Potion(0x39177e7d, 0xe4c9f6a8, 0xab5e85cf, 0x423ac4b1, 0x1baa4224, 0x2453270c));
    addPotion(L"awkward", new Potion(0xb8c39575, 0xe4c9f6a8, 0xbdd4ee07, 0x5b0327a9, 0x9a47236c, 0x2453270c));

    addPotion(L"night_vision", new Potion(new MobEffectInstance(MobEffects::NIGHT_VISION, 0xe10), 0x82dff027, 0x22abfc9e, 0xd9226ae5, 0xd646308b, 0x5965c116, 0xc833bd8d));
    addPotion(L"long_night_vision", new Potion(new MobEffectInstance(MobEffects::NIGHT_VISION, 0x2580), 0x82dff027, 0x22abfc9e, 0xd9226ae5, 0xd646308b, 0x5965c116, 0xc833bd8d));
    addPotion(L"invisibility", new Potion(new MobEffectInstance(MobEffects::INVISIBILITY, 0xe10), 0x5abfd449, 0x4edeb57, 0x6cbf3ab, 0x67e1ca95, 0x7143ce8, 0xb58f337c));
    addPotion(L"long_invisibility", new Potion(new MobEffectInstance(MobEffects::INVISIBILITY, 0x2580), 0x5abfd449, 0x4edeb57, 0x6cbf3ab, 0x67e1ca95, 0x7143ce8, 0xb58f337c));
    addPotion(L"leaping", new Potion(new MobEffectInstance(MobEffects::JUMP, 0xe10), 0x88233440, 0x552a6dc4, 0x88acb12, 0x135f499c, 0x2b9de1e9, 0x569cc9e7));
    addPotion(L"long_leaping", new Potion(new MobEffectInstance(MobEffects::JUMP, 0x2580), 0x88233440, 0x552a6dc4, 0x88acb12, 0x135f499c, 0x2b9de1e9, 0x569cc9e7));
    addPotion(L"strong_leaping", new Potion(new MobEffectInstance(MobEffects::JUMP, 0x708, 1), 0x88233440, 0x552a6dc4, 0x88acb12, 0x135f499c, 0x2b9de1e9, 0x569cc9e7));
    addPotion(L"fire_resistance", new Potion(new MobEffectInstance(MobEffects::FIRE_RESISTANCE, 0xe10), 0x59433554, 0xa44e128d, 0x8011be46, 0xaaaa85d8, 0x9f68dfed, 0x5aea28c6));
    addPotion(L"long_fire_resistance", new Potion(new MobEffectInstance(MobEffects::FIRE_RESISTANCE, 0x2580), 0x59433554, 0xa44e128d, 0x8011be46, 0xaaaa85d8, 0x9f68dfed, 0x5aea28c6));
    addPotion(L"swiftness", new Potion(new MobEffectInstance(MobEffects::MOVEMENT_SPEED, 0xe10), 0x34ad93a4, 0x48e45d20, 0x3ec63d6, 0xf447f588, 0x8657504d, 0x26657e73));
    addPotion(L"long_swiftness", new Potion(new MobEffectInstance(MobEffects::MOVEMENT_SPEED, 0x2580), 0x34ad93a4, 0x48e45d20, 0x3ec63d6, 0xf447f588, 0x8657504d, 0x26657e73));
    addPotion(L"strong_swiftness", new Potion(new MobEffectInstance(MobEffects::MOVEMENT_SPEED, 0x708, 1), 0x34ad93a4, 0x48e45d20, 0x3ec63d6, 0xf447f588, 0x8657504d, 0x26657e73));
    addPotion(L"slowness", new Potion(new MobEffectInstance(MobEffects::MOVEMENT_SLOWDOWN, 0x708), 0x179a0664, 0x7ab4111a, 0xd5eb12c6, 0xa4129e78, 0x64f822b5, 0x94fb9df1));
    addPotion(L"long_slowness", new Potion(new MobEffectInstance(MobEffects::MOVEMENT_SLOWDOWN, 0x12c0), 0x179a0664, 0x7ab4111a, 0xd5eb12c6, 0xa4129e78, 0x64f822b5, 0x94fb9df1));
    addPotion(L"water_breathing", new Potion(new MobEffectInstance(MobEffects::WATER_BREATHING, 0xe10), 0x2c9cba02, 0xa04d26fb, 0x20e4a1d0, 0x6154418e, 0xb83a165b, 0x3b39e3d0));
    addPotion(L"long_water_breathing", new Potion(new MobEffectInstance(MobEffects::WATER_BREATHING, 0x2580), 0x2c9cba02, 0xa04d26fb, 0x20e4a1d0, 0x6154418e, 0xb83a165b, 0x3b39e3d0));
    addPotion(L"healing", new Potion(new MobEffectInstance(MobEffects::HEAL, 1), 0x4ab4e7b8, 0xe14ddaf6, 0x2acb95aa, 0x17d26a4, 0x4e79911, 0xe95f67cd));
    addPotion(L"strong_healing", new Potion(new MobEffectInstance(MobEffects::HEAL, 1, 1), 0x4ab4e7b8, 0xe14ddaf6, 0x2acb95aa, 0x17d26a4, 0x4e79911, 0xe95f67cd));
    addPotion(L"harming", new Potion(new MobEffectInstance(MobEffects::HARM, 1), 0x14582bb6, 0xa123a098, 0x944d9544, 0x2a15225a, 0x13eb821f, 0x98a01983));
    addPotion(L"strong_harming", new Potion(new MobEffectInstance(MobEffects::HARM, 1, 1), 0x14582bb6, 0xa123a098, 0x944d9544, 0x2a15225a, 0x13eb821f, 0x98a01983));
    addPotion(L"poison", new Potion(new MobEffectInstance(MobEffects::POISON, 900), 0xcb6aa344, 0x17bb33aa, 0x73076146, 0x88569108, 0x3defbe45, 0x4b120591));
    addPotion(L"long_poison", new Potion(new MobEffectInstance(MobEffects::POISON, 0x960), 0xcb6aa344, 0x17bb33aa, 0x73076146, 0x88569108, 0x3defbe45, 0x4b120591));
    addPotion(L"strong_poison", new Potion(new MobEffectInstance(MobEffects::POISON, 0x1c2, 1), 0xcb6aa344, 0x17bb33aa, 0x73076146, 0x88569108, 0x3defbe45, 0x4b120591));
    addPotion(L"regeneration", new Potion(new MobEffectInstance(MobEffects::REGENERATION, 900), 0x6698ce69, 0xe788a6b7, 0x594e366b, 0x310be8d5, 0x52cb2b68, 0x9fa1491c));
    addPotion(L"long_regeneration", new Potion(new MobEffectInstance(MobEffects::REGENERATION, 0x960), 0x6698ce69, 0xe788a6b7, 0x594e366b, 0x310be8d5, 0x52cb2b68, 0x9fa1491c));
    addPotion(L"strong_regeneration", new Potion(new MobEffectInstance(MobEffects::REGENERATION, 0x1c2, 1), 0x6698ce69, 0xe788a6b7, 0x594e366b, 0x310be8d5, 0x52cb2b68, 0x9fa1491c));
    addPotion(L"strength", new Potion(new MobEffectInstance(MobEffects::DAMAGE_BOOST, 0xe10), 0x599a0995, 0xd7a27a48, 0x5fd1fd37, 0x20073079, 0xa3da5c54, 0x326b491b));
    addPotion(L"long_strength", new Potion(new MobEffectInstance(MobEffects::DAMAGE_BOOST, 0x2580), 0x599a0995, 0xd7a27a48, 0x5fd1fd37, 0x20073079, 0xa3da5c54, 0x326b491b));
    addPotion(L"strong_strength", new Potion(new MobEffectInstance(MobEffects::DAMAGE_BOOST, 0x708, 1), 0x599a0995, 0xd7a27a48, 0x5fd1fd37, 0x20073079, 0xa3da5c54, 0x326b491b));
    addPotion(L"weakness", new Potion(new MobEffectInstance(MobEffects::WEAKNESS, 0x708), 0xaf0f6803, 0x7b7bc06d, 0x8d252dc1, 0xa66f350f, 0x2776aee2, 0x7dc95056));
    addPotion(L"long_weakness", new Potion(new MobEffectInstance(MobEffects::WEAKNESS, 0x12c0), 0xaf0f6803, 0x7b7bc06d, 0x8d252dc1, 0xa66f350f, 0x2776aee2, 0x7dc95056));
    addPotion(L"luck", new Potion(new MobEffectInstance(MobEffects::LUCK, 6000), 0x30346491, 0x39dd65bf, 0x4ffa0733, 0x3b80873d, 0xe2eb8eb0, 0xb2304fe4));

    std::vector<MobEffectInstance*> turtleMasterEffects;
    turtleMasterEffects.push_back(new MobEffectInstance(MobEffects::MOVEMENT_SLOWDOWN, 400, 3));
    turtleMasterEffects.push_back(new MobEffectInstance(MobEffects::DAMAGE_RESISTANCE, 400, 2));
    addPotion(L"turtle_master", new Potion(turtleMasterEffects, 0x958f494d, 0x9ffd73, 0xd67ec2bf, 0x97a0cd21, 0x6b05e554, 0x8d05d200));

    std::vector<MobEffectInstance*> longTurtleMasterEffects;
    longTurtleMasterEffects.push_back(new MobEffectInstance(MobEffects::MOVEMENT_SLOWDOWN, 800, 3));
    longTurtleMasterEffects.push_back(new MobEffectInstance(MobEffects::DAMAGE_RESISTANCE, 800, 2));
    addPotion(L"long_turtle_master", new Potion(longTurtleMasterEffects, 0x958f494d, 0x9ffd73, 0xd67ec2bf, 0x97a0cd21, 0x6b05e554, 0x8d05d200));

    std::vector<MobEffectInstance*> strongTurtleMasterEffects;
    strongTurtleMasterEffects.push_back(new MobEffectInstance(MobEffects::MOVEMENT_SLOWDOWN, 400, 5));
    strongTurtleMasterEffects.push_back(new MobEffectInstance(MobEffects::DAMAGE_RESISTANCE, 400, 3));
    addPotion(L"strong_turtle_master", new Potion(strongTurtleMasterEffects, 0x958f494d, 0x9ffd73, 0xd67ec2bf, 0x97a0cd21, 0x6b05e554, 0x8d05d200));

    addPotion(L"wither", new Potion(new MobEffectInstance(MobEffects::WITHER, 800, 1), 0x602719cd, 0x5ac37593, 0x38b01a8f, 0x287b5bf1, 0xa97113fc, 0xc6f93c28));
    addPotion(L"slow_falling", new Potion(new MobEffectInstance(MobEffects::SLOW_FALLING, 0x708), 0x834adb07, 0x4f030b39, 0x7c4a66e5, 0xf0c683fb, 0xed2ccbf6, 0xba9ada2));
    addPotion(L"long_slow_falling", new Potion(new MobEffectInstance(MobEffects::SLOW_FALLING, 0x12c0), 0x834adb07, 0x4f030b39, 0x7c4a66e5, 0xf0c683fb, 0xed2ccbf6, 0xba9ada2));
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
