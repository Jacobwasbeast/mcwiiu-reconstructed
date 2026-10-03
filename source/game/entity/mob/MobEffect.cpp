#include "game/entity/mob/MobEffect.hpp"
#include "game/core/StringIds.hpp"

#include <cmath>
#include <cwchar>

#include <boost/shared_ptr.hpp>

#include "game/core/BaseAttributeMap.hpp"
#include "game/entity/mob/AbsoptionMobEffect.hpp"
#include "game/entity/mob/AttackDamageMobEffect.hpp"
#include "game/entity/mob/HealthBoostMobEffect.hpp"
#include "game/entity/mob/InstantenousMobEffect.hpp"
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

MobEffect* MobEffect::setIcon(EMobEffectIcon effectIcon)
{
    icon = (unsigned int)effectIcon;
    return this;
}

MobEffect* MobEffect::setDescriptionId(int id)
{
    descriptionId = id;
    return this;
}

MobEffect* MobEffect::setDurationModifier(double modifier)
{
    durationModifier = modifier;
    return this;
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
            entity->hurt(DamageSource::POISON, 1.0f);
        }
    }
    else if (this == MobEffects::WITHER)
    {
        entity->hurt(DamageSource::WITHER, 1.0f);
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
            entity->hurt(DamageSource::INSTANT_DAMAGE, (float)(6 << amplifier));
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
            DamageSource* damage =
                DamageSource::CreateIndirectMagic(source, indirectSource, DamageSource::INSTANT_DAMAGE);
            entity->hurt(damage, amount);
            delete damage;
        }
        else
        {
            entity->hurt(DamageSource::MAGIC, amount);
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

MobEffect* MobEffect::setBeneficial()
{
    beneficial = true;
    return this;
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

void MobEffect::staticCtor()
{
    effectCount = 0;

    MobEffect* effect = new MobEffect(false, eMinecraftColour_Effect_MovementSpeed);
    effect->setDescriptionId(IDS_EFFECT_MOVESPEED);
    effect->setIcon(EMobEffectIcon_Speed);
    effect->addAttributeModifier(SharedMonsterAttributes::MOVEMENT_SPEED, eMODIFIER_ID_PotionMoveSpeed,
                                 0.20000000298023224, 2);
    effect->setBeneficial();
    mobEffectRegistry->registerMapping(++effectCount, ResourceLocation(L"minecraft", L"speed"), effect);

    effect = new MobEffect(true, eMinecraftColour_Effect_MovementSlowDown);
    effect->setDescriptionId(IDS_EFFECT_MOVESLOWDOWN);
    effect->setIcon(EMobEffectIcon_Slowness);
    effect->addAttributeModifier(SharedMonsterAttributes::MOVEMENT_SPEED, eMODIFIER_ID_PotionMoveSlowdown,
                                 -0.15000000596046448, 2);
    mobEffectRegistry->registerMapping(++effectCount, ResourceLocation(L"minecraft", L"slowness"), effect);

    effect = new MobEffect(false, eMinecraftColour_Effect_DigSpeed);
    effect->setDescriptionId(IDS_EFFECT_DIGSPEED);
    effect->setDurationModifier(1.5);
    effect->setIcon(EMobEffectIcon_Haste);
    effect->setBeneficial();
    effect->addAttributeModifier(SharedMonsterAttributes::ATTACK_SPEED, eMODIFIER_ID_PotionDigSpeed,
                                 0.10000000149011612, 2);
    mobEffectRegistry->registerMapping(++effectCount, ResourceLocation(L"minecraft", L"haste"), effect);

    effect = new MobEffect(true, eMinecraftColour_Effect_DigSlowdown);
    effect->setDescriptionId(IDS_EFFECT_DIGSLOWDOWN);
    effect->setIcon(EMobEffectIcon_MiningFatigue);
    effect->addAttributeModifier(SharedMonsterAttributes::ATTACK_SPEED, eMODIFIER_ID_PotionDigSpeed,
                                 -0.10000000149011612, 2);
    mobEffectRegistry->registerMapping(++effectCount, ResourceLocation(L"minecraft", L"mining_fatigue"), effect);

    effect = new AttackDamageMobEffect(false, eMinecraftColour_Effect_DamageBoost, 3.0);
    effect->setDescriptionId(IDS_EFFECT_DAMAGEBOOST);
    effect->setIcon(EMobEffectIcon_Strength);
    effect->addAttributeModifier(SharedMonsterAttributes::ATTACK_DAMAGE, eMODIFIER_ID_PotionDamageBoost, 0.0, 0);
    effect->setBeneficial();
    mobEffectRegistry->registerMapping(++effectCount, ResourceLocation(L"minecraft", L"strength"), effect);

    effect = new InstantenousMobEffect(false, eMinecraftColour_Effect_Heal);
    effect->setDescriptionId(IDS_EFFECT_HEAL);
    effect->setBeneficial();
    mobEffectRegistry->registerMapping(++effectCount, ResourceLocation(L"minecraft", L"instant_health"), effect);

    effect = new InstantenousMobEffect(true, eMinecraftColour_Effect_Harm);
    effect->setDescriptionId(IDS_EFFECT_HARM);
    effect->setBeneficial();
    mobEffectRegistry->registerMapping(++effectCount, ResourceLocation(L"minecraft", L"instant_damage"), effect);

    effect = new MobEffect(false, eMinecraftColour_Effect_Jump);
    effect->setDescriptionId(IDS_EFFECT_JUMP);
    effect->setIcon(EMobEffectIcon_JumpBoost);
    effect->setBeneficial();
    mobEffectRegistry->registerMapping(++effectCount, ResourceLocation(L"minecraft", L"jump_boost"), effect);

    effect = new MobEffect(true, eMinecraftColour_Effect_Confusion);
    effect->setDescriptionId(IDS_EFFECT_CONFUSION);
    effect->setDurationModifier(0.25);
    effect->setIcon(EMobEffectIcon_Nausea);
    mobEffectRegistry->registerMapping(++effectCount, ResourceLocation(L"minecraft", L"nausea"), effect);

    effect = new MobEffect(false, eMinecraftColour_Effect_Regeneration);
    effect->setDescriptionId(IDS_EFFECT_REGENERATION);
    effect->setDurationModifier(0.25);
    effect->setIcon(EMobEffectIcon_Regeneration);
    effect->setBeneficial();
    mobEffectRegistry->registerMapping(++effectCount, ResourceLocation(L"minecraft", L"regeneration"), effect);

    effect = new MobEffect(false, eMinecraftColour_Effect_DamageResistance);
    effect->setDescriptionId(IDS_EFFECT_RESISTANCE);
    effect->setIcon(EMobEffectIcon_Resistance);
    effect->setBeneficial();
    mobEffectRegistry->registerMapping(++effectCount, ResourceLocation(L"minecraft", L"resistance"), effect);

    effect = new MobEffect(false, eMinecraftColour_Effect_FireResistance);
    effect->setDescriptionId(IDS_EFFECT_FIRERESISTANCE);
    effect->setIcon(EMobEffectIcon_FireResistance);
    effect->setBeneficial();
    mobEffectRegistry->registerMapping(++effectCount, ResourceLocation(L"minecraft", L"fire_resistance"), effect);

    effect = new MobEffect(false, eMinecraftColour_Effect_WaterBreathing);
    effect->setDescriptionId(IDS_EFFECT_WATERBREATHING);
    effect->setIcon(EMobEffectIcon_WaterBreathing);
    effect->setBeneficial();
    mobEffectRegistry->registerMapping(++effectCount, ResourceLocation(L"minecraft", L"water_breathing"), effect);

    effect = new MobEffect(false, eMinecraftColour_Effect_Invisiblity);
    effect->setDescriptionId(IDS_EFFECT_INVISIBILITY);
    effect->setIcon(EMobEffectIcon_Invisibility);
    effect->setBeneficial();
    mobEffectRegistry->registerMapping(++effectCount, ResourceLocation(L"minecraft", L"invisibility"), effect);

    effect = new MobEffect(true, eMinecraftColour_Effect_Blindness);
    effect->setDescriptionId(IDS_EFFECT_BLINDNESS);
    effect->setDurationModifier(0.25);
    effect->setIcon(EMobEffectIcon_Blindness);
    mobEffectRegistry->registerMapping(++effectCount, ResourceLocation(L"minecraft", L"blindness"), effect);

    effect = new MobEffect(false, eMinecraftColour_Effect_NightVision);
    effect->setDescriptionId(IDS_EFFECT_NIGHTVISION);
    effect->setIcon(EMobEffectIcon_NightVision);
    effect->setBeneficial();
    mobEffectRegistry->registerMapping(++effectCount, ResourceLocation(L"minecraft", L"night_vision"), effect);

    effect = new MobEffect(true, eMinecraftColour_Effect_Hunger);
    effect->setDescriptionId(IDS_EFFECT_HUNGER);
    effect->setIcon(EMobEffectIcon_Hunger);
    mobEffectRegistry->registerMapping(++effectCount, ResourceLocation(L"minecraft", L"hunger"), effect);

    effect = new AttackDamageMobEffect(true, eMinecraftColour_Effect_Weakness, -4.0);
    effect->setDescriptionId(IDS_EFFECT_WEAKNESS);
    effect->setIcon(EMobEffectIcon_Weakness);
    effect->addAttributeModifier(SharedMonsterAttributes::ATTACK_DAMAGE, eMODIFIER_ID_PotionWeakness, 0.0, 0);
    mobEffectRegistry->registerMapping(++effectCount, ResourceLocation(L"minecraft", L"weakness"), effect);

    effect = new MobEffect(true, eMinecraftColour_Effect_Poison);
    effect->setDescriptionId(IDS_EFFECT_POISON);
    effect->setDurationModifier(0.25);
    effect->setIcon(EMobEffectIcon_Poison);
    mobEffectRegistry->registerMapping(++effectCount, ResourceLocation(L"minecraft", L"poison"), effect);

    effect = new MobEffect(true, eMinecraftColour_Effect_Wither);
    effect->setDescriptionId(IDS_EFFECT_WITHER);
    effect->setIcon(EMobEffectIcon_Wither);
    effect->setDurationModifier(0.25);
    mobEffectRegistry->registerMapping(++effectCount, ResourceLocation(L"minecraft", L"wither"), effect);

    effect = new HealthBoostMobEffect(false, eMinecraftColour_Effect_HealthBoost);
    effect->setDescriptionId(IDS_EFFECT_HEALTHBOOST);
    effect->setIcon(EMobEffectIcon_HealthBoost);
    effect->addAttributeModifier(SharedMonsterAttributes::MAX_HEALTH, eMODIFIER_ID_PotionHealthBoost, 4.0, 0);
    effect->setBeneficial();
    mobEffectRegistry->registerMapping(++effectCount, ResourceLocation(L"minecraft", L"health_boost"), effect);

    effect = new AbsoptionMobEffect(false, eMinecraftColour_Effect_Absorption);
    effect->setDescriptionId(IDS_EFFECT_ABSORPTION);
    effect->setIcon(EMobEffectIcon_Absorption);
    effect->setBeneficial();
    mobEffectRegistry->registerMapping(++effectCount, ResourceLocation(L"minecraft", L"absorption"), effect);

    effect = new InstantenousMobEffect(false, eMinecraftColour_Effect_Saturation);
    effect->setDescriptionId(IDS_EFFECT_SATURATION);
    effect->setBeneficial();
    mobEffectRegistry->registerMapping(++effectCount, ResourceLocation(L"minecraft", L"saturation"), effect);

    effect = new MobEffect(false, eMinecraftColour_Effect_Glowing);
    effect->setDescriptionId(IDS_EFFECT_GLOWING);
    effect->setIcon(EMobEffectIcon_Glowing);
    mobEffectRegistry->registerMapping(++effectCount, ResourceLocation(L"minecraft", L"glowing"), effect);

    effect = new MobEffect(true, eMinecraftColour_Effect_Levitation);
    effect->setDescriptionId(IDS_EFFECT_LEVITATION);
    effect->setIcon(EMobEffectIcon_Levitation);
    mobEffectRegistry->registerMapping(++effectCount, ResourceLocation(L"minecraft", L"levitation"), effect);

    effect = new MobEffect(false, eMinecraftColour_Effect_Luck);
    effect->setDescriptionId(IDS_EFFECT_LUCK);
    effect->setIcon(EMobEffectIcon_Luck);
    effect->setBeneficial();
    effect->addAttributeModifier(SharedMonsterAttributes::LUCK, eMODIFIER_ID_PotionLuck, 1.0, 0);
    mobEffectRegistry->registerMapping(++effectCount, ResourceLocation(L"minecraft", L"luck"), effect);

    effect = new MobEffect(true, eMinecraftColour_Effect_BadLuck);
    effect->setDescriptionId(IDS_EFFECT_UNLUCK);
    effect->setIcon(EMobEffectIcon_Unluck);
    effect->addAttributeModifier(SharedMonsterAttributes::LUCK, eMODIFIER_ID_PotionLuck, -1.0, 0);
    mobEffectRegistry->registerMapping(++effectCount, ResourceLocation(L"minecraft", L"unluck"), effect);

    effect = new MobEffect(false, eMinecraftColour_Effect_ConduitPower);
    effect->setDescriptionId(IDS_EFFECT_CONDUITPOWER);
    effect->setIcon(EMobEffectIcon_ConduitPower);
    effect->setBeneficial();
    mobEffectRegistry->registerMapping(++effectCount, ResourceLocation(L"minecraft", L"conduit_power"), effect);

    effect = new MobEffect(false, eMinecraftColour_Effect_SlowFall);
    effect->setDescriptionId(IDS_EFFECT_SLOWFALLING);
    effect->setIcon(EMobEffectIcon_SlowFalling);
    effect->setBeneficial();
    mobEffectRegistry->registerMapping(++effectCount, ResourceLocation(L"minecraft", L"slow_falling"), effect);
}
