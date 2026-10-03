#include "game/world/generation/ShulkerHeadLayer.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/ModelPart.hpp"
#include "game/client/render/ShulkerModel.hpp"
#include "game/client/render/ShulkerRenderer.hpp"
#include "game/core/Direction.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Shulker.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation SHULKER_LOCATION(TN_MOB_SHULKER_ENDERGOLEM);

ShulkerHeadLayer::ShulkerHeadLayer(ShulkerRenderer* renderer)
    : mRenderer(renderer)
{
}

ShulkerHeadLayer::~ShulkerHeadLayer()
{
}

void ShulkerHeadLayer::render(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                              float partialTicks, float ageInTicks, float netHeadYaw, float headPitch,
                              float scale, bool bUseDisplayList)
{
    boost::shared_ptr<Shulker> shulker = boost::static_pointer_cast<Shulker>(entity);
    GlStateManager::pushMatrix();
    const Direction* direction = shulker->getAttachFace();

    switch (direction->get3DDataValue())
    {
    case 1:
        GlStateManager::rotatef(180.0f, 1.0f, 0.0f, 0.0f);
        GlStateManager::translatef(0.0f, -2.0f, 0.0f);
        break;
    case 2:
        GlStateManager::rotatef(90.0f, 1.0f, 0.0f, 0.0f);
        GlStateManager::translatef(0.0f, -1.0f, -1.0f);
        break;
    case 3:
        GlStateManager::rotatef(180.0f, 0.0f, 0.0f, 1.0f);
        GlStateManager::rotatef(90.0f, 1.0f, 0.0f, 0.0f);
        GlStateManager::translatef(0.0f, -1.0f, 1.0f);
        break;
    case 4:
        GlStateManager::rotatef(-90.0f, 0.0f, 0.0f, 1.0f);
        GlStateManager::rotatef(90.0f, 1.0f, 0.0f, 0.0f);
        GlStateManager::translatef(-1.0f, -1.0f, 0.0f);
        GlStateManager::rotatef(180.0f, 0.0f, 1.0f, 0.0f);
        break;
    case 5:
        GlStateManager::rotatef(90.0f, 0.0f, 0.0f, 1.0f);
        GlStateManager::rotatef(90.0f, 1.0f, 0.0f, 0.0f);
        GlStateManager::translatef(1.0f, -1.0f, 0.0f);
        GlStateManager::rotatef(180.0f, 0.0f, 1.0f, 0.0f);
        break;
    default:
        break;
    }

    ShulkerModel* model = static_cast<ShulkerModel*>(mRenderer->getModel());
    model->mHead->mYRot = netHeadYaw * 0.017453292f;
    model->mHead->mXRot = headPitch * 0.017453292f;
    mRenderer->bindTexture(&SHULKER_LOCATION);
    model->mHead->render(scale, bUseDisplayList, false);

    GlStateManager::popMatrix();
}

bool ShulkerHeadLayer::colorsOnDamage()
{
    return false;
}
