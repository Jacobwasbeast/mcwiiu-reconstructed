#include "game/ui/UIControl_MinecraftPlayer.hpp"

#include <cmath>

#include <boost/shared_ptr.hpp>

#include "game/client/render/EntityRenderDispatcher.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Lighting.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Options.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/ui/ScreenSizeCalculator.hpp"
#include "game/ui/UIScene_AbstractContainerMenu.hpp"
#include "game/ui/fuiRect.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"

UIControl_MinecraftPlayer::UIControl_MinecraftPlayer()
    : mScreenWidth(0.0f), mScreenHeight(0.0f), mScaledWidth(0.0f), mScaledHeight(0.0f)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    m_controlType = UIControl::eUIControlType_MinecraftPlayer;
    mScreenWidth = (float)minecraft->mWidth;
    mScreenHeight = (float)minecraft->mHeight;
    ScreenSizeCalculator calculator(minecraft->mOptions, minecraft->mWidth, minecraft->mHeight, -1);
    mScaledWidth = (float)calculator.mWidthScale;
    mScaledHeight = (float)calculator.mHeightScale;
}

UIControl_MinecraftPlayer::~UIControl_MinecraftPlayer()
{
}

void UIControl_MinecraftPlayer::render(fuiRect* rect)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    GlStateManager::enableRescaleNormal();
    GlStateManager::enableColorMaterial();
    GlStateManager::pushMatrix();
    float width = rect->xMax - rect->xMin;
    float height = rect->yMax - rect->yMin;
    GlStateManager::translatef(width * 0.5f, height - height / 9.0f, 50.0f);
    float scale = width / (mScreenWidth / mScreenHeight);
    GlStateManager::scalef(-scale, scale, scale);
    GlStateManager::rotatef(180.0f, 0.0f, 0.0f, 1.0f);

    UIScene_AbstractContainerMenu* scene = static_cast<UIScene_AbstractContainerMenu*>(m_scene);
    boost::shared_ptr<MultiplayerLocalPlayer> player = minecraft->mLocalPlayers[scene->getPad()];
    float oldYRot = player->mYRot;
    float oldYRotO = player->mYRotO;
    float oldXRot = player->mXRot;
    float oldYHeadRot = player->mYHeadRot;
    bool oldOnFire = player->getSharedFlag(Entity::FLAG_ONFIRE);

    float lookX = (m_x + m_width * 0.5f - scene->m_pointerPos.x) / 40.0f;
    float lookY = (m_y + m_height * 0.5f - 40.0f - scene->m_pointerPos.y) / 40.0f;

    GlStateManager::rotatef(135.0f, 0.0f, 1.0f, 0.0f);
    Lighting::turnOn();
    GlStateManager::rotatef(-135.0f, 0.0f, 1.0f, 0.0f);
    GlStateManager::rotatef(-std::atan(lookY) * 20.0f, 1.0f, 0.0f, 0.0f);
    float bodyRotation = player->mYBodyRot;
    GlStateManager::rotatef(-(std::atan(lookX) * 20.0f - bodyRotation), 0.0f, 1.0f, 0.0f);
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);

    player->mYRot = bodyRotation;
    player->mYRotO = player->mYRot;
    player->mYHeadRot = player->mYRot;
    player->mXRot = -std::atan(lookY) * 20.0f;
    player->disableFallFlyingState(true);

    Minecraft::GetInstance()->getEntityRenderDispatcher()->mCameraYRot = 180.0f;
    bool oldHideGui = minecraft->mOptions->mHideGui;
    minecraft->mOptions->mHideGui = true;
    double x = player->isSleeping() ? player->mBedOffsetX : 0.0;
    double z = player->isSleeping() ? player->mBedOffsetZ : 0.0;
    boost::shared_ptr<Entity> entity = player;
    Minecraft::GetInstance()->getEntityRenderDispatcher()->render(entity, -x, 0.0, -z, 0.0f, 1.0f, false, false, false, true);
    minecraft->mOptions->mHideGui = oldHideGui;

    player->mYRot = oldYRot;
    player->mYRotO = oldYRotO;
    player->mXRot = oldXRot;
    player->mYHeadRot = oldYHeadRot;
    player->setSharedFlag(Entity::FLAG_ONFIRE, oldOnFire);
    player->disableFallFlyingState(false);

    GlStateManager::popMatrix();
    Lighting::turnOff();
    GlStateManager::disableRescaleNormal();
}
