#include "game/client/render/EntityRenderer.hpp"

#include <algorithm>
#include <cmath>

#include "game/client/render/EntityRenderDispatcher.hpp"
#include "game/client/render/GameRenderer.hpp"
#include "game/client/render/Model.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/client/render/TextureAtlasSprite.hpp"
#include "game/client/render/TexturePack.hpp"
#include "game/client/render/Textures.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/Culler.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/util/AABB.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Iterator.hpp"

static ResourceLocation sShadowTexture(L"textures/misc/shadow.png");

EntityRenderer::EntityRenderer(EntityRenderDispatcher* dispatcher)
    : mDispatcher(dispatcher), mShadowRadius(0.0F), mShadowStrength(1.0F), mSolidRender(false), mModel(NULL), mOriginalModel(NULL)
{
}

EntityRenderer::~EntityRenderer()
{
}

bool EntityRenderer::shouldRender(boost::shared_ptr<Entity>& entity, Culler* culler, double x, double y, double z)
{
    AABB* box = entity->getBoundingBoxForCulling()->grow(0.5);
    if (box->hasNaN() || box->getSize() == 0.0)
    {
        box = AABB::newTemp(entity->mX - 2.0, entity->mY - 2.0, entity->mZ - 2.0,
                            entity->mX + 2.0, entity->mY + 2.0, entity->mZ + 2.0);
    }

    return entity->shouldRender(x, y, z) && (entity->mAddedToChunk || culler->isVisible(entity->mBoundingBox));
}

bool EntityRenderer::shouldRender(boost::shared_ptr<Entity>& entity, Culler* culler, double distance)
{
    AABB* box = entity->getBoundingBoxForCulling()->grow(0.5);
    if (box->hasNaN() || box->getSize() == 0.0)
    {
        box = AABB::newTemp(entity->mX - 2.0, entity->mY - 2.0, entity->mZ - 2.0,
                            entity->mX + 2.0, entity->mY + 2.0, entity->mZ + 2.0);
    }

    return entity->shouldRender(distance) && (entity->mAddedToChunk || culler->isVisible(entity->mBoundingBox));
}

void EntityRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float, float)
{
    if (!mSolidRender)
        renderName(entity, x, y, z);
}

void EntityRenderer::render(AABB* box, double x, double y, double z)
{
    GlStateManager::disableTexture();
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    builder->color(1.0F, 1.0F, 1.0F, 1.0F);
    builder->begin();
    builder->offset((float)x, (float)y, (float)z);

    builder->normal(0.0F, 0.0F, -1.0F);
    builder->vertexDouble(box->mMinX, box->mMaxY, box->mMinZ);
    builder->vertexDouble(box->mMaxX, box->mMaxY, box->mMinZ);
    builder->vertexDouble(box->mMaxX, box->mMinY, box->mMinZ);
    builder->vertexDouble(box->mMinX, box->mMinY, box->mMinZ);
    builder->normal(0.0F, 0.0F, 1.0F);
    builder->vertexDouble(box->mMinX, box->mMinY, box->mMaxZ);
    builder->vertexDouble(box->mMaxX, box->mMinY, box->mMaxZ);
    builder->vertexDouble(box->mMaxX, box->mMaxY, box->mMaxZ);
    builder->vertexDouble(box->mMinX, box->mMaxY, box->mMaxZ);
    builder->normal(0.0F, -1.0F, 0.0F);
    builder->vertexDouble(box->mMinX, box->mMinY, box->mMinZ);
    builder->vertexDouble(box->mMaxX, box->mMinY, box->mMinZ);
    builder->vertexDouble(box->mMaxX, box->mMinY, box->mMaxZ);
    builder->vertexDouble(box->mMinX, box->mMinY, box->mMaxZ);
    builder->normal(0.0F, 1.0F, 0.0F);
    builder->vertexDouble(box->mMinX, box->mMaxY, box->mMaxZ);
    builder->vertexDouble(box->mMaxX, box->mMaxY, box->mMaxZ);
    builder->vertexDouble(box->mMaxX, box->mMaxY, box->mMinZ);
    builder->vertexDouble(box->mMinX, box->mMaxY, box->mMinZ);
    builder->normal(-1.0F, 0.0F, 0.0F);
    builder->vertexDouble(box->mMinX, box->mMinY, box->mMaxZ);
    builder->vertexDouble(box->mMinX, box->mMaxY, box->mMaxZ);
    builder->vertexDouble(box->mMinX, box->mMaxY, box->mMinZ);
    builder->vertexDouble(box->mMinX, box->mMinY, box->mMinZ);
    builder->normal(1.0F, 0.0F, 0.0F);
    builder->vertexDouble(box->mMaxX, box->mMinY, box->mMinZ);
    builder->vertexDouble(box->mMaxX, box->mMaxY, box->mMinZ);
    builder->vertexDouble(box->mMaxX, box->mMaxY, box->mMaxZ);
    builder->vertexDouble(box->mMaxX, box->mMinY, box->mMaxZ);

    builder->offset(0.0F, 0.0F, 0.0F);
    builder->end();
    GlStateManager::enableTexture();
}

void EntityRenderer::renderName(boost::shared_ptr<Entity>& entity, double x, double y, double z)
{
    if (shouldShowName(entity))
        renderNameTag(entity, entity->getName(), x, y, z, 64, 0xFF000000);
}

bool EntityRenderer::shouldShowName(boost::shared_ptr<Entity>& entity)
{
    return entity->shouldShowName() && entity->hasCustomName();
}

void EntityRenderer::renderNameTags(boost::shared_ptr<Entity>& entity, double x, double y, double z, const std::wstring& name, double, int color)
{
    renderNameTag(entity, name, x, y, z, 64, color);
}

bool EntityRenderer::bindTexture(boost::shared_ptr<Entity>& entity)
{
    const ResourceLocation* location = getTextureLocation(entity);
    if (location == NULL)
        return false;
    bindTexture(location);
    return true;
}

void EntityRenderer::bindTexture(const ResourceLocation* location)
{
    mDispatcher->mTextures->bindTexture(location);
}

bool EntityRenderer::bindTexture(const std::wstring& path, int index)
{
    Textures* textures = mDispatcher->mTextures;
    int texture = textures->loadMemTexture(path, index);
    if (texture < 0)
        return false;
    GlStateManager::bindTexture(texture);
    textures->clearLastBoundId();
    return true;
}

bool EntityRenderer::bindTexture(const std::wstring& path, const std::wstring& alternate)
{
    Textures* textures = mDispatcher->mTextures;
    int texture = textures->loadMemTexture(path, alternate);
    if (texture < 0)
        return false;
    GlStateManager::bindTexture(texture);
    textures->clearLastBoundId();
    return true;
}

const ResourceLocation* EntityRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return NULL;
}

void EntityRenderer::renderFlame(boost::shared_ptr<Entity>& entity, double x, double y, double z, float)
{
    GlStateManager::disableLighting();
    FireBlock* fire = static_cast<FireBlock*>(Blocks::FIRE);
    TextureAtlasSprite* fire0 = fire->getTextureLayer(0);
    TextureAtlasSprite* fire1 = fire->getTextureLayer(1);
    GlStateManager::pushMatrix();
    GlStateManager::translatef((float)x, (float)y, (float)z);
    float scale = entity->mBbWidth * 1.4F;
    GlStateManager::scalef(scale, scale, scale);
    bindTexture(mDispatcher->mTextures->getTextureLocation(0));

    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    float halfWidth = 0.5F;
    float height = entity->mBbHeight / scale;
    float yOffset = (float)(entity->mY - entity->mBoundingBox->mMinY);
    GlStateManager::rotatef(-mDispatcher->mCameraYRot, 0.0F, 1.0F, 0.0F);
    GlStateManager::translatef(0.0F, 0.0F, (float)std::floor(height) * 0.02F - 0.3F);
    GlStateManager::color4f(1.0F, 1.0F, 1.0F, 1.0F);

    float depth = 0.0F;
    int layer = 0;
    builder->begin();
    while (height > 0.0F)
    {
        TextureAtlasSprite* sprite = (layer & 1) == 0 ? fire0 : fire1;
        float u0 = (float)sprite->getU0(false);
        float v0 = (float)sprite->getV0(false);
        float u1 = (float)sprite->getU1(false);
        float v1 = (float)sprite->getV1(false);
        if (((layer / 2) & 1) == 0)
            std::swap(u0, u1);
        builder->vertexUV(halfWidth, -yOffset, depth, u1, v1);
        builder->vertexUV(-halfWidth, -yOffset, depth, u0, v1);
        builder->vertexUV(-halfWidth, 1.4F - yOffset, depth, u0, v0);
        builder->vertexUV(halfWidth, 1.4F - yOffset, depth, u1, v0);
        height -= 0.45F;
        depth += 0.03F;
        yOffset -= 0.45F;
        halfWidth *= 0.9F;
        ++layer;
    }
    builder->end();
    GlStateManager::popMatrix();
    GlStateManager::enableLighting();
}

void EntityRenderer::renderShadow(boost::shared_ptr<Entity>& entity, double x, double y, double z, float alpha, float partialTick)
{
    GlStateManager::disableLighting();
    GlStateManager::enableBlend();
    GlStateManager::blendFunc(4, 5);
    GlStateManager::polygonOffset(-3.0F, -0.3F);
    mDispatcher->mTextures->bindTexture(&sShadowTexture);
    Level* level = getLevel();
    GlStateManager::depthMask(false);

    float radius = mShadowRadius;
    if ((entity->GetType() & 0x0D000000ULL) == 0x0D000000ULL)
    {
        boost::shared_ptr<Mob> mob = boost::static_pointer_cast<Mob>(entity);
        radius *= (float)mob->getSizeScale();
        if ((mob->GetType() & 0x0D600000ULL) == 0x0D600000ULL)
        {
            boost::shared_ptr<Animal> animal = boost::static_pointer_cast<Animal>(mob);
            if (animal->isBaby())
                radius *= 0.5F;
        }
    }

    double entityX = entity->mXOld + (entity->mX - entity->mXOld) * partialTick;
    double entityY = entity->mYOld + (entity->mY - entity->mYOld) * partialTick;
    double entityZ = entity->mZOld + (entity->mZ - entity->mZOld) * partialTick;
    int minX = (int)std::floor(entityX - radius);
    int maxX = (int)std::floor(entityX + radius);
    int minY = (int)std::floor(entityY - radius);
    int maxY = (int)std::floor(entityY);
    int minZ = (int)std::floor(entityZ - radius);
    int maxZ = (int)std::floor(entityZ + radius);

    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    builder->begin();
    Iterator<BlockPos*>* positions = BlockPos::between(BlockPos(minX, minY, minZ), BlockPos(maxX, maxY, maxZ));
    while (positions->hasNext())
    {
        BlockPos pos = *positions->next();
        const BlockState* state = level->getBlockState(pos.below());
        if (state->getRenderShape() != -1 && level->getRawBrightness(pos) > 3)
            renderBlockShadow(state, x, y, z, pos, alpha, radius, x - entityX, y - entityY, z - entityZ);
    }
    delete positions;
    builder->end();
    GlStateManager::color4f(1.0F, 1.0F, 1.0F, 1.0F);
    GlStateManager::disableBlend();
    GlStateManager::depthMask(true);
    GlStateManager::enableLighting();
    GlStateManager::polygonOffset(0.0F, 0.0F);
}

Level* EntityRenderer::getLevel()
{
    return mDispatcher->mLevel;
}

void EntityRenderer::renderBlockShadow(const BlockState* state, double x, double y, double z, const BlockPos& pos,
                                       float alpha, float radius, double offsetX, double offsetY, double offsetZ)
{
    if (!state->isSolidRender())
        return;
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    Level* level = getLevel();
    float shade = (float)(alpha - (y - ((double)pos.y + offsetY)) * 0.5) * 0.5F;
    shade *= level->getBrightness(pos);
    if (shade < 0.0F)
        return;
    if (shade > 1.0F)
        shade = 1.0F;

    builder->color(1.0F, 1.0F, 1.0F, shade);
    const AABB* shape = state->getShape(level, pos);
    double minX = (double)pos.x + shape->mMinX + offsetX;
    double maxX = (double)pos.x + shape->mMaxX + offsetX;
    double minY = (double)pos.y + shape->mMinY + offsetY + 0.015625;
    double minZ = (double)pos.z + shape->mMinZ + offsetZ;
    double maxZ = (double)pos.z + shape->mMaxZ + offsetZ;
    float minU = (float)((x - minX) * 0.5 / radius + 0.5);
    float maxU = (float)((x - maxX) * 0.5 / radius + 0.5);
    float minV = (float)((z - minZ) * 0.5 / radius + 0.5);
    float maxV = (float)((z - maxZ) * 0.5 / radius + 0.5);
    builder->vertexUV((float)minX, (float)minY, (float)minZ, minU, minV);
    builder->vertexUV((float)minX, (float)minY, (float)maxZ, minU, maxV);
    builder->vertexUV((float)maxX, (float)minY, (float)maxZ, maxU, maxV);
    builder->vertexUV((float)maxX, (float)minY, (float)minZ, maxU, minV);
}

void EntityRenderer::postRender(boost::shared_ptr<Entity>& entity, double x, double y, double z, float, float partialTick, bool renderShadowFlag)
{
    if (mDispatcher->mOptions != NULL)
    {
        if (!mDispatcher->mRenderHitBoxes && renderShadowFlag && mDispatcher->mOptions->mFancyGraphics
            && mShadowRadius > 0.0F && !entity->isInvisible() && mDispatcher->shouldRenderShadow())
        {
            double distance = mDispatcher->distanceToSqr(entity->mX, entity->mY, entity->mZ);
            float alpha = (float)(1.0 - distance / 256.0) * mShadowStrength;
            if (alpha > 0.0F)
                renderShadow(entity, x, y, z, alpha, partialTick);
        }

        if (entity->displayFireAnimation())
        {
            bool spectator = false;
            if ((entity->GetType() & ENTITY_TYPE_PLAYER) == ENTITY_TYPE_PLAYER)
                spectator = boost::static_pointer_cast<Player>(entity)->isSpectator();
            if (!spectator)
                renderFlame(entity, x, y, z, partialTick);
        }
    }
}

Font* EntityRenderer::getFont()
{
    return mDispatcher->getFont();
}

void EntityRenderer::renderNameTag(boost::shared_ptr<Entity>& entity, const std::wstring& name, double x, double y, double z, int maxDistance, int color)
{
    boost::shared_ptr<Entity> camera = mDispatcher->mCamera;
    double distance = entity->distanceToSqr(camera);
    if (distance > (double)(maxDistance * maxDistance))
        return;

    bool sneaking = entity->isSneaking();
    float yOffset = entity->mBbHeight + 0.5F;
    if (sneaking)
        yOffset -= 0.25F;
    std::wstring displayName(name);
    int nameOffset = 0;
    if ((entity->GetType() & ENTITY_TYPE_PLAYER) == ENTITY_TYPE_PLAYER)
    {
        boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);
        if (CMinecraftApp::sInstance.isXuidDeadmau5(player->mPlayerUID))
            nameOffset = -10;
        displayName = name;
    }

    int fadeDistance = 256;
    if (!gRenderer->IsHiDef() || CMinecraftApp::sInstance.GetLocalPlayerCount() > 2)
        fadeDistance = 64;
    float alpha = 1.0F;
    if (distance >= (double)fadeDistance)
    {
        int fade = (int)std::sqrt(distance - fadeDistance);
        if (fade <= fadeDistance)
        {
            int divisor = fade / 2;
            alpha = divisor == 0 ? 1.0F : 1.0F / divisor;
            if (alpha > 1.0F)
                alpha = 1.0F;
        }
        else
        {
            alpha = 0.0F;
        }
    }

    bool renderSneaking = sneaking && !Minecraft::InMiniGame(eMiniGameId_Any, true);
    GameRenderer::renderNameTagInWorld(getFont(), displayName, (float)x, (float)y + yOffset, (float)z,
                                       nameOffset, mDispatcher->mCameraYRot, mDispatcher->mCameraXRot,
                                       mDispatcher->mOptions->mThirdPersonView == 2, renderSneaking, color, alpha);
}

bool EntityRenderer::hasSecondPass()
{
    return false;
}

void EntityRenderer::renderSecondPass(boost::shared_ptr<Entity>&, double, double, double, float, float)
{
}

void EntityRenderer::setupForTexturePack(TexturePack* pack)
{
    std::wstring modelName(L"");
    std::wstring originalModelName(L"");
    Model* model = NULL;
    if (mModel != NULL)
    {
        modelName = getModelName();
        originalModelName = getOriginalModelName();
        model = mModel;
    }
    if (mOriginalModel == NULL)
        mOriginalModel = model;
    if (model != mOriginalModel)
    {
        if (model != NULL)
            delete model;
        mModel = NULL;
    }
    if (pack->HasCustomModel(modelName))
        mModel = createModel(*pack->GetCustomModel(modelName));
    else if (pack->HasCustomModel(originalModelName))
        mModel = createModel(*pack->GetCustomModel(originalModelName));
    else
        mModel = mOriginalModel;
}

void EntityRenderer::SetItemFrame(bool)
{
}

Model* EntityRenderer::getModel()
{
    return mModel;
}

std::wstring EntityRenderer::getModelName()
{
    return L"";
}

Model* EntityRenderer::createModel(const ModelDefinition&)
{
    return NULL;
}

EntityRenderDispatcher* EntityRenderer::getDispatcher()
{
    return mDispatcher;
}

std::wstring EntityRenderer::getOriginalModelName()
{
    if (mModel != NULL)
        return mModel->getName();
    return L"";
}

unsigned int EntityRenderer::getTeamColor(boost::shared_ptr<Entity>&)
{
    return 0x00FFFFFFU;
}

void EntityRenderer::setSolidRender(bool solid)
{
    mSolidRender = solid;
}
