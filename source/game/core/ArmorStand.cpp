#include "game/core/ArmorStand.hpp"

#include <cmath>

#include "game/core/EquipmentSlot.hpp"
#include "game/core/HumanoidArm.hpp"
#include "game/core/DataWalker.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/Direction.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/data/ListTag.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/ItemListDataWalker.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/EntityDataAccessor_Base.hpp"

ArmorStand::Pose::Pose(const Vec3* headPose, const Vec3* bodyPose,
                       const Vec3* leftArmPose, const Vec3* rightArmPose,
                       const Vec3* leftLegPose, const Vec3* rightLegPose,
                       const Vec3* rightItemPose)
    : head(const_cast<Vec3*>(headPose)), body(const_cast<Vec3*>(bodyPose)),
      leftArm(const_cast<Vec3*>(leftArmPose)), rightArm(const_cast<Vec3*>(rightArmPose)),
      leftLeg(const_cast<Vec3*>(leftLegPose)), rightLeg(const_cast<Vec3*>(rightLegPose)),
      rightItem(const_cast<Vec3*>(rightItemPose))
{
}

ArmorStand::ArmorStand(Level* level) : LivingEntity(level), mHandItems(), mArmorItems()
{
    _init();
}

ArmorStand::ArmorStand(Level* level, double x, double y, double z)
    : LivingEntity(level), mHandItems(), mArmorItems()
{
    _init();
    setPos(x, y, z);
}

ArmorStand::~ArmorStand()
{
}

void ArmorStand::staticCtor()
{
}

void ArmorStand::addDataWalkers(DataFixerUpper* fixer)
{
    std::vector<std::wstring> itemLists;
    itemLists.push_back(L"ArmorItems");
    itemLists.push_back(L"HandItems");
    fixer->addWalker(DataFixTypes::eDataFixTypes_Entity,
                     (DataWalker*)new ItemListDataWalker(ENTITY_TYPE_ARMOR_STAND, itemLists));
}

void ArmorStand::_init()
{
    mFlags = 0;
    mPoseIndex = 0;
    mShowArms = true;
    mNoBasePlate = false;
    mMarker = false;
    mDisabledSlots = 0;
    mLastSignal = 0;
    mHandItems = NonNullList<not_null_ptr<ItemInstance> >::withSize(2, ItemInstance::EMPTY);
    mArmorItems = NonNullList<not_null_ptr<ItemInstance> >::withSize(4, ItemInstance::EMPTY);
    setSize(0.5f, 1.975f);
}

unsigned char ArmorStand::setBit(unsigned char value, int bit, bool enabled)
{
    return enabled ? (unsigned char)(value | bit) : (unsigned char)(value & ~bit);
}

void ArmorStand::setShowArms(bool showArms) { mShowArms = showArms; }
std::vector<boost::shared_ptr<ItemInstance> >* ArmorStand::getHandSlots() { return NULL; }
std::vector<boost::shared_ptr<ItemInstance> >* ArmorStand::getArmorSlots() { return NULL; }
not_null_ptr<ItemInstance> ArmorStand::getItemSlot(const EquipmentSlot* slot)
{
    if (slot->getType() == EquipmentSlot::HAND)
    {
        return mHandItems[slot->getIndex()];
    }
    if (slot->getType() == EquipmentSlot::ARMOR)
    {
        return mArmorItems[slot->getIndex()];
    }
    return ItemInstance::EMPTY;
}
void ArmorStand::setItemSlot(const EquipmentSlot* slot, not_null_ptr<ItemInstance> item)
{
    if (slot->getType() == EquipmentSlot::HAND)
    {
        playEquipSound(item);
        mHandItems[slot->getIndex()] = item;
    }
    else if (slot->getType() == EquipmentSlot::ARMOR)
    {
        playEquipSound(item);
        mArmorItems[slot->getIndex()] = item;
    }
}
bool ArmorStand::setSlot(int slot, const not_null_ptr<ItemInstance>& item)
{
    const EquipmentSlot* equipmentSlot;
    if (slot == 98)
    {
        equipmentSlot = EquipmentSlot::MAINHAND;
    }
    else if (slot == 99)
    {
        equipmentSlot = EquipmentSlot::OFFHAND;
    }
    else if (slot == EquipmentSlot::FEET->getIndex() + 100)
    {
        equipmentSlot = EquipmentSlot::FEET;
    }
    else if (slot == EquipmentSlot::LEGS->getIndex() + 100)
    {
        equipmentSlot = EquipmentSlot::LEGS;
    }
    else if (slot == EquipmentSlot::CHEST->getIndex() + 100)
    {
        equipmentSlot = EquipmentSlot::CHEST;
    }
    else if (slot == EquipmentSlot::HEAD->getIndex() + 100)
    {
        equipmentSlot = EquipmentSlot::HEAD;
    }
    else
    {
        return false;
    }

    if (!item->isEmpty() && !Mob::isValidSlotForItem(equipmentSlot, item)
        && equipmentSlot != EquipmentSlot::HEAD)
    {
        return false;
    }

    setItemSlot(equipmentSlot, item);
    return true;
}
bool ArmorStand::isEffectiveAi() { return false; }
bool ArmorStand::isSmall() { return mFlags & 1; }
bool ArmorStand::isNoBasePlate() { return mNoBasePlate; }
bool ArmorStand::isMarker() { return mMarker; }
int ArmorStand::getPoseIndex() { return mPoseIndex; }
void ArmorStand::addAdditonalSaveData(CompoundTag* tag)
{
    ListTag* armorItems = new ListTag();
    unsigned int index;
    for (index = 0; index < mArmorItems.size(); ++index)
    {
        CompoundTag* itemTag = new CompoundTag();
        if (!mArmorItems[index]->isEmpty())
        {
            mArmorItems[index]->save(itemTag);
        }
        armorItems->add(itemTag);
    }
    tag->put(L"ArmorItems", armorItems);

    ListTag* handItems = new ListTag();
    for (index = 0; index < mHandItems.size(); ++index)
    {
        CompoundTag* itemTag = new CompoundTag();
        if (!mHandItems[index]->isEmpty())
        {
            mHandItems[index]->save(itemTag);
        }
        handItems->add(itemTag);
    }
    tag->put(L"HandItems", handItems);
    tag->putBoolean(L"Invisible", isInvisible());
    tag->putBoolean(L"Small", isSmall());
    tag->putInt(L"DisabledSlots", mDisabledSlots);
    tag->putBoolean(L"NoGravity", isNoGravity());
    tag->putBoolean(L"NoBasePlate", isNoBasePlate());
    if (isMarker())
    {
        tag->putBoolean(L"Marker", true);
    }

    CompoundTag* pose = new CompoundTag();
    pose->putInt(L"PoseIndex", getPoseIndex());
    pose->putInt(L"LastSignal", mLastSignal);
    tag->put(L"Pose", pose);
}
void ArmorStand::setSmall(bool small) { mFlags = setBit((unsigned char)mFlags, 1, small); }
void ArmorStand::setNoBasePlate(bool noBasePlate) { mNoBasePlate = noBasePlate; }
void ArmorStand::setMarker(bool marker) { mMarker = marker; }
void ArmorStand::setPoseIndex(int pose) { mPoseIndex = pose; }
void ArmorStand::readAdditionalSaveData(CompoundTag* tag)
{
    if (tag->contains(L"ArmorItems", 9))
    {
        ListTag* armorItems = tag->getList(L"ArmorItems", 10);
        int count = armorItems->size();
        if (count > (int)mArmorItems.size())
        {
            count = (int)mArmorItems.size();
        }
        for (int index = 0; index < count; ++index)
        {
            mArmorItems[index] = not_null_ptr<ItemInstance>(new ItemInstance(armorItems->getCompound(index)));
        }
    }
    if (tag->contains(L"HandItems", 9))
    {
        ListTag* handItems = tag->getList(L"HandItems", 10);
        int count = handItems->size();
        if (count > (int)mHandItems.size())
        {
            count = (int)mHandItems.size();
        }
        for (int index = 0; index < count; ++index)
        {
            mHandItems[index] = not_null_ptr<ItemInstance>(new ItemInstance(handItems->getCompound(index)));
        }
    }

    setInvisible(tag->getBoolean(L"Invisible"));
    setSmall(tag->getBoolean(L"Small"));
    mDisabledSlots = tag->getInt(L"DisabledSlots");
    setNoGravity(tag->getBoolean(L"NoGravity"));
    setNoBasePlate(tag->getBoolean(L"NoBasePlate"));
    setMarker(tag->getBoolean(L"Marker"));
    if (tag->contains(L"Pose", 10))
    {
        CompoundTag* pose = tag->getCompound(L"Pose");
        setPoseIndex(pose->getInt(L"PoseIndex"));
        mLastSignal = pose->getInt(L"LastSignal");
    }
}
bool ArmorStand::isPushable(boost::shared_ptr<Entity> entity) { return entity.get() != this; }
const EquipmentSlot* ArmorStand::getClickedSlot(const Vec3* hit)
{
    double y = hit->y;
    bool small = isSmall();
    if (small)
    {
        y *= 2.0;
    }

    if (y >= 0.1 && y < (small ? 0.8 : 0.45) + 0.1 && hasItemInSlot(EquipmentSlot::FEET))
    {
        return EquipmentSlot::FEET;
    }
    if (y >= (small ? 0.3 : 0.05) + 0.9
        && y < (small ? 1.0 : 0.7) + 0.9
        && hasItemInSlot(EquipmentSlot::CHEST))
    {
        return EquipmentSlot::CHEST;
    }
    if (y >= 0.4 && y < (small ? 0.7 : 0.8) + 0.4
        && hasItemInSlot(EquipmentSlot::LEGS))
    {
        return EquipmentSlot::LEGS;
    }
    if (y >= 1.6 && hasItemInSlot(EquipmentSlot::HEAD))
    {
        return EquipmentSlot::HEAD;
    }
    return EquipmentSlot::MAINHAND;
}

bool ArmorStand::isDisabled(const EquipmentSlot* slot)
{
    return (mDisabledSlots & (1 << slot->getFilterFlag())) != 0;
}

bool ArmorStand::swapItem(const boost::shared_ptr<Player>& player, const EquipmentSlot* slot,
                          const not_null_ptr<ItemInstance>& item,
                          InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> equipped = getItemSlot(slot);
    if (!equipped->isEmpty()
        && (mDisabledSlots & (1 << (slot->getFilterFlag() + 8))) != 0)
    {
        return false;
    }
    if (equipped->isEmpty()
        && (mDisabledSlots & (1 << (slot->getFilterFlag() + 16))) != 0)
    {
        return false;
    }

    if (!item->isEmpty() && item->getCount() > 1)
    {
        if (!equipped->isEmpty())
        {
            return false;
        }
        not_null_ptr<ItemInstance> singleItem = item->copy();
        singleItem->setCount(1);
        setItemSlot(slot, singleItem);
        item->shrink(1);
        return true;
    }

    setItemSlot(slot, item);
    if (player)
    {
        ((LivingEntity*)player.get())->setItemInHand(hand, equipped);
    }
    return true;
}
bool ArmorStand::isShowArms() { return mShowArms; }
bool ArmorStand::interactAt(const boost::shared_ptr<Player>& player, Vec3* hit,
                            InteractionHand::EInteractionHand hand)
{
    if (!player || hit == NULL || isMarker())
    {
        return false;
    }
    not_null_ptr<ItemInstance> heldItem = ((LivingEntity*)player.get())->getItemInHand(hand);
    return test_interactAt(player, hit, heldItem, hand);
}
bool ArmorStand::test_interactAt(boost::shared_ptr<Player> player, Vec3* hit,
                                 not_null_ptr<ItemInstance> item,
                                 InteractionHand::EInteractionHand hand)
{
    if (!player || hit == NULL || isMarker())
    {
        return false;
    }

    const EquipmentSlot* slot;
    if (item->isEmpty())
    {
        slot = getClickedSlot(hit);
        if (getItemSlot(slot)->isEmpty())
        {
            return false;
        }
    }
    else
    {
        const EquipmentSlot* equipmentSlot = Mob::getEquipmentSlotForItem(item);
        int slotIndex = equipmentSlot->getEIndex();
        if (slotIndex < EquipmentSlot::eIndex_MainHand || slotIndex > EquipmentSlot::eIndex_Head)
        {
            slot = EquipmentSlot::MAINHAND;
        }
        else
        {
            slot = EquipmentSlot::SLOTS[slotIndex];
        }
    }

    if (isDisabled(slot) || (slot->getType() == EquipmentSlot::HAND && !isShowArms()))
    {
        return false;
    }
    return swapItem(player, slot, item, hand);
}
unsigned int ArmorStand::GetInteractTooltip(const EntityTooltipDataHolder& tooltipData)
{
    if (!tooltipData.m_bAllowedToInteract)
    {
        return 0xffffffffU;
    }
    if (!tooltipData.m_pItemInstance->isEmpty()
        && tooltipData.m_pItemInstance->hasCustomHoverName())
    {
        return 0x5592c692U;
    }
    if (tooltipData.m_pPlayer && !tooltipData.m_pItemInstance->isEmpty())
    {
        return 0x3c2d9ad6U;
    }
    return 0xffffffffU;
}
unsigned int ArmorStand::GetHitTooltip(const EntityTooltipDataHolder& tooltipData)
{
    return tooltipData.m_bAllowedToHurt ? 0x5fb7a25aU : 0xffffffffU;
}
void ArmorStand::playBrokenSound() {}
void ArmorStand::brokenByAnything() {}
void ArmorStand::causeDamage(float damage)
{
    float remainingHealth = getHealth() - damage;
    if (remainingHealth <= 0.5f)
    {
        brokenByAnything();
        remove();
    }
    else
    {
        setHealth(remainingHealth);
    }
}
void ArmorStand::showBreakingParticles() {}
void ArmorStand::brokenByPlayer() {}
bool ArmorStand::hurt(const DamageSource* source, float damage)
{
    if (source == NULL || isMarker() || isInvulnerableTo(source))
    {
        return false;
    }
    if (source == DamageSource::FALL)
    {
        return false;
    }
    if (source->isExplosion())
    {
        brokenByAnything();
        remove();
        return false;
    }
    if (source->isCreativePlayer())
    {
        playBrokenSound();
        showBreakingParticles();
        brokenByPlayer();
        remove();
        return true;
    }
    causeDamage(damage > 0.0f ? damage : 0.15f);
    return true;
}
void ArmorStand::handleEntityEvent(unsigned char event, int data)
{
    if (event == 0x20)
    {
        playBrokenSound();
        mLastSignal = mLevel == NULL ? data : (int)mLevel->getGameTime();
    }
    else
    {
        LivingEntity::handleEntityEvent(event, data);
    }
}
bool ArmorStand::shouldRenderAtSqrDistance(double distanceSqr) { return distanceSqr < 4096.0; }
void ArmorStand::setInvisible(bool invisible) { Entity::setInvisible(invisible); }
bool ArmorStand::isBaby() { return false; }
void ArmorStand::kill() { remove(); }
bool ArmorStand::ignoreExplosion() { return true; }
int ArmorStand::getPistonPushReaction() { return 0; }
bool ArmorStand::isAffectedByPotions() { return false; }
void ArmorStand::onSyncedDataUpdated(const EntityDataAccessor_Base* accessor)
{
    if (accessor != NULL && accessor->getId() >= 0)
    {
        setSize(0.5f, 1.975f);
    }
    LivingEntity::onSyncedDataUpdated(accessor);
}
bool ArmorStand::attackable() { return false; }
Vec3* ArmorStand::getHeadPose() { return NULL; }
Vec3* ArmorStand::getBodyPose() { return NULL; }
Vec3* ArmorStand::getLeftArmPose() { return NULL; }
Vec3* ArmorStand::getRightArmPose() { return NULL; }
Vec3* ArmorStand::getLeftLegPose() { return NULL; }
Vec3* ArmorStand::getRightLegPose() { return NULL; }
Vec3* ArmorStand::getRightItemPose() { return NULL; }
bool ArmorStand::isPickable() { return !mRemoved; }
const HumanoidArm* ArmorStand::getMainArm() { return HumanoidArm::RIGHT; }
float ArmorStand::tickHeadTurn(float bodyRot, float headTurnDelta)
{
    mYBodyRotO = mYRotO;
    mYHeadRotO = mYRotO;
    if (headTurnDelta == 0.0f)
    {
        mYBodyRot = bodyRot;
    }
    return 0.0f;
}
double ArmorStand::getRidingHeight() { return 0.0; }
double ArmorStand::getEyeHeight() { return mMarker ? 0.0 : (mFlags & 1 ? 0.9875 : 1.8075); }
void ArmorStand::travel(float xxa, float yya, float zza)
{
    if (!isNoGravity())
    {
        LivingEntity::travel(xxa, yya, zza);
    }
}
void ArmorStand::setYBodyRot(float bodyRot) { LivingEntity::setYBodyRot(bodyRot); }
void ArmorStand::setYHeadRot(float headRot) { LivingEntity::setYHeadRot(headRot); }
bool ArmorStand::updatePoseForSignal(const BlockPos& pos, const Direction* direction)
{
    BlockPos inputPos = pos.relative(direction->getOpposite());
    if (mLevel->hasSignal(inputPos, direction))
    {
        int signal = mLevel->getSignal(pos.relative(direction), direction);
        if (signal > 0)
        {
            if (signal != mLastSignal)
            {
                int pose = signal > 13 ? 13 : signal;
                if (getPoseIndex() != pose - 1)
                {
                    setPoseIndex(pose - 1);
                }
            }
            mLastSignal = signal;
            return true;
        }
    }
    return false;
}
void ArmorStand::updateBoundingBox(bool marker)
{
    if (marker)
    {
        setSize(0.0f, 0.0f);
    }
    else
    {
        setSize(0.5f, 1.975f);
    }
}
void ArmorStand::tick() { LivingEntity::tick(); }
const SoundEvent* ArmorStand::getHurtSound(const DamageSource*) { return SoundEvents::ARMORSTAND_HIT; }
const SoundEvent* ArmorStand::getDeathSound() { return SoundEvents::ARMORSTAND_BREAK; }
const SoundEvent* ArmorStand::getFallDamageSound(int) { return SoundEvents::ARMORSTAND_FALL; }
void ArmorStand::setSize(float width, float height) { Entity::setSize(width, height); }
void ArmorStand::defineSynchedData() { LivingEntity::defineSynchedData(); }
void ArmorStand::doPush(boost::shared_ptr<Entity>) {}
void ArmorStand::pushEntities() {}
void ArmorStand::updateInvisibilityStatus() { LivingEntity::updateInvisibilityStatus(); }
bool ArmorStand::generatesBubbles() { return false; }
void ArmorStand::setAirSupply(int airSupply) { Entity::setAirSupply(airSupply); }
unsigned long long ArmorStand::GetType() { return ENTITY_TYPE_ARMOR_STAND; }
void ArmorStand::thunderHit(const LightningBolt*) {}
void ArmorStand::create(Level* level) { new ArmorStand(level); }

bool ArmorStand::RidableMinecartEntitySelector::apply(boost::shared_ptr<Entity> entity) const
{
    return entity.get() != NULL && entity->isVehicle();
}

ArmorStand::RidableMinecartEntitySelector::~RidableMinecartEntitySelector()
{
}
