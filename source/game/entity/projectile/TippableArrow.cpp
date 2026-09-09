#include "game/entity/projectile/TippableArrow.hpp"

#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/core/Minecraft.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/Potion.hpp"
#include "game/item/Potions.hpp"
#include "game/item/PotionUtils.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/world/Level.hpp"

const EntityDataAccessor<int>* TippableArrow::DATA_EFFECT_COLOR_ID = NULL;

TippableArrow::TippableArrow(Level* level)
    : Arrow(level)
{
    init();
}

TippableArrow::TippableArrow(Level* level, double x, double y, double z)
    : Arrow(level, x, y, z)
{
    init();
}

TippableArrow::TippableArrow(Level* level, boost::shared_ptr<LivingEntity> owner)
    : Arrow(level, owner)
{
    init();
}

TippableArrow::~TippableArrow()
{
}

void TippableArrow::create(Level* level)
{
    new TippableArrow(level);
}

void TippableArrow::staticCtor()
{
    DATA_EFFECT_COLOR_ID = SynchedEntityData::defineId(0x8A020002ULL,
        0x8A020000ULL, EntityDataSerializers::INT);
}

void TippableArrow::init()
{
    fjDerivedCtorCalls();
    mFixedColor = false;
    mPotion = Potions::EMPTY;
}

int TippableArrow::getCustomColor(const not_null_ptr<ItemInstance>& itemInstance)
{
    CompoundTag* tag = itemInstance->getTag();
    if (tag != NULL && tag->contains(L"CustomPotionColor", 99))
        return tag->getInt(L"CustomPotionColor");
    return -1;
}

void TippableArrow::updateColor()
{
    mFixedColor = false;
    if (mPotion == Potions::EMPTY && mEffects.empty())
    {
        mEntityData->set(DATA_EFFECT_COLOR_ID, -1, true);
        return;
    }

    std::vector<MobEffectInstance*> effects;
    PotionUtils::getAllEffects(mPotion, &effects);
    effects.insert(effects.end(), mEffects.begin(), mEffects.end());
    mEntityData->set(DATA_EFFECT_COLOR_ID, PotionUtils::getColor(&effects), true);
}

void TippableArrow::setFixedColor(int color)
{
    mFixedColor = true;
    mEntityData->set(DATA_EFFECT_COLOR_ID, color, true);
}

void TippableArrow::setEffectsFromItem(const not_null_ptr<ItemInstance>& itemInstance)
{
    if (itemInstance->getItem() != Items::tippedArrow)
    {
        if (itemInstance->getItem() == Items::arrow)
        {
            mEffects.clear();
            mPotion = Potions::EMPTY;
            mEntityData->set(DATA_EFFECT_COLOR_ID, -1, true);
        }
        return;
    }

    mPotion = PotionUtils::getPotion(itemInstance->getTag());
    std::vector<MobEffectInstance*>* effects = PotionUtils::getCustomEffects(itemInstance);
    if (effects != NULL)
    {
        mEffects.insert(mEffects.end(), effects->begin(), effects->end());
        delete effects;
    }

    int color = getCustomColor(itemInstance);
    if (color != -1)
        setFixedColor(color);
    else
        updateColor();
}

void TippableArrow::addEffect(MobEffectInstance* effect)
{
    mEffects.push_back(effect);
    if (mPotion == Potions::EMPTY && mEffects.begin() == mEffects.end())
        mEntityData->set(DATA_EFFECT_COLOR_ID, -1, true);
    else
    {
        std::vector<MobEffectInstance*> effects;
        PotionUtils::getAllEffects(mPotion, &effects);
        effects.insert(effects.end(), mEffects.begin(), mEffects.end());
        mEntityData->set(DATA_EFFECT_COLOR_ID, PotionUtils::getColor(&effects), true);
    }
}

void TippableArrow::defineSynchedData()
{
    AbstractRotatedItemProjectile::defineSynchedData();
    mEntityData->createDateItem(DATA_EFFECT_COLOR_ID, -1);
}

int TippableArrow::getColor()
{
    return mEntityData->get(DATA_EFFECT_COLOR_ID);
}

void TippableArrow::makeParticle(int count)
{
    int color = getColor();
    if (color == -1 || count <= 0)
        return;

    double red = static_cast<double>((color >> 16) & 255) / 255.0;
    double green = static_cast<double>((color >> 8) & 255) / 255.0;
    double blue = static_cast<double>(color & 255) / 255.0;
    for (int i = 0; i < count; ++i)
    {
        mLevel->addParticle(PARTICLE_MOB_SPELL,
            mX + (mRandom->nextDouble() - 0.5) * mBbWidth,
            mY + mRandom->nextDouble() * mBbHeight,
            mZ + (mRandom->nextDouble() - 0.5) * mBbWidth,
            red, green, blue, arrayWithLength<int>());
    }
}

void TippableArrow::tick()
{
    AbstractRotatedItemProjectile::tick();
    if (mLevel->mIsClientSide)
    {
        if (!mInGround)
            makeParticle(2);
        else if (mInGroundTime % 5 == 0)
            makeParticle(1);
    }
    else if (mInGround && !mEffects.empty() && mInGroundTime > 599)
    {
        mEffects.clear();
        mPotion = NULL;
        updateColor();
    }
}

void TippableArrow::addDataWalkers(DataFixerUpper* fixer)
{
    Arrow::addDataWalkers(fixer, 0x8A020002ULL);
}

void TippableArrow::addAdditonalSaveData(CompoundTag* tag)
{
    Arrow::addAdditonalSaveData(tag);
    if (mPotion != Potions::EMPTY)
    {
        not_null_ptr<ItemInstance> item(new ItemInstance(Items::tippedArrow));
        PotionUtils::setPotion(item, mPotion);
        tag->putString(L"Potion", PotionUtils::getPotionID(item).toString());
    }
    if (mFixedColor)
        tag->putInt(L"Color", getColor());
    if (!mEffects.empty())
    {
        ListTag* list = new ListTag(10);
        for (std::vector<MobEffectInstance*>::iterator it = mEffects.begin(); it != mEffects.end(); ++it)
            list->add((*it)->save(new CompoundTag()));
        tag->put(L"CustomPotionEffects", list);
    }
}

void TippableArrow::readAdditionalSaveData(CompoundTag* tag)
{
    Arrow::readAdditionalSaveData(tag);
    if (tag->contains(L"Potion", 8))
        mPotion = PotionUtils::getPotion(tag);
    std::vector<MobEffectInstance*>* effects = PotionUtils::getCustomEffects(tag);
    if (effects != NULL)
    {
        for (std::vector<MobEffectInstance*>::iterator it = effects->begin(); it != effects->end(); ++it)
            addEffect(*it);
        delete effects;
    }
    updateColor();
    if (tag->contains(L"Color", 99))
        setFixedColor(tag->getInt(L"Color"));
}

void TippableArrow::doPostHurtEffects(const boost::shared_ptr<LivingEntity>& entity)
{
    AbstractRotatedItemProjectile::doPostHurtEffects(entity);
    std::vector<MobEffectInstance*>& potionEffects = mPotion->getEffects();
    for (std::vector<MobEffectInstance*>::iterator it = potionEffects.begin(); it != potionEffects.end(); ++it)
    {
        int duration = (*it)->getDuration() /
            (Minecraft::InMiniGame(eMiniGameId_Battle, false) ? 3 : 8);
        if (duration < 1)
            duration = 1;
        entity->addEffect(new MobEffectInstance((*it)->getEffect(), duration,
            (*it)->getAmplifier(), (*it)->isAmbient(), (*it)->isVisible()),
            boost::shared_ptr<Entity>());
    }
    for (std::vector<MobEffectInstance*>::iterator it = mEffects.begin(); it != mEffects.end(); ++it)
        entity->addEffect(new MobEffectInstance(**it), boost::shared_ptr<Entity>());
}

not_null_ptr<ItemInstance> TippableArrow::getPickupItem()
{
    if (mPotion == Potions::EMPTY && mEffects.empty())
        return not_null_ptr<ItemInstance>(new ItemInstance(Items::arrow));

    not_null_ptr<ItemInstance> item(new ItemInstance(Items::tippedArrow));
    PotionUtils::setPotion(item, mPotion);
    PotionUtils::setCustomEffects(item, &mEffects);
    if (mFixedColor)
    {
        CompoundTag* tag = item->getTag();
        if (tag == NULL)
        {
            tag = new CompoundTag();
            item->setTag(tag);
        }
        tag->putInt(L"CustomPotionColor", getColor());
    }
    return item;
}

void TippableArrow::handleEntityEvent(unsigned char event, int data)
{
    if (event == 0)
        makeParticle(data);
    else
        Entity::handleEntityEvent(event, data);
}

unsigned long long TippableArrow::GetType()
{
    return 0x000000008A020002ULL;
}
