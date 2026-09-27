#include "game/item/PotionUtils.hpp"
#include "game/core/StringIds.hpp"

#include "game/core/HtmlString.hpp"
#include "game/core/L10N.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Potion.hpp"
#include "game/item/Potions.hpp"
#include "game/world/block/DefaultedMappedRegistry.hpp"

const std::wstring PotionUtils::TAG_POTION(L"Potion");
static const std::wstring customEffectsKey(L"CustomPotionEffects");

not_null_ptr<ItemInstance> PotionUtils::setPotion(not_null_ptr<ItemInstance> item, Potion* potion)
{
    ResourceLocation key = Potion::potionRegistry->getKey(potion);
    if (potion == Potions::EMPTY)
    {
        if (item->hasTag())
        {
            CompoundTag* tag = item->getTag();
            tag->remove(TAG_POTION);
            if (tag->isEmpty())
                item->setTag(NULL);
        }
    }
    else
    {
        CompoundTag* tag = item->hasTag() ? item->getTag() : new CompoundTag();
        tag->putString(TAG_POTION, key.toString());
        item->setTag(tag);
    }
    return item;
}

Potion* PotionUtils::getPotion(CompoundTag* tag)
{
    return tag == NULL ? Potion::byId(0) : Potion::byName(tag->getString(TAG_POTION));
}

void PotionUtils::getCustomEffects(CompoundTag* tag, std::vector<MobEffectInstance*>* effects)
{
    if (tag == NULL || effects == NULL || !tag->contains(customEffectsKey, Tag::TAG_LIST)) return;
    ListTag* list = tag->getList(customEffectsKey, Tag::TAG_COMPOUND);
    for (int i = 0; i < list->size(); ++i)
    {
        MobEffectInstance* effect = MobEffectInstance::load(list->getCompound(i));
        if (effect != NULL) effects->push_back(effect);
    }
}

std::vector<MobEffectInstance*>* PotionUtils::getAllEffects(CompoundTag* tag)
{
    std::vector<MobEffectInstance*>* effects = new std::vector<MobEffectInstance*>();
    getAllEffects(getPotion(tag), effects);
    getCustomEffects(tag, effects);
    return effects;
}

std::vector<MobEffectInstance*>* PotionUtils::getMobEffects(not_null_ptr<ItemInstance> item)
{
    return getAllEffects(item->getTag());
}

bool PotionUtils::hasEffect(not_null_ptr<ItemInstance> item, MobEffect* effect)
{
    std::vector<MobEffectInstance*>* effects = getMobEffects(item);
    for (std::vector<MobEffectInstance*>::iterator it = effects->begin(); it != effects->end(); ++it)
    {
        if ((*it)->getEffect() == effect)
        {
            delete effects;
            return true;
        }
    }
    delete effects;
    return false;
}

Potion* PotionUtils::getPotion(not_null_ptr<ItemInstance> item)
{
    return getPotion(item->getTag());
}

void PotionUtils::addPotionTooltip(not_null_ptr<ItemInstance> item, std::vector<HtmlString>* lines, float durationFactor, boost::shared_ptr<Player> player)
{
    std::vector<MobEffectInstance*>* effects = getMobEffects(item);
    if (effects->empty())
    {
        lines->push_back(HtmlString(L10N::GetString(IDS_POTION_EFFECT_EMPTY), eMinecraftColour_Potion_BaseColour, false, false, 0));
    }
    else
    {
        for (std::vector<MobEffectInstance*>::iterator it = effects->begin(); it != effects->end(); ++it)
        {
            MobEffectInstance* effect = *it;
            std::wstring text(L10N::GetString(effect->getDescriptionId()));
            int amplifier = effect->getAmplifier();
            if (amplifier > 0)
            {
                if (amplifier == 1)
                {
                    text += L" ";
                    text += L10N::GetString(0xe0296084);
                }
                else if (amplifier == 2)
                {
                    text += L" ";
                    text += L10N::GetString(0x5230cfbf);
                }
                else if (amplifier == 3)
                {
                    text += L" ";
                    text += L10N::GetString(0x2c2e5556);
                }
                else if (amplifier == 4)
                {
                    text += L" ";
                    text += L10N::GetString(0x6e21f149);
                }
                else if (amplifier == 5)
                {
                    text += L" ";
                    text += L10N::GetString(0x481f76e0);
                }
                else
                {
                    text += L10N::GetString(0x62bdaed);
                }
            }
            if (effect->getDuration() > 20)
            {
                text += L" (";
                text += MobEffect::formatDuration(effect, durationFactor);
                text += L")";
            }
            lines->push_back(HtmlString(text, effect->getEffect()->isHarmful() ? eMinecraftColour_HTMLColor_c : eMinecraftColour_HTMLColor_7, false, false, 0));
        }
    }
    (void)player;
    delete effects;
}

bool PotionUtils::isFoil(not_null_ptr<ItemInstance> item)
{
    if (item->getTag() == NULL || !item->getTag()->contains(TAG_POTION)) return false;
    std::vector<MobEffectInstance*>* effects = getMobEffects(item);
    bool result = !effects->empty();
    delete effects;
    return result;
}

std::vector<MobEffectInstance*>* PotionUtils::getAllEffects(Potion* potion, std::vector<MobEffectInstance*>* effects)
{
    std::vector<MobEffectInstance*>* result = new std::vector<MobEffectInstance*>();
    if (potion != NULL) result->insert(result->end(), potion->getEffects().begin(), potion->getEffects().end());
    if (effects != NULL) result->insert(result->end(), effects->begin(), effects->end());
    return result;
}

std::vector<MobEffectInstance*>* PotionUtils::getCustomEffects(CompoundTag* tag)
{
    std::vector<MobEffectInstance*>* effects = new std::vector<MobEffectInstance*>();
    getCustomEffects(tag, effects);
    return effects;
}

std::vector<MobEffectInstance*>* PotionUtils::getCustomEffects(not_null_ptr<ItemInstance> item)
{
    return getCustomEffects(item->getTag());
}

int PotionUtils::getColor(std::vector<MobEffectInstance*>* effects)
{
    if (effects == NULL || effects->empty()) return 0x385dc6;
    return MobEffectInstance::getColorValue(effects);
}

int PotionUtils::getColor(const not_null_ptr<ItemInstance>& item)
{
    CompoundTag* tag = item->getTag();
    if (tag != NULL && tag->contains(L"CustomPotionColor", Tag::TAG_ANY_NUMERIC)) return tag->getInt(L"CustomPotionColor");
    if (getPotion(item) == Potion::byId(0)) return 0xf800f8;
    std::vector<MobEffectInstance*>* effects = getMobEffects(item);
    int color = getColor(effects);
    delete effects;
    return color;
}

int PotionUtils::getColor(Potion* potion)
{
    if (potion == Potion::byId(0)) return 0xf800f8;
    return getColor(&potion->getEffects());
}

std::wstring PotionUtils::getPotionID(not_null_ptr<ItemInstance> item)
{
    CompoundTag* tag = item->getTag();
    if (tag == NULL)
        return L"";
    std::wstring name = tag->getString(TAG_POTION);
    std::wstring::size_type separator = name.find(L":");
    return std::wstring(name, separator + 1, std::wstring::npos);
}

not_null_ptr<ItemInstance> PotionUtils::setCustomEffects(not_null_ptr<ItemInstance> item, std::vector<MobEffectInstance*>* effects)
{
    if (effects == NULL || effects->empty()) return item;
    CompoundTag* tag = item->getTag() == NULL ? new CompoundTag() : item->getTag()->copy();
    ListTag* list = tag->getList(customEffectsKey, Tag::TAG_LIST);
    for (std::vector<MobEffectInstance*>::iterator it = effects->begin(); it != effects->end(); ++it)
        list->add((*it)->save(new CompoundTag()));
    tag->put(customEffectsKey, list);
    item->setTag(tag);
    return item;
}

not_null_ptr<ItemInstance> PotionUtils::setPotionAndCustomEffects(not_null_ptr<ItemInstance> target, not_null_ptr<ItemInstance> source)
{
    target = setPotion(target, getPotion(source));
    return setCustomEffects(target, getCustomEffects(source));
}

void PotionUtils::setPotionName(not_null_ptr<ItemInstance> item, const std::wstring& name)
{
    CompoundTag* tag = item->getTag() == NULL ? new CompoundTag() : item->getTag()->copy();
    tag->putString(TAG_POTION, name);
    item->setTag(tag);
}
