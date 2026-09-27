#include "game/ui/MenuBuilder.hpp"

#include <string>
#include <vector>

#include "game/core/BannerPattern.hpp"
#include "game/core/DyeColor.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/item/BannerItem.hpp"
#include "game/item/DyeItem.hpp"
#include "game/item/Item.hpp"
#include "game/item/Items.hpp"
#include "game/item/Potion.hpp"
#include "game/item/PotionUtils.hpp"
#include "game/item/SpawnEggItem.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/block/BannerBlockEntity.hpp"

void MenuBuilder::ITEM_BLOCKSTATE(const BlockState* state)
{
    ADD(not_null_ptr<ItemInstance>(
        new ItemInstance(state->getBlock(), 1, state->getBlock()->convertBlockStateToLegacyData(state))));
}

void MenuBuilder::ADD(const not_null_ptr<ItemInstance>& item)
{
    ItemInstance* copy = item->copy_not_shared();
    mItems->push_back(new ItemInstanceClonerBuilder(*copy));
    *copy = *ItemInstance::EMPTY;
    delete copy;
}

void MenuBuilder::BANNER(const DyeColor* color, const BannerPattern* pattern)
{
    not_null_ptr<ItemInstance> banner(BannerItem::makeBanner(color, NULL));
    BannerBlockEntity::addTags(banner, color->getItemData(), NULL);
    if (pattern != NULL)
        BannerBlockEntity::fjAddPatternToItem(banner, pattern, DyeColor::BLACK);
    ADD(banner);
}

void MenuBuilder::FIREWORK(unsigned char type, int colour, int flight, bool flicker, bool trail, int fadeColour)
{
    CompoundTag* explosion = new CompoundTag();

    std::vector<int> colours;
    colours.push_back(DyeItem::DYE_COLORS[colour]);
    if (flicker)
        explosion->putBoolean(L"Flicker", true);
    if (trail)
        explosion->putBoolean(L"Trail", true);
    explosion->putIntArray(L"Colors", arrayWithLength<int>(colours));
    explosion->putByte(L"Type", type);

    if (fadeColour != -1)
    {
        std::vector<int> fadeColours;
        fadeColours.push_back(DyeItem::DYE_COLORS[fadeColour]);
        explosion->putIntArray(L"FadeColors", arrayWithLength<int>(fadeColours));
    }

    CompoundTag* tag = new CompoundTag();
    CompoundTag* fireworks = new CompoundTag();
    ListTag* explosions = new ListTag(0);
    explosions->add(explosion);
    fireworks->put(L"Explosions", explosions);
    fireworks->putByte(L"Flight", static_cast<unsigned char>(flight));
    tag->put(L"Fireworks", fireworks);

    ItemInstance rocket(Items::fireworks);
    rocket.setTag(tag);
    mItems->push_back(new ItemInstanceClonerBuilder(rocket));
    rocket = *ItemInstance::EMPTY;
}

void MenuBuilder::POTION(Item* item, Potion* potion)
{
    class PotionItemBuilder : public ItemInstanceClonerBuilder
    {
    public:
        PotionItemBuilder(const ItemInstance& item, Potion* potion)
            : ItemInstanceClonerBuilder(item), mPotion(potion)
        {
        }

        virtual not_null_ptr<ItemInstance> build() const
        {
            return PotionUtils::setPotion(mItem.copy(), mPotion);
        }

    private:
        Potion* mPotion;
    };

    ItemInstance instance(item);
    mItems->push_back(new PotionItemBuilder(instance, potion));
}

void MenuBuilder::DEBUG_SPAWN_EGG(const std::wstring& entityId, const std::wstring& debugTag)
{
    class SpawnEggItemBuilder : public ItemInstanceClonerBuilder
    {
    public:
        SpawnEggItemBuilder(const std::wstring& entityId, const std::wstring& debugTag)
            : ItemInstanceClonerBuilder(ItemInstance(Items::spawnEgg)), mEntityId(entityId), mDebugTag(debugTag)
        {
        }

        virtual not_null_ptr<ItemInstance> build() const
        {
            not_null_ptr<ItemInstance> egg = mItem.copy();
            SpawnEggItem::addSpawnEntity(egg, mEntityId);
            if (!mDebugTag.empty())
                egg->getTag()->putString(L"DebugTag", mDebugTag);
            return egg;
        }

    private:
        ResourceLocation mEntityId;
        std::wstring mDebugTag;
    };

    mItems->push_back(new SpawnEggItemBuilder(entityId, debugTag));
}
