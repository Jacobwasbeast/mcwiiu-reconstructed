#include "game/item/BedItem.hpp"

#include "game/client/audio/SoundSource.hpp"
#include "game/client/audio/SoundType.hpp"
#include "game/core/Direction.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Stats.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BedBlock.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/generation/Player.hpp"

static const unsigned int BED_DESCRIPTION_IDS[16] = {
    0x6BD9B3C4, 0x70D97B2B, 0x6E73A676, 0xC985D454,
    0x29C060C7, 0xC872BBEE, 0x2FFA2DDF, 0x6C532C86,
    0x4B6015EC, 0x0702020E, 0x6E39E7BD, 0xD5F99695,
    0x09C5C185, 0x9EF3B3D0, 0xF0DD2368, 0x5555A4BC
};

BedItem::BedItem()
    : Item(),
      mBedIcon(NULL)
{
    setMaxDamage(0);
    setStackedByData(true);
}

BedItem::~BedItem()
{
}

InteractionResult::EInteractionResult BedItem::useOn(boost::shared_ptr<Player> player,
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

    BlockPos footPos = pos.relative(direction);
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);

    if (!player->mayUseItemAt(footPos, direction, itemInstance)
        || !level->mayPlace(Blocks::bed, footPos, false, direction, boost::shared_ptr<Entity>()))
    {
        return InteractionResult::eInteractionResult_Fail;
    }

    const Direction* facing = Direction::fromYRot((double)player->mYRot);
    BlockPos headPos = footPos.relative(facing);

    if (!level->mayPlace(Blocks::bed, headPos, false, direction, boost::shared_ptr<Entity>()))
    {
        return InteractionResult::eInteractionResult_Fail;
    }

    if (!level->mIsClientSide && !simulate)
    {
        TypedBoxed<const Direction*> facingValue(facing);
        const BlockState* state = Blocks::bed->defaultBlockState()->setBoxedValue(
            HorizontalDirectionalBlock::FACING_PROPERTY, &facingValue);
        level->setBlock(footPos, state, 3, false);
        level->setBlock(headPos, state, 3, false);

        const SoundType* soundType = Blocks::bed->getSoundType();
        player->awardStat(Stats::blockPlacedStats.data[Block::getId(Blocks::bed)],
                          GenericStats::param_blocksPlaced(Blocks::bed->getBlockState(itemInstance->getAuxValue()), 1));
        level->playSound(player, footPos, soundType->getPlaceSound(), SoundSource::eSoundSource_Blocks,
                         (soundType->getVolume() + 1.0f) * 0.5f,
                         soundType->getPitch() * 0.8f,
                         16.0f);
        itemInstance->shrink(1);
    }

    return InteractionResult::eInteractionResult_Success;
}

TextureAtlasSprite* BedItem::getLayerIcon(int auxValue, int layer, not_null_ptr<ItemInstance> itemInstance)
{
    if (layer != 0)
    {
        return Item::getLayerIcon(auxValue, layer, itemInstance);
    }

    return mBedIcon;
}

bool BedItem::hasMultipleSpriteLayers()
{
    return true;
}

void BedItem::registerIcons(IconRegister* iconRegister)
{
    Item::registerIcons(iconRegister);
    mBedIcon = iconRegister->registerIcon(getIconName());
}

int BedItem::getDescriptionId(not_null_ptr<ItemInstance> itemInstance)
{
    unsigned int auxValue = (unsigned int)itemInstance->getAuxValue();
    if (auxValue > 15)
    {
        auxValue = 0;
    }
    return (int)BED_DESCRIPTION_IDS[auxValue];
}
