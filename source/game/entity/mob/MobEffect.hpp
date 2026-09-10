#ifndef GAME_ENTITY_MOB_MOBEFFECT_HPP
#define GAME_ENTITY_MOB_MOBEFFECT_HPP

#include <functional>
#include <string>

#include <boost/functional/hash.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

#include "game/core/AttributeModifier.hpp"
#include "game/core/ColourTable.hpp"
#include "game/world/block/IdMapper.hpp"
#include "game/world/block/MappedRegistry.hpp"
#include "game/util/ResourceLocation.hpp"

class BaseAttributeMap;
class DamageSource;
class Entity;
class LivingEntity;
class MobEffectInstance;

class MobEffect : public boost::unordered_map<Attribute*, AttributeModifier*>
{
public:
    enum EMobEffectIcon
    {
        EMobEffectIcon_0 = 0
    };

    static MappedRegistry<ResourceLocation, MobEffect*>* mobEffectRegistry;

    MobEffect(bool harmful, eMinecraftColour color);
    virtual ~MobEffect();

    MobEffect* setIcon(EMobEffectIcon icon);
    MobEffect* setDescriptionId(int descriptionId);
    MobEffect* setDurationModifier(double durationModifier);
    static void staticCtor();
    static unsigned int GetEffectCount();
    static MobEffect* byId(int id);
    static int getId(MobEffect* effect);
    static MobEffect* byName(const std::wstring& name);

    virtual void applyEffectTick(const boost::shared_ptr<LivingEntity>& entity, int amplifier);
    virtual void applyInstantenousEffect(const boost::shared_ptr<Entity>& source,
                                         const boost::shared_ptr<Entity>& indirectSource,
                                         const boost::shared_ptr<LivingEntity>& entity,
                                         int amplifier, double health);
    virtual bool isDurationEffectTick(int duration, int amplifier);
    virtual bool isInstantenous();
    int getDescriptionId(int auxiliaryValue);
    bool hasIcon() const;
    unsigned int getIcon() const;
    bool isHarmful() const;
    virtual double getDurationModifier() const;
    virtual MobEffect* setBeneficial();
    virtual bool isBeneficial() const;
    virtual eMinecraftColour getColor() const;
    virtual MobEffect* addAttributeModifier(Attribute* attribute, eMODIFIER_ID id,
                                             double amount, int operation);
    virtual void getAttributeModifiers();
    virtual void removeAttributeModifiers(const boost::shared_ptr<LivingEntity>& entity,
                                          BaseAttributeMap* attributes, int amplifier);
    virtual void addAttributeModifiers(const boost::shared_ptr<LivingEntity>& entity,
                                       BaseAttributeMap* attributes, int amplifier);
    virtual double getAttributeModifierValue(int amplifier, AttributeModifier* modifier);
    static std::wstring formatDuration(MobEffectInstance* effect, float durationFactor);

private:
    int descriptionId;
    unsigned int icon;
    bool harmful;
    double durationModifier;
    bool beneficial;
    eMinecraftColour color;
};

#endif
