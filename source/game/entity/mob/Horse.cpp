#include "game/entity/mob/Horse.hpp"

#include "game/core/AttributeInstance.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/item/ItemDataWalker.hpp"
#include "game/entity/mob/HorseArmorItem.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/SpawnEggItem.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundType.hpp"
#include "game/world/generation/Player.hpp"

const EntityDataAccessor<int>* Horse::DATA_VARIANT = NULL;
const EntityDataAccessor<not_null_ptr<ItemInstance> >* Horse::DATA_ARMOR = NULL;

Horse::HorseGroupData::HorseGroupData(int variant) : MobGroupData(), mVariant(variant) { }
Horse::HorseGroupData::~HorseGroupData() { }

Horse::Horse(Level* level)
    : AbstractHorse(level), mArmorType(NULL), mLayeredTextureHashName(L""),
      mLayeredTextureLayers(4, true), mWearingDyedArmor(false), mDyedArmorColor(0),
      mDyedArmorLayer(-1), mArmorItem()
{
    fjDerivedCtorCalls();
    unsigned int layer;
    for (layer = 0; layer < mLayeredTextureLayers.length; ++layer)
    {
        mLayeredTextureLayers.data[layer] = -1;
    }
}

Horse::~Horse() { }

void Horse::create(Level* level) { new Horse(level); }

HorseArmorItem* Horse::getArmorType()
{
    not_null_ptr<ItemInstance> item = getEntityData()->get(DATA_ARMOR);
    if (HorseArmorItem::isHorseArmor(item->getItem()))
        return static_cast<HorseArmorItem*>(item->getItem());
    return NULL;
}

void Horse::containerChanged(Container* container)
{
    HorseArmorItem* oldArmor = mArmorType;
    AbstractHorse::containerChanged(container);
    updateEquipment();
    if (oldArmor != mArmorType && mArmorType != NULL && mArmorType->getTier() != 0)
    {
        clearLayeredTextureInfo();
    }
}

void Horse::addDataWalkers(DataFixerUpper* fixer)
{
    AbstractHorse::addDataWalkers(fixer, 0x2d690001ULL);
    fixer->addWalker(DataFixTypes::eDataFixTypes_Entity,
                     new ItemDataWalker(0x2d690001ULL, L"ArmorItem"));
}
void Horse::staticCtor()
{
    DATA_VARIANT = SynchedEntityData::defineId<int>(0x2d690001ULL, 0x0d690000ULL,
                                                     EntityDataSerializers::INT);
    DATA_ARMOR = SynchedEntityData::defineId<not_null_ptr<ItemInstance> >(0x2d690001ULL, 0x0d690000ULL,
                                                                          EntityDataSerializers::ITEM_INSTANCE);
}
void Horse::defineSynchedData()
{
    AbstractHorse::defineSynchedData();
    getEntityData()->createDateItem(DATA_VARIANT, 0);
    getEntityData()->createDateItem(DATA_ARMOR, ItemInstance::EMPTY);
}
int Horse::getVariant() { return getEntityData()->get(DATA_VARIANT); }

void Horse::addAdditonalSaveData(CompoundTag* tag)
{
    AbstractHorse::addAdditonalSaveData(tag);
    tag->putInt(L"Variant", getVariant());
    if (mArmorType != NULL && !mArmorItem->isEmpty())
    {
        tag->put(L"ArmorItem", mArmorItem->save(new CompoundTag()));
    }
}

void Horse::clearLayeredTextureInfo()
{
    mLayeredTextureHashName.clear();
}

void Horse::setVariant(int variant)
{
    getEntityData()->set(DATA_VARIANT, variant, true);
    clearLayeredTextureInfo();
}

void Horse::readAdditionalSaveData(CompoundTag* tag)
{
    AbstractHorse::readAdditionalSaveData(tag);
    if (tag->contains(L"Variant"))
    {
        setVariant(tag->getInt(L"Variant"));
    }
}

void Horse::rebuildLayeredTextureInfo()
{
    mLayeredTextureHashName = L"horse_";
    unsigned int layer;
    for (layer = 0; layer < mLayeredTextureLayers.length; ++layer)
    {
        mLayeredTextureLayers.data[layer] = -1;
    }
    layer = 0;
    const int variant = getVariant();
    mLayeredTextureLayers.data[layer++] = variant & 7;
    mLayeredTextureLayers.data[layer++] = (variant >> 8) % 5;
    if (mArmorType != NULL)
    {
        int i;
        for (i = 0; i < mArmorType->getTextureCount() && layer < mLayeredTextureLayers.length; ++i)
        {
            mLayeredTextureLayers.data[layer++] = mArmorType->getTexture(i);
        }
    }
}

std::wstring Horse::getLayeredTextureHashName()
{
    if (mLayeredTextureHashName.empty()) rebuildLayeredTextureInfo();
    return mLayeredTextureHashName;
}

arrayWithLength<int> Horse::getLayeredTextureLayers()
{
    if (mLayeredTextureHashName.empty()) rebuildLayeredTextureInfo();
    return mLayeredTextureLayers;
}

bool Horse::isWearingDyedArmor()
{
    HorseArmorItem* armor = getArmorType();
    return armor != NULL && armor->isDyeable();
}

int Horse::getDyedArmorColor()
{
    if (mArmorType != NULL && mWearingDyedArmor) mDyedArmorColor = mArmorType->getDyedColor(mArmorItem);
    return mDyedArmorColor;
}

int Horse::getDyedArmorLayer()
{
    rebuildLayeredTextureInfo();
    if (mLayeredTextureLayers.data[3] == -1)
        return 1;
    return 2;
}

void Horse::setArmorType(const not_null_ptr<ItemInstance>& item)
{
    mArmorItem = item;
    mArmorType = NULL;
    if (!item->isEmpty() && HorseArmorItem::isHorseArmor(item->getItem()))
    {
        mArmorType = static_cast<HorseArmorItem*>(item->getItem());
        mWearingDyedArmor = mArmorType->isDyeable();
        mDyedArmorLayer = mArmorType->getTextureCount() > 0 ? 0 : -1;
    }
    else
    {
        mWearingDyedArmor = false;
        mDyedArmorLayer = -1;
    }
    clearLayeredTextureInfo();
}

void Horse::updateEquipment()
{
    AbstractHorse::updateEquipment();
    clearLayeredTextureInfo();
}

void Horse::onSyncedDataUpdated(const EntityDataAccessor_Base* accessor)
{
    AgableMob::onSyncedDataUpdated(accessor);
    clearLayeredTextureInfo();
}

void Horse::playGallopSound(const SoundType* sound)
{
    AbstractHorse::playGallopSound(sound);
    if (getRandom()->nextInt(10) == 0)
        playSound(SoundEvents::HORSE_BREATHE, sound->getVolume() * 0.6f, sound->getPitch());
}
void Horse::registerAttributes()
{
    AbstractHorse::registerAttributes();
    getAttributes()->registerAttribute(JUMP_STRENGTH);
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(generateRandomMaxHealth());
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(generateRandomSpeed());
    getAttribute(JUMP_STRENGTH)->setBaseValue(generateRandomJumpStrength());
}

void Horse::tick()
{
    AbstractHorse::tick();
    if (mLayeredTextureHashName.empty()) rebuildLayeredTextureInfo();
}

const SoundEvent* Horse::getAmbientSound() { AbstractHorse::getAmbientSound(); return SoundEvents::HORSE_AMBIENT; }
const SoundEvent* Horse::getDeathSound() { AbstractHorse::getDeathSound(); return SoundEvents::HORSE_DEATH; }
const SoundEvent* Horse::getHurtSound(const DamageSource* source) { AbstractHorse::getHurtSound(source); return SoundEvents::HORSE_HURT; }
const SoundEvent* Horse::getAngrySound() { AbstractHorse::getAngrySound(); return SoundEvents::HORSE_ANGRY; }
ResourceLocation* Horse::getDefaultLootTable() { return &BuiltInLootTables::HORSE; }

bool Horse::mobInteract(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> item = player->getItemInHand(hand);
    if (!item->isEmpty() && item->getItem() == Items::spawnEgg)
        return AbstractHorse::mobInteract(player, hand);
    if (isLeashedToPlayer(player))
        return false;
    if (!isBaby())
    {
        if (isTamed() && player->isSneaking())
        {
            openInventory(player);
            return true;
        }
        if (isVehicle())
            return AbstractHorse::mobInteract(player, hand);
    }
    if (!item->isEmpty())
    {
        if (handleEating(player, item))
        {
            if (!player->mAbilities.mInstabuild)
                item->shrink(1);
            return true;
        }
        if (item->interactEnemy(player,
                                boost::static_pointer_cast<LivingEntity>(shared_from_this()),
                                hand))
            return true;
        if (!isTamed())
        {
            doPlayerRide(player);
            return true;
        }
        if (HorseArmorItem::isHorseArmor(item->getItem())
            || (!isBaby() && !isSaddled() && item->getItem() == Items::saddle))
        {
            openInventory(player);
            return true;
        }
    }
    if (isBaby())
        return AbstractHorse::mobInteract(player, hand);
    doPlayerRide(player);
    return true;
}

unsigned int Horse::GetInteractTooltip(const EntityTooltipDataHolder& tooltipData)
{
    if (!tooltipData.m_bAllowedToInteract)
        return 0xffffffffU;

    not_null_ptr<ItemInstance> item = tooltipData.m_pItemInstance;
    unsigned int itemId = Item::getId(item->getItem());
    bool food = itemId == 170 || itemId == 260 || itemId == 296
             || itemId == 322 || itemId == 353 || itemId == 396;
    bool breedFood = itemId == 396 || itemId == 322;
    bool horseArmor = itemId >= 417 && itemId <= 419 || itemId == 453;
    boost::shared_ptr<Player> player(tooltipData.m_pPlayer,
                                     (Player*)tooltipData.m_pPlayer.get());

    if (itemId == 383)
    {
        if (SpawnEggItem::fjGetType(item) == GetType())
            return 0x4c36cd96U;
        if (isLeashedToPlayer(player))
            return 0x551269b5U;
        return 0xffffffffU;
    }

    if (isLeashedToPlayer(player))
        return 0x551269b5U;

    if (itemId == 420)
    {
        if (!isLeashed())
            return 0xa1b34a96U;
        if (!isTamed() || !getPassengers()->empty())
            return 0xffffffffU;
        return 0x497adcf0U;
    }

    if (itemId == 421)
    {
        if (!player->isSneaking() || !isTamed())
        {
            if (!item->hasCustomHoverName())
                return 0xffffffffU;
            if (hasCustomName() && item->getHoverName() == getCustomName())
                return 0xffffffffU;
            return 0x5592c692U;
        }
    }

    if (isBaby())
        return food ? 0x82f8d90fU : 0xffffffffU;

    if (!isTamed())
    {
        if (!item->isEmpty())
        {
            if (food && itemId != 170)
                return 0x82f8d90fU;
            return 0xffffffffU;
        }
        if (getPassengers()->empty())
            return 0x95cfa728U;
        return 0xffffffffU;
    }

    bool openInventory = true;
    if (!player->isSneaking())
    {
        if ((itemId != 329 || isSaddled()) && (!wearsArmor() || !horseArmor))
            openInventory = false;
    }
    else if (isVehicle())
    {
        if (!mobInteract(player, InteractionHand::eInteractionHand_MainHand))
        {
            if ((itemId != 329 || isSaddled()) && (!wearsArmor() || !horseArmor))
                openInventory = false;
        }
    }

    if (openInventory)
        return 0x41590ad1U;

    if (breedFood && !isInLove() && getPassengers()->empty() && !isBreedCoolingDown())
        return 0xb47a2ab6U;

    if (food && getHealth() < getMaxHealth())
        return 0x56b3f98dU;

    if (getPassengers()->empty())
        return 0x497adcf0U;

    return 0xffffffffU;
}

bool Horse::HideUseTooltips() { return true; }

bool Horse::canMate(const boost::shared_ptr<Animal>& other)
{
    return AbstractHorse::canMate(other) && other.get() != this;
}

boost::shared_ptr<AgableMob> Horse::getBreedOffspring(boost::shared_ptr<AgableMob> partner)
{
    Horse* child = new Horse(mLevel);
    child->setVariant(getVariant());
    Horse* horse = dynamic_cast<Horse*>(partner.get());
    if (horse != NULL) child->setVariant((getVariant() + horse->getVariant()) / 2);
    return boost::shared_ptr<AgableMob>(child);
}

bool Horse::wearsArmor() { return true; }

bool Horse::isArmor(const not_null_ptr<ItemInstance>& item)
{
    return !item->isEmpty() && HorseArmorItem::isHorseArmor(item->getItem());
}

MobGroupData* Horse::finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data, const std::wstring& name)
{
    MobGroupData* result = AbstractHorse::finalizeMobSpawn(difficulty, data, name);
    HorseGroupData* group = dynamic_cast<HorseGroupData*>(result);
    if (group == NULL)
    {
        group = new HorseGroupData(getRandom()->nextInt(7));
        result = group;
    }
    setVariant(group->mVariant | (getRandom()->nextInt(5) << 8));
    return result;
}

bool Horse::removeWhenFarAway()
{
    return AbstractHorse::removeWhenFarAway() && mArmorType == NULL;
}

unsigned long long Horse::GetType() { return 0x000000002D690001ULL; }
