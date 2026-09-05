#include "game/entity/mob/MobEffect.hpp"

#include <cmath>
#include <cwchar>

#include <boost/shared_ptr.hpp>

#include "game/core/BaseAttributeMap.hpp"
#include "game/core/FoodData.hpp"
#include "game/core/FoodConstants.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/entity/player/Player.hpp"
#include "game/util/ResourceLocation.hpp"

MappedRegistry<ResourceLocation, MobEffect*>* MobEffect::mobEffectRegistry =
    new MappedRegistry<ResourceLocation, MobEffect*>();

MobEffect::MobEffect(bool isHarmful, eMinecraftColour effectColor)
    : descriptionId(-1), icon(0), harmful(isHarmful), durationModifier(isHarmful ? 0.5 : 1.0),
      beneficial(false), color(effectColor)
{
}

MobEffect::~MobEffect()
{
}

void MobEffect::setIcon(EMobEffectIcon effectIcon)
{
    icon = (unsigned int)effectIcon;
}

void MobEffect::setDescriptionId(int id)
{
    descriptionId = id;
}

void MobEffect::setDurationModifier(double modifier)
{
    durationModifier = modifier;
}

unsigned int MobEffect::GetEffectCount()
{
    return (unsigned int)mobEffectRegistry->keySet().size();
}

MobEffect* MobEffect::byId(int id)
{
    return mobEffectRegistry->byId(id);
}

int MobEffect::getId(MobEffect* effect)
{
    return mobEffectRegistry->getId(effect);
}

MobEffect* MobEffect::byName(const std::wstring& name)
{
    return mobEffectRegistry->get(ResourceLocation(name));
}

void MobEffect::applyEffectTick(const boost::shared_ptr<LivingEntity>& entity, int amplifier)
{
    if (this == MobEffects::REGENERATION)
    {
        if (entity->getHealth() < entity->getMaxHealth())
        {
            entity->heal((float)(1 << amplifier));
        }
    }
    else if (this == MobEffects::POISON)
    {
        if (entity->getHealth() > 1.0f)
        {
            entity->hurt(NULL, 1.0f);
        }
    }
    else if (this == MobEffects::WITHER)
    {
        entity->hurt(NULL, 1.0f);
    }
    else if (this == MobEffects::HUNGER)
    {
        boost::shared_ptr<Player> player = boost::dynamic_pointer_cast<Player>(entity);
        if (player)
        {
            player->causeFoodExhaustion((float)(FoodConstants::ExhaustionMine() * (amplifier + 1)));
        }
    }
    else if (this == MobEffects::SATURATION)
    {
        boost::shared_ptr<Player> player = boost::dynamic_pointer_cast<Player>(entity);
        if (player && !player->isCreative())
        {
            player->getFoodData()->eat(amplifier + 1, 1.0f);
        }
    }
}

void MobEffect::applyInstantenousEffect(const boost::shared_ptr<Entity>& source,
                                        const boost::shared_ptr<Entity>& indirectSource,
                                        const boost::shared_ptr<LivingEntity>& entity,
                                        int amplifier, double health)
{
    double amount = health * (double)(1 << amplifier);
    if (this == MobEffects::HEAL)
    {
        entity->heal((float)amount);
    }
    else if (this == MobEffects::HARM)
    {
        entity->hurt(NULL, (float)amount);
    }
    else if (this == MobEffects::POISON || this == MobEffects::WITHER)
    {
        entity->hurt(NULL, (float)amount);
    }
}

bool MobEffect::isDurationEffectTick(int duration, int amplifier)
{
    if (this == MobEffects::REGENERATION)
    {
        int interval = 50 >> amplifier;
        return interval > 0 && duration % interval == 0;
    }
    if (this == MobEffects::POISON)
    {
        int interval = 25 >> amplifier;
        return interval > 0 && duration % interval == 0;
    }
    if (this == MobEffects::WITHER)
    {
        int interval = 40 >> amplifier;
        return interval > 0 && duration % interval == 0;
    }
    return this == MobEffects::HUNGER;
}

bool MobEffect::isInstantenous()
{
    return false;
}

int MobEffect::getDescriptionId(int) 
{
    return descriptionId;
}

bool MobEffect::hasIcon() const
{
    return icon != 0;
}

unsigned int MobEffect::getIcon() const
{
    return icon;
}

bool MobEffect::isHarmful() const
{
    return harmful;
}

double MobEffect::getDurationModifier() const
{
    return durationModifier;
}

void MobEffect::setBeneficial()
{
    beneficial = true;
}

bool MobEffect::isBeneficial() const
{
    return beneficial;
}

eMinecraftColour MobEffect::getColor() const
{
    return color;
}

MobEffect* MobEffect::addAttributeModifier(Attribute* attribute, eMODIFIER_ID id,
                                            double amount, int operation)
{
    (*this)[attribute] = new AttributeModifier(id, amount, operation);
    return this;
}

void MobEffect::getAttributeModifiers()
{
}

void MobEffect::removeAttributeModifiers(const boost::shared_ptr<LivingEntity>&,
                                         BaseAttributeMap* attributes, int)
{
    for (iterator it = begin(); it != end(); ++it)
    {
        AttributeInstance* instance = attributes->getInstance(it->first);
        if (instance)
        {
            instance->removeModifier(it->second);
        }
    }
}

void MobEffect::addAttributeModifiers(const boost::shared_ptr<LivingEntity>&,
                                      BaseAttributeMap* attributes, int amplifier)
{
    for (iterator it = begin(); it != end(); ++it)
    {
        AttributeInstance* instance = attributes->getInstance(it->first);
        if (instance)
        {
            AttributeModifier* modifier = it->second;
            instance->removeModifier(modifier);
            instance->addModifier(new AttributeModifier(modifier->getId(),
                                                        getAttributeModifierValue(amplifier, modifier),
                                                        modifier->getOperation()));
        }
    }
}

double MobEffect::getAttributeModifierValue(int amplifier, AttributeModifier* modifier)
{
    return modifier->getAmount() * (double)(amplifier + 1);
}

std::wstring MobEffect::formatDuration(MobEffectInstance* effect, float durationFactor)
{
    if (effect->isNoCounter())
    {
        return L"∞";
    }
    int ticks = (int)std::floor((float)effect->getDuration() * durationFactor);
    wchar_t buffer[16];
    std::swprintf(buffer, 16, ticks / 1200 == 0 ? L"%d:%02d" : L"%d:%02d", ticks / 1200,
                  (ticks / 20) % 60);
    return std::wstring(buffer);
}

static void registerMobEffect(int id, const wchar_t* name, bool harmful, eMinecraftColour color,
                              int descriptionId, unsigned int icon)
{
    MobEffect* effect = new MobEffect(harmful, color);
    effect->setDescriptionId(descriptionId);
    effect->setIcon((MobEffect::EMobEffectIcon)icon);
    MobEffect::mobEffectRegistry->registerMapping(id, ResourceLocation(L"minecraft", name), effect);
}

void MobEffect::staticCtor()
{
    registerMobEffect(253, L"speed", false, (eMinecraftColour)253, 0xa549ae42, 14);
    registerMobEffect(254, L"slowness", true, (eMinecraftColour)254, 0x25d65b68, 13);
    registerMobEffect(255, L"haste", false, (eMinecraftColour)255, 0x7df665f9, 3);
    registerMobEffect(256, L"mining_fatigue", true, (eMinecraftColour)256, 0xf483c9eb, 7);
    registerMobEffect(257, L"strength", false, (eMinecraftColour)257, 0x8708edca, 15);
    registerMobEffect(258, L"instant_health", false, (eMinecraftColour)258, 0x327028e4, 0);
    registerMobEffect(259, L"instant_damage", true, (eMinecraftColour)259, 0x21220fea, 0);
    registerMobEffect(260, L"jump_boost", false, (eMinecraftColour)260, 0x0912c386, 6);
    registerMobEffect(261, L"nausea", true, (eMinecraftColour)261, 0x3d3be8ce, 8);
    registerMobEffect(262, L"regeneration", false, (eMinecraftColour)262, 0xb28e79a5, 11);
    registerMobEffect(263, L"resistance", false, (eMinecraftColour)263, 0x91395347, 12);
    registerMobEffect(264, L"fire_resistance", false, (eMinecraftColour)264, 0x18a0f8bf, 2);
    registerMobEffect(265, L"water_breathing", false, (eMinecraftColour)265, 0x37c990c9, 16);
    registerMobEffect(266, L"invisibility", false, (eMinecraftColour)266, 0x77869fc5, 5);
    registerMobEffect(267, L"blindness", true, (eMinecraftColour)267, 0x8fa1bcb2, 1);
    registerMobEffect(268, L"night_vision", false, (eMinecraftColour)268, 0x671094dc, 9);
    registerMobEffect(269, L"hunger", true, (eMinecraftColour)269, 0x79345cbf, 4);
    registerMobEffect(270, L"weakness", true, (eMinecraftColour)270, 0xd4431b9f, 17);
    registerMobEffect(271, L"poison", true, (eMinecraftColour)271, 0x54f76d78, 10);
    registerMobEffect(272, L"wither", true, (eMinecraftColour)272, 0x132dcc1, 18);
    registerMobEffect(273, L"health_boost", false, (eMinecraftColour)273, 0xe7478e85, 19);
    registerMobEffect(274, L"absorption", false, (eMinecraftColour)274, 0x824126cd, 20);
    registerMobEffect(275, L"saturation", false, (eMinecraftColour)275, 0xe132cfd0, 0);
    registerMobEffect(276, L"glowing", false, (eMinecraftColour)276, 0x52e542df, 21);
    registerMobEffect(277, L"levitation", true, (eMinecraftColour)277, 0x13f37be3, 22);
    registerMobEffect(278, L"luck", false, (eMinecraftColour)278, 0x474605cd, 23);
    registerMobEffect(279, L"unluck", true, (eMinecraftColour)279, 0xa35cc356, 24);
    registerMobEffect(280, L"conduit_power", false, (eMinecraftColour)280, 0x92a57635, 27);
    registerMobEffect(281, L"slow_falling", false, (eMinecraftColour)281, 0xa7f1591c, 26);
}
