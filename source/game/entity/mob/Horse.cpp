#include "game/entity/mob/Horse.hpp"

#include "game/core/BuiltInLootTables.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/entity/mob/HorseArmorItem.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/client/audio/SoundEvents.hpp"

const EntityDataAccessor<int>* Horse::DATA_VARIANT = NULL;

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

HorseArmorItem* Horse::getArmorType() { return mArmorType; }

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
}
void Horse::staticCtor()
{
    DATA_VARIANT = SynchedEntityData::defineId<int>(0x2d690001ULL, 0x0d690000ULL,
                                                     EntityDataSerializers::INT);
}
void Horse::defineSynchedData()
{
    AbstractHorse::defineSynchedData();
    getEntityData()->createDateItem(DATA_VARIANT, 0);
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

bool Horse::isWearingDyedArmor() { return mWearingDyedArmor; }

int Horse::getDyedArmorColor()
{
    if (mArmorType != NULL && mWearingDyedArmor) mDyedArmorColor = mArmorType->getDyedColor(mArmorItem);
    return mDyedArmorColor;
}

int Horse::getDyedArmorLayer() { return mDyedArmorLayer; }

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

void Horse::playGallopSound(const SoundType* sound) { AbstractHorse::playGallopSound(sound); }
void Horse::registerAttributes() { AbstractHorse::registerAttributes(); }

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
    return AbstractHorse::mobInteract(player, hand);
}

unsigned int Horse::GetInteractTooltip(const EntityTooltipDataHolder& tooltipData)
{
    return AbstractHorse::GetInteractTooltip(tooltipData);
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
