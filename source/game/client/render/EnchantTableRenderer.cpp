#include "game/client/render/EnchantTableRenderer.hpp"

#include "game/client/render/BookModel.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Mth.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/block/BlockEntity.hpp"

class EnchantmentTableEntity;

static ResourceLocation BOOK_TEXTURE((_TEXTURE_NAME)57);

EnchantTableRenderer::EnchantTableRenderer()
    : mBookModel(new BookModel(ModelDefinition()))
{
}

EnchantTableRenderer::~EnchantTableRenderer()
{
    delete mBookModel;
}

void EnchantTableRenderer::render(boost::shared_ptr<BlockEntity> blockEntity, double x, double y, double z, float a, int breakProgress, bool bTranslucent, float alpha, bool bUseDisplayList)
{
    boost::shared_ptr<EnchantmentTableEntity> enchantmentTable = boost::static_pointer_cast<EnchantmentTableEntity>(blockEntity);

    GlStateManager::pushMatrix();
    GlStateManager::translatef((float)x + 0.5F, (float)y + 0.75F, (float)z + 0.5F);

    float time = (float)enchantmentTable->mTickCount + a;
    GlStateManager::translatef(0.0F, Mth::sin(time * 0.1F) * 0.01F + 0.1F, 0.0F);

    float rotation = enchantmentTable->mBookRotation - enchantmentTable->mBookRotationPrev;
    while (rotation >= 3.1415927F)
    {
        rotation -= 6.2831855F;
    }
    while (rotation < -3.1415927F)
    {
        rotation += 6.2831855F;
    }

    GlStateManager::rotatef(-(enchantmentTable->mBookRotationPrev + rotation * a) * 57.295776F, 0.0F, 1.0F, 0.0F);
    GlStateManager::rotatef(80.0F, 0.0F, 0.0F, 1.0F);
    bindTexture(&BOOK_TEXTURE);

    float spread = enchantmentTable->mBookSpreadPrev + (enchantmentTable->mBookSpread - enchantmentTable->mBookSpreadPrev) * a + 0.25F;
    spread = (spread - (float)Mth::fastFloor((double)spread)) * 1.6F - 0.3F;
    if (spread < 0.0F)
    {
        spread = 0.0F;
    }
    if (spread > 1.0F)
    {
        spread = 1.0F;
    }

    float spread2 = enchantmentTable->mBookSpreadPrev + (enchantmentTable->mBookSpread - enchantmentTable->mBookSpreadPrev) * a + 0.75F;
    spread2 = (spread2 - (float)Mth::fastFloor((double)spread2)) * 1.6F - 0.3F;
    if (spread2 < 0.0F)
    {
        spread2 = 0.0F;
    }
    if (spread2 > 1.0F)
    {
        spread2 = 1.0F;
    }

    float rotation2 = enchantmentTable->mBookRotation2Prev + (enchantmentTable->mBookRotation2 - enchantmentTable->mBookRotation2Prev) * a;
    boost::shared_ptr<Entity> entity;
    GlStateManager::enableCull();
    mBookModel->render(entity, time, spread, spread2, rotation2, 0.0F, 0.0625F, true);

    GlStateManager::popMatrix();
}
