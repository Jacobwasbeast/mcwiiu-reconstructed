#include "game/ui/UIControl_MinecraftPlayer.hpp"

#include <cmath>

#include <boost/shared_ptr.hpp>

#include "game/client/render/EntityRenderDispatcher.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Lighting.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/ui/ScreenSizeCalculator.hpp"
#include "game/ui/UIScene_AbstractContainerMenu.hpp"
#include "game/ui/fuiRect.hpp"
#include "game/world/generation/Player.hpp"

UIControl_MinecraftPlayer::UIControl_MinecraftPlayer()
    : mScreenWidth(0.0f), mScreenHeight(0.0f), mScaledWidth(0.0f), mScaledHeight(0.0f)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    m_controlType = 10;
    mScreenWidth = (float)minecraft->mWidth;
    mScreenHeight = (float)minecraft->mHeight;
    ScreenSizeCalculator calculator(minecraft->mOptions, minecraft->mWidth, minecraft->mHeight, -1);
    mScaledWidth = (float)calculator.getWidth();
    mScaledHeight = (float)calculator.getHeight();
}

UIControl_MinecraftPlayer::~UIControl_MinecraftPlayer()
{
}

void UIControl_MinecraftPlayer::render(fuiRect* rect)
{
    float width = rect->xMax - rect->xMin;
    float height = rect->yMax - rect->yMin;
    float scale = width / (mScreenWidth / mScreenHeight);

    GlStateManager::enableRescaleNormal();
    GlStateManager::enableColorMaterial();
    GlStateManager::pushMatrix();
    GlStateManager::translatef(width * 0.5f, height - height / 9.0f, 50.0f);
    GlStateManager::scalef(-scale, scale, scale);
    GlStateManager::rotatef(180.0f, 0.0f, 0.0f, 1.0f);

    Minecraft* minecraft = Minecraft::GetInstance();
    UIScene_AbstractContainerMenu* scene = static_cast<UIScene_AbstractContainerMenu*>(m_scene);
    boost::shared_ptr<Player> player = minecraft->GetPlayerByPlayerIndex(scene->getPad());
    if (player)
    {
        float oldYRot = player->mYRot;
        float oldYRotO = player->mYRotO;
        float oldXRot = player->mXRot;
        float oldYBodyRot = player->mYBodyRot;
        float oldYHeadRot = player->mYHeadRot;
        bool oldSharedFlag = player->getSharedFlag(0);
        bool oldFallFlyingStateDisabled = player->mFallFlyingStateDisabled;

        float bodyRotation = player->mYBodyRot;
        float xRotation = (m_y + m_height * 0.5f - 40.0f) / 40.0f;
        float yRotation = (m_x + m_width * 0.5f - 40.0f) / 40.0f;

        GlStateManager::rotatef(135.0f, 0.0f, 1.0f, 0.0f);
        Lighting::turnOn();
        GlStateManager::rotatef(-135.0f, 1.0f, 0.0f, 0.0f);
        GlStateManager::rotatef(-std::atan(xRotation) * 20.0f, 1.0f, 0.0f, 0.0f);
        GlStateManager::rotatef(-(std::atan(yRotation) * 20.0f - bodyRotation), 0.0f, 1.0f, 0.0f);
        GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);

        player->mYBodyRot = bodyRotation;
        player->mYHeadRot = bodyRotation;
        player->mYRot = bodyRotation;
        player->mYRotO = bodyRotation;
        player->mXRot = -std::atan(xRotation) * 20.0f;
        player->disableFallFlyingState(true);

        EntityRenderDispatcher* dispatcher = minecraft->getEntityRenderDispatcher();
        boost::shared_ptr<Entity> entity = player;
        dispatcher->render(entity, 0.0, 0.0, 0.0, 0.0f, 1.0f, false, false, false, true);

        player->mYRot = oldYRot;
        player->mYRotO = oldYRotO;
        player->mXRot = oldXRot;
        player->mYBodyRot = oldYBodyRot;
        player->mYHeadRot = oldYHeadRot;
        player->setSharedFlag(0, oldSharedFlag);
        player->disableFallFlyingState(!oldFallFlyingStateDisabled);
    }

    GlStateManager::popMatrix();
    Lighting::turnOff();
    GlStateManager::disableRescaleNormal();
}
