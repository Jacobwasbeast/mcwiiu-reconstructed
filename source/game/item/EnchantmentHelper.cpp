#include "game/item/EnchantmentHelper.hpp"
#include "game/item/Enchantment.hpp"
#include "game/item/EnchantmentInstance.hpp"
#include "game/item/Enchantments.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/EnchantedBookItem.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/network/ClientboundChatPacket.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/Mth.hpp"
#include "game/util/Math.hpp"
#include "game/util/Random.hpp"
#include "game/util/WeighedRandom.hpp"

EnchantmentHelper::EnchantmentIterationMethod::~EnchantmentIterationMethod() {}

EnchantmentHelper::GetDamageProtectionIteration::GetDamageProtectionIteration(int* protection, const DamageSource* source) : mProtection(protection), mSource(source) {}
EnchantmentHelper::GetDamageProtectionIteration::~GetDamageProtectionIteration() {}
void EnchantmentHelper::GetDamageProtectionIteration::doEnchantment(Enchantment* enchantment, int level) { *mProtection += enchantment->getDamageProtection(level, mSource); }

EnchantmentHelper::GetDamageBonusIteration::GetDamageBonusIteration(float* bonus, const boost::shared_ptr<LivingEntity>& target) : mBonus(bonus), mTarget(target) {}
EnchantmentHelper::GetDamageBonusIteration::~GetDamageBonusIteration() {}
void EnchantmentHelper::GetDamageBonusIteration::doEnchantment(Enchantment* enchantment, int level) { *mBonus += enchantment->getDamageBonus(level, mTarget); }

EnchantmentHelper::DoPostHurtIteration::DoPostHurtIteration(const boost::shared_ptr<LivingEntity>& target, const boost::shared_ptr<Entity>& attacker, int level) : mTarget(target), mAttacker(attacker), mLevel(level) {}
EnchantmentHelper::DoPostHurtIteration::~DoPostHurtIteration() {}
void EnchantmentHelper::DoPostHurtIteration::doEnchantment(Enchantment* enchantment, int level) { enchantment->doPostHurt(mTarget, mAttacker, level); }

EnchantmentHelper::DoPostDamageIteration::DoPostDamageIteration(const boost::shared_ptr<LivingEntity>& attacker, const boost::shared_ptr<Entity>& target, int level) : mAttacker(attacker), mTarget(target), mLevel(level) {}
EnchantmentHelper::DoPostDamageIteration::~DoPostDamageIteration() {}
void EnchantmentHelper::DoPostDamageIteration::doEnchantment(Enchantment* enchantment, int level) { enchantment->doPostAttack(mAttacker, mTarget, level); }

int EnchantmentHelper::getItemEnchantmentLevel(const Enchantment* enchantment, const not_null_ptr<ItemInstance>& item)
{
    boost::unordered_map<Enchantment*, int>* enchantments = getEnchantments(item);
    if (enchantments == NULL) return 0;
    boost::unordered_map<Enchantment*, int>::const_iterator it = enchantments->find(const_cast<Enchantment*>(enchantment));
    return it == enchantments->end() ? 0 : it->second;
}

boost::unordered_map<Enchantment*, int>* EnchantmentHelper::getEnchantments(const not_null_ptr<ItemInstance>& item)
{
    boost::unordered_map<Enchantment*, int>* result = new boost::unordered_map<Enchantment*, int>();
    Tag* tags = item->getEnchantmentTags();
    if (tags == NULL) return result;
    ListTag* list = static_cast<ListTag*>(tags);
    for (int i = 0; i < list->size(); ++i)
    {
        CompoundTag* tag = list->getCompound(i);
        Enchantment* enchantment = Enchantment::byId(tag->getShort(L"id"));
        if (enchantment != NULL) (*result)[enchantment] = tag->getShort(L"lvl");
    }
    return result;
}

void EnchantmentHelper::setEnchantments(boost::unordered_map<Enchantment*, int>* enchantments, const not_null_ptr<ItemInstance>& item)
{
    ListTag* list = new ListTag();
    if (enchantments != NULL)
    {
        boost::unordered_map<Enchantment*, int>::const_iterator it;
        for (it = enchantments->begin(); it != enchantments->end(); ++it)
        {
            CompoundTag* tag = new CompoundTag();
            tag->putShort(L"id", static_cast<short>(Enchantment::getId(it->first)));
            tag->putShort(L"lvl", static_cast<short>(it->second));
            list->add(tag);
        }
    }
    if (!list->isEmpty()) item->addTagElement(L"ench", list);
    else if (item->hasTag()) item->getTag()->remove(L"ench");
}

void EnchantmentHelper::runIterationOnItem(EnchantmentIterationMethod& method, const not_null_ptr<ItemInstance>& item)
{
    boost::unordered_map<Enchantment*, int>* enchantments = getEnchantments(item);
    boost::unordered_map<Enchantment*, int>::const_iterator it;
    for (it = enchantments->begin(); it != enchantments->end(); ++it) method.doEnchantment(it->first, it->second);
    delete enchantments;
}

void EnchantmentHelper::runIterationOnInventory(EnchantmentIterationMethod& method, NonNullList<not_null_ptr<ItemInstance> > items)
{ for (unsigned int i = 0; i < items.size(); ++i) runIterationOnItem(method, items[i]); }

int EnchantmentHelper::getDamageProtection(NonNullList<not_null_ptr<ItemInstance> > items, const DamageSource* source)
{ int result = 0; GetDamageProtectionIteration method(&result, source); runIterationOnInventory(method, items); return result; }

double EnchantmentHelper::getDamageBonus(const not_null_ptr<ItemInstance>& item, const boost::shared_ptr<LivingEntity>& entity)
{ float result = 0.0f; GetDamageBonusIteration method(&result, entity); runIterationOnItem(method, item); return result; }

void EnchantmentHelper::doPostHurtEffects(const boost::shared_ptr<LivingEntity>& target, const boost::shared_ptr<Entity>& attacker)
{
    DoPostHurtIteration method(target, attacker, 0);
    std::vector<boost::shared_ptr<ItemInstance> >* items = target->getAllSlots();
    if (items != NULL)
    {
        for (std::vector<boost::shared_ptr<ItemInstance> >::iterator it = items->begin(); it != items->end(); ++it)
            if (it->get() != NULL) runIterationOnItem(method, not_null_ptr<ItemInstance>(it->get()));
    }
    delete items;
}
void EnchantmentHelper::doPostDamageEffects(const boost::shared_ptr<LivingEntity>& attacker, const boost::shared_ptr<Entity>& target)
{
    DoPostDamageIteration method(attacker, target, 0);
    std::vector<boost::shared_ptr<ItemInstance> >* items = attacker->getAllSlots();
    if (items != NULL)
    {
        for (std::vector<boost::shared_ptr<ItemInstance> >::iterator it = items->begin(); it != items->end(); ++it)
            if (it->get() != NULL) runIterationOnItem(method, not_null_ptr<ItemInstance>(it->get()));
    }
    delete items;
}

int EnchantmentHelper::getKnockbackBonus(const boost::shared_ptr<LivingEntity>& attacker, const boost::shared_ptr<LivingEntity>& target) { return getEnchantmentLevel(Enchantments::KNOCKBACK, attacker); }
int EnchantmentHelper::getFireAspect(const boost::shared_ptr<LivingEntity>& entity) { return getEnchantmentLevel(Enchantments::FIRE_ASPECT, entity); }
int EnchantmentHelper::getRespiration(const boost::shared_ptr<LivingEntity>& entity) { return getEnchantmentLevel(Enchantments::RESPIRATION, entity); }
int EnchantmentHelper::getDepthStrider(const boost::shared_ptr<LivingEntity>& entity) { return getEnchantmentLevel(Enchantments::DEPTH_STRIDER, entity); }
int EnchantmentHelper::getBlockEfficiency(const boost::shared_ptr<LivingEntity>& entity) { return getEnchantmentLevel(Enchantments::BLOCK_EFFICIENCY, entity); }
int EnchantmentHelper::getFishingLuckBonus(const not_null_ptr<ItemInstance>& item) { return getItemEnchantmentLevel(Enchantments::FISHING_LUCK, item); }
int EnchantmentHelper::getFishingSpeedBonus(const not_null_ptr<ItemInstance>& item) { return getItemEnchantmentLevel(Enchantments::FISHING_SPEED, item); }
int EnchantmentHelper::getMobLooting(const boost::shared_ptr<LivingEntity>& entity) { return getEnchantmentLevel(Enchantments::MOB_LOOTING, entity); }
bool EnchantmentHelper::hasAquaAffinity(const boost::shared_ptr<LivingEntity>& entity) { return getEnchantmentLevel(Enchantments::AQUA_AFFINITY, entity) > 0; }
bool EnchantmentHelper::hasFrostWalker(const boost::shared_ptr<LivingEntity>& entity) { return getEnchantmentLevel(Enchantments::FROST_WALKER, entity) > 0; }
bool EnchantmentHelper::hasBindingCurse(const not_null_ptr<ItemInstance>& item) { return getItemEnchantmentLevel(Enchantments::BINDING_CURSE, item) > 0; }
bool EnchantmentHelper::hasVanishingCurse(const not_null_ptr<ItemInstance>& item) { return getItemEnchantmentLevel(Enchantments::VANISHING_CURSE, item) > 0; }
bool EnchantmentHelper::hasChanneling(const not_null_ptr<ItemInstance>& item) { return getItemEnchantmentLevel(Enchantments::CHANNELING, item) > 0; }

boost::shared_ptr<ItemInstance> EnchantmentHelper::getRandomItemWith(Enchantment* enchantment, const boost::shared_ptr<LivingEntity>& entity)
{
    std::vector<boost::weak_ptr<ItemInstance> > items = enchantment->getSlotItems(entity);
    std::vector<boost::weak_ptr<ItemInstance> >::iterator it;
    for (it = items.begin(); it != items.end(); ++it)
    {
        boost::shared_ptr<ItemInstance> item = it->lock();
        if (item && getItemEnchantmentLevel(enchantment, not_null_ptr<ItemInstance>(item.get())) > 0) return item;
    }
    return boost::shared_ptr<ItemInstance>();
}

int EnchantmentHelper::getEnchantmentLevel(Enchantment* enchantment, const boost::shared_ptr<LivingEntity>& entity)
{
    int level = 0;
    if (entity)
    {
        std::vector<boost::shared_ptr<ItemInstance> >* items = entity->getAllSlots();
        if (items != NULL)
        {
            for (std::vector<boost::shared_ptr<ItemInstance> >::iterator it = items->begin(); it != items->end(); ++it)
                if (it->get() != NULL) level = Math::_max(level, getItemEnchantmentLevel(enchantment, not_null_ptr<ItemInstance>(it->get())));
        }
        delete items;
    }
    return level;
}

int EnchantmentHelper::getEnchantmentCost(Random* random, int slot, int level, const not_null_ptr<ItemInstance>& item)
{ if (item->getItem()->getMaxStackSize() < 1) return 0; if (level > 15) level = 15; int value = random->nextInt(8) + 1 + (level >> 1) + random->nextInt(level + 1); if (slot == 0) return Math::_max(value / 3, 1); if (slot == 1) return value * 2 / 3 + 1; return Math::_max(level * 2, value); }

std::vector<EnchantmentInstance*>* EnchantmentHelper::getAvailableEnchantmentResults(int cost, const not_null_ptr<ItemInstance>& item, bool treasure)
{
    std::vector<EnchantmentInstance*>* result = new std::vector<EnchantmentInstance*>();
    for (int id = 0; id < 256; ++id)
    {
        Enchantment* enchantment = Enchantment::byId(id);
        if (enchantment == NULL || (!treasure && enchantment->isTreasureOnly()) || !enchantment->canEnchant(item)) continue;
        for (int level = enchantment->getMaxLevel(); level >= enchantment->getMinLevel(); --level)
            if (cost >= enchantment->getMinCost(level) && cost <= enchantment->getMaxCost(level)) { result->push_back(new EnchantmentInstance(enchantment, level)); break; }
    }
    return result;
}
void EnchantmentHelper::filterCompatibleEnchantments(std::vector<EnchantmentInstance*>* enchantments, const EnchantmentInstance* selected) { std::vector<EnchantmentInstance*>::iterator it = enchantments->begin(); while (it != enchantments->end()) { if (!(*it)->enchantment->isCompatibleWith(selected->enchantment)) { delete *it; it = enchantments->erase(it); } else ++it; } }
std::vector<EnchantmentInstance*>* EnchantmentHelper::selectEnchantment(Random* random, const not_null_ptr<ItemInstance>& item, int level, bool treasure) { return getAvailableEnchantmentResults(level, item, treasure); }
void EnchantmentHelper::enchantItem(Random* random, not_null_ptr<ItemInstance> item, int level, bool treasure) { std::vector<EnchantmentInstance*>* list = selectEnchantment(random, item, level, treasure); if (list != NULL) { for (std::vector<EnchantmentInstance*>::iterator it = list->begin(); it != list->end(); ++it) { if (item->getItem() != NULL) item->enchant((*it)->enchantment, (*it)->level); delete *it; } delete list; } }
