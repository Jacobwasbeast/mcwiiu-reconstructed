#include "game/world/generation/Villager.hpp"

#include "game/data/CompoundTag.hpp"
#include "game/core/DifficultyInstance.hpp"
#include "game/core/BaseAttributeMap.hpp"
#include "game/item/Item.hpp"
#include "game/item/Items.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/MerchantRecipe.hpp"
#include "game/item/MerchantRecipeList.hpp"
#include "game/item/Enchantment.hpp"
#include "game/item/EnchantmentInstance.hpp"
#include "game/item/EnchantedBookItem.hpp"
#include "game/world/block/MappedRegistry.hpp"
#include "game/core/Mth.hpp"
#include "game/util/Random.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/item/ItemListDataWalker.hpp"
#include "game/core/UniqueTypeWalker.hpp"
#include "game/entity/mob/EntityIO.hpp"
#include "game/data/DataFixerUpper.hpp"

Villager::PriceRange::PriceRange() : mMin(0), mMax(0)
{
}

Villager::PriceRange::PriceRange(int min, int max) : mMin(min), mMax(max)
{
}

int Villager::PriceRange::getValue(Random* random)
{
    if (mMin >= mMax) return mMin;
    return random->nextInt(mMin, mMax);
}

Villager::EmeraldForItems::EmeraldForItems(Item* item, PriceRange price) : mItem(item), mPrice(price)
{
}

void Villager::EmeraldForItems::addMerchantRecipe(boost::shared_ptr<Merchant>, MerchantRecipeList* recipes, Random* random)
{
    not_null_ptr<ItemInstance> buy(new ItemInstance(mItem));
    not_null_ptr<ItemInstance> emerald(new ItemInstance(Items::emerald));
    buy->setCount(mPrice.getValue(random));
    recipes->push_back(new MerchantRecipe(emerald, buy));
}

Villager::ItemForEmeralds::ItemForEmeralds(Item* item, const PriceRange& price) : mItem(item), mPrice(price)
{
}

Villager::ItemForEmeralds::ItemForEmeralds(const ItemInstance& item, const PriceRange& price) : mItem(item), mPrice(price)
{
}

void Villager::ItemForEmeralds::addMerchantRecipe(boost::shared_ptr<Merchant>, MerchantRecipeList* recipes, Random* random)
{
    not_null_ptr<ItemInstance> emerald(new ItemInstance(Items::emerald));
    not_null_ptr<ItemInstance> sell = mItem.copy();
    sell->setCount(mPrice.getValue(random));
    recipes->push_back(new MerchantRecipe(emerald, sell));
}

Villager::EnchantedItemForEmeralds::EnchantedItemForEmeralds(Item* item, const PriceRange& price) : mItem(item), mPrice(price)
{
}

void Villager::EnchantedItemForEmeralds::addMerchantRecipe(boost::shared_ptr<Merchant>, MerchantRecipeList* recipes, Random* random)
{
    not_null_ptr<ItemInstance> emerald(new ItemInstance(Items::emerald));
    not_null_ptr<ItemInstance> sell = mItem.copy();
    sell->setCount(1);
    recipes->push_back(new MerchantRecipe(emerald, sell));
    (void)random;
}

Villager::ItemAndEmeraldToItem::ItemAndEmeraldToItem(Item* firstItem, const PriceRange& firstPrice, Item* secondItem, const PriceRange& secondPrice)
    : mFirstItem(firstItem), mFirstPrice(firstPrice), mSecondItem(secondItem), mSecondPrice(secondPrice)
{
}

void Villager::ItemAndEmeraldToItem::addMerchantRecipe(boost::shared_ptr<Merchant>, MerchantRecipeList* recipes, Random* random)
{
    not_null_ptr<ItemInstance> first = mFirstItem.copy();
    not_null_ptr<ItemInstance> second = mSecondItem.copy();
    not_null_ptr<ItemInstance> sell(new ItemInstance(Items::emerald));
    first->setCount(mFirstPrice.getValue(random));
    second->setCount(mSecondPrice.getValue(random));
    recipes->push_back(new MerchantRecipe(first, second, sell, 12, 1));
}

Villager::EnchantBookForEmeralds::EnchantBookForEmeralds()
{
}

void Villager::EnchantBookForEmeralds::addMerchantRecipe(boost::shared_ptr<Merchant>, MerchantRecipeList* recipes, Random* random)
{
    Enchantment* enchantment = Enchantment::enchantmentRegistry->getRandom(random);
    int level = Mth::nextInt(random, enchantment->getMinLevel(), enchantment->getMaxLevel());
    EnchantmentInstance enchantmentInstance(enchantment, level);
    not_null_ptr<ItemInstance> enchantedBook = EnchantedBookItem::createForEnchantment(&enchantmentInstance);
    int emeraldCount = random->nextInt(level * 10 + 5) + 2 + level * 3;
    if (enchantment->isTreasureOnly())
        emeraldCount *= 2;
    if (emeraldCount > 64)
        emeraldCount = 64;
    not_null_ptr<ItemInstance> book(new ItemInstance(Items::book));
    not_null_ptr<ItemInstance> emerald(new ItemInstance(Items::emerald, emeraldCount));
    recipes->push_back(new MerchantRecipe(book, emerald, enchantedBook, 0, 7));
}

Villager::TreasureMapForEmeralds::TreasureMapForEmeralds(const PriceRange& price, std::wstring name, const MapDecoration::Type* icon)
    : mPrice(price), mName(name), mIcon(icon), mUnknown(0)
{
}

void Villager::TreasureMapForEmeralds::addMerchantRecipe(boost::shared_ptr<Merchant>, MerchantRecipeList*, Random*)
{
}

Villager::Villager(Level* level)
    : AgableMob(level), mProfession(0), mCareer(0), mTrading(false), mInLove(false), mChasing(false),
      mWillingToBreed(false), mOffers(NULL), mVillagerLevel(level)
{
}

Villager::~Villager()
{
    delete mOffers;
}

void Villager::create(Level* level)
{
    new Villager(level);
}

int Villager::getProfession() { return mProfession; }
unsigned int Villager::getCareer() { return mCareer; }
std::wstring Villager::getDisplayName() { return std::wstring(); }
void Villager::setTrading(int trading) { mTrading = trading != 0; }
void Villager::setTradingPlayer(const boost::shared_ptr<Player>& player) { mTradingPlayer = player; }
boost::shared_ptr<Player> Villager::getTradingPlayer() { return mTradingPlayer; }
void Villager::setCareer(int career) { mCareer = career; }
void Villager::increaseMerchantCareer() { ++mCareer; }
MerchantRecipeList* Villager::getOffers(const boost::shared_ptr<Player>&) { return mOffers; }
void Villager::overrideOffers(MerchantRecipeList* offers) { if (mOffers != offers) { delete mOffers; mOffers = offers; } }
void Villager::notifyTrade(MerchantRecipe* recipe) { if (recipe != NULL) recipe->increaseUses(); }
void Villager::notifyTradeUpdated(const not_null_ptr<ItemInstance>&) { }
bool Villager::isAvailableToTrade() { return !mTrading; }
Level* Villager::getLevel() { return mVillagerLevel; }
BlockPos Villager::getPosition() { return BlockPos(this); }
void Villager::setProfession(int profession) { mProfession = profession; }
void Villager::_init(int profession) { mProfession = profession; mCareer = 0; }
MobGroupData* Villager::finalizeMobSpawn(const DifficultyInstance&, MobGroupData* data, bool, const std::wstring&) { return data; }
void Villager::setRewardPlayersInVillage() { }
unsigned int Villager::countFood(int multiplier)
{
    getProfession();
    if (mInventory == NULL)
        return 0;
    for (unsigned int slot = 0; slot < mInventory->getContainerSize(); ++slot)
    {
        not_null_ptr<ItemInstance> item = mInventory->getItem(slot);
        if (item->isEmpty())
            continue;
        int required = 0;
        if (item->getItem() == Items::bread)
            required = multiplier * 3;
        else if (item->getItem() == Items::potato || item->getItem() == Items::carrot ||
                 item->getItem() == Items::beetroot)
            required = multiplier * 12;
        else if (item->getItem() == Items::wheat)
            required = multiplier * 9;
        if (required != 0 && item->getCount() >= required)
            return 1;
        if (required == 0 && (item->getItem() == Items::bread || item->getItem() == Items::potato ||
                              item->getItem() == Items::carrot || item->getItem() == Items::beetroot ||
                              item->getItem() == Items::wheat))
            return 1;
    }
    return 0;
}
bool Villager::hasExcessFood() { return countFood(0) > 0; }
bool Villager::wantsMoreFood() { return !hasExcessFood(); }
Inventory* Villager::getInventory() { return mInventory; }
void Villager::staticCtor() { }
void Villager::loadPersonalAiGoals() { }
void Villager::grewUp() { AgableMob::grewUp(); }
void Villager::registerAttributes()
{
    Mob::registerAttributes();
    getAttributes()->getInstance(eATTRIBUTE_ID_MovementSpeed)->setBaseValue(0.5);
}
void Villager::registerGoals() { }
bool Villager::isTrading() { return mTrading; }
void Villager::customServerAiStep()
{
    if (mTrading && mTradingPlayer.get() == NULL)
        mTrading = false;
}
bool Villager::mobInteract(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand) { return AgableMob::mobInteract(player, hand); }
bool Villager::hurt(const DamageSource* source, float amount) { return LivingEntity::hurt(source, amount); }
unsigned int Villager::GetInteractTooltip(const EntityTooltipDataHolder& holder) { return AgableMob::GetInteractTooltip(holder); }
bool Villager::HideUseTooltips() { return getProfession() == 5; }
void Villager::defineSynchedData() { AgableMob::defineSynchedData(); }
void Villager::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer, ENTITY_TYPE_VILLAGER);
    fixer->addWalker(DataFixTypes::eDataFixTypes_Entity,
                     (DataWalker*)new ItemListDataWalker(ENTITY_TYPE_VILLAGER, L"Inventory"));
    fixer->addWalker(DataFixTypes::eDataFixTypes_Entity,
                     (DataWalker*)new UniqueTypeWalker(EntityIO::getKey(ENTITY_TYPE_VILLAGER)));
}
void Villager::addAdditonalSaveData(CompoundTag* tag) { AgableMob::addAdditonalSaveData(tag); }
void Villager::readAdditionalSaveData(CompoundTag* tag) { AgableMob::readAdditionalSaveData(tag); }
bool Villager::removeWhenFarAway() { return false; }
const SoundEvent* Villager::getAmbientSound()
{
    return isTrading() ? SoundEvents::VILLAGER_TRADING : SoundEvents::VILLAGER_AMBIENT;
}
const SoundEvent* Villager::getHurtSound(const DamageSource*) { return SoundEvents::VILLAGER_HURT; }
const SoundEvent* Villager::getDeathSound() { return SoundEvents::VILLAGER_DEATH; }
unsigned int Villager::getTrading() { return mTrading ? 1U : 0U; }
bool Villager::isInLove() { return mInLove; }
void Villager::setInLove(bool value) { mInLove = value; }
void Villager::setChasing(bool value) { mChasing = value; }
bool Villager::isChasing() { return mChasing; }
void Villager::setLastHurtByMob(boost::shared_ptr<LivingEntity>) { }
void Villager::die(const DamageSource*) { }
void Villager::hasEnoughFoodToBreed() { }
bool Villager::isWillingToBreed(bool value) { return value && mWillingToBreed; }
void Villager::setWillingToBreed(bool value) { mWillingToBreed = value; }
const char* Villager::getDefaultLootTable() { return reinterpret_cast<const char*>(&BuiltInLootTables::VILLAGER); }
double Villager::getEyeHeight() { return 1.62; }
void Villager::addParticlesAroundSelf(const ParticleType*) { }
void Villager::handleEntityEvent(unsigned char event, int data)
{
    if (event == 12)
    {
        addParticlesAroundSelf(PARTICLE_ANGRY_VILLAGER);
        return;
    }
    if (event == 13)
    {
        addParticlesAroundSelf(PARTICLE_HAPPY_VILLAGER);
        return;
    }
    if (event == 14)
    {
        addParticlesAroundSelf(PARTICLE_HEART);
        return;
    }
    Mob::handleEntityEvent(event, data);
}
void Villager::finalizeMobSpawn(const DifficultyInstance&, MobGroupData*, const std::wstring&) { }
boost::shared_ptr<AgableMob> Villager::getBreedOffspring(boost::shared_ptr<AgableMob>) { return boost::shared_ptr<AgableMob>(); }
bool Villager::canBeLeashed(const boost::shared_ptr<Player>&) { return !mTrading; }
void Villager::thunderHit(const LightningBolt* bolt)
{
    Entity::thunderHit(bolt);
}
bool Villager::wantsItem(Item* item)
{
    return item == Items::wheat || item == Items::wheatSeeds || item == Items::bread ||
           item == Items::carrot || item == Items::potato || item == Items::beetroot ||
           item == Items::beetrootSeeds;
}
void Villager::pickUpItem(boost::shared_ptr<ItemEntity>) { }
bool Villager::hasFarmSeeds()
{
    if (mInventory == NULL)
        return false;
    for (unsigned int slot = 0; slot < mInventory->getContainerSize(); ++slot)
    {
        not_null_ptr<ItemInstance> item = mInventory->getItem(slot);
        if (item->isEmpty())
            continue;
        if (item->getItem() == Items::carrot || item->getItem() == Items::potato)
            return true;
    }
    return false;
}
bool Villager::setSlot(int slot, const not_null_ptr<ItemInstance>& item)
{
    if (Mob::setSlot(slot, item))
        return true;
    const int inventorySlot = slot - 300;
    if (inventorySlot < 0 || mInventory == NULL ||
        static_cast<unsigned int>(inventorySlot) >= mInventory->getContainerSize())
        return false;
    mInventory->setItem(static_cast<unsigned int>(inventorySlot), item);
    return true;
}
unsigned long long Villager::GetType() { return ENTITY_TYPE_VILLAGER; }
