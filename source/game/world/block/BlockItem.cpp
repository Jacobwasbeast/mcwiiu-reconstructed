#include "game/world/block/BlockItem.hpp"

#include "game/client/audio/SoundSource.hpp"
#include "game/client/audio/SoundType.hpp"
#include "game/core/Direction.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/Mth.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/ConduitBlockEntity.hpp"
#include "game/world/generation/Player.hpp"
#include "game/core/IconRegister.hpp"

static boost::shared_ptr<Player> currentlyPlacingBlockPlayer;

BlockItem::BlockItem(Block* block)
    : Item(), mBlock(block), mIcon(NULL)
{
}

BlockItem::~BlockItem()
{
}

Block* BlockItem::getBlock()
{
    return mBlock;
}

BlockItem* BlockItem::setDescriptionId(unsigned int descriptionId)
{
    Item::setDescriptionId((int)descriptionId);
    return this;
}

InteractionResultHolder BlockItem::use(Level* level, boost::shared_ptr<Player> player,
                                       InteractionHand::EInteractionHand hand)
{
    return Item::use(level, player, hand);
}

bool BlockItem::updateCustomBlockEntityTag(Level* level, boost::shared_ptr<Player> player,
                                            const BlockPos& pos, not_null_ptr<ItemInstance> itemInstance)
{
    if (level->getServer() == NULL)
    {
        return false;
    }

    CompoundTag* tag = (CompoundTag*)itemInstance->getTagElement(L"BlockEntityTag");
    if (tag == NULL)
    {
        return true;
    }

    boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(pos);
    if (blockEntity.get() != NULL && !level->mIsClientSide && blockEntity->onlyOpCanSetNbt()
        && (player.get() == NULL || !player->canUseCommandBlocks()))
    {
        return false;
    }

    if (blockEntity.get() == NULL)
    {
        return true;
    }

    CompoundTag* entityTag = new CompoundTag;
    CompoundTag* savedTag = blockEntity->save(entityTag);
    CompoundTag* originalTag = savedTag->copy();
    entityTag->merge(tag);
    entityTag->putInt(L"x", pos.x);
    entityTag->putInt(L"y", pos.y);
    entityTag->putInt(L"z", pos.z);
    if (!entityTag->equals(originalTag))
    {
        blockEntity->load(entityTag);
        blockEntity->setChanged();
    }
    return true;
}

InteractionResult::EInteractionResult BlockItem::useOn(boost::shared_ptr<Player> player,
                                                        Level* level,
                                                        const BlockPos& pos,
                                                        InteractionHand::EInteractionHand hand,
                                                        const Direction* direction,
                                                        float hitX,
                                                        float hitY,
                                                        float hitZ,
                                                        bool simulate)
{
    BlockPos placePos = pos;
    const BlockState* state = level->getBlockState(placePos);
    Block* block = state->getBlock();
    const Direction* placeDirection = direction;
    if (block == Blocks::AIR || block == Blocks::tallgrass || block == Blocks::sea_grass)
    {
        placeDirection = Direction::UP;
    }
    else if (!block->mayReplaceWithPlace(level, placePos))
    {
        placePos = placePos.relative(direction);
    }

    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    if (itemInstance->isEmpty() || !player->mayUseItemAt(placePos, placeDirection, itemInstance)
        || !level->mayPlace(mBlock, placePos, false, placeDirection, boost::shared_ptr<Entity>()))
    {
        return InteractionResult::eInteractionResult_Fail;
    }

    boost::shared_ptr<Player> soundPlayer = player;
    if (mBlock == Blocks::beacon)
    {
        soundPlayer.reset();
        if (level->mIsClientSide)
        {
            return InteractionResult::eInteractionResult_Success;
        }
    }

    if (!simulate)
    {
        int auxValue = getLevelDataForAuxValue(itemInstance->getAuxValue());
        const BlockState* placedState =
            mBlock->getStateForPlacement(level, placePos, placeDirection, hitX, hitY, hitZ,
                                         auxValue, player);
        currentlyPlacingBlockPlayer = player;
        if (!level->setBlock(placePos, placedState, 11, false))
        {
            if (mBlock == Blocks::beacon)
            {
                player->displayClientMessage((int)0xef12cc40, true);
                currentlyPlacingBlockPlayer.reset();
                return InteractionResult::eInteractionResult_Fail;
            }
        }
        else
        {
            player->awardStat(gStats->get_blocksPlaced(Block::getId(mBlock)),
                              GenericStats::param_blocksPlaced(placedState, 1));
            player->swing(hand);

            const BlockState* actualState = level->getBlockState(placePos);
            if (actualState->getBlock() == mBlock)
            {
                updateCustomBlockEntityTag(level, player, placePos, itemInstance);
                mBlock->setPlacedBy(level, placePos, actualState, player, itemInstance);
            }

            boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(placePos);
            if (blockEntity.get() != NULL && blockEntity->GetType() == 0x01200212ULL)
            {
                boost::shared_ptr<ConduitBlockEntity> conduit =
                    boost::static_pointer_cast<ConduitBlockEntity>(blockEntity);
                int rotation = Mth::absFloor((double)(player->mYRot * 16.0f / 360.0f) + 0.5) & 15;
                conduit->mActiveRotation = (float)rotation * 360.0f / 16.0f;
            }

            const SoundType* soundType = mBlock->getSoundType();
            level->playSound(soundPlayer, placePos, soundType->getPlaceSound(),
                             SoundSource::eSoundSource_Blocks,
                             (soundType->getVolume() + 1.0f) * 0.5f,
                             soundType->getPitch() * 0.8f, 16.0f);
            itemInstance->shrink(1);
        }
    }

    currentlyPlacingBlockPlayer.reset();
    return InteractionResult::eInteractionResult_Success;
}

int BlockItem::getDescriptionId(not_null_ptr<ItemInstance>)
{
    return mBlock->getDescriptionId(-1);
}

int BlockItem::getDescriptionId(int)
{
    return mBlock->getDescriptionId(-1);
}

int BlockItem::getUseDescriptionId(not_null_ptr<ItemInstance>)
{
    return mBlock->getUseDescriptionId(-1);
}

int BlockItem::getUseDescriptionId()
{
    return mBlock->getUseDescriptionId(-1);
}

bool BlockItem::mayPlace(Level* level, const BlockPos& pos, const Direction* direction,
                         boost::shared_ptr<Player>, not_null_ptr<ItemInstance>)
{
    BlockPos placePos = pos;
    const BlockState* state = level->getBlockState(placePos);
    Block* block = state->getBlock();
    const Direction* placeDirection = direction;
    if (block == Blocks::AIR || block == Blocks::tallgrass)
    {
        placeDirection = Direction::UP;
    }
    else if (!block->mayReplaceWithPlace(level, placePos))
    {
        placePos = placePos.relative(direction);
    }
    return level->mayPlace(mBlock, placePos, false, placeDirection, boost::shared_ptr<Entity>());
}

void BlockItem::appendHoverText(not_null_ptr<ItemInstance> itemInstance,
                                boost::shared_ptr<Player> player,
                                std::vector<HtmlString>* lines,
                                bool advanced)
{
    Item::appendHoverText(itemInstance, player, lines, advanced);
    mBlock->appendHoverText(itemInstance, player, lines, advanced);
}

int BlockItem::getIconType()
{
    return !mBlock->getTileItemIconName().empty();
}

TextureAtlasSprite* BlockItem::getIcon(int auxValue)
{
    if (mIcon != NULL)
    {
        return mIcon;
    }
    return mBlock->getIconTexture(Direction::UP, mBlock->getBlockState(auxValue));
}

void BlockItem::registerIcons(IconRegister* iconRegister)
{
    std::wstring iconName = mBlock->getTileItemIconName();
    if (!iconName.empty())
    {
        mIcon = iconRegister->registerIcon(iconName);
    }
}

boost::shared_ptr<Player> BlockItem::fjGetCurrentlyPlacingBlockPlayer()
{
    return currentlyPlacingBlockPlayer;
}
