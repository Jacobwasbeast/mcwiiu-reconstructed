#include "game/world/generation/Villager.hpp"

#include "game/data/CompoundTag.hpp"
#include "game/core/DifficultyInstance.hpp"
#include "game/item/Item.hpp"
#include "game/item/Items.hpp"
#include "game/item/MerchantRecipe.hpp"
#include "game/item/MerchantRecipeList.hpp"
#include "game/util/Random.hpp"

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

void Villager::EnchantBookForEmeralds::addMerchantRecipe(boost::shared_ptr<Merchant>, MerchantRecipeList*, Random*)
{
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
unsigned long long Villager::getPosition() { return 0; }
void Villager::setProfession(int profession) { mProfession = profession; }
void Villager::_init(int profession) { mProfession = profession; mCareer = 0; }
MobGroupData* Villager::finalizeMobSpawn(const DifficultyInstance&, MobGroupData* data, bool, const std::wstring&) { return data; }
void Villager::setRewardPlayersInVillage() { }
unsigned int Villager::countFood(int) { return 0; }
bool Villager::hasExcessFood() { return countFood(0) > 0; }
bool Villager::wantsMoreFood() { return !hasExcessFood(); }
Inventory* Villager::getInventory() { return mInventory; }
void Villager::staticCtor() { }
void Villager::loadPersonalAiGoals() { }
void Villager::grewUp() { AgableMob::grewUp(); }
void Villager::registerAttributes() { }
void Villager::registerGoals() { }
bool Villager::isTrading() { return mTrading; }
void Villager::customServerAiStep() { }
bool Villager::mobInteract(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand) { return AgableMob::mobInteract(player, hand); }
bool Villager::hurt(const DamageSource* source, float amount) { return LivingEntity::hurt(source, amount); }
unsigned int Villager::GetInteractTooltip(const EntityTooltipDataHolder& holder) { return AgableMob::GetInteractTooltip(holder); }
bool Villager::HideUseTooltips() { return false; }
void Villager::defineSynchedData() { AgableMob::defineSynchedData(); }
void Villager::addDataWalkers(DataFixerUpper*) { }
void Villager::addAdditonalSaveData(CompoundTag* tag) { AgableMob::addAdditonalSaveData(tag); }
void Villager::readAdditionalSaveData(CompoundTag* tag) { AgableMob::readAdditionalSaveData(tag); }
bool Villager::removeWhenFarAway() { return false; }
const SoundEvent* Villager::getAmbientSound() { return NULL; }
const SoundEvent* Villager::getHurtSound(const DamageSource*) { return NULL; }
const SoundEvent* Villager::getDeathSound() { return NULL; }
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
const char* Villager::getDefaultLootTable() { return NULL; }
double Villager::getEyeHeight() { return 1.62; }
void Villager::addParticlesAroundSelf(const ParticleType*) { }
void Villager::handleEntityEvent(unsigned char, int) { }
void Villager::finalizeMobSpawn(const DifficultyInstance&, MobGroupData*, const std::wstring&) { }
boost::shared_ptr<AgableMob> Villager::getBreedOffspring(boost::shared_ptr<AgableMob>) { return boost::shared_ptr<AgableMob>(); }
bool Villager::canBeLeashed(const boost::shared_ptr<Player>&) { return !mTrading; }
void Villager::thunderHit(const LightningBolt*) { }
bool Villager::wantsItem(Item*) { return wantsMoreFood(); }
void Villager::pickUpItem(boost::shared_ptr<ItemEntity>) { }
bool Villager::hasFarmSeeds() { return false; }
bool Villager::setSlot(int, const not_null_ptr<ItemInstance>&) { return true; }
unsigned long long Villager::GetType() { return 0; }
