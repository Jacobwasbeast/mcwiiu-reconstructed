#include "game/world/block/EnchantmentTableBlock.hpp"

#include "game/client/render/ParticleType.hpp"
#include "game/core/IconRegister.hpp"
#include "game/entity/mob/EnchantmentTableEntity.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
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

EnchantmentTableBlock::EnchantmentTableBlock()
    : BaseEntityBlock(Material::STONE, MaterialColor::STONE), EntityBlock()
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
    return NULL;
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
            if (random->nextInt(16) == 0)
            {
                BlockPos bookshelfPos = pos.offset(x, 0, z);
                if (level->getBlockState(bookshelfPos)->getBlock() == Blocks::bookshelf)
                {
                    BlockPos particlePos = bookshelfPos.offset(x / 2, 0, z / 2);
                    if (level->isEmptyBlock(particlePos))
                    {
                        double particleX = (double)pos.x + 0.5 + (double)x + (double)random->nextFloat();
                        double particleY = (double)pos.y + 2.0;
                        double particleZ = (double)pos.z + 0.5 + (double)z + (double)random->nextFloat();
                        level->addParticle(PARTICLE_ENCHANTMENT_TABLE, particleX, particleY, particleZ,
                                           (double)x + (double)random->nextFloat() - 0.5,
                                           -0.5 - (double)random->nextFloat(),
                                           (double)z + (double)random->nextFloat() - 0.5,
                                           arrayWithLength<int>());
                    }
                }

                bookshelfPos = pos.offset(x, 1, z);
                if (level->getBlockState(bookshelfPos)->getBlock() == Blocks::bookshelf)
                {
                    BlockPos particlePos = bookshelfPos.offset(x / 2, 0, z / 2);
                    if (level->isEmptyBlock(particlePos))
                    {
                        double particleX = (double)pos.x + 0.5 + (double)x + (double)random->nextFloat();
                        double particleY = (double)pos.y + 2.0;
                        double particleZ = (double)pos.z + 0.5 + (double)z + (double)random->nextFloat();
                        level->addParticle(PARTICLE_ENCHANTMENT_TABLE, particleX, particleY, particleZ,
                                           (double)x + (double)random->nextFloat() - 0.5,
                                           -(double)random->nextFloat(),
                                           (double)z + (double)random->nextFloat() - 0.5,
                                           arrayWithLength<int>());
                    }
                }
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
    return BLOCK_RENDER_ENTITY;
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
        player->startBlockInteraction(table.get());
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
    mIcon = iconRegister->registerIcon(L"enchanting_table_side");
    mBottomTexture = iconRegister->registerIcon(L"enchanting_table_bottom");
    mTopTexture = iconRegister->registerIcon(L"enchanting_table_top");
}

unsigned int EnchantmentTableBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if ((tooltipData.m_bUseItem || tooltipData.m_bCanPlace) && tooltipData.m_bCanDestroy)
    {
        return 0xea4e897e;
    }
    return Block::GetInteractTooltip(tooltipData);
}
