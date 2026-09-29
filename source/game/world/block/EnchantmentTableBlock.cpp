#include "game/world/block/EnchantmentTableBlock.hpp"
#include "game/core/StringIds.hpp"

#include "game/client/render/ParticleType.hpp"
#include "game/core/IconRegister.hpp"
#include "game/entity/mob/EnchantmentTableEntity.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"

static const AABB* gEnchantmentTableShape = AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.75, 1.0);

EnchantmentTableBlock::EnchantmentTableBlock()
    : BaseEntityBlock(Material::STONE, MaterialColor::STONE)
{
    DerivedInit();
    setLightBlock(0);
}

EnchantmentTableBlock::~EnchantmentTableBlock()
{
}

boost::shared_ptr<BlockEntity> EnchantmentTableBlock::newBlockEntity(Level*, int data)
{
    boost::shared_ptr<EnchantmentTableEntity> entity(new EnchantmentTableEntity());
    entity->setData(data);
    return entity;
}

const AABB* EnchantmentTableBlock::getShape(const BlockState*, LevelSource*, const BlockPos&)
{
    return gEnchantmentTableShape;
}

bool EnchantmentTableBlock::isCubeShaped(const BlockState*)
{
    return false;
}

void EnchantmentTableBlock::animateTick(const BlockState* state, Level* level, const BlockPos& pos, Random* random)
{
    Block::animateTick(state, level, pos, random);

    for (int x = -2; x <= 2; ++x)
    {
        for (int z = -2; z <= 2; ++z)
        {
            if (x > -2 && x < 2 && z == -1)
            {
                z = 2;
            }

            if (random->nextInt(16) != 0)
            {
                continue;
            }

            for (int y = 0; y <= 1; ++y)
            {
                if (level->getBlockState(pos.offset(x, y, z))->getBlock() != Blocks::bookshelf)
                {
                    continue;
                }

                if (!level->isEmptyBlock(pos.offset(x / 2, 0, z / 2)))
                {
                    break;
                }

                const float xa = (float)x + random->nextFloat();
                const float ya = (float)y - random->nextFloat() - 1.0f;
                const float za = (float)z + random->nextFloat();
                level->addParticle(PARTICLE_ENCHANTMENT_TABLE, (double)pos.x + 0.5, (double)pos.y + 2.0,
                                   (double)pos.z + 0.5, (double)xa - 0.5, (double)ya, (double)za - 0.5,
                                   arrayWithLength<int>());
            }
        }
    }
}

bool EnchantmentTableBlock::isSolidRender(const BlockState*) const
{
    return false;
}

EBlockRenderShape EnchantmentTableBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_MODEL;
}

bool EnchantmentTableBlock::use(Level* level, const BlockPos& pos, const BlockState*,
                                 boost::shared_ptr<Player> player, InteractionHand::EInteractionHand,
                                 const Direction*, float, float, float, bool clientSide)
{
    if (clientSide)
    {
        return false;
    }

    boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(pos);
    boost::shared_ptr<EnchantmentTableEntity> table =
        boost::dynamic_pointer_cast<EnchantmentTableEntity>(blockEntity);
    if (table.get() != NULL)
    {
        player->startBlockInteraction(*table);
    }
    return true;
}

void EnchantmentTableBlock::setPlacedBy(Level* level, const BlockPos& pos, const BlockState* state,
                                        boost::shared_ptr<LivingEntity> placer,
                                        not_null_ptr<ItemInstance> itemInstance)
{
    Block::setPlacedBy(level, pos, state, placer, itemInstance);
    if (itemInstance->hasCustomHoverName())
    {
        boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(pos);
        boost::shared_ptr<EnchantmentTableEntity> table =
            boost::dynamic_pointer_cast<EnchantmentTableEntity>(blockEntity);
        if (table.get() != NULL)
        {
            table->setCustomName(itemInstance->getHoverNameUnfiltered(), itemInstance->getOwnerUUID(),
                                 itemInstance->wasCreatedByRestrictedPlayer(), itemInstance->wasCreatedOnHost());
        }
    }
}

EBlockFaceShape EnchantmentTableBlock::getBlockFaceShape(LevelSource*, const BlockState*,
                                                         const BlockPos&, const Direction* direction)
{
    return direction == Direction::DOWN ? BLOCK_FACE_SOLID : BLOCK_FACE_UNDEFINED;
}

TextureAtlasSprite* EnchantmentTableBlock::getTexture(const Direction* direction, const BlockState*)
{
    if (direction == Direction::UP)
    {
        return mTopTexture;
    }
    if (direction == Direction::DOWN)
    {
        return mBottomTexture;
    }
    return mIcon;
}

void EnchantmentTableBlock::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(L"enchantment_side");
    mTopTexture = iconRegister->registerIcon(L"enchantment_top");
    mBottomTexture = iconRegister->registerIcon(L"enchantment_bottom");
}

unsigned int EnchantmentTableBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if ((tooltipData.m_bUseBlock || tooltipData.m_bUseItemOnBlock) && tooltipData.m_bCanPlace)
    {
        return IDS_TOOLTIPS_USE;
    }
    return Block::GetInteractTooltip(tooltipData);
}
