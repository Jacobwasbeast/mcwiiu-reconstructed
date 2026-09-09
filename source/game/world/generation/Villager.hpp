#ifndef GAME_WORLD_GENERATION_VILLAGER_HPP
#define GAME_WORLD_GENERATION_VILLAGER_HPP

#include <string>
#include <boost/shared_ptr.hpp>

#include "game/entity/mob/AgableMob.hpp"
#include "game/core/MapDecoration.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/world/block/BlockPos.hpp"

class CompoundTag;
class DataFixerUpper;
class DamageSource;
class DifficultyInstance;
class EntityTooltipDataHolder;
class Item;
class ItemEntity;
class Inventory;
class Level;
class LightningBolt;
class Merchant;
class MerchantRecipe;
class MerchantRecipeList;
class MobGroupData;
class ParticleType;
class Random;
class LivingEntity;
class Player;
class MapDecoration;
class SoundEvent;

class Villager : public AgableMob
{
public:
    class PriceRange
    {
    public:
        PriceRange();
        PriceRange(int min, int max);
        int getValue(Random* random);
    private:
        int mMin;
        int mMax;
    };

    class EmeraldForItems
    {
    public:
        EmeraldForItems(Item* item, PriceRange price);
        void addMerchantRecipe(boost::shared_ptr<Merchant> merchant, MerchantRecipeList* recipes, Random* random);
    private:
        Item* mItem;
        PriceRange mPrice;
    };

    class ItemForEmeralds
    {
    public:
        ItemForEmeralds(Item* item, const PriceRange& price);
        ItemForEmeralds(const ItemInstance& item, const PriceRange& price);
        void addMerchantRecipe(boost::shared_ptr<Merchant> merchant, MerchantRecipeList* recipes, Random* random);
    private:
        ItemInstance mItem;
        PriceRange mPrice;
    };

    class EnchantedItemForEmeralds
    {
    public:
        EnchantedItemForEmeralds(Item* item, const PriceRange& price);
        void addMerchantRecipe(boost::shared_ptr<Merchant> merchant, MerchantRecipeList* recipes, Random* random);
    private:
        ItemInstance mItem;
        PriceRange mPrice;
    };

    class ItemAndEmeraldToItem
    {
    public:
        ItemAndEmeraldToItem(Item* firstItem, const PriceRange& firstPrice, Item* secondItem, const PriceRange& secondPrice);
        void addMerchantRecipe(boost::shared_ptr<Merchant> merchant, MerchantRecipeList* recipes, Random* random);
    private:
        ItemInstance mFirstItem;
        PriceRange mFirstPrice;
        ItemInstance mSecondItem;
        PriceRange mSecondPrice;
    };

    class EnchantBookForEmeralds
    {
    public:
        EnchantBookForEmeralds();
        void addMerchantRecipe(boost::shared_ptr<Merchant> merchant, MerchantRecipeList* recipes, Random* random);
    };

    class TreasureMapForEmeralds
    {
    public:
        TreasureMapForEmeralds(const PriceRange& price, std::wstring name, const MapDecoration::Type* icon);
        void addMerchantRecipe(boost::shared_ptr<Merchant> merchant, MerchantRecipeList* recipes, Random* random);
    private:
        PriceRange mPrice;
        std::wstring mName;
        const MapDecoration::Type* mIcon;
        int mUnknown;
    };

    Villager(Level* level);
    Villager(Level* level, int profession);
    virtual ~Villager();
    static void create(Level* level);
    int getProfession();
    unsigned int getCareer();
    virtual std::wstring getDisplayName();
    void setTrading(int trading);
    void setTradingPlayer(const boost::shared_ptr<Player>& player);
    boost::shared_ptr<Player> getTradingPlayer();
    void setCareer(int career);
    void increaseMerchantCareer();
    MerchantRecipeList* getOffers(const boost::shared_ptr<Player>& player);
    void overrideOffers(MerchantRecipeList* offers);
    void notifyTrade(MerchantRecipe* recipe);
    void notifyTradeUpdated(const not_null_ptr<ItemInstance>& item);
    virtual bool isAvailableToTrade();
    Level* getLevel();
    BlockPos getPosition();
    void setProfession(int profession);
    void _init(int profession);
    MobGroupData* finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data, bool flag, const std::wstring& name);
    void setRewardPlayersInVillage();
    unsigned int countFood(int amount);
    bool hasExcessFood();
    bool wantsMoreFood();
    Inventory* getInventory();
    static void staticCtor();
    void loadPersonalAiGoals();
    virtual void grewUp();
    void registerAttributes();
    void registerGoals();
    bool isTrading();
    virtual void customServerAiStep();
    virtual bool mobInteract(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand);
    virtual bool hurt(const DamageSource* source, float amount);
    virtual unsigned int GetInteractTooltip(const EntityTooltipDataHolder& holder);
    virtual bool HideUseTooltips();
    virtual void defineSynchedData();
    static void addDataWalkers(DataFixerUpper* fixer);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual bool removeWhenFarAway();
    virtual const SoundEvent* getAmbientSound();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getDeathSound();
    unsigned int getTrading();
    virtual bool isInLove();
    virtual void setInLove(bool value);
    virtual void setChasing(bool value);
    virtual bool isChasing();
    virtual void setLastHurtByMob(boost::shared_ptr<LivingEntity> entity);
    virtual void die(const DamageSource* source);
    virtual void hasEnoughFoodToBreed();
    virtual bool isWillingToBreed(bool value);
    virtual void setWillingToBreed(bool value);
    virtual const char* getDefaultLootTable();
    virtual double getEyeHeight();
    virtual void addParticlesAroundSelf(const ParticleType* particle);
    virtual void handleEntityEvent(unsigned char event, int data);
    virtual void finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data, const std::wstring& name);
    virtual boost::shared_ptr<AgableMob> getBreedOffspring(boost::shared_ptr<AgableMob> partner);
    virtual bool canBeLeashed(const boost::shared_ptr<Player>& player);
    virtual void thunderHit(const LightningBolt* bolt);
    virtual bool wantsItem(Item* item);
    virtual void pickUpItem(boost::shared_ptr<ItemEntity> item);
    virtual bool hasFarmSeeds();
    virtual bool setSlot(int slot, const not_null_ptr<ItemInstance>& item);
    virtual unsigned long long GetType();

private:
    int mProfession;
    int mCareer;
    bool mTrading;
    bool mInLove;
    bool mChasing;
    bool mWillingToBreed;
    MerchantRecipeList* mOffers;
    boost::shared_ptr<Player> mTradingPlayer;
    Level* mVillagerLevel;
    Inventory* mInventory;
};

#endif
