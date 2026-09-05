#include "game/item/DyeItem.hpp"

#include <cmath>
#include <string>

#include "game/client/render/ParticleType.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/core/InteractionHand.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BonemealableBlock.hpp"
#include "game/world/block/Block.hpp"

class Player
{
public:
    not_null_ptr<ItemInstance> getItemInHand(InteractionHand::EInteractionHand hand);
};

class EntityAccess
{
public:
    virtual unsigned long long GetType() = 0;
};

class Sheep
{
public:
    bool isSheared();
    int getColor();
    void setColor(const DyeColor* color);
};

DyeItem::DyeItem()
    : Item()
{
    setStackedByData(true);
    setMaxDamage(0);
    for (int i = 0; i < 16; ++i)
    {
        mIcons[i] = NULL;
    }
}

DyeItem::~DyeItem()
{
}

int DyeItem::getDescriptionId(not_null_ptr<ItemInstance> itemInstance)
{
    return DyeColor::byItemData(itemInstance->getAuxValue())->getName();
}

int DyeItem::getUseDescriptionId(not_null_ptr<ItemInstance> itemInstance)
{
    return DyeColor::byItemData(itemInstance->getAuxValue())->getUseName();
}

bool DyeItem::growCrop(not_null_ptr<ItemInstance> itemInstance, Level* level,
                       const BlockPos& pos, bool simulate)
{
    const BlockState* state = level->getBlockState(pos);
    if (state == NULL)
    {
        return false;
    }
    BonemealableBlock* crop = dynamic_cast<BonemealableBlock*>(state->getBlock());
    if (crop == NULL || !crop->isValidBonemealTarget(level, pos, state, level->mIsClientSide) ||
        !crop->isBonemealSuccess(level, level->mRandom, pos, state))
    {
        return false;
    }
    if (!simulate)
    {
        crop->performBonemeal(level, level->mRandom, pos, state);
        itemInstance->shrink(1);
    }
    return true;
}

InteractionResult::EInteractionResult DyeItem::useOn(boost::shared_ptr<Player> player,
    Level* level, const BlockPos& pos, InteractionHand::EInteractionHand hand,
    const Direction* direction, float hitX, float hitY, float hitZ, bool simulate)
{
    const BlockPos relativePos = pos.relative(direction);
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    const DyeColor* color = DyeColor::byItemData(itemInstance->getAuxValue());
    if (color == DyeColor::WHITE && growCrop(itemInstance, level, pos, simulate))
    {
        if (!level->mIsClientSide)
        {
            level->levelEvent(2005, pos, 0);
        }
        return InteractionResult::eInteractionResult_Success;
    }
    const BlockState* state = level->getBlockState(pos);
    if (state != NULL && color == DyeColor::WHITE &&
        growCrop(itemInstance, level, relativePos, simulate))
    {
        if (!level->mIsClientSide)
        {
            level->levelEvent(2005, relativePos, 0);
        }
        return InteractionResult::eInteractionResult_Success;
    }
    return InteractionResult::eInteractionResult_Pass;
}

void DyeItem::addGrowthParticles(Level* level, const BlockPos& pos, int count)
{
    if (count == 0)
    {
        count = 15;
    }
    const BlockState* state = level->getBlockState(pos);
    if (state == NULL || count <= 0)
    {
        return;
    }
    for (int i = 0; i < count; ++i)
    {
        const double x = (double)pos.x + level->mRandom->nextFloat();
        const double y = (double)pos.y + level->mRandom->nextFloat();
        const double z = (double)pos.z + level->mRandom->nextFloat();
        level->addParticle(PARTICLE_HAPPY_VILLAGER, x, y, z,
                           level->mRandom->nextGaussian() * 0.02,
                           level->mRandom->nextGaussian() * 0.02,
                           level->mRandom->nextGaussian() * 0.02,
                           arrayWithLength<int>());
    }
}

bool DyeItem::interactEnemy(not_null_ptr<ItemInstance> itemInstance,
    boost::shared_ptr<Player> player, boost::shared_ptr<LivingEntity> target,
    InteractionHand::EInteractionHand hand)
{
    if (static_cast<EntityAccess*>(static_cast<void*>(target.get()))->GetType() != ENTITY_TYPE_SHEEP)
    {
        return false;
    }
    Sheep* sheep = static_cast<Sheep*>(static_cast<void*>(target.get()));
    const DyeColor* color = DyeColor::byItemData(itemInstance->getAuxValue());
    if (!sheep->isSheared() && sheep->getColor() != color->getItemData())
    {
        sheep->setColor(color);
        itemInstance->shrink(1);
    }
    return true;
}

TextureAtlasSprite* DyeItem::getIcon(int auxValue)
{
    return mIcons[Mth::clamp(auxValue, 0, 15)];
}

void DyeItem::registerIcons(IconRegister* iconRegister)
{
    for (int i = 0; i < 16; ++i)
    {
        std::wstring iconName = getIconName() + L"_" + DyeColor::byItemData(i)->getSerializedName();
        mIcons[i] = iconRegister->registerIcon(iconName);
    }
}
