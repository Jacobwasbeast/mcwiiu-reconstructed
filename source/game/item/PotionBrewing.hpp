#ifndef GAME_ITEM_POTIONBREWING_HPP
#define GAME_ITEM_POTIONBREWING_HPP

#include <vector>

#include "game/item/Item.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/world/block/Predicate.hpp"

class ItemInstance;
class Potion;
class PotionItem;

class PotionBrewing
{
public:
    class ItemInstancePredicate : public Predicate<not_null_ptr<ItemInstance> >
    {
    public:
        ItemInstancePredicate(Item* item);
        ItemInstancePredicate(Item* item, int auxValue);
        virtual bool apply(not_null_ptr<ItemInstance> itemInstance) const;
        virtual ~ItemInstancePredicate();

    private:
        Item* mItem;
        int mAuxValue;
    };

    class PredicateItemInstance : public Predicate<not_null_ptr<ItemInstance> >
    {
    public:
        virtual bool apply(not_null_ptr<ItemInstance> itemInstance) const;
        virtual ~PredicateItemInstance();

    private:
        int mUnknown0;
        int mUnknown1;
        std::vector<Predicate<not_null_ptr<ItemInstance> >*>* mPredicates;
    };

    template <class T>
    class Mix
    {
    public:
        T* from;
        Predicate<not_null_ptr<ItemInstance> >* ingredient;
        T* result;
    };

    static bool isContainerIngredient(not_null_ptr<ItemInstance> itemInstance);
    static bool isPotionIngredient(not_null_ptr<ItemInstance> itemInstance);
    static bool isIngredient(not_null_ptr<ItemInstance> itemInstance);
    static bool hasContainerMix(not_null_ptr<ItemInstance> item, not_null_ptr<ItemInstance> ingredient);
    static bool hasPotionMix(not_null_ptr<ItemInstance> item, not_null_ptr<ItemInstance> ingredient);
    static bool hasMix(not_null_ptr<ItemInstance> item, not_null_ptr<ItemInstance> ingredient);
    static void mix(not_null_ptr<ItemInstance> item, not_null_ptr<ItemInstance> ingredient);
    static void addContainer(ItemInstancePredicate* predicate);
    static void addContainerRecipe(PotionItem* from, ItemInstancePredicate* ingredient, PotionItem* result);
    static void addMix(Potion* from, Predicate<not_null_ptr<ItemInstance> >* ingredient, Potion* result);
    static void StaticCtor();
};

#endif
