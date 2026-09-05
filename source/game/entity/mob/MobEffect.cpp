#include "game/entity/mob/MobEffect.hpp"

#include <cmath>
#include <cwchar>

#include <boost/shared_ptr.hpp>

#include "game/core/BaseAttributeMap.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/FoodData.hpp"
#include "game/core/FoodConstants.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/world/generation/Player.hpp"
#include "game/util/ResourceLocation.hpp"

MappedRegistry<ResourceLocation, MobEffect*>* MobEffect::mobEffectRegistry =
    new MappedRegistry<ResourceLocation, MobEffect*>();
static unsigned int effectCount;

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
    return effectCount;
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
            entity->heal(1.0f);
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
        if (player)
        {
            player->getFoodData()->eat(amplifier + 1, 1.0f);
        }
    }
    else
    {
        bool inverted = entity->isInvertedHealAndHarm();
        if ((this == MobEffects::HEAL && !inverted) ||
            (this == MobEffects::HARM && inverted))
        {
            entity->heal((float)(4 << amplifier));
        }
        else if ((this == MobEffects::HARM && !inverted) ||
                 (this == MobEffects::HEAL && inverted))
        {
            entity->hurt(NULL, (float)(6 << amplifier));
        }
    }
}

void MobEffect::applyInstantenousEffect(const boost::shared_ptr<Entity>& source,
                                        const boost::shared_ptr<Entity>& indirectSource,
                                        const boost::shared_ptr<LivingEntity>& entity,
                                        int amplifier, double health)
{
    bool inverted = entity->isInvertedHealAndHarm();
    if ((this == MobEffects::HEAL && !inverted) || (this == MobEffects::HARM && inverted))
    {
        entity->heal((float)std::floor(health * (double)(4 << amplifier) + 0.5));
    }
    else if ((this == MobEffects::HARM && !inverted) ||
             (this == MobEffects::HEAL && inverted))
    {
        float amount = (float)std::floor(health * (double)(6 << amplifier) + 0.5);
        if (source)
        {
            boost::shared_ptr<DamageSource> damage =
                DamageSource::CreateIndirectMagic(source, indirectSource, NULL);
            entity->hurt(damage.get(), amount);
        }
        else
        {
            entity->hurt(NULL, amount);
        }
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
    insert(value_type(attribute, new AttributeModifier(id, amount, operation)));
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
        return L"**:**";
    }
    int ticks = (int)std::floor((float)effect->getDuration() * durationFactor);
    int seconds = (ticks / 20) % 60;
    wchar_t buffer[8];
    std::swprintf(buffer, 8, seconds < 10 ? L"%d:0%d" : L"%d:%d", ticks / 1200, seconds);
    return std::wstring(buffer);
}

static MobEffect* registerMobEffect(int id, const wchar_t* name, bool harmful,
                                    eMinecraftColour color, int descriptionId,
                                    unsigned int icon, double durationModifier,
                                    bool beneficial)
{
    MobEffect* effect = new MobEffect(harmful, color);
    effect->setDescriptionId(descriptionId);
    effect->setIcon((MobEffect::EMobEffectIcon)icon);
    effect->setDurationModifier(durationModifier);
    if (beneficial)
    {
        effect->setBeneficial();
    }
    effectCount = (unsigned int)id;
    MobEffect::mobEffectRegistry->registerMapping(id, ResourceLocation(L"minecraft", name), effect);
    return effect;
}

void MobEffect::staticCtor()
{
    MobEffect* effect = registerMobEffect(1, L"speed", false, (eMinecraftColour)253,
                                         0xa549ae42, 14, 1.0, true);
    effect->addAttributeModifier(SharedMonsterAttributes::MOVEMENT_SPEED,
                                 (eMODIFIER_ID)10, 0.20000000298023224, 2);
    effect = registerMobEffect(2, L"slowness", true, (eMinecraftColour)254,
                               0x25d65b68, 13, 0.5, false);
    effect->addAttributeModifier(SharedMonsterAttributes::MOVEMENT_SPEED,
                                 (eMODIFIER_ID)11, -0.15000000596046448, 2);
    effect = registerMobEffect(3, L"haste", false, (eMinecraftColour)255,
                               0x7df665f9, 3, 1.5, true);
    effect->addAttributeModifier(SharedMonsterAttributes::ATTACK_SPEED,
                                 (eMODIFIER_ID)13, 0.10000000149011612, 2);
    effect = registerMobEffect(4, L"mining_fatigue", true, (eMinecraftColour)256,
                               0xf483c9eb, 7, 0.5, false);
    effect->addAttributeModifier(SharedMonsterAttributes::ATTACK_SPEED,
                                 (eMODIFIER_ID)13, -0.10000000149011612, 2);
    effect = registerMobEffect(5, L"strength", false, (eMinecraftColour)257,
                               0x8708edca, 15, 1.0, true);
    effect->addAttributeModifier(SharedMonsterAttributes::ATTACK_DAMAGE,
                                 (eMODIFIER_ID)8, 0.0, 0);
    registerMobEffect(6, L"instant_health", false, (eMinecraftColour)258,
                      0x327028e4, 0, 1.0, true);
    registerMobEffect(7, L"instant_damage", true, (eMinecraftColour)259,
                      0x21220fea, 0, 0.5, false);
    registerMobEffect(8, L"jump_boost", false, (eMinecraftColour)260,
                      0x0912c386, 6, 1.0, true);
    registerMobEffect(9, L"nausea", true, (eMinecraftColour)261,
                      0x3d3be8ce, 8, 0.25, false);
    registerMobEffect(10, L"regeneration", false, (eMinecraftColour)262,
                      0xb28e79a5, 11, 0.25, true);
    registerMobEffect(11, L"resistance", false, (eMinecraftColour)263,
                      0x91395347, 12, 1.0, true);
    registerMobEffect(12, L"fire_resistance", false, (eMinecraftColour)264,
                      0x18a0f8bf, 2, 1.0, true);
    registerMobEffect(13, L"water_breathing", false, (eMinecraftColour)265,
                      0x37c990c9, 16, 1.0, true);
    registerMobEffect(14, L"invisibility", false, (eMinecraftColour)266,
                      0x77869fc5, 5, 1.0, true);
    registerMobEffect(15, L"blindness", true, (eMinecraftColour)267,
                      0x8fa1bcb2, 1, 0.25, false);
    registerMobEffect(16, L"night_vision", false, (eMinecraftColour)268,
                      0x671094dc, 9, 1.0, true);
    registerMobEffect(17, L"hunger", true, (eMinecraftColour)269,
                      0x79345cbf, 4, 0.5, false);
    effect = registerMobEffect(18, L"weakness", true, (eMinecraftColour)270,
                               0xd4431b9f, 17, 0.5, false);
    effect->addAttributeModifier(SharedMonsterAttributes::ATTACK_DAMAGE,
                                 (eMODIFIER_ID)12, 0.0, 0);
    registerMobEffect(19, L"poison", true, (eMinecraftColour)271,
                      0x54f76d78, 10, 0.25, false);
    registerMobEffect(20, L"wither", true, (eMinecraftColour)272,
                      0x0132dcc1, 18, 0.25, false);
    effect = registerMobEffect(21, L"health_boost", false, (eMinecraftColour)273,
                               0xe7478e85, 19, 1.0, true);
    effect->addAttributeModifier(SharedMonsterAttributes::MAX_HEALTH,
                                 (eMODIFIER_ID)9, 4.0, 0);
    registerMobEffect(22, L"absorption", false, (eMinecraftColour)274,
                      0x824126cd, 20, 1.0, true);
    registerMobEffect(23, L"saturation", false, (eMinecraftColour)275,
                      0xe132cfd0, 0, 1.0, true);
    registerMobEffect(24, L"glowing", false, (eMinecraftColour)276,
                      0x52e542df, 21, 1.0, false);
    registerMobEffect(25, L"levitation", true, (eMinecraftColour)277,
                      0x13f37be3, 22, 0.5, false);
    effect = registerMobEffect(26, L"luck", false, (eMinecraftColour)278,
                               0x474605cd, 23, 1.0, true);
    effect->addAttributeModifier(SharedMonsterAttributes::LUCK,
                                 (eMODIFIER_ID)14, 1.0, 0);
    effect = registerMobEffect(27, L"unluck", true, (eMinecraftColour)279,
                               0xa35cc356, 24, 0.5, false);
    effect->addAttributeModifier(SharedMonsterAttributes::LUCK,
                                 (eMODIFIER_ID)14, -1.0, 0);
    registerMobEffect(28, L"conduit_power", false, (eMinecraftColour)282,
                      0x92a57635, 27, 1.0, true);
    registerMobEffect(29, L"slow_falling", false, (eMinecraftColour)281,
                      0xa7f1591c, 26, 1.0, true);
}
