#include "game/client/render/EntityBlockRenderer.hpp"
#include "game/core/EntityType.hpp"

#include "game/client/render/BlockEntityRenderDispatcher.hpp"
#include "game/client/render/SkullBlockRenderer.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/item/Item.hpp"
#include "game/item/Items.hpp"
#include "game/world/block/BannerBlockEntity.hpp"
#include "game/world/block/BedBlockEntity.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/ChestBlockEntity.hpp"
#include "game/world/block/ConduitBlockEntity.hpp"
#include "game/world/block/EnderChestBlockEntity.hpp"
#include "game/world/block/ShulkerBoxBlockEntity.hpp"
#include "game/world/block/SkullBlockEntity.hpp"

EntityBlockRenderer* EntityBlockRenderer::instance = NULL;

EntityBlockRenderer::EntityBlockRenderer()
{
    mChestBlockEntity.reset(new ChestBlockEntity(false));
    mTrappedChestBlockEntity.reset(new ChestBlockEntity(ChestBlock::TYPE_SINGLE, false));
    mEnderChestBlockEntity.reset(new EnderChestBlockEntity());
    mBannerBlockEntity.reset(new BannerBlockEntity());
    mSkullBlockEntity.reset(new SkullBlockEntity());
    mShulkerBoxBlockEntity.reset(new ShulkerBoxBlockEntity());
    mBedBlockEntity.reset(new BedBlockEntity());
    mConduitBlockEntity.reset(new ConduitBlockEntity());
}

void EntityBlockRenderer::renderByItem(not_null_ptr<ItemInstance> item, float partialTicks,
                                       bool bTranslucent, bool bUseDisplayList)
{
    Item* itemType = item->getItem();

    if (itemType == Items::banner)
    {
        mBannerBlockEntity->setItemValues(item, false);
        BlockEntityRenderDispatcher::instance->render(mBannerBlockEntity, 0.0, 0.0, 0.0,
                                             partialTicks, true, 1.0F, true);
    }
    else if (itemType == Items::bed)
    {
        mBedBlockEntity->setItemValues(item);
        BlockEntityRenderDispatcher::instance->render(mBedBlockEntity, 0.0, 0.0, 0.0,
                                             partialTicks, true, 1.0F, true);
    }
    else if (itemType == Items::skull)
    {
        if (BlockEntityRenderDispatcher::instance != NULL &&
            BlockEntityRenderDispatcher::instance->getRenderer(BlockEntityType::SKULL) != NULL)
        {
            GlStateManager::pushMatrix();
            GlStateManager::disableCull();
            static_cast<SkullBlockRenderer*>(BlockEntityRenderDispatcher::instance->getRenderer(BlockEntityType::SKULL))->renderSkull(0.0F, 0.0F, 0.0F, NULL, 180.0F,
                                              0.0F, item->getAuxValue(), -1, 0.5F,
                                              bUseDisplayList);
            GlStateManager::enableCull();
            GlStateManager::popMatrix();
        }
    }
    else if (itemType == Item::byBlock(Blocks::ender_chest))
    {
        BlockEntityRenderDispatcher::instance->render(mEnderChestBlockEntity, 0.0, 0.0, 0.0,
                                             partialTicks, bTranslucent, 1.0F,
                                             bUseDisplayList);
    }
    else if (itemType == Item::byBlock(Blocks::trapped_chest))
    {
        BlockEntityRenderDispatcher::instance->render(mTrappedChestBlockEntity, 0.0, 0.0, 0.0,
                                             partialTicks, bTranslucent, 1.0F,
                                             bUseDisplayList);
    }
    else if (itemType == Item::byBlock(Blocks::conduit))
    {
        BlockEntityRenderDispatcher::instance->render(mConduitBlockEntity, 0.0, 0.0, 0.0,
                                             partialTicks, bTranslucent, 1.0F,
                                             bUseDisplayList);
    }
    else
    {
        Block* blockItem = Block::byItem(itemType);
        const DyeColor* color = NULL;

        if (blockItem == Blocks::white_shulker_box)
            color = DyeColor::WHITE;
        else if (blockItem == Blocks::orange_shulker_box)
            color = DyeColor::ORANGE;
        else if (blockItem == Blocks::magenta_shulker_box)
            color = DyeColor::MAGENTA;
        else if (blockItem == Blocks::light_blue_shulker_box)
            color = DyeColor::LIGHT_BLUE;
        else if (blockItem == Blocks::yellow_shulker_box)
            color = DyeColor::YELLOW;
        else if (blockItem == Blocks::lime_shulker_box)
            color = DyeColor::LIME;
        else if (blockItem == Blocks::pink_shulker_box)
            color = DyeColor::PINK;
        else if (blockItem == Blocks::gray_shulker_box)
            color = DyeColor::GRAY;
        else if (blockItem == Blocks::silver_shulker_box)
            color = DyeColor::SILVER;
        else if (blockItem == Blocks::cyan_shulker_box)
            color = DyeColor::CYAN;
        else if (blockItem == Blocks::purple_shulker_box)
            color = DyeColor::PURPLE;
        else if (blockItem == Blocks::blue_shulker_box)
            color = DyeColor::BLUE;
        else if (blockItem == Blocks::brown_shulker_box)
            color = DyeColor::BROWN;
        else if (blockItem == Blocks::green_shulker_box)
            color = DyeColor::GREEN;
        else if (blockItem == Blocks::red_shulker_box)
            color = DyeColor::RED;
        else if (blockItem == Blocks::black_shulker_box)
            color = DyeColor::BLACK;

        if (color == NULL)
        {
            BlockEntityRenderDispatcher::instance->render(mChestBlockEntity, 0.0, 0.0, 0.0,
                                                 partialTicks, bTranslucent, 1.0F,
                                                 bUseDisplayList);
        }
        else
        {
            ShulkerBoxBlockEntity shulkerBoxEntity(color);
            *mShulkerBoxBlockEntity = shulkerBoxEntity;
            BlockEntityRenderDispatcher::instance->render(mShulkerBoxBlockEntity, 0.0, 0.0, 0.0,
                                                 partialTicks, bTranslucent, 1.0F,
                                                 bUseDisplayList);
        }
    }
}
