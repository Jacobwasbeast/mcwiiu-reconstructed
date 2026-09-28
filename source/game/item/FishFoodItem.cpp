#include "game/item/FishFoodItem.hpp"
#include "game/core/StringIds.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/IconRegister.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/Level.hpp"

FishFoodItem::Type* gFishFoodTypes[4] = {
    new FishFoodItem::Type(0, 0xFB51767E, IDS_DESC_FISH_COD_RAW, IDS_ITEM_FISH_COD_COOKED, IDS_DESC_FISH_COD_COOKED, 2, 0.6f, 5, 0.8f, L"fishRaw", L"fishCooked"),
    new FishFoodItem::Type(1, IDS_ITEM_FISH_SALMON_RAW, IDS_DESC_FISH_SALMON_RAW, IDS_ITEM_FISH_SALMON_COOKED, IDS_DESC_FISH_SALMON_COOKED, 2, 0.8f, 6, 1.0f, L"fish_salmon_raw", L"fish_salmon_cooked"),
    new FishFoodItem::Type(2, 0x0B072C6C, 0xCC0E58F0, 1, 0.8f, L"fish_clownfish_raw"),
    new FishFoodItem::Type(3, IDS_ITEM_FISH_PUFFERFISH_RAW, IDS_DESC_FISH_PUFFERFISH_RAW, 1, 0.8f, L"fish_pufferfish_raw")
};
FishFoodItem::Type* gPufferfishFoodType = gFishFoodTypes[3];
FishFoodItem::Type* FishFoodItem::Type::COD = gFishFoodTypes[0];
FishFoodItem::Type* FishFoodItem::Type::SALMON = gFishFoodTypes[1];
FishFoodItem::Type* FishFoodItem::Type::CLOWNFISH = gFishFoodTypes[2];
FishFoodItem::Type* FishFoodItem::Type::PUFFERFISH = gFishFoodTypes[3];

FishFoodItem::FishFoodItem(bool cooked)
    : FoodItem(0, 0.0f, false), mCooked(cooked)
{
}

FishFoodItem::~FishFoodItem()
{
}

FishFoodItem::Type::Type(int id, unsigned int name, int description,
                         unsigned int cookedName, int cookedDescription,
                         unsigned int nutrition, float saturation,
                         unsigned int cookedNutrition, float cookedSaturation,
                         const std::wstring& icon,
                         const std::wstring& cookedIcon)
    : mId(id), mName(name), mCookedName(cookedName), mRawNutrition(nutrition),
      mRawSaturation(saturation), mCookedNutrition(cookedNutrition),
      mCookedSaturation(cookedSaturation), mCanBeCooked(1), mIcon(icon),
      mCookedIcon(cookedIcon), mRawIcon(0), mCookedIconValue(0),
      mDescription(description), mCookedDescription(cookedDescription)
{
}

FishFoodItem::Type::Type(int id, unsigned int name, int description,
                         unsigned int nutrition, float saturation,
                         const std::wstring& icon)
    : mId(id), mName(name), mCookedName(-1), mRawNutrition(nutrition),
      mRawSaturation(saturation), mCookedNutrition(0), mCookedSaturation(0.0f),
      mCanBeCooked(0), mIcon(icon), mCookedIcon(L""), mRawIcon(0),
      mCookedIconValue(0), mDescription(description), mCookedDescription(-1)
{
}

FishFoodItem::Type* FishFoodItem::Type::getById(int id)
{
    if (id < 0 || id > 3)
    {
        return NULL;
    }
    return gFishFoodTypes[id];
}

FishFoodItem::Type* FishFoodItem::Type::getByItem(not_null_ptr<ItemInstance> itemInstance)
{
    Item* item = itemInstance->getItem();
    FishFoodItem* fish = dynamic_cast<FishFoodItem*>(item);
    if (fish != NULL)
    {
        return getById(itemInstance->getAuxValue());
    }
    return NULL;
}

unsigned char FishFoodItem::Type::canBeCooked() const
{
    return mCanBeCooked;
}

unsigned int FishFoodItem::Type::getCookedNutrition() const
{
    return mCookedNutrition;
}

unsigned int FishFoodItem::Type::getRawNutrition() const
{
    return mRawNutrition;
}

double FishFoodItem::Type::getCookedSaturation() const
{
    return (double)mCookedSaturation;
}

double FishFoodItem::Type::getRawSaturation() const
{
    return (double)mRawSaturation;
}

int FishFoodItem::Type::getName(bool cooked) const
{
    if (cooked && mCookedName != -1)
    {
        return mCookedName;
    }
    return mName;
}

int FishFoodItem::Type::getDescription(bool cooked) const
{
    if (cooked && mCookedDescription != -1)
    {
        return mCookedDescription;
    }
    return mDescription;
}

TextureAtlasSprite* FishFoodItem::Type::getIcon(bool cooked)
{
    if (cooked && mCookedIconValue != 0)
    {
        return mCookedIconValue;
    }
    return mRawIcon;
}

void FishFoodItem::Type::registerIcons(IconRegister* iconRegister)
{
    mRawIcon = iconRegister->registerIcon(mIcon);
    if (!mCookedIcon.empty())
    {
        mCookedIconValue = iconRegister->registerIcon(mCookedIcon);
    }
}

unsigned int FishFoodItem::Type::getId() const
{
    return mId;
}

unsigned int FishFoodItem::Type::getData() const
{
    return mId;
}

int FishFoodItem::getNutrition(not_null_ptr<ItemInstance> itemInstance)
{
    Type* type = Type::getByItem(itemInstance);
    if (mCooked && type->canBeCooked())
    {
        return (int)type->getCookedNutrition();
    }
    return (int)type->getRawNutrition();
}

float FishFoodItem::getSaturationModifier(not_null_ptr<ItemInstance> itemInstance)
{
    Type* type = Type::getByItem(itemInstance);
    if (mCooked && type->canBeCooked())
    {
        return (float)type->getCookedSaturation();
    }
    return (float)type->getRawSaturation();
}

void FishFoodItem::addEatEffect(not_null_ptr<ItemInstance> itemInstance, Level* level,
                                boost::shared_ptr<Player> player)
{
    if (Type::getByItem(itemInstance) == gPufferfishFoodType)
    {
        LivingEntity* entity = (LivingEntity*)player.get();
        entity->addEffect(new MobEffectInstance(MobEffects::CONFUSION, 1200, 3),
                           boost::shared_ptr<Entity>());
        entity->addEffect(new MobEffectInstance(MobEffects::HUNGER, 300, 2),
                           boost::shared_ptr<Entity>());
        entity->addEffect(new MobEffectInstance(MobEffects::POISON, 300, 1),
                           boost::shared_ptr<Entity>());
    }
    FoodItem::addEatEffect(itemInstance, level, player);
}

int FishFoodItem::getDescriptionId(not_null_ptr<ItemInstance> itemInstance)
{
    Item* item = itemInstance->getItem();
    Type* type = Type::getByItem(itemInstance);
    return type->getName(dynamic_cast<FishFoodItem*>(item)->mCooked);
}

int FishFoodItem::getUseDescriptionId(not_null_ptr<ItemInstance> itemInstance)
{
    Item* item = itemInstance->getItem();
    Type* type = Type::getByItem(itemInstance);
    return type->getDescription(dynamic_cast<FishFoodItem*>(item)->mCooked);
}

TextureAtlasSprite* FishFoodItem::getIcon(int auxValue)
{
    return Type::getById(auxValue)->getIcon(mCooked);
}

void FishFoodItem::registerIcons(IconRegister* iconRegister)
{
    gFishFoodTypes[0]->registerIcons(iconRegister);
    gFishFoodTypes[1]->registerIcons(iconRegister);
    gFishFoodTypes[2]->registerIcons(iconRegister);
    gFishFoodTypes[3]->registerIcons(iconRegister);
}
