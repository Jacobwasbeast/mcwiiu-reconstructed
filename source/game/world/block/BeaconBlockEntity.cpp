#include "game/world/block/BeaconBlockEntity.hpp"
#include "game/network/ClientboundContainerOpenPacket.hpp"
#include "game/core/EntityType.hpp"

#include <algorithm>
#include <cstring>
#include <typeinfo>

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/world/generation/Player.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/network/ClientboundBlockEntityDataPacket.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/util/AABB.hpp"
#include "game/ui/BeaconMenu.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/StainedGlassBlock.hpp"
#include "game/world/block/StainedGlassPaneBlock.hpp"

namespace
{
MobEffect* sBeaconEffects[12] = {0};
MobEffect* sBeaconEffectOrder[6] = {0};
int sBeaconEffectCount = 0;
}

BeaconBlockEntity::BeaconBeamSection::BeaconBeamSection(arrayWithLength<float> color)
    : mRed(color.length > 0 ? color.data[0] : 0.0F),
      mGreen(color.length > 1 ? color.data[1] : 0.0F),
      mBlue(color.length > 2 ? color.data[2] : 0.0F), mHeight(1)
{
}

float* BeaconBlockEntity::BeaconBeamSection::getColor()
{
    return &mRed;
}

void BeaconBlockEntity::BeaconBeamSection::setHeight(int height)
{
    mHeight = height;
}

void BeaconBlockEntity::BeaconBeamSection::increaseHeight()
{
    ++mHeight;
}

unsigned int BeaconBlockEntity::BeaconBeamSection::getHeight()
{
    return mHeight;
}

BeaconBlockEntity::BeaconBlockEntity()
{
    mPrimaryPower = NULL;
    mSecondaryPower = NULL;
    mNextClientSideUpdate = 0;
    mLevels = -1;
    mClientSideScale = 0.0F;
    mActive = false;
    mItem = ItemInstance::EMPTY;
    mName = boost::shared_ptr<CensoredString>(new CensoredString());
    mBeamSections = std::vector<BeaconBeamSection>();
    mBeamDirection = 0;
    mBeamLength = 0;
    mLastBeamBlockCount = -1;
    mBeamRebuildTicks = 15;
    for (int i = 0; i < 16; ++i)
    {
        mBeamColorCounts[i] = 0;
    }
    mTickCount = 0;
}

BeaconBlockEntity::~BeaconBlockEntity()
{
}

std::wstring BeaconBlockEntity::getName()
{
    if (hasCustomName())
    {
        return mName->get();
    }
    return std::wstring(L"");
}

std::wstring BeaconBlockEntity::getCustomName()
{
    if (hasCustomName())
    {
        return mName->get();
    }
    return std::wstring(L"");
}

bool BeaconBlockEntity::hasCustomName()
{
    if (!mName->checkParentalFilter())
    {
        return false;
    }
    return mName->raw().length() != 0;
}

unsigned int BeaconBlockEntity::getContainerSize()
{
    return 1;
}

bool BeaconBlockEntity::isEmpty()
{
    return mItem->isEmpty();
}

not_null_ptr<ItemInstance> BeaconBlockEntity::getItem(unsigned int slot)
{
    if (slot == 0)
    {
        return mItem;
    }
    return ItemInstance::EMPTY;
}

not_null_ptr<ItemInstance> BeaconBlockEntity::removeItem(unsigned int slot, int count)
{
    if (slot == 0 && !mItem->isEmpty())
    {
        int stackCount = mItem->getCount();
        if (stackCount <= count)
        {
            not_null_ptr<ItemInstance> result = mItem;
            mItem = ItemInstance::EMPTY;
            return result;
        }
        mItem->split(count);
    }
    return ItemInstance::EMPTY;
}

not_null_ptr<ItemInstance> BeaconBlockEntity::removeItemNoUpdate(int slot)
{
    if (slot != 0)
    {
        return ItemInstance::EMPTY;
    }
    not_null_ptr<ItemInstance> result = mItem;
    mItem = ItemInstance::EMPTY;
    return result;
}

void BeaconBlockEntity::setItem(unsigned int slot, not_null_ptr<ItemInstance> item)
{
    if (slot == 0)
    {
        mItem = item;
        if (mItem != NULL && mItem->getCount() > getMaxStackSize())
        {
            mItem->setCount(getMaxStackSize());
        }
        setChanged();
    }
}

int BeaconBlockEntity::getMaxStackSize()
{
    return 1;
}

void BeaconBlockEntity::setChanged()
{
    BlockEntity::setChanged();
}

bool BeaconBlockEntity::stillValid(boost::shared_ptr<Player> player)
{
    Entity* entity = (Entity*)player.get();
    return entity->distanceAsIfHitscanned(mPos, *entity->mBoundingBox) <= 30.25;
}

void BeaconBlockEntity::startOpen(boost::shared_ptr<Player> player)
{
}

void BeaconBlockEntity::stopOpen(boost::shared_ptr<Player> player)
{
}

bool BeaconBlockEntity::canPlaceItem(int, not_null_ptr<ItemInstance> item)
{
    return item->getItem() == Items::emerald || item->getItem() == Items::diamond
        || item->getItem() == Items::goldIngot || item->getItem() == Items::ironIngot;
}

int BeaconBlockEntity::getContainerData(int id)
{
    switch (id)
    {
    case 0:
        return mLevels;
    case 1:
        return MobEffect::getId(mPrimaryPower);
    case 2:
        return MobEffect::getId(mSecondaryPower);
    default:
        return 0;
    }
}

MobEffect* BeaconBlockEntity::getValidEffectById(int id)
{
    MobEffect* effect = MobEffect::byId(id);
    for (int i = 0; i < sBeaconEffectCount; ++i)
    {
        if (sBeaconEffectOrder[i] == effect)
            return effect;
    }
    return NULL;
}

void BeaconBlockEntity::setContainerData(int id, int value)
{
    switch (id)
    {
    case 0:
        mLevels = value;
        break;
    case 1:
        mPrimaryPower = getValidEffectById(value);
        mLevel->playSound(boost::shared_ptr<Player>(), getBlockPos(), SoundEvents::BEACON_POWER_SELECT,
                          SoundSource::eSoundSource_Blocks, 1.0F, 1.0F, 1.0F);
        break;
    case 2:
        mSecondaryPower = getValidEffectById(value);
        break;
    }
}

int BeaconBlockEntity::getContainerDataCount()
{
    return 3;
}

void BeaconBlockEntity::clearContent()
{
    mItem = ItemInstance::EMPTY;
}

AbstractContainerMenu* BeaconBlockEntity::createMenu(boost::shared_ptr<Inventory> inventory, boost::shared_ptr<Player> player)
{
    (void)player;
    boost::shared_ptr<Container> beacon = boost::dynamic_pointer_cast<Container>(shared_from_this());
    boost::shared_ptr<Container> playerContainer = boost::static_pointer_cast<Container>(inventory);
    return new BeaconMenu(playerContainer, beacon);
}

int BeaconBlockEntity::getTypeId(bool large)
{
    return ClientboundContainerOpenPacket::BEACON;
}

InteractionObject::NameData BeaconBlockEntity::GetNetworkNameData()
{
    return InteractionObject::NameData(mName->raw(), mName->getUUID(), mName->isUUIDLocal(), mName->isChatCensored());
}

arrayWithLength<int> BeaconBlockEntity::getSlotsForFace(const Direction* face)
{
    return arrayWithLength<int>();
}

bool BeaconBlockEntity::canPlaceItemThroughFace(int slot, not_null_ptr<ItemInstance> item, const Direction* face)
{
    return canPlaceItem(slot, item);
}

bool BeaconBlockEntity::canTakeItemThroughFace(int slot, not_null_ptr<ItemInstance> item, const Direction* face)
{
    return slot == 0;
}

void BeaconBlockEntity::setCustomName(const std::wstring& name, const std::wstring& uuid, bool chatCensored, bool uuidLocal)
{
    if (mName != NULL) mName->set(name, uuid, chatCensored, uuidLocal);
}

int BeaconBlockEntity::getBeamDirection()
{
    if (Minecraft::GetInstance()->GetMiniGame()->HasFeatureCustomBeacons())
        return mBeamDirection;
    return eBEAM_DIRECTION_POSITIVE_Y;
}

void BeaconBlockEntity::updateShapeForGlide()
{
    mLevels = 0;
    mBeamSections.clear();
    mActive = true;
    if (getBeamDirection() != eBEAM_DIRECTION_NONE)
    {
        float color[3] = { 1.0F, 1.0F, 1.0F };
        mBeamSections.push_back(BeaconBeamSection(arrayWithLength<float>(color, 3)));
        BeaconBeamSection& section = mBeamSections[mBeamSections.size() - 1];
        if (getBeamDirection() != eBEAM_DIRECTION_NEGATIVE_Y)
        {
            section.setHeight(mBeamLength);
        }
        else
        {
            section.setHeight(-mBeamLength);
        }
    }
    else
    {
        mActive = false;
    }
}

bool BeaconBlockEntity::NeedsRebuild()
{
    short colorCounts[16] = {0};
    short blockCount = 0;
    for (int y = mPos.y + 1; y < mLevel->getHeight(); ++y)
    {
        BlockPos pos(mPos.x, y, mPos.z);
        const BlockState* state = mLevel->getBlockState(pos);
        if (state->getBlock() == Blocks::air || state->getBlock() == Blocks::water)
            continue;
        ++blockCount;
        if (state->getBlock() == Blocks::stained_glass)
            ++colorCounts[state->getValue(StainedGlassBlock::COLOR)->getBlockData()];
        else if (state->getBlock() == Blocks::stained_glass_pane)
            ++colorCounts[state->getValue(StainedGlassPaneBlock::COLOR)->getBlockData()];
    }

    bool unchanged = true;
    for (int i = 0; i < 16; ++i)
    {
        if (colorCounts[i] != mBeamColorCounts[i])
        {
            unchanged = false;
            mBeamColorCounts[i] = colorCounts[i];
        }
    }

    if (blockCount != mLastBeamBlockCount)
    {
        mLastBeamBlockCount = blockCount;
        return true;
    }
    return !unchanged;
}

void BeaconBlockEntity::updateShape()
{
    if (Minecraft::GetInstance()->GetMiniGame()->HasFeatureCustomBeacons())
    {
        updateShapeForGlide();
        return;
    }

    int oldLevels = mLevels;
    int x = mPos.x;
    int y = mPos.y;
    int z = mPos.z;

    mLevels = 0;
    for (int layer = 1; layer <= 4; mLevels = layer++)
    {
        int layerY = y - layer;
        if (layerY < 0)
            break;
        bool complete = true;
        for (int layerX = x - layer; layerX <= x + layer && complete; ++layerX)
        {
            for (int layerZ = z - layer; layerZ <= z + layer; ++layerZ)
            {
                Block* block = mLevel->getBlockState(BlockPos(layerX, layerY, layerZ))->getBlock();
                if (block != Blocks::emerald_block && block != Blocks::gold_block && block != Blocks::diamond_block
                    && block != Blocks::iron_block)
                {
                    complete = false;
                    break;
                }
            }
        }
        if (!complete)
            break;
    }
    mActive = mLevels > 0;

    if (mActive)
    {
        if (NeedsRebuild() || --mBeamRebuildTicks == 0)
        {
            mBeamSections.clear();
            mBeamRebuildTicks = 15;

            float color[3] = {1.0F, 1.0F, 1.0F};
            arrayWithLength<const float> white = DyeColor::WHITE->getTextureDiffuseColors();
            memcpy(color, white.data, sizeof(color));
            mBeamSections.push_back(BeaconBeamSection(arrayWithLength<float>(color, 3)));
            BeaconBeamSection* section = &mBeamSections[mBeamSections.size() - 1];

            bool first = true;
            for (int beamY = y + 1; beamY < mLevel->getHeight(); ++beamY)
            {
                BlockPos beamPos(x, beamY, z);
                const BlockState* state = mLevel->getBlockState(beamPos);
                int lightBlock = state->getLightBlock();
                int extraLightBlock = mLevel->getExtraBlockState(beamPos)->getLightBlock();
                if (lightBlock < extraLightBlock)
                    lightBlock = extraLightBlock;

                arrayWithLength<const float> glassColor;
                if (state->getBlock() == Blocks::stained_glass)
                {
                    glassColor = state->getValue(StainedGlassBlock::COLOR)->getTextureDiffuseColors();
                }
                else if (state->getBlock() == Blocks::stained_glass_pane)
                {
                    glassColor = state->getValue(StainedGlassPaneBlock::COLOR)->getTextureDiffuseColors();
                }
                else
                {
                    if (lightBlock < 15 || state->getBlock() == Blocks::bedrock)
                    {
                        section->increaseHeight();
                        continue;
                    }
                    mActive = false;
                    break;
                }

                memcpy(color, glassColor.data, sizeof(color));
                if (!first)
                {
                    color[0] = (section->getColor()[0] + color[0]) * 0.5F;
                    color[1] = (section->getColor()[1] + color[1]) * 0.5F;
                    color[2] = (section->getColor()[2] + color[2]) * 0.5F;
                }
                float* sectionColor = section->getColor();
                if (color[0] == sectionColor[0] && color[1] == sectionColor[1] && color[2] == sectionColor[2])
                {
                    section->increaseHeight();
                }
                else
                {
                    mBeamSections.push_back(BeaconBeamSection(arrayWithLength<float>(color, 3)));
                    section = &mBeamSections[mBeamSections.size() - 1];
                }
                first = false;
            }
        }
    }

    if (!mActive)
    {
        mBeamSections.clear();
        mLastBeamBlockCount = -1;
        for (int i = 0; i < 16; ++i)
            mBeamColorCounts[i] = 0;
        mBeamRebuildTicks = 15;
    }

    if (!mLevel->mIsClientSide && mLevels == 4 && oldLevels < 4)
    {
        AABB* area = AABB::newTemp(x, y, z, x, y - 4, z)->grow(10.0, 5.0, 10.0);
        std::vector<boost::shared_ptr<Entity> >* players = mLevel->getEntitiesOfClass(typeid(Player), area);
        for (std::vector<boost::shared_ptr<Entity> >::iterator it = players->begin(); it != players->end(); ++it)
        {
            boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(*it);
            player->awardStat(GenericStats::fullBeacon(), GenericStats::param_fullBeacon());
        }
        delete players;
    }
}

void BeaconBlockEntity::applyEffects()
{
    if (!mActive || mLevels <= 0 || mLevel->mIsClientSide)
        return;

    if (mPrimaryPower == NULL)
    {
        if (mLevels >= 4 && mSecondaryPower != NULL)
        {
            double range = mLevels * 10 + 10;
            int duration = mLevels * 40 + 180;
            int height = mLevel->getMaxBuildHeight();
            AABB* area = AABB::newTemp(mPos.x, mPos.y, mPos.z, mPos.x + 1, mPos.y + 1, mPos.z + 1)
                             ->grow(range, range, range)
                             ->expand(0.0, height, 0.0);
            std::vector<boost::shared_ptr<Entity> >* players = mLevel->getEntitiesOfClass(typeid(Player), area);
            for (std::vector<boost::shared_ptr<Entity> >::iterator it = players->begin(); it != players->end(); ++it)
            {
                boost::shared_ptr<Player> player = boost::dynamic_pointer_cast<Player>(*it);
                player->addEffect(new MobEffectInstance(mSecondaryPower, duration, 0, true, true),
                                  boost::shared_ptr<Entity>());
            }
            delete players;
        }
        return;
    }

    double range = mLevels * 10 + 10;
    int amplifier = mLevels >= 4 && mPrimaryPower == mSecondaryPower ? 1 : 0;
    int duration = mLevels * 40 + 180;
    int height = mLevel->getMaxBuildHeight();
    AABB* area = AABB::newTemp(mPos.x, mPos.y, mPos.z, mPos.x + 1, mPos.y + 1, mPos.z + 1)
                     ->grow(range, range, range)
                     ->expand(0.0, height, 0.0);
    std::vector<boost::shared_ptr<Entity> >* players = mLevel->getEntitiesOfClass(typeid(Player), area);
    for (std::vector<boost::shared_ptr<Entity> >::iterator it = players->begin(); it != players->end(); ++it)
    {
        boost::shared_ptr<Player> player = boost::dynamic_pointer_cast<Player>(*it);
        player->addEffect(new MobEffectInstance(mPrimaryPower, duration, amplifier, true, true),
                          boost::shared_ptr<Entity>());
    }
    if (mLevels >= 4 && mPrimaryPower != mSecondaryPower && mSecondaryPower != NULL)
    {
        for (std::vector<boost::shared_ptr<Entity> >::iterator it = players->begin(); it != players->end(); ++it)
        {
            boost::shared_ptr<Player> player = boost::dynamic_pointer_cast<Player>(*it);
            player->addEffect(new MobEffectInstance(mSecondaryPower, duration, 0, true, true),
                              boost::shared_ptr<Entity>());
        }
    }
    delete players;
}

void BeaconBlockEntity::checkValidity()
{
    if (mLevel == NULL) return;
    updateShape();
    applyEffects();
}

boost::shared_ptr<BlockEntity> BeaconBlockEntity::clone()
{
    boost::shared_ptr<BeaconBlockEntity> copy(new BeaconBlockEntity());
    BlockEntity::clone(copy);
    copy->mPrimaryPower = mPrimaryPower;
    copy->mSecondaryPower = mSecondaryPower;
    copy->mLevels = mLevels;
    copy->mName->set(mName->raw(), mName->getUUID(), mName->isChatCensored(), mName->isUUIDLocal());
    return copy;
}

void BeaconBlockEntity::AddEffect(int level, int slot, MobEffect* effect)
{
    sBeaconEffects[level * 3 + slot] = effect;
    sBeaconEffectOrder[sBeaconEffectCount++] = effect;
}

void BeaconBlockEntity::staticCtor()
{
    for (int i = 0; i < 12; ++i) sBeaconEffects[i] = NULL;
    AddEffect(0, 0, MobEffects::MOVEMENT_SPEED);
    AddEffect(0, 1, MobEffects::DIG_SPEED);
    AddEffect(1, 0, MobEffects::DAMAGE_RESISTANCE);
    AddEffect(1, 1, MobEffects::JUMP);
    AddEffect(2, 0, MobEffects::DAMAGE_BOOST);
    AddEffect(3, 0, MobEffects::REGENERATION);
}

void BeaconBlockEntity::tick()
{
    bool wasActive = mActive;
    if (!mLevel->mIsClientSide && mLevels < 0)
        checkValidity();
    else if (mLevel->getGameTime() % 80 == 0)
        checkValidity();

    ++mTickCount;
    if (!mLevel->mIsClientSide)
        return;

    BlockPos pos = getBlockPos();
    long long seed = pos.hashCode();
    Random random(seed);
    int phase = mTickCount + random.nextInt(9001);
    if (phase % 80 == 0 && mActive)
    {
        BlockPos soundPos = getBlockPos();
        mLevel->playLocalSound(soundPos.x, getBlockPos().y, getBlockPos().z, SoundEvents::BEACON_AMBIENT,
                               SoundSource::eSoundSource_Blocks, 0.5F, 1.0F, false, 16.0F);
    }

    if (wasActive != mActive)
    {
        BlockPos soundPos = getBlockPos();
        const SoundEvent* sound = mActive ? SoundEvents::BEACON_ACTIVATE : SoundEvents::BEACON_DEACTIVATE;
        mLevel->playLocalSound(soundPos.x, getBlockPos().y, getBlockPos().z, sound, SoundSource::eSoundSource_Blocks,
                               1.0F, 1.0F, false, 16.0F);
    }
}

std::vector<BeaconBlockEntity::BeaconBeamSection>* BeaconBlockEntity::getBeamSections()
{
    return &mBeamSections;
}

double BeaconBlockEntity::getAndUpdateClientSideScale()
{
    float zero = 0.0F;
    if (!mActive)
    {
        return zero;
    }
    int elapsed = (int)(mLevel->getGameTime() - mNextClientSideUpdate);
    mNextClientSideUpdate = mLevel->getGameTime();
    float scale;
    if (elapsed > 1)
    {
        float decayed = mClientSideScale - (float)elapsed / 40.0F;
        if (decayed < zero)
        {
            decayed = zero;
        }
        scale = decayed + 0.025F;
    }
    else
    {
        scale = mClientSideScale + 0.025F;
    }
    if (scale > 1.0F)
    {
        scale = 1.0F;
    }
    mClientSideScale = scale;
    return scale;
}

boost::shared_ptr<Packet> BeaconBlockEntity::getUpdatePacket()
{
    int dimension = mLevel->mDimension->getType()->getId();
    CompoundTag* tag = getUpdateTag();
    return boost::shared_ptr<Packet>(new ClientboundBlockEntityDataPacket(mPos, 3, dimension, tag));
}

CompoundTag* BeaconBlockEntity::getUpdateTag()
{
    return save(new CompoundTag());
}

double BeaconBlockEntity::getViewDistance()
{
    return 65536.0;
}

void BeaconBlockEntity::load(CompoundTag* tag)
{
    BlockEntity::load(tag);
    mName->loadFromTag(tag);
    mPrimaryPower = getValidEffectById(tag->getInt(L"Primary"));
    mSecondaryPower = getValidEffectById(tag->getInt(L"Secondary"));
    mLevels = tag->getInt(L"Levels");
    if (Minecraft::GetInstance()->GetMiniGame()->HasFeatureCustomBeacons())
    {
        mBeamDirection = tag->getInt(L"BeamDirection");
        mBeamLength = tag->getInt(L"BeamLength");
        updateShapeForGlide();
    }
}

CompoundTag* BeaconBlockEntity::save(CompoundTag* tag)
{
    BlockEntity::save(tag);
    mName->saveToTag(tag);
    tag->putInt(L"Primary", MobEffect::getId(mPrimaryPower));
    tag->putInt(L"Secondary", MobEffect::getId(mSecondaryPower));
    tag->putInt(L"Levels", mLevels);
    if (Minecraft::GetInstance()->GetMiniGame()->HasFeatureCustomBeacons())
    {
        tag->putInt(L"BeamDirection", mBeamDirection);
        tag->putInt(L"BeamLength", mBeamLength);
    }
    return tag;
}

bool BeaconBlockEntity::triggerEvent(int id, int param)
{
    if (id == 1)
    {
        checkValidity();
        return true;
    }
    return BlockEntity::triggerEvent(id, param);
}

void BeaconBlockEntity::setBeamDirection(eBEAM_DIRECTION direction)
{
    mBeamDirection = direction;
}

void BeaconBlockEntity::setBeamLength(int length)
{
    mBeamLength = length;
}

std::wstring BeaconBlockEntity::getDisplayName()
{
    return LockableContainerBlockEntity::getDisplayName();
}

BlockEntity* BeaconBlockEntity::create()
{
    return new BeaconBlockEntity();
}

unsigned long long BeaconBlockEntity::GetType()
{
    return BlockEntityType::BEACON;
}
