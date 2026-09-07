#include "game/client/render/EntityRenderDispatcher.hpp"

#include "game/client/render/ItemRenderer.hpp"
#include "game/client/render/PlayerRenderer.hpp"
#include "game/client/render/TexturePack.hpp"
#include "game/client/render/Textures.hpp"
#include "game/core/Culler.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/world/generation/Player.hpp"
#include "game/core/Minecraft.hpp"

EntityRenderDispatcher::EntityRenderDispatcher(Textures* textures, ItemRenderer*)
    : boost::unordered_map<unsigned long long, EntityRenderer*>(), mPlayerRenderer(NULL), mFont(NULL),
      mX(0.0), mY(0.0), mZ(0.0), mTextures(textures), mLevel(NULL), mCamera(), mPlayer(),
      mOptions(NULL), mCameraYRot(0.0F), mCameraXRot(0.0F), mPlayerRotY(0.0F), mRenderName(false),
      mRenderLeash(false), mRenderShadow(true), mRenderHitBoxes(false), mSolidRendering(false)
{
    GlStateManager::enableLighting();
    mPlayerRenderer = new PlayerRenderer(this, false, false);
    (*this)[0x0C800000ULL] = mPlayerRenderer;
    mPlayerRenderers[EPLAYER_MODEL_DEFAULT] = mPlayerRenderer;
    mPlayerRenderers[EPLAYER_MODEL_SLIM] = new PlayerRenderer(this, false, true);
    mPlayerRenderers[EPLAYER_MODEL_SLIM_64] = new PlayerRenderer(this, true, true);
    GlStateManager::disableLighting();
}

double EntityRenderDispatcher::distanceToSqr(double x, double y, double z)
{
    x -= mX;
    y -= mY;
    z -= mZ;
    return x * x + y * y + z * z;
}

unsigned char EntityRenderDispatcher::shouldRenderShadow()
{
    return mRenderShadow;
}

Font* EntityRenderDispatcher::getFont()
{
    return mFont;
}

void EntityRenderDispatcher::setPosition(double x, double y, double z)
{
    mX = x;
    mY = y;
    mZ = z;
}

EntityRenderer* EntityRenderDispatcher::getRenderer(unsigned long long type)
{
    if ((type & 0x0C800000ULL) == 0x0C800000ULL)
        return mPlayerRenderer;
    iterator it = find(type);
    if (it == end())
        return NULL;
    return it->second;
}

PlayerRenderer* EntityRenderDispatcher::getPlayerRenderer(EPlayerModelType modelType)
{
    boost::unordered_map<EPlayerModelType, PlayerRenderer*>::iterator it = mPlayerRenderers.find(modelType);
    if (it != mPlayerRenderers.end())
        return it->second;
    return mPlayerRenderer;
}

EntityRenderer* EntityRenderDispatcher::getRenderer(boost::shared_ptr<Entity> entity)
{
    if (!entity)
        return NULL;
    unsigned long long type = entity->GetType();
    if ((type & 0x0C800000ULL) != 0x0C800000ULL)
        return getRenderer(type);
    boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);
    if (player->SpectateAsOtherEntity())
        return getRenderer(player->GetSpectateEntity());
    return getPlayerRenderer(static_cast<EPlayerModelType>(player->getModelName()));
}

void EntityRenderDispatcher::prepare(Level* level, Font* font, boost::shared_ptr<Entity> camera,
                                     boost::shared_ptr<Entity> player, Options* options, float partialTicks)
{
    mLevel = level;
    mFont = font;
    mCamera = camera;
    mPlayer = player;
    mOptions = options;
    if (!camera)
        return;
    mCameraYRot = (camera->mYRot - camera->mYRotO) * partialTicks + camera->mYRotO;
    mCameraXRot = (camera->mXRot - camera->mXRotO) * partialTicks + camera->mXRotO;
    mX = (camera->mX - camera->mXOld) * partialTicks + camera->mXOld;
    mY = (camera->mY - camera->mYOld) * partialTicks + camera->mYOld;
    mZ = (camera->mZ - camera->mZOld) * partialTicks + camera->mZOld;
}

bool EntityRenderDispatcher::hasSecondPass(boost::shared_ptr<Entity>& entity)
{
    EntityRenderer* renderer = getRenderer(entity);
    return renderer != NULL && renderer->hasSecondPass();
}

bool EntityRenderDispatcher::shouldRender(boost::shared_ptr<Entity>& entity, Culler* culler,
                                          double x, double y, double z)
{
    EntityRenderer* renderer = getRenderer(entity);
    return renderer != NULL && renderer->shouldRender(entity, culler, x, y, z);
}

bool EntityRenderDispatcher::shouldRender(boost::shared_ptr<Entity>& entity, Culler* culler, double distance)
{
    EntityRenderer* renderer = getRenderer(entity);
    return renderer != NULL && renderer->shouldRender(entity, culler, distance);
}

void EntityRenderDispatcher::renderHitbox(boost::shared_ptr<Entity>& entity, double x, double y, double z,
                                          float, float, int color)
{
    if (!entity)
        return;
    GlStateManager::disableTexture();
    GlStateManager::disableLighting();
    GlStateManager::color4f((float)((color >> 16) & 255) / 255.0F,
                            (float)((color >> 8) & 255) / 255.0F,
                            (float)(color & 255) / 255.0F, 1.0F);
    EntityRenderer* renderer = getRenderer(entity);
    if (renderer != NULL)
        renderer->render(entity->mBoundingBox, x, y, z);
    GlStateManager::enableLighting();
    GlStateManager::enableTexture();
}

void EntityRenderDispatcher::render(boost::shared_ptr<Entity>& entity, double x, double y, double z,
                                    float yaw, float partialTicks, bool renderShadow, bool renderName,
                                    bool renderFire, bool renderLeash)
{
    EntityRenderer* renderer = getRenderer(entity);
    if (renderer == NULL || mTextures == NULL)
        return;
    renderer->setSolidRender(mSolidRendering);
    mRenderName = renderName;
    mRenderLeash = renderLeash;
    renderer->render(entity, x, y, z, yaw, partialTicks);
    renderer->postRender(entity, x, y, z, yaw, partialTicks, renderShadow);
}

void EntityRenderDispatcher::render(boost::shared_ptr<Entity>& entity, float partialTicks, bool renderShadow)
{
    if (!entity)
        return;
    double x = entity->mXOld + (entity->mX - entity->mXOld) * partialTicks - mX;
    double y = entity->mYOld + (entity->mY - entity->mYOld) * partialTicks - mY;
    double z = entity->mZOld + (entity->mZ - entity->mZOld) * partialTicks - mZ;
    render(entity, x, y, z, entity->mYRot, partialTicks, renderShadow, true, true, true);
}

void EntityRenderDispatcher::renderSecondPass(boost::shared_ptr<Entity>& entity, float partialTicks)
{
    EntityRenderer* renderer = getRenderer(entity);
    if (renderer != NULL)
        renderer->renderSecondPass(entity, mX, mY, mZ, entity->mYRot, partialTicks);
}

void EntityRenderDispatcher::setupForTexturePack(TexturePack* texturePack)
{
    for (iterator it = begin(); it != end(); ++it)
        if (it->second != NULL)
            it->second->setupForTexturePack(texturePack);
}
