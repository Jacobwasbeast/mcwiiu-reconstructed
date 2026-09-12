#include "game/client/render/LivingEntityRenderer.hpp"

#include <cmath>

#include "game/client/render/Model.hpp"
#include "game/client/render/RenderLayer.hpp"
#include "game/client/render/TexturePack.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/client/render/EntityRenderDispatcher.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/core/Options.hpp"
#include "game/core/Team.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"
#include "vendor/_SkinAdjustments.hpp"

void PIXBeginNamedEvent(float colour, const char* name, ...);
void PIXEndNamedEvent();

double lerp(double from, double to, double amount)
{
    return (to - from) * amount + from;
}

LivingEntityRenderer::LivingEntityRenderer(EntityRenderDispatcher* dispatcher, Model* model, float shadowRadius)
    : EntityRenderer(dispatcher), mLayers(), mItemFrame(false)
{
    mModel = model;
    mShadowRadius = shadowRadius;
}

LivingEntityRenderer::~LivingEntityRenderer()
{
}

bool LivingEntityRenderer::addLayer(RenderLayer* layer)
{
    mLayers.push_back(layer);
    return true;
}

bool LivingEntityRenderer::removeLayer(RenderLayer* layer)
{
    std::vector<RenderLayer*>::iterator it = mLayers.begin();
    while (it != mLayers.end() && *it != layer)
        ++it;
    if (it == mLayers.end())
        return false;
    mLayers.erase(it);
    return true;
}

Model* LivingEntityRenderer::getModel()
{
    return mModel;
}

Model* LivingEntityRenderer::getModel(boost::shared_ptr<LivingEntity>&)
{
    return mModel;
}

float LivingEntityRenderer::rotlerp(float from, float to, float delta)
{
    float difference = to - from;
    while (difference < -180.0F)
        difference += 360.0F;
    while (difference >= 180.0F)
        difference -= 360.0F;
    return from + delta * difference;
}

void LivingEntityRenderer::translateWeaponItem()
{
}

float LivingEntityRenderer::setupScale(boost::shared_ptr<LivingEntity>& entity, float partialTicks)
{
    GlStateManager::enableRescaleNormal();
    GlStateManager::scalef(-1.0F, -1.0F, 1.0F);
    scale(entity, partialTicks);
    GlStateManager::translatef(0.0F, 3.0F, 0.0F);
    return 0.0625F;
}

bool LivingEntityRenderer::setupSolidState(boost::shared_ptr<LivingEntity>&)
{
    GlStateManager::disableLighting();
    GlStateManager::activeTexture(1);
    GlStateManager::disableTexture();
    GlStateManager::activeTexture(0);
    return true;
}

bool LivingEntityRenderer::setupOverlayColor(boost::shared_ptr<LivingEntity>&, float, bool overlay)
{
    if (!overlay)
        return false;
    GlStateManager::color4f(1.0F, 1.0F, 1.0F, 1.0F);
    return true;
}

void LivingEntityRenderer::setupOverlayColor(boost::shared_ptr<LivingEntity>& entity, float partialTicks)
{
    setupOverlayColor(entity, partialTicks, true);
}

void LivingEntityRenderer::teardownOverlayColor()
{
    GlStateManager::color4f(0.0F, 0.0F, 0.0F, 0.0F);
}

void LivingEntityRenderer::tearDownSolidState()
{
    GlStateManager::enableLighting();
    GlStateManager::activeTexture(1);
    GlStateManager::enableTexture();
    GlStateManager::activeTexture(0);
}

void LivingEntityRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw,
                                  float partialTick)
{
    if ((entity->GetType() & ENTITY_TYPE_PLAYER) == ENTITY_TYPE_PLAYER)
    {
        boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);
        if (player->isSpectator() && player->hasInvisiblePrivilege())
            return;
    }

    PIXBeginNamedEvent(16711935.0F, "LivingEntityRenderer %d", (int)entity.get());
    boost::shared_ptr<LivingEntity> living = boost::static_pointer_cast<LivingEntity>(entity);

    GlStateManager::pushMatrix();
    GlStateManager::disableCull();

    float attackAnim = getAttackAnim(living, partialTick);
    (void)attackAnim;
    (void)getBob(living, partialTick);

    bool ridingOnLivingVehicle = false;
    if (living->isPassenger())
    {
        boost::shared_ptr<Entity> vehicle = living->getVehicle();
        if (vehicle)
        {
            if ((vehicle->GetType() & ENTITY_TYPE_LIVING_ENTITY) == ENTITY_TYPE_LIVING_ENTITY)
                ridingOnLivingVehicle = true;
        }
    }

    float bodyRot = rotlerp(living->mYBodyRotO, living->mYBodyRot, partialTick);
    float headRot = rotlerp(living->mYHeadRotO, living->mYHeadRot, partialTick);
    float netHeadYaw = headRot - bodyRot;

    if (ridingOnLivingVehicle)
    {
        boost::shared_ptr<LivingEntity> vehicleLiving =
            boost::static_pointer_cast<LivingEntity>(living->getVehicle());
        float vehicleBodyRot = rotlerp(vehicleLiving->mYBodyRotO, vehicleLiving->mYBodyRot, partialTick);
        float wrapped = Mth::wrapDegrees(headRot - vehicleBodyRot);
        float clamped = wrapped;
        if (clamped < -85.0F)
            clamped = -85.0F;
        else if (clamped > 85.0F)
            clamped = 85.0F;
        float clampedSq = clamped * clamped;
        if (2500.0F < clampedSq)
            netHeadYaw = headRot - (clamped * 0.5128204822540283F + netHeadYaw);
        else
            netHeadYaw = headRot - clamped;
    }

    float headPitch = rotlerp(living->mXRotO, living->mXRot, partialTick);
    float ageInTicks = (float)living->mTickCount + partialTick;

    setupPosition(living, x, y, z);
    setupRotations(living, ageInTicks, bodyRot, partialTick);
    float scale = setupScale(living, partialTick);

    float limbSwingAmount = 0.0F;
    float limbSwing = 0.0F;
    if (!living->isPassenger())
    {
        limbSwingAmount = living->mAnimationSpeedOld +
                          (living->mAnimationSpeed - living->mAnimationSpeedOld) * partialTick;
        limbSwing = -(living->mAnimationSpeed * (1.0F - partialTick) - living->mAnimationPosition);
        if (living->isBaby())
            limbSwing *= 3.0F;
        if (limbSwing > 1.0F)
            limbSwing = 1.0F;
    }

    GlStateManager::enableAlphaTest();
    PIXBeginNamedEvent(0.0F, "prepareMobModel");
    mModel->prepareMobModel(living, limbSwing, limbSwingAmount, ageInTicks);
    PIXEndNamedEvent();

    PIXBeginNamedEvent(0.0F, "setupAnim");
    _SkinAdjustments skinAdjustments;
    if ((entity->GetType() & ENTITY_TYPE_PLAYER) == ENTITY_TYPE_PLAYER)
        skinAdjustments = boost::static_pointer_cast<Player>(entity)->getSkinAdjustments();
    boost::shared_ptr<Entity> renderEntity = boost::static_pointer_cast<Entity>(living);
    mModel->setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, renderEntity,
                      skinAdjustments);
    PIXEndNamedEvent();

    if (!mSolidRender)
    {
        bool overlay = setupOverlayColor(living, partialTick, true);
        renderModel(living, limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale);
        if (overlay)
            teardownOverlayColor();
        GlStateManager::depthMask(true);

        bool renderLayerPass = true;
        if ((entity->GetType() & ENTITY_TYPE_PLAYER) == ENTITY_TYPE_PLAYER)
        {
            boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);
            if (player->isSpectator() && player->SpectateAsFloatingHead())
                renderLayerPass = false;
        }

        if (renderLayerPass)
        {
            PIXBeginNamedEvent(0.0F, "renderLayers");
            renderLayers(living, limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, partialTick,
                         true);
            PIXEndNamedEvent();
        }

        GlStateManager::disableRescaleNormal();
        GlStateManager::activeTexture(1);
        GlStateManager::enableTexture();
        GlStateManager::activeTexture(0);
        GlStateManager::enableCull();
        GlStateManager::popMatrix();
        PIXBeginNamedEvent(0.0F, "EntityRender");
        EntityRenderer::render(entity, x, y, z, yaw, partialTick);
        PIXEndNamedEvent();
        postRender(entity, x, y, z, yaw, partialTick, true);
    }
    else
    {
        bool solid = setupSolidState(living);
        PIXBeginNamedEvent(0.0F, "solidRender");
        GlStateManager::enableColorMaterial();
        unsigned int teamColor = getTeamColor(entity);
        GlStateManager::setupSolidRenderingTextureCombine((int)teamColor);
        if (!mItemFrame)
        {
            bool overlay = setupOverlayColor(living, partialTick, true);
            renderModel(living, limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale);
            if (overlay)
                teardownOverlayColor();
        }

        bool renderLayerPass = true;
        if ((entity->GetType() & ENTITY_TYPE_PLAYER) == ENTITY_TYPE_PLAYER)
        {
            boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);
            if (player->isSpectator() && player->SpectateAsFloatingHead())
                renderLayerPass = false;
        }

        if (renderLayerPass)
            renderLayers(living, limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, partialTick,
                         true);

        GlStateManager::tearDownSolidRenderingTextureCombine();
        GlStateManager::disableColorMaterial();
        PIXEndNamedEvent();
        if (solid)
            tearDownSolidState();
        GlStateManager::disableRescaleNormal();
        GlStateManager::activeTexture(1);
        GlStateManager::enableTexture();
        GlStateManager::activeTexture(0);
        GlStateManager::enableCull();
        GlStateManager::popMatrix();
        PIXBeginNamedEvent(0.0F, "EntityRender");
        EntityRenderer::render(entity, x, y, z, yaw, partialTick);
        PIXEndNamedEvent();
        postRender(entity, x, y, z, yaw, partialTick, true);
    }

    PIXEndNamedEvent();
}

void LivingEntityRenderer::setupForTexturePack(TexturePack* pack)
{
    EntityRenderer::setupForTexturePack(pack);
    std::vector<RenderLayer*>::iterator it = mLayers.begin();
    while (it != mLayers.end())
    {
        (*it)->setupForTexturePack(pack);
        ++it;
    }
}

void LivingEntityRenderer::renderModel(boost::shared_ptr<LivingEntity>& entity, float limbSwing,
                                       float limbSwingAmount, float ageInTicks, float netHeadYaw,
                                       float headPitch, float scale)
{
    if (!entity || mModel == NULL)
        return;

    boost::shared_ptr<Entity> baseEntity = boost::static_pointer_cast<Entity>(entity);
    mModel->prepareMobModel(entity, limbSwing, limbSwingAmount, ageInTicks);
    mModel->render(baseEntity, limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch,
                   scale, true);
}

bool LivingEntityRenderer::isVisible(boost::shared_ptr<LivingEntity>& entity)
{
    if (entity->isInvisible() && !mItemFrame)
        return false;
    return true;
}

void LivingEntityRenderer::setupPosition(boost::shared_ptr<LivingEntity>&, double x, double y, double z)
{
    GlStateManager::translatef((float)x, (float)y, (float)z);
}

void LivingEntityRenderer::setupRotations(boost::shared_ptr<LivingEntity>&, float, float rotationYaw, float)
{
    GlStateManager::rotatef(180.0F - rotationYaw, 0.0F, 1.0F, 0.0F);
}

float LivingEntityRenderer::getAttackAnim(boost::shared_ptr<LivingEntity>&, float)
{
    return mModel == NULL ? 0.0F : mModel->mAttackTime;
}

double LivingEntityRenderer::getBob(boost::shared_ptr<LivingEntity>& entity, float partialTicks)
{
    return entity->mOBob + (entity->mBob - entity->mOBob) * partialTicks;
}

void LivingEntityRenderer::renderLayers(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                                        float ageInTicks, float netHeadYaw, float headPitch, float scale,
                                        float partialTick, bool overlay)
{
    std::vector<RenderLayer*>::iterator it = mLayers.begin();
    while (it != mLayers.end())
    {
        (*it)->render(entity, limbSwing, limbSwingAmount, partialTick, ageInTicks, netHeadYaw, headPitch, scale,
                      overlay);
        ++it;
    }
}

double LivingEntityRenderer::getFlipDegrees(boost::shared_ptr<LivingEntity>&)
{
    return 90.0F;
}

int LivingEntityRenderer::getOverlayColor(boost::shared_ptr<LivingEntity>&, float, float)
{
    return 0;
}

void LivingEntityRenderer::scale(boost::shared_ptr<LivingEntity>&, float)
{
}

void LivingEntityRenderer::renderName(boost::shared_ptr<Entity>& entity, double x, double y, double z)
{
    if (!shouldShowName(entity) && !Minecraft::renderDebug())
        return;

    boost::shared_ptr<Player> player = Minecraft::GetInstance()->mPlayer;
    boost::shared_ptr<Entity> camera = player->GetCamera();
    double distanceSqr = entity->distanceToSqr(camera);

    int maxDistance = entity->isSneaking() ? 32 : 64;
    double maxDistanceSqr = (double)(maxDistance * maxDistance);
    if (distanceSqr >= maxDistanceSqr)
        return;

    std::wstring name = entity->getDisplayName();
    if (name.empty())
        return;

    if (GameSettings::GetGameSettings(static_cast<eGameSetting>(0x17)) == 0 ||
        Minecraft::GetInstance()->mOptions->mHideGui)
        return;

    GlStateManager::alphaFunc(4, 0.009999999776482582f);
    renderNameTags(entity, x, y, z, name, distanceSqr, 0xFF000000);
}

bool LivingEntityRenderer::shouldShowName(boost::shared_ptr<Entity>& entity)
{
    boost::shared_ptr<LivingEntity> living = boost::static_pointer_cast<LivingEntity>(entity);
    boost::shared_ptr<Player> localPlayer = Minecraft::GetInstance()->mPlayer;
    bool visibleToPlayer = !living->isInvisibleTo(localPlayer);

    if (visibleToPlayer && entity.get() != mDispatcher->mPlayer.get())
    {
        if (!living->shouldShowName())
            return false;
    }
    else if (visibleToPlayer)
    {
        return false;
    }

    if ((entity->GetType() & ENTITY_TYPE_PLAYER) != ENTITY_TYPE_PLAYER)
    {
        return Minecraft::renderNames() && entity->hasCustomName();
    }

    if (entity.get() == localPlayer.get())
        return Minecraft::renderNames() && entity->hasCustomName();

    bool showName = false;
    if (!Minecraft::InMiniGame(eMiniGameId_Tumble, true))
    {
        showName = entity.get() == mDispatcher->mPlayer.get();
    }
    else
    {
        if (living->distanceTo(boost::static_pointer_cast<Entity>(localPlayer)) < 3.0)
        {
            if (!localPlayer->isSpectator())
                showName = true;
        }
    }

    boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(living);
    if (player->isSpectator())
    {
        if (player->SpectateAsInvisible())
            return showName;
        if (player->SpectateAsFloatingHead())
            return showName;
    }

    if (!Minecraft::InMiniGame(eMiniGameId_Lobby, true))
    {
        Team* entityTeam = living->getTeam();
        Team* playerTeam = localPlayer->getTeam();
        if (entityTeam != NULL)
        {
            Team::Visibility visibility = entityTeam->getNameTagVisibility();
            if (visibility == Team::ALWAYS)
                return visibleToPlayer;
            if (visibility == Team::NEVER)
                return showName;
            if (visibility == Team::HIDE_FOR_OTHER_TEAMS)
            {
                if (!showName && !localPlayer->isSpectator())
                {
                    bool result = visibleToPlayer;
                    if (playerTeam != NULL)
                    {
                        result = false;
                        if (entityTeam->isAlliedTo(playerTeam) &&
                            (entityTeam->isAllowFriendlyFire() || visibleToPlayer))
                            result = true;
                    }
                    return result;
                }
                return true;
            }
            if (visibility == Team::HIDE_FOR_OWN_TEAM)
            {
                if (!showName && !localPlayer->isSpectator())
                {
                    bool result = visibleToPlayer;
                    if (playerTeam != NULL)
                    {
                        result = false;
                        if (!entityTeam->isAlliedTo(playerTeam) && visibleToPlayer)
                            result = true;
                    }
                    return result;
                }
                return true;
            }
        }
        return true;
    }

    if (Minecraft::InMiniGame(eMiniGameId_Any, true))
    {
        const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
        if (miniGame != NULL && miniGame->GetShowNametags())
            return true;
        return showName;
    }

    if (!Minecraft::renderNames())
        return false;
    if (g_pApp->GetGameHostOption(static_cast<eGameHostOption>(4)) == 0)
        return false;
    return true;
}
