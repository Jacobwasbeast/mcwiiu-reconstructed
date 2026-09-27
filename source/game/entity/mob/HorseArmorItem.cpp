#include "game/entity/mob/HorseArmorItem.hpp"
#include "game/item/Items.hpp"

#include "game/core/IconRegister.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/item/ItemInstance.hpp"

static const wchar_t* HORSE_ARMOR_COLOR = L"customColor";

HorseArmorItem::HorseArmorItem(int tier, int protection, int itemId, const wchar_t* hash)
    : Item(), mTier(tier), mProtection(protection), mTextures(), mHash(hash == NULL ? L"" : hash), mArmorIcon(NULL)
{
    setBaseItemTypeAndMaterial(itemId, 0);
}

HorseArmorItem::HorseArmorItem(int tier, int protection, const int* textures, int textureCount, const wchar_t* hash)
    : Item(), mTier(tier), mProtection(protection), mTextures(), mHash(hash == NULL ? L"" : hash), mArmorIcon(NULL)
{
    int i;
    for (i = 0; i < textureCount; ++i)
    {
        mTextures.push_back(textures[i]);
    }
}

HorseArmorItem::HorseArmorItem(int tier, int protection)
    : Item(), mTier(tier), mProtection(protection), mTextures(1, -1), mHash(L""), mArmorIcon(NULL)
{
}

HorseArmorItem::~HorseArmorItem()
{
}

bool HorseArmorItem::isHorseArmor(Item* item)
{
    return item == Items::leatherHorseArmor || item == Items::ironHorseArmor || item == Items::goldenHorseArmor
        || item == Items::diamondHorseArmor;
}

int HorseArmorItem::getTier() const
{
    return mTier;
}

std::wstring HorseArmorItem::getHash() const
{
    return mHash;
}

int HorseArmorItem::getProtection() const
{
    return mProtection;
}

int HorseArmorItem::getTexture(int index) const
{
    if (index < 0 || index >= (int)mTextures.size())
    {
        return -1;
    }
    return mTextures[(unsigned int)index];
}

int HorseArmorItem::getTextureCount() const
{
    return (int)mTextures.size();
}

TextureAtlasSprite* HorseArmorItem::getIcon(int auxValue)
{
    return mIcon;
}

TextureAtlasSprite* HorseArmorItem::getLayerIcon(int auxValue, int layer, not_null_ptr<ItemInstance> itemInstance)
{
    if (mTier == 1 && layer == 0 && mArmorIcon != NULL)
    {
        return mArmorIcon;
    }
    return Item::getLayerIcon(auxValue, layer, itemInstance);
}

bool HorseArmorItem::hasMultipleSpriteLayers()
{
    return true;
}

void HorseArmorItem::registerIcons(IconRegister* iconRegister)
{
    Item::registerIcons(iconRegister);
    if (mTier == 1)
    {
        mArmorIcon = iconRegister->registerIcon(L"leather_horse_armor_base");
    }
}

bool HorseArmorItem::hasCustomColor(const not_null_ptr<ItemInstance>& itemInstance)
{
    return itemInstance->hasTag() && itemInstance->getTag()->contains(HORSE_ARMOR_COLOR);
}

int HorseArmorItem::getDyedColor(not_null_ptr<ItemInstance> itemInstance) const
{
    if (!itemInstance->hasTag() || !itemInstance->getTag()->contains(HORSE_ARMOR_COLOR))
    {
        return -1;
    }
    return itemInstance->getTag()->getInt(HORSE_ARMOR_COLOR);
}

int HorseArmorItem::getColor(not_null_ptr<ItemInstance> itemInstance, int auxValue)
{
    int color = getDyedColor(itemInstance);
    return color < 0 ? auxValue : color;
}

void HorseArmorItem::clearColor(const not_null_ptr<ItemInstance>& itemInstance)
{
    if (itemInstance->hasTag())
    {
        itemInstance->getTag()->remove(HORSE_ARMOR_COLOR);
    }
}

void HorseArmorItem::setColor(const not_null_ptr<ItemInstance>& itemInstance, int color)
{
    if (!itemInstance->hasTag())
    {
        itemInstance->setTag(new CompoundTag());
    }
    itemInstance->getTag()->putInt(HORSE_ARMOR_COLOR, color);
}

bool HorseArmorItem::isDyeable() const
{
    return mTier == 1;
}

std::wstring HorseArmorItem::getTexture() const
{
    return mHash;
}

bool HorseArmorItem::hasMultipleTextures() const
{
    return mTextures.size() > 1;
}
