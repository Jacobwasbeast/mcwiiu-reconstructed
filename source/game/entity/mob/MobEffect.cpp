#include "game/entity/mob/MobEffect.hpp"

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

MobEffect::~MobEffect()
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

static void registerMobEffect(const wchar_t* name, MobEffect* effect)
{
    ++effectCount;
    MobEffect::mobEffectRegistry->registerMapping(effectCount,
                                                 ResourceLocation(L"minecraft", name), effect);
}

void MobEffect::staticCtor()
{
    effectCount = 0;

    MobEffect* effect = new MobEffect(false, (eMinecraftColour)253);
    effect->setDescriptionId(0xa549ae42);
    effect->setIcon((EMobEffectIcon)14);
    effect->addAttributeModifier(SharedMonsterAttributes::MOVEMENT_SPEED, (eMODIFIER_ID)10,
                                 0.20000000298023224, 2);
    effect->setBeneficial();
    registerMobEffect(L"speed", effect);

    effect = new MobEffect(true, (eMinecraftColour)254);
    effect->setDescriptionId(0x25d65b68);
    effect->setIcon((EMobEffectIcon)13);
    effect->addAttributeModifier(SharedMonsterAttributes::MOVEMENT_SPEED, (eMODIFIER_ID)11,
                                 -0.15000000596046448, 2);
    registerMobEffect(L"slowness", effect);

    effect = new MobEffect(false, (eMinecraftColour)255);
    effect->setDescriptionId(0x7df665f9);
    effect->setDurationModifier(1.5);
    effect->setIcon((EMobEffectIcon)3);
    effect->setBeneficial();
    effect->addAttributeModifier(SharedMonsterAttributes::ATTACK_SPEED, (eMODIFIER_ID)13,
                                 0.10000000149011612, 2);
    registerMobEffect(L"haste", effect);

    effect = new MobEffect(true, (eMinecraftColour)256);
    effect->setDescriptionId(0xf483c9eb);
    effect->setIcon((EMobEffectIcon)7);
    effect->addAttributeModifier(SharedMonsterAttributes::ATTACK_SPEED, (eMODIFIER_ID)13,
                                 -0.10000000149011612, 2);
    registerMobEffect(L"mining_fatigue", effect);

    effect = new AttackDamageMobEffect(false, (eMinecraftColour)257, 3.0);
    effect->setDescriptionId(0x8708edca);
    effect->setIcon((EMobEffectIcon)15);
    effect->addAttributeModifier(SharedMonsterAttributes::ATTACK_DAMAGE, (eMODIFIER_ID)8, 0.0, 0);
    effect->setBeneficial();
    registerMobEffect(L"strength", effect);

    effect = new InstantenousMobEffect(false, (eMinecraftColour)258);
    effect->setDescriptionId(0x327028e4);
    effect->setBeneficial();
    registerMobEffect(L"instant_health", effect);

    effect = new InstantenousMobEffect(true, (eMinecraftColour)259);
    effect->setDescriptionId(0x21220fea);
    effect->setBeneficial();
    registerMobEffect(L"instant_damage", effect);

    effect = new MobEffect(false, (eMinecraftColour)260);
    effect->setDescriptionId(0x0912c386);
    effect->setIcon((EMobEffectIcon)6);
    effect->setBeneficial();
    registerMobEffect(L"jump_boost", effect);

    effect = new MobEffect(true, (eMinecraftColour)261);
    effect->setDescriptionId(0x3d3be8ce);
    effect->setDurationModifier(0.25);
    effect->setIcon((EMobEffectIcon)8);
    registerMobEffect(L"nausea", effect);

    effect = new MobEffect(false, (eMinecraftColour)262);
    effect->setDescriptionId(0xb28e79a5);
    effect->setDurationModifier(0.25);
    effect->setIcon((EMobEffectIcon)11);
    effect->setBeneficial();
    registerMobEffect(L"regeneration", effect);

    effect = new MobEffect(false, (eMinecraftColour)263);
    effect->setDescriptionId(0x91395347);
    effect->setIcon((EMobEffectIcon)12);
    effect->setBeneficial();
    registerMobEffect(L"resistance", effect);

    effect = new MobEffect(false, (eMinecraftColour)264);
    effect->setDescriptionId(0x18a0f8bf);
    effect->setIcon((EMobEffectIcon)2);
    effect->setBeneficial();
    registerMobEffect(L"fire_resistance", effect);

    effect = new MobEffect(false, (eMinecraftColour)265);
    effect->setDescriptionId(0x37c990c9);
    effect->setIcon((EMobEffectIcon)16);
    effect->setBeneficial();
    registerMobEffect(L"water_breathing", effect);

    effect = new MobEffect(false, (eMinecraftColour)266);
    effect->setDescriptionId(0x77869fc5);
    effect->setIcon((EMobEffectIcon)5);
    effect->setBeneficial();
    registerMobEffect(L"invisibility", effect);

    effect = new MobEffect(true, (eMinecraftColour)267);
    effect->setDescriptionId(0x8fa1bcb2);
    effect->setDurationModifier(0.25);
    effect->setIcon((EMobEffectIcon)1);
    registerMobEffect(L"blindness", effect);

    effect = new MobEffect(false, (eMinecraftColour)268);
    effect->setDescriptionId(0x671094dc);
    effect->setIcon((EMobEffectIcon)9);
    effect->setBeneficial();
    registerMobEffect(L"night_vision", effect);

    effect = new MobEffect(true, (eMinecraftColour)269);
    effect->setDescriptionId(0x79345cbf);
    effect->setIcon((EMobEffectIcon)4);
    registerMobEffect(L"hunger", effect);

    effect = new AttackDamageMobEffect(true, (eMinecraftColour)270, -4.0);
    effect->setDescriptionId(0xd4431b9f);
    effect->setIcon((EMobEffectIcon)17);
    effect->addAttributeModifier(SharedMonsterAttributes::ATTACK_DAMAGE, (eMODIFIER_ID)12, 0.0, 0);
    registerMobEffect(L"weakness", effect);

    effect = new MobEffect(true, (eMinecraftColour)271);
    effect->setDescriptionId(0x54f76d78);
    effect->setDurationModifier(0.25);
    effect->setIcon((EMobEffectIcon)10);
    registerMobEffect(L"poison", effect);

    effect = new MobEffect(true, (eMinecraftColour)272);
    effect->setDescriptionId(0x0132dcc1);
    effect->setIcon((EMobEffectIcon)18);
    effect->setDurationModifier(0.25);
    registerMobEffect(L"wither", effect);

    effect = new HealthBoostMobEffect(false, (eMinecraftColour)273);
    effect->setDescriptionId(0xe7478e85);
    effect->setIcon((EMobEffectIcon)19);
    effect->addAttributeModifier(SharedMonsterAttributes::MAX_HEALTH, (eMODIFIER_ID)9, 4.0, 0);
    effect->setBeneficial();
    registerMobEffect(L"health_boost", effect);

    effect = new AbsoptionMobEffect(false, (eMinecraftColour)274);
    effect->setDescriptionId(0x824126cd);
    effect->setIcon((EMobEffectIcon)20);
    effect->setBeneficial();
    registerMobEffect(L"absorption", effect);

    effect = new InstantenousMobEffect(false, (eMinecraftColour)275);
    effect->setDescriptionId(0xe132cfd0);
    effect->setBeneficial();
    registerMobEffect(L"saturation", effect);

    effect = new MobEffect(false, (eMinecraftColour)276);
    effect->setDescriptionId(0x52e542df);
    effect->setIcon((EMobEffectIcon)21);
    registerMobEffect(L"glowing", effect);

    effect = new MobEffect(true, (eMinecraftColour)277);
    effect->setDescriptionId(0x13f37be3);
    effect->setIcon((EMobEffectIcon)22);
    registerMobEffect(L"levitation", effect);

    effect = new MobEffect(false, (eMinecraftColour)278);
    effect->setDescriptionId(0x474605cd);
    effect->setIcon((EMobEffectIcon)23);
    effect->setBeneficial();
    effect->addAttributeModifier(SharedMonsterAttributes::LUCK, (eMODIFIER_ID)14, 1.0, 0);
    registerMobEffect(L"luck", effect);

    effect = new MobEffect(true, (eMinecraftColour)279);
    effect->setDescriptionId(0xa35cc356);
    effect->setIcon((EMobEffectIcon)24);
    effect->addAttributeModifier(SharedMonsterAttributes::LUCK, (eMODIFIER_ID)14, -1.0, 0);
    registerMobEffect(L"unluck", effect);

    effect = new MobEffect(false, (eMinecraftColour)282);
    effect->setDescriptionId(0x92a57635);
    effect->setIcon((EMobEffectIcon)27);
    effect->setBeneficial();
    registerMobEffect(L"conduit_power", effect);

    effect = new MobEffect(false, (eMinecraftColour)281);
    effect->setDescriptionId(0xa7f1591c);
    effect->setIcon((EMobEffectIcon)26);
    effect->setBeneficial();
    registerMobEffect(L"slow_falling", effect);
}
