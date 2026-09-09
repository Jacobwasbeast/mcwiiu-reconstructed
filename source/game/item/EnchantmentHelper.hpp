#ifndef GAME_ITEM_ENCHANTMENTHELPER_HPP
#define GAME_ITEM_ENCHANTMENTHELPER_HPP

#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

#include "game/item/not_null_ptr.hpp"
#include "game/item/NonNullList.hpp"

class DamageSource;
class Enchantment;
class EnchantmentInstance;
class ItemInstance;
class LivingEntity;
class Entity;
class Random;

class EnchantmentHelper
{
public:
    class EnchantmentIterationMethod
    {
    public:
        virtual ~EnchantmentIterationMethod();
        virtual void doEnchantment(Enchantment* enchantment, int level) = 0;
    };

    class GetDamageProtectionIteration : public EnchantmentIterationMethod
    {
    public:
        GetDamageProtectionIteration(int* protection, const DamageSource* source);
        virtual ~GetDamageProtectionIteration();
        virtual void doEnchantment(Enchantment* enchantment, int level);
    private:
        int* mProtection;
        const DamageSource* mSource;
    };

    class GetDamageBonusIteration : public EnchantmentIterationMethod
    {
    public:
        GetDamageBonusIteration(float* bonus, const boost::shared_ptr<LivingEntity>& target);
        virtual ~GetDamageBonusIteration();
        virtual void doEnchantment(Enchantment* enchantment, int level);
    private:
        float* mBonus;
        boost::shared_ptr<LivingEntity> mTarget;
    };

    class DoPostHurtIteration : public EnchantmentIterationMethod
    {
    public:
        DoPostHurtIteration(const boost::shared_ptr<LivingEntity>& target,
                            const boost::shared_ptr<Entity>& attacker,
                            int level);
        virtual ~DoPostHurtIteration();
        virtual void doEnchantment(Enchantment* enchantment, int level);
    private:
        boost::shared_ptr<LivingEntity> mTarget;
        boost::shared_ptr<Entity> mAttacker;
        int mLevel;
    };

    class DoPostDamageIteration : public EnchantmentIterationMethod
    {
    public:
        DoPostDamageIteration(const boost::shared_ptr<LivingEntity>& attacker,
                              const boost::shared_ptr<Entity>& target,
                              int level);
        virtual ~DoPostDamageIteration();
        virtual void doEnchantment(Enchantment* enchantment, int level);
    private:
        boost::shared_ptr<LivingEntity> mAttacker;
        boost::shared_ptr<Entity> mTarget;
        int mLevel;
    };

    static int getItemEnchantmentLevel(const Enchantment* enchantment, const not_null_ptr<ItemInstance>& item);
    static boost::unordered_map<Enchantment*, int>* getEnchantments(const not_null_ptr<ItemInstance>& item);
    static void setEnchantments(boost::unordered_map<Enchantment*, int>* enchantments, const not_null_ptr<ItemInstance>& item);
    static int getEnchantmentLevel(Enchantment* enchantment, const boost::shared_ptr<LivingEntity>& entity);
    static void runIterationOnItem(EnchantmentIterationMethod& method, const not_null_ptr<ItemInstance>& item);
    static void runIterationOnInventory(EnchantmentIterationMethod& method, NonNullList<not_null_ptr<ItemInstance> > items);
    static int getDamageProtection(NonNullList<not_null_ptr<ItemInstance> > items, const DamageSource* source);
    static double getDamageBonus(const not_null_ptr<ItemInstance>& item, const boost::shared_ptr<LivingEntity>& entity);
    static void doPostHurtEffects(const boost::shared_ptr<LivingEntity>& target, const boost::shared_ptr<Entity>& attacker);
    static void doPostDamageEffects(const boost::shared_ptr<LivingEntity>& attacker, const boost::shared_ptr<Entity>& target);
    static int getKnockbackBonus(const boost::shared_ptr<LivingEntity>& attacker, const boost::shared_ptr<LivingEntity>& target);
    static int getFireAspect(const boost::shared_ptr<LivingEntity>& entity);
    static int getRespiration(const boost::shared_ptr<LivingEntity>& entity);
    static int getDepthStrider(const boost::shared_ptr<LivingEntity>& entity);
    static int getBlockEfficiency(const boost::shared_ptr<LivingEntity>& entity);
    static int getFishingLuckBonus(const not_null_ptr<ItemInstance>& item);
    static int getFishingSpeedBonus(const not_null_ptr<ItemInstance>& item);
    static int getMobLooting(const boost::shared_ptr<LivingEntity>& entity);
    static bool hasAquaAffinity(const boost::shared_ptr<LivingEntity>& entity);
    static bool hasFrostWalker(const boost::shared_ptr<LivingEntity>& entity);
    static bool hasBindingCurse(const not_null_ptr<ItemInstance>& item);
    static bool hasVanishingCurse(const not_null_ptr<ItemInstance>& item);
    static bool hasChanneling(const not_null_ptr<ItemInstance>& item);
    static boost::shared_ptr<ItemInstance> getRandomItemWith(Enchantment* enchantment, const boost::shared_ptr<LivingEntity>& entity);
    static int getEnchantmentCost(Random* random, int slot, int level, const not_null_ptr<ItemInstance>& item);
    static std::vector<EnchantmentInstance*>* getAvailableEnchantmentResults(int cost, const not_null_ptr<ItemInstance>& item, bool treasure);
    static void filterCompatibleEnchantments(std::vector<EnchantmentInstance*>* enchantments, const EnchantmentInstance* selected);
    static std::vector<EnchantmentInstance*>* selectEnchantment(Random* random, const not_null_ptr<ItemInstance>& item, int level, bool treasure);
    static not_null_ptr<ItemInstance> enchantItem(Random* random, not_null_ptr<ItemInstance> item, int level, bool treasure);
    static float getSweepingDamageRatio(const boost::shared_ptr<LivingEntity>& entity);
};

#endif
