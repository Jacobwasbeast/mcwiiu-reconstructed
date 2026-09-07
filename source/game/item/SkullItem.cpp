#include "game/item/SkullItem.hpp"

#include <string>

#include "game/core/Direction.hpp"
#include "game/core/Mth.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/SkullBlock.hpp"
#include "game/world/block/SkullBlockEntity.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/generation/Player.hpp"

static const int SKULL_DESCRIPTION_IDS[] = {
    1318443285,
    593636577,
    529811274,
    -1235002899,
    -663435654,
    -1024091147
};

SkullItem::SkullItem()
{
    setMaxDamage(0);
    setStackedByData(true);
}

SkullItem::~SkullItem()
{
}

InteractionResult::EInteractionResult SkullItem::useOn(boost::shared_ptr<Player> player,
                                                        Level* level,
                                                        const BlockPos& pos,
                                                        InteractionHand::EInteractionHand hand,
                                                        const Direction* direction,
                                                        float hitX,
                                                        float hitY,
                                                        float hitZ,
                                                        bool simulate)
{
    if (direction == Direction::DOWN)
    {
        return InteractionResult::eInteractionResult_Fail;
    }

    BlockPos placePos = pos;
    const BlockState* state = level->getBlockState(placePos);
    bool replaceable = state->getBlock()->mayReplaceWithPlace(level, placePos);
    if (!state->getMaterial()->isSolid() && !replaceable)
    {
        return InteractionResult::eInteractionResult_Fail;
    }
    if (replaceable && direction != Direction::UP)
    {
        return InteractionResult::eInteractionResult_Fail;
    }
    if (!replaceable)
    {
        placePos = pos.relative(direction);
    }

    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    if (!player->mayUseItemAt(placePos, direction, itemInstance)
        || !Blocks::skull->mayPlace(level, placePos))
    {
        return InteractionResult::eInteractionResult_Fail;
    }

    if (!level->mIsClientSide && !simulate)
    {
        TypedBoxed<const Direction*> facing(direction);
        level->setBlock(placePos,
                        Blocks::skull->defaultBlockState()->setBoxedValue(SkullBlock::FACING, &facing),
                        11,
                        false);

        int rotation = 0;
        if (direction == Direction::UP)
        {
            rotation = Mth::absFloor((double)(player->mYRot * 16.0f / 360.0f) + 0.5) & 15;
        }

        boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(placePos);
        if (blockEntity && blockEntity->GetType() == 0x0120020700000000ULL)
        {
            boost::shared_ptr<SkullBlockEntity> skull =
                boost::static_pointer_cast<SkullBlockEntity>(blockEntity);
            skull->setSkullType(itemInstance->getAuxValue(), L"");
            skull->setRotation(rotation);
            static_cast<SkullBlock*>(Blocks::skull)->checkMobSpawn(level, placePos, skull, player);
        }

        itemInstance->shrink(1);
    }

    return InteractionResult::eInteractionResult_Success;
}

int SkullItem::getLevelDataForAuxValue(int auxValue)
{
    return auxValue;
}

int SkullItem::getDescriptionId(int auxValue)
{
    if (auxValue > 5)
    {
        auxValue = 0;
    }
    return SKULL_DESCRIPTION_IDS[auxValue];
}

int SkullItem::getDescriptionId(not_null_ptr<ItemInstance> itemInstance)
{
    return getDescriptionId(itemInstance->getAuxValue());
}

std::wstring SkullItem::getName(not_null_ptr<ItemInstance> itemInstance)
{
    if (itemInstance->getAuxValue() == 3 && itemInstance->hasTag())
    {
        CompoundTag* tag = itemInstance->getTag();
        if (tag->contains(L"SkullOwner", 8))
        {
            return tag->getString(L"SkullOwner");
        }
        if (tag->contains(L"SkullOwner", 10))
        {
            CompoundTag* owner = tag->getCompound(L"SkullOwner");
            if (owner->contains(L"Name", 8))
            {
                return owner->getString(L"Name");
            }
        }
    }
    return Item::getName(itemInstance);
}

bool SkullItem::verifyTagAfterLoad(CompoundTag* tag)
{
    return true;
}
