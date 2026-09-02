#include "game/item/ArmorStandItem.hpp"

#include <vector>

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/core/ArmorStand.hpp"
#include "game/core/Direction.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/SpawnEggItem.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/generation/Player.hpp"

ArmorStandItem::ArmorStandItem()
{
}

ArmorStandItem::~ArmorStandItem()
{
}

InteractionResult::EInteractionResult ArmorStandItem::useOn(boost::shared_ptr<Player> player,
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

    if (!level->getBlockState(pos)->getBlock()->mayReplaceWithPlace(level, pos))
    {
        placePos = pos.relative(direction);
    }

    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);

    if (!player->mayUseItemAt(placePos, direction, itemInstance))
    {
        return InteractionResult::eInteractionResult_Fail;
    }

    BlockPos abovePos = placePos.above();

    bool blocked = !level->isEmptyBlock(placePos)
        && !level->getBlockState(placePos)->getBlock()->mayReplaceWithPlace(level, placePos);
    bool blockedAbove = !level->isEmptyBlock(abovePos)
        && !level->getBlockState(abovePos)->getBlock()->mayReplaceWithPlace(level, abovePos);

    if (blocked || blockedAbove)
    {
        return InteractionResult::eInteractionResult_Fail;
    }

    double x = placePos.x;
    double y = placePos.y;
    double z = placePos.z;

    std::vector<boost::shared_ptr<Entity> >* entities = level->getEntities(
        boost::shared_ptr<Entity>(), AABB::minmax(x, y, z, x + 1.0, y + 2.0, z + 1.0), NULL);

    if (entities->size() != 0)
    {
        return InteractionResult::eInteractionResult_Fail;
    }

    if (simulate)
    {
        return InteractionResult::eInteractionResult_Success;
    }

    if (!level->mIsClientSide)
    {
        if (!level->canCreateMore(0x000000000C000001ULL, Level::SPAWN_TYPE_SPAWN_EGG))
        {
            player->displayClientMessage(0x8ECD701E, true);
            return InteractionResult::eInteractionResult_Fail;
        }

        boost::shared_ptr<ArmorStand> armorStand(new ArmorStand(level, x + 0.5, y, z + 0.5));
        float yRot = (float)Mth::floor((Mth::wrapDegrees(player->mYRot - 180.0f) + 22.5f) / 45.0f) * 45.0f;

        armorStand->moveTo(x + 0.5, y, z + 0.5, yRot, 0.0f);

        SpawnEggItem::updateCustomEntityTag(level, player, itemInstance, armorStand);
        level->addEntity(armorStand);
        level->playSound(boost::shared_ptr<Player>(),
                         armorStand->mX,
                         armorStand->mY,
                         armorStand->mZ,
                         SoundEvents::ARMORSTAND_PLACE,
                         SoundSource::eSoundSource_Blocks,
                         0.75f,
                         0.8f,
                         16.0f);
    }

    itemInstance->shrink(1);

    return InteractionResult::eInteractionResult_Success;
}
