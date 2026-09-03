#include "game/item/PotionUtils.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/entity/mob/MobEffect.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Potion.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/core/HtmlString.hpp"

static const std::wstring potionKey(L"Potion");
static const std::wstring effectsKey(L"CustomPotionEffects");

not_null_ptr<ItemInstance> PotionUtils::setPotion(not_null_ptr<ItemInstance> item, Potion* potion)
{
    CompoundTag* tag = item->getTag();
    if (potion == NULL)
    {
        if (tag != NULL)
        {
            tag->remove(potionKey);
            if (tag->isEmpty()) item->setTag(NULL);
        }
    }
    else
    {
        if (tag == NULL) tag = new CompoundTag();
        tag->putString(potionKey, potion->getName(Potion::NORMAL));
        item->setTag(tag);
    }
    return item;
}

Potion* PotionUtils::getPotion(CompoundTag* tag)
{
    if (tag == NULL) return Potion::byId(0);
    return Potion::byName(tag->getString(potionKey));
}

void PotionUtils::getCustomEffects(CompoundTag* tag, std::vector<MobEffectInstance*>* effects)
{
    if (tag == NULL || effects == NULL || !tag->contains(effectsKey, 9)) return;
    ListTag* list = tag->getList(effectsKey, 10);
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
    bool found = false;
    for (std::vector<MobEffectInstance*>::iterator it = effects->begin(); it != effects->end(); ++it)
        if ((*it)->getEffect() == effect) found = true;
    delete effects;
    return found;
}

Potion* PotionUtils::getPotion(not_null_ptr<ItemInstance> item)
{
    return getPotion(item->getTag());
}

void PotionUtils::addPotionTooltip(not_null_ptr<ItemInstance> item, std::vector<HtmlString>* lines, float durationFactor, boost::shared_ptr<Player> player)
{
    std::vector<MobEffectInstance*>* effects = getMobEffects(item);
    if (lines != NULL) { (void)durationFactor; (void)player; }
    delete effects;
}

bool PotionUtils::isFoil(not_null_ptr<ItemInstance> item)
{
    std::vector<MobEffectInstance*>* effects = getMobEffects(item);\n    bool result = item->hasTag() && item->getTag()->contains(potionKey) && !effects->empty();\n    delete effects;\n    return result;
}

std::vector<MobEffectInstance*>* PotionUtils::getAllEffects(Potion* potion, std::vector<MobEffectInstance*>* effects)
{
    std::vector<MobEffectInstance*>* result = new std::vector<MobEffectInstance*>();
    if (potion != NULL) result->insert(result->end(), potion->getEffects()->begin(), potion->getEffects()->end());
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
    if (effects == NULL || effects->empty()) return 0;
    return MobEffectInstance::getColorValue(effects);
}

int PotionUtils::getColor(const not_null_ptr<ItemInstance>& item)
{
    return getColor(getMobEffects(item));
}

int PotionUtils::getColor(Potion* potion)
{
    return getColor(getAllEffects(potion, NULL));
}

ResourceLocation PotionUtils::getPotionID(not_null_ptr<ItemInstance> item)
{
    std::wstring name = item->hasTag() ? item->getTag()->getString(potionKey) : std::wstring();
    std::wstring::size_type pos = name.find(L':');
    if (pos != std::wstring::npos) name = name.substr(pos + 1);
    return ResourceLocation(name);
}

not_null_ptr<ItemInstance> PotionUtils::setCustomEffects(not_null_ptr<ItemInstance> item, std::vector<MobEffectInstance*>* effects)
{
    CompoundTag* tag = item->getTag();
    if (tag == NULL) tag = new CompoundTag();
    ListTag* list = tag->getList(effectsKey, 9);
    if (effects != NULL)
        for (std::vector<MobEffectInstance*>::iterator it = effects->begin(); it != effects->end(); ++it)
            list->add((*it)->save(new CompoundTag()));
    tag->put(effectsKey, list);
    item->setTag(tag);
    return item;
}

not_null_ptr<ItemInstance> PotionUtils::setPotionAndCustomEffects(not_null_ptr<ItemInstance> target, not_null_ptr<ItemInstance> source)
{
    setPotion(target, getPotion(source));
    return setCustomEffects(target, getCustomEffects(source));
}

void PotionUtils::setPotionName(not_null_ptr<ItemInstance> item, const std::wstring& name)
{
    CompoundTag* tag = item->getTag();
    if (tag == NULL) tag = new CompoundTag();
    tag->putString(potionKey, name);
    item->setTag(tag);
}
