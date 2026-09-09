#include "game/client/render/BlockEntityRenderDispatcher.hpp"

#include <boost/unordered_map.hpp>

#include "game/client/render/BannerRenderer.hpp"
#include "game/client/render/BeaconRenderer.hpp"
#include "game/client/render/BedRenderer.hpp"
#include "game/client/render/ChestRenderer.hpp"
#include "game/client/render/ConduitRenderer.hpp"
#include "game/client/render/EnderChestRenderer.hpp"
#include "game/client/render/EnchantTableRenderer.hpp"
#include "game/client/render/Font.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/MobSpawnerRenderer.hpp"
#include "game/client/render/PistonHeadRenderer.hpp"
#include "game/client/render/ShulkerBoxRenderer.hpp"
#include "game/client/render/ShulkerModel.hpp"
#include "game/client/render/SignRenderer.hpp"
#include "game/client/render/SkullBlockRenderer.hpp"
#include "game/client/render/TexturePack.hpp"
#include "game/client/render/Textures.hpp"
#include "game/client/render/TheEndGatewayRenderer.hpp"
#include "game/client/render/TheEndPortalRenderer.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/HitResult.hpp"
#include "game/core/Lighting.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockEntity.hpp"

BlockEntityRenderDispatcher* BlockEntityRenderDispatcher::instance = NULL;
double BlockEntityRenderDispatcher::xOff = 0.0;
double BlockEntityRenderDispatcher::yOff = 0.0;
double BlockEntityRenderDispatcher::zOff = 0.0;

BlockEntityRenderDispatcher::BlockEntityRenderDispatcher()
    : boost::unordered_map<unsigned long long, BlockEntityRenderer*>(),
      mFont(NULL), mTextures(NULL), mLevel(NULL), mCamera(), mCameraYRot(0.0f),
      mCameraXRot(0.0f), mCameraHitResult(NULL), mX(0.0), mY(0.0), mZ(0.0)
{
    GlStateManager::enableLighting();

    (*this)[0x01000202ULL] = new SignRenderer();
    (*this)[0x01200201ULL] = new MobSpawnerRenderer();
    (*this)[0x01200205ULL] = new PistonHeadRenderer();
    (*this)[0x01E00201ULL] = new ChestRenderer();
    (*this)[0x01200208ULL] = new EnderChestRenderer();
    (*this)[0x01200206ULL] = new EnchantTableRenderer();
    (*this)[0x01300200ULL] = new TheEndPortalRenderer();
    (*this)[0x01300201ULL] = new TheEndGatewayRenderer();
    (*this)[0x01200207ULL] = new SkullBlockRenderer();
    (*this)[0x01E00202ULL] = NULL;
    (*this)[0x01E00204ULL] = new BeaconRenderer();
    (*this)[0x0100020CULL] = new BannerRenderer();
    ModelDefinition shulkerModelDefinition;
    (*this)[0x0120020FULL] =
        new ShulkerBoxRenderer(new ShulkerModel(shulkerModelDefinition));
    (*this)[0x01000210ULL] = new BedRenderer();
    (*this)[0x01200212ULL] = new ConduitRenderer();

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
    instance = new BlockEntityRenderDispatcher();
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
    static_cast<SignRenderer*>((*this)[0x01000202ULL])->tick();
}

void BlockEntityRenderDispatcher::render(boost::shared_ptr<BlockEntity> blockEntity, double x,
                                         double y, double z, float a, int breakProgress,
                                         bool bTranslucent, float alpha, bool bUseDisplayList)
{
    BlockEntityRenderer* renderer = getRenderer(blockEntity);
    if (renderer != NULL)
    {
        if (Minecraft::GetInstance()->GetMiniGame()->HasFeatureCustomBeacons()
            && blockEntity->GetType() == 0x01E00204ULL)
        {
            static_cast<BeaconRenderer*>(renderer)->renderSpecialBeamDirection(
                blockEntity, x, y, z, mX, mY, mZ, a, breakProgress, bTranslucent, alpha,
                bUseDisplayList);
        }
        else
        {
            renderer->render(blockEntity, x, y, z, a, breakProgress, bTranslucent, alpha,
                             bUseDisplayList);
        }
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
    if (blockEntity->distanceToSqr(mX, mY, mZ) < blockEntity->getViewDistance())
    {
        Lighting::turnOn();
        BlockPos position = blockEntity->getBlockPos();
        int lightColor = mLevel->getLightColor(position, 0, -1);
        GlStateManager::multiTexCoord2f((float)(lightColor % 65536),
                                        (float)(lightColor / 65536));
        if (bTranslucent)
        {
            GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
        }
        position = blockEntity->getBlockPos();
        render(blockEntity, (double)position.x - xOff, (double)position.y - yOff,
               (double)position.z - zOff, partialTicks, breakProgress, bTranslucent, 1.0f,
               true);
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
