#include "game/world/generation/Deadmau5EarsLayer.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/GlStateManager.hpp"
#include "game/client/render/PlayerModel.hpp"
#include "game/client/render/PlayerRenderer.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/world/generation/Player.hpp"

Deadmau5EarsLayer::Deadmau5EarsLayer(PlayerRenderer* renderer)
    : mRenderer(renderer)
{
}

Deadmau5EarsLayer::~Deadmau5EarsLayer()
{
}

void Deadmau5EarsLayer::render(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                               float partialTicks, float ageInTicks, float netHeadYaw, float headPitch,
                               float scale, bool bUseDisplayList)
{
    boost::shared_ptr<Player> player = boost::dynamic_pointer_cast<Player>(entity);

    if (!g_pApp->isXuidDeadmau5(player->mXuid) || player->isInvisible())
        return;

    PlayerModel* model = mRenderer->getModel();
    float yRot = player->mYRotO + (player->mYRot - player->mYRotO) * partialTicks;
    float yBodyRot = player->mYBodyRotO + (player->mYBodyRot - player->mYBodyRotO) * partialTicks;
    float yaw = yRot - yBodyRot;
    float xRot = player->mXRotO + (player->mXRot - player->mXRotO) * partialTicks;

    for (int side = -1; side <= 1; side += 2)
    {
        GlStateManager::pushMatrix();
        GlStateManager::rotatef(yaw, 0.0f, 1.0f, 0.0f);
        GlStateManager::rotatef(xRot, 1.0f, 0.0f, 0.0f);
        GlStateManager::translatef(0.375f * (float)side, 0.0f, 0.0f);
        GlStateManager::translatef(0.0f, 0.5f, 0.0f);
        GlStateManager::rotatef(-xRot, 1.0f, 0.0f, 0.0f);
        GlStateManager::rotatef(-yaw, 0.0f, 1.0f, 0.0f);
        GlStateManager::scalef(0.0625f, 0.0625f, 0.0625f);
        model->renderEars(0.0625f);
        GlStateManager::popMatrix();
    }
}

bool Deadmau5EarsLayer::colorsOnDamage()
{
    return true;
}
