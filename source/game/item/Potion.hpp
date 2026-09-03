#ifndef GAME_ITEM_POTION_HPP
#define GAME_ITEM_POTION_HPP

#include <string>
#include <vector>

class MobEffectInstance;
class ResourceLocation;
template <class K, class V, class H, class E> class SimpleRegistry;

class Potion
{
public:
    enum EType
    {
        NORMAL = 0,
        EXTENDED = 1,
        AMPLIFIED = 2,
        SPLASH = 3
    };

    static Potion* byId(int id);
    static int getId(Potion* potion);
    static Potion* byName(const std::wstring& name);
    Potion(int id, int nameId, int extendedNameId, int amplifiedNameId, int useDescriptionId, int splashUseDescriptionId);
    Potion(MobEffectInstance* effect, int id, int nameId, int extendedNameId, int amplifiedNameId, int useDescriptionId, int splashUseDescriptionId);
    Potion(const std::vector<MobEffectInstance*>& effects, int id, int nameId, int extendedNameId, int amplifiedNameId, int useDescriptionId, int splashUseDescriptionId);
    std::wstring getName(EType type);
    std::vector<MobEffectInstance*>& getEffects();
    int getUseDescriptionId(EType type);
    static void addPotion(const std::wstring& name, Potion* potion);
    static void StaticCtor();
    static Potion* getBasePotion(EType type);
    bool hasInstantEffects();

    static SimpleRegistry<ResourceLocation, Potion*, struct ResourceLocationHash, struct ResourceLocationEqual>* potionRegistry;

private:
    int mId;
    int mNameId;
    int mExtendedNameId;
    int mAmplifiedNameId;
    int mUseDescriptionId;
    int mSplashUseDescriptionId;
    std::vector<MobEffectInstance*> mEffects;
};

#endif
