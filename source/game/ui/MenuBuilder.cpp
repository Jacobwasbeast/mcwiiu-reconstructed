#include "game/ui/MenuBuilder.hpp"

#include <string>
#include <vector>

#include "game/core/BannerPattern.hpp"
#include "game/core/DyeColor.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/item/BannerItem.hpp"
#include "game/item/DyeItem.hpp"
#include "game/item/FireworksItem.hpp"
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
        explosion->putBoolean(FireworksItem::TAG_E_FLICKER, true);
    if (trail)
        explosion->putBoolean(FireworksItem::TAG_E_TRAIL, true);
    explosion->putIntArray(FireworksItem::TAG_E_COLORS, arrayWithLength<int>(colours));
    explosion->putByte(FireworksItem::TAG_E_TYPE, type);

    if (fadeColour != -1)
    {
        std::vector<int> fadeColours;
        fadeColours.push_back(DyeItem::DYE_COLORS[fadeColour]);
        explosion->putIntArray(FireworksItem::TAG_E_FADECOLORS, arrayWithLength<int>(fadeColours));
    }

    CompoundTag* tag = new CompoundTag();
    CompoundTag* fireworks = new CompoundTag();
    ListTag* explosions = new ListTag(0);
    explosions->add(explosion);
    fireworks->put(FireworksItem::TAG_EXPLOSIONS, explosions);
    fireworks->putByte(FireworksItem::TAG_FLIGHT, static_cast<unsigned char>(flight));
    tag->put(FireworksItem::TAG_FIREWORKS, fireworks);

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
