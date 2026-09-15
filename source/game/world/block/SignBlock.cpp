#include "game/world/block/SignBlock.hpp"

#include "game/core/IconRegister.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/SignBlockEntity.hpp"

SignBlock::SignBlock()
    : BaseEntityBlock(Material::WOOD)
{
}

SignBlock::~SignBlock()
{
}

boost::shared_ptr<BlockEntity> SignBlock::newBlockEntity(Level* level, int)
{
    boost::shared_ptr<SignBlockEntity> entity(new SignBlockEntity());
    entity->setLevel(level);
    return entity;
}

const AABB* SignBlock::getShape(const BlockState*, LevelSource*, const BlockPos&)
{
    return NULL;
}

const AABB* SignBlock::getClipAABB(const BlockState*, LevelSource*, const BlockPos&)
{
    return NULL;
}

bool SignBlock::isCubeShaped(const BlockState*)
{
    return false;
}

bool SignBlock::hasCustomBreakingProgress(const BlockState*)
{
    return true;
}

bool SignBlock::isPathfindable(LevelSource*, const BlockPos&)
{
    return true;
}

bool SignBlock::isSolidRender(const BlockState*) const
{
    return false;
}

bool SignBlock::isPossibleToRespawnInThis()
{
    return true;
}

Item* SignBlock::getResource(const BlockState*, Random*, int)
{
    return Items::sign;
}

not_null_ptr<ItemInstance> SignBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState*)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(Items::sign));
}

bool SignBlock::use(Level* level, const BlockPos& pos, const BlockState*, boost::shared_ptr<Player> player,
                    InteractionHand::EInteractionHand, const Direction*, float, float, float, bool clientSide)
{
    if (clientSide)
        return true;

    boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(pos);
    if (blockEntity.get() == NULL || blockEntity->GetType() != 0x01000202ULL)
        return false;

    boost::shared_ptr<SignBlockEntity> sign = boost::static_pointer_cast<SignBlockEntity>(blockEntity);
    return sign->executeClickCommands(player);
}

bool SignBlock::mayPlace(Level* level, const BlockPos& pos)
{
    return !touchesCactus(level, pos) && Block::mayPlace(level, pos);
}

EPistonPushReaction SignBlock::getPistonPushReaction(const BlockState*)
{
    return PISTON_PUSH_DESTROY;
}

EBlockFaceShape SignBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction*)
{
    return BLOCK_FACE_UNDEFINED;
}

unsigned int SignBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    unsigned int result = Block::GetInteractTooltip(tooltipData);
    if (tooltipData.m_pPlayer != NULL && !tooltipData.m_pPlayer->isSneaking()
        && (result == 0x4C36CD96u || result == 0xCE1819A5u))
        return 0xFFFFFFFFu;
    return result;
}

TextureAtlasSprite* SignBlock::getTexture(const Direction* direction, const BlockState* state)
{
    return Blocks::planks->getTexture(direction, state);
}

void SignBlock::registerIcons(IconRegister*)
{
}

EBlockRenderShape SignBlock::getRenderShape(const BlockState*)
{
    return static_cast<EBlockRenderShape>(-1);
}
