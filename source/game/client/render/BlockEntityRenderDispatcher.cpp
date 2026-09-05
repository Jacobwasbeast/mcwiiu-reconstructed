#include "game/client/render/BlockEntityRenderDispatcher.hpp"

#include <boost/unordered_map.hpp>

#include "game/client/render/BannerRenderer.hpp"
#include "game/client/render/ChestRenderer.hpp"
#include "game/client/render/ConduitRenderer.hpp"
#include "game/client/render/EnchantTableRenderer.hpp"
#include "game/client/render/SkullBlockRenderer.hpp"
#include "game/client/render/TheEndPortalRenderer.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/HitResult.hpp"
#include "game/core/Lighting.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/world/block/BlockEntity.hpp"

static BlockEntityRenderDispatcher* gBlockEntityRenderDispatcher = NULL;

BlockEntityRenderDispatcher::BlockEntityRenderDispatcher()
    : boost::unordered_map<unsigned long long, BlockEntityRenderer*>(),
      mFont(NULL), mTextures(NULL), mLevel(NULL), mCamera(), mCameraYRot(90.0f),
      mCameraXRot(90.0f), mCameraHitResult(NULL), mX(1.0), mY(1.0), mZ(1.0)
{
    GlStateManager::enableLighting();

    (*this)[0x01E0020100000000ULL] = new ChestRenderer();
    (*this)[0x0120020600000000ULL] = new EnchantTableRenderer();
    (*this)[0x0130020000000000ULL] = new TheEndPortalRenderer();
    (*this)[0x0130020100000000ULL] = new TheEndPortalRenderer();
    (*this)[0x0120020700000000ULL] = new SkullBlockRenderer();
    (*this)[0x0100020C00000000ULL] = new BannerRenderer();
    (*this)[0x0120021200000000ULL] = new ConduitRenderer();

    GlStateManager::disableLighting();
    for (iterator it = begin(); it != end(); ++it)
    {
        if (it->second != NULL)
        {
            it->second->init(this);
        }
    }
}

Font* BlockEntityRenderDispatcher::getFont()
{
    return mFont;
}

void BlockEntityRenderDispatcher::staticCtor()
{
    gBlockEntityRenderDispatcher = new BlockEntityRenderDispatcher();
}

BlockEntityRenderer* BlockEntityRenderDispatcher::getRenderer(unsigned long long type)
{
    iterator it = find(type);
    if (it == end())
    {
        return NULL;
    }
    return it->second;
}

BlockEntityRenderer* BlockEntityRenderDispatcher::getRenderer(boost::shared_ptr<BlockEntity> blockEntity)
{
    if (!blockEntity)
    {
        return NULL;
    }
    return getRenderer(blockEntity->GetType());
}

bool BlockEntityRenderDispatcher::hasRenderer(boost::shared_ptr<BlockEntity> blockEntity)
{
    return getRenderer(blockEntity) != NULL;
}

void BlockEntityRenderDispatcher::setLevel(Level* level)
{
    mLevel = level;
    if (level == NULL)
    {
        mCamera.reset();
    }
    for (iterator it = begin(); it != end(); ++it)
    {
        if (it->second != NULL)
        {
            it->second->onNewLevel(level);
        }
    }
}

void BlockEntityRenderDispatcher::prepare(Level* level, Textures* textures, Font* font,
                                          boost::shared_ptr<Entity> camera,
                                          const HitResult* cameraHitResult,
                                          float partialTicks)
{
    if (mLevel != level)
    {
        setLevel(level);
    }
    mTextures = textures;
    mFont = font;
    mCamera = camera;
    mCameraHitResult = cameraHitResult;
    mCameraYRot = (camera->mYRot - camera->mYRotO) * partialTicks + camera->mYRotO;
    mCameraXRot = (camera->mXRot - camera->mXRotO) * partialTicks + camera->mXRotO;
    mX = (camera->mX - camera->mXOld) * partialTicks + camera->mXOld;
    mY = (camera->mY - camera->mYOld) * partialTicks + camera->mYOld;
    mZ = (camera->mZ - camera->mZOld) * partialTicks + camera->mZOld;
}

void BlockEntityRenderDispatcher::render(boost::shared_ptr<BlockEntity> blockEntity, double x,
                                         double y, double z, float a, int breakProgress,
                                         bool bTranslucent, float alpha, bool bUseDisplayList)
{
    BlockEntityRenderer* renderer = getRenderer(blockEntity);
    if (renderer != NULL)
    {
        renderer->render(blockEntity, x, y, z, a, breakProgress, bTranslucent, alpha,
                         bUseDisplayList);
    }
}

void BlockEntityRenderDispatcher::render(boost::shared_ptr<BlockEntity> blockEntity,
                                         float partialTicks, int breakProgress,
                                         bool bTranslucent)
{
    BlockEntityRenderer* renderer = getRenderer(blockEntity);
    if (renderer == NULL)
    {
        return;
    }
    if (blockEntity->distanceToSqr(mX, mY, mZ) < blockEntity->getViewDistance()
        || renderer->shouldRenderOffScreen(blockEntity))
    {
        Lighting::turnOn();
        BlockPos position = blockEntity->getBlockPos();
        render(blockEntity, (double)position.x - mX, (double)position.y - mY,
               (double)position.z - mZ, partialTicks, breakProgress, bTranslucent, 1.0f, true);
    }
}

void BlockEntityRenderDispatcher::render(boost::shared_ptr<BlockEntity> blockEntity,
                                         float partialTicks)
{
    render(blockEntity, partialTicks, -1, true);
}

void BlockEntityRenderDispatcher::render(boost::shared_ptr<BlockEntity> blockEntity, double x,
                                         double y, double z, float a, bool bTranslucent,
                                         float alpha, bool bUseDisplayList)
{
    render(blockEntity, x, y, z, a, -1, bTranslucent, alpha, bUseDisplayList);
}

void BlockEntityRenderDispatcher::setupForTexturePack(TexturePack* texturePack)
{
    for (iterator it = begin(); it != end(); ++it)
    {
        if (it->second != NULL)
        {
            it->second->setupForTexturePack(texturePack);
        }
    }
}
