#include "game/client/render/GameRenderer.hpp"
#include "game/core/EntityType.hpp"
#include "game/client/render/Font.hpp"
#include "game/client/render/MapRenderer.hpp"
#include "game/client/render/LevelRenderer.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/client/render/Textures.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/BufferedImage.hpp"
#include "game/core/CriticalSection.hpp"
#include "game/core/Options.hpp"
#include "game/core/GameType.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Mth.hpp"
#include "game/util/Math.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/core/HitResult.hpp"
#include "game/core/FloatBuffer.hpp"
#include "game/core/MemoryTracker.hpp"
#include "game/core/Minecraft.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Inventory.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/biome/BiomeSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/generation/Player.hpp"
#include "game/core/Minecraft.hpp"
#include "game/world/generation/AbstractClientPlayer.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/HangingEntity.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/minigame/ThermalAreaRuleDefinition.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/util/AABB.hpp"
#include "game/world/LevelRuleset.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/client/render/ParticleEngine.hpp"
#include "game/client/render/TexturePackRepository.hpp"
#include "game/core/Camera.hpp"
#include "game/core/CameraController.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ColourTable.hpp"
#include "game/core/Direction.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/System.hpp"
#include "game/entity/mob/Animal.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/HorizontalDirectionalBlock.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/core/Options.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/generation/LocalPlayer.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/core/Frustum.hpp"
#include "game/core/FrustumCuller.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/InfoBarManager.hpp"
#include "game/core/Lighting.hpp"
#include "game/core/GhostController.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/client/render/EntityRenderDispatcher.hpp"
#include "game/client/render/ItemInHandRenderer.hpp"
#include "game/client/render/PlayerRenderer.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/core/Gui.hpp"
#include "game/core/BossHealthOverlay.hpp"
#include "game/data/ElementStorage.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/generation/Player.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "game/client/render/HumanoidModel.hpp"
#include "game/client/render/ElytraModel.hpp"
#include "game/item/Items.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/world/generation/Player.hpp"
#include "game/item/Inventory.hpp"
#include "game/client/render/ItemRenderer.hpp"
#include "game/core/Compression.hpp"
#include "game/core/IntCache.hpp"
#include "game/core/ShutdownManager.hpp"
#include "game/core/PostChain.hpp"
#include "game/world/block/CompressedBlockStorage.hpp"
#include "game/data/ElementStorage.hpp"
#include "game/data/SparseDataStorage.hpp"
#include "game/data/SparseLightStorage.hpp"
#include "game/ui/Screen.hpp"
#include "game/ui/ScreenSizeCalculator.hpp"
#include "platform/wiiu/C4JEventArrayImpl.hpp"
#include "platform/wiiu/C4JThreadImpl.hpp"
#include "game/core/MemSect.hpp"

#include <cmath>
#include <cstring>
#include <vector>

extern bool gFestiveEvent;

namespace
{
struct ElementStorageWithTick
{
    int tick;
    ElementStorage* storage;
};

C4JEventArrayImpl* sRenderUpdateEvents = NULL;
C4JThreadImpl* sRenderUpdateThread = NULL;
OSFastMutex sDeleteMutex;
bool sRenderUpdateThreadActive = false;

std::vector<LevelChunk*> sPendingChunkDeletes[2];
int sPendingChunkDeleteList = 0;

std::vector<unsigned char*> sPendingByteDeletes[2];
int sPendingByteDeleteList = 0;

std::vector<SparseLightStorage*> sPendingLightDeletes[2];
int sPendingLightDeleteList = 0;

std::vector<CompressedBlockStorage*> sPendingCompressedDeletes[2];
int sPendingCompressedDeleteList = 0;

std::vector<SparseDataStorage*> sPendingDataDeletes[2];
int sPendingDataDeleteList = 0;

std::vector<ElementStorageWithTick> sPendingElementStorageDeletes;

int sAnaglyphEyeIndex = 0;

static const BlockPos sNetherFogOffsets[] = {
    BlockPos(0, 0, 0),
    BlockPos(-12, 0, 0),
    BlockPos(-8, 0, -8),
    BlockPos(0, 0, -12),
    BlockPos(8, 0, -8),
    BlockPos(12, 0, 0),
    BlockPos(8, 0, 8),
    BlockPos(0, 0, 12),
    BlockPos(-8, 0, 8),
    BlockPos(0, -3, 0),
    BlockPos(-12, -3, 0),
    BlockPos(-8, -3, -8),
    BlockPos(0, -3, -12),
    BlockPos(8, -3, -8),
    BlockPos(12, -3, 0),
    BlockPos(8, -3, 8),
    BlockPos(0, -3, 12),
    BlockPos(-8, -3, 8),
    BlockPos(0, 3, 0),
    BlockPos(-12, 3, 0),
    BlockPos(-8, 3, -8),
    BlockPos(0, 3, -12),
    BlockPos(8, 3, -8),
    BlockPos(12, 3, 0),
    BlockPos(8, 3, 8),
    BlockPos(0, 3, 12),
    BlockPos(-8, 3, 8),
};

static const BlockPos sEndFogOffsets[] = {
    BlockPos(0, 0, 0),
    BlockPos(-8, 0, 0),
    BlockPos(-5, 0, -5),
    BlockPos(0, 0, -8),
    BlockPos(5, 0, -5),
    BlockPos(8, 0, 0),
    BlockPos(5, 0, 5),
    BlockPos(0, 0, 8),
    BlockPos(-5, 0, 5),
    BlockPos(0, -2, 0),
    BlockPos(-8, -2, 0),
    BlockPos(-5, -2, -5),
    BlockPos(0, -2, -8),
    BlockPos(5, -2, -5),
    BlockPos(8, -2, 0),
    BlockPos(5, -2, 5),
    BlockPos(0, -2, 8),
    BlockPos(-5, -2, 5),
    BlockPos(0, 2, 0),
    BlockPos(-8, 2, 0),
    BlockPos(-5, 2, -5),
    BlockPos(0, 2, -8),
    BlockPos(5, 2, -5),
    BlockPos(8, 2, 0),
    BlockPos(5, 2, 5),
    BlockPos(0, 2, 8),
    BlockPos(-5, 2, 5),
};

static const BlockPos sOverworldFogOffsets[] = {
    BlockPos(0, 0, 0),
    BlockPos(-4, 0, 0),
    BlockPos(-2, 0, -2),
    BlockPos(0, 0, -4),
    BlockPos(2, 0, -2),
    BlockPos(4, 0, 0),
    BlockPos(2, 0, 2),
    BlockPos(0, 0, 4),
    BlockPos(-2, 0, 2),
    BlockPos(0, -1, 0),
    BlockPos(-4, -1, 0),
    BlockPos(-2, -1, -2),
    BlockPos(0, -1, -4),
    BlockPos(2, -1, -2),
    BlockPos(4, -1, 0),
    BlockPos(2, -1, 2),
    BlockPos(0, -1, 4),
    BlockPos(-2, -1, 2),
    BlockPos(0, 1, 0),
    BlockPos(-4, 1, 0),
    BlockPos(-2, 1, -2),
    BlockPos(0, 1, -4),
    BlockPos(2, 1, -2),
    BlockPos(4, 1, 0),
    BlockPos(2, 1, 2),
    BlockPos(0, 1, 4),
    BlockPos(-2, 1, 2),
};

}

GameRenderer::GameRenderer(Minecraft* minecraft, ResourceManager* resourceManager)
    : mUpdateThreadDisableMask(0), mMinecraft(minecraft), mResourceManager(resourceManager), mRandom(),
      mFogDistance(0.0f), mItemInHandRenderer(minecraft->getItemInHandRenderer()),
      mMapRenderer(new MapRenderer(minecraft->mFont, minecraft->mOptions, minecraft->mTextures)), mTickCount(0),
      mPointedEntity(), mThirdPersonDistance(4.0f), mThirdPersonDistanceO(4.0f), mDebugCamYaw(0.0f),
      mDebugCamYawO(0.0f), mDebugCamPitch(0.0f), mDebugCamPitchO(0.0f), mSmoothCamYaw(0.0f), mSmoothCamPitch(0.0f),
      mSmoothCamFilterX(0.0f), mSmoothCamFilterY(0.0f), mSmoothCamPartialTicks(0.0f),
      mZeroVector(Vec3::newPermanent(0.0, 0.0, 0.0)), mFovSetting(70.0f), mDarkenWorldAmount(0.0f),
      mDarkenWorldAmountO(0.0f), mCloudFog(false), mRenderHand(true), mRenderBlockOutline(true),
      mLastActiveTime(Minecraft::currentTimeMillis()), mBlockLightRedFlicker(0.0f), mBlockLightRedFlickerSpeed(0.0f),
      mRainSoundTime(0), mFogBuffer(NULL), mFogRed(0.0f), mFogGreen(0.0f), mFogBlue(0.0f), mFogBrightnessO(0.0f),
      mFogBrightness(0.0f), mFogHeightLocked(false), mFogHeight(0.0), mPanoramicSection(0), mPanoramicMode(false),
      mZoom(1.0f), mZoomX(0.0f), mZoomY(0.0f), mPostEffect(NULL), mEffectIndex(24), mEffectActive(false),
      mFrameCount(0), mLastNanos(0), mLightTextureDirty(false), mUnusedBuffer(NULL), mDebugView(0)
{
    InitializeCriticalSection(&mUpdateThreadMutex);

    mUnusedBuffer = MemoryTracker::createFloatBuffer(16);
    mFogBuffer = MemoryTracker::createFloatBuffer(16);
    mOwnedArrays[0] = NULL;
    mOwnedArrays[1] = NULL;
    for (int playerIndex = 0; playerIndex < 4; ++playerIndex)
    {
        mFov[playerIndex] = 1.0f;
        mFovO[playerIndex] = 1.0f;
        mUnusedPlayerFloats[playerIndex] = 0.0f;
        mPlayerFogRed[playerIndex] = 0.0f;
        mPlayerFogGreen[playerIndex] = 0.0f;
        mPlayerFogBlue[playerIndex] = 0.0f;
        mPlayerFogDistance[playerIndex] = 0.0f;
        mTargetFogRed[playerIndex] = 0.0f;
        mTargetFogGreen[playerIndex] = 0.0f;
        mTargetFogBlue[playerIndex] = 0.0f;
        mTargetFogDistance[playerIndex] = 0.0f;
        mFogColourChangeTime[playerIndex] = 0;
        mFogDistanceChangeTime[playerIndex] = 0;
        mItemActivationItems[playerIndex] = ItemInstance::EMPTY;
        mItemActivationTicks[playerIndex] = 0;
        mItemActivationOffX[playerIndex] = 0.0f;
        mItemActivationOffY[playerIndex] = 0.0f;
    }

    for (int z = 0; z < 32; ++z)
    {
        for (int x = 0; x < 32; ++x)
        {
            const float dz = static_cast<float>(z - 16);
            const float dx = static_cast<float>(x - 16);
            const float length = std::sqrt(dx * dx + dz * dz);
            mRainSizeX[z << 5 | x] = -dz / length;
            mRainSizeZ[z << 5 | x] = dx / length;
        }
    }

    GlStateManager::enableLighting();
    mMinecraft->mLocalItemInHandRenderers[0] = new ItemInHandRenderer(minecraft);
    mMinecraft->mLocalItemInHandRenderers[1] = new ItemInHandRenderer(minecraft);
    mMinecraft->mLocalItemInHandRenderers[2] = new ItemInHandRenderer(minecraft);
    mMinecraft->mLocalItemInHandRenderers[3] = new ItemInHandRenderer(minecraft);
    GlStateManager::disableLighting();

    BufferedImage* lightImage = new BufferedImage(16, 16, 1);
    for (int playerIndex = 0; playerIndex < 4; ++playerIndex)
        mLightTextures[playerIndex] =
            mMinecraft->mTextures->getTexture(lightImage, C4JRender::eTextureFormat_RGBA8, true);
    delete lightImage;

    for (int playerIndex = 0; playerIndex < 4; ++playerIndex)
        mLightPixels[playerIndex] = arrayWithLength<int>(256, true);

    sRenderUpdateEvents = new C4JEventArrayImpl(2, C4JEventArray::Mode0);
    sRenderUpdateEvents->Set(1);
    InitializeCriticalSection(&sDeleteMutex);
    sRenderUpdateThread = new C4JThreadImpl(runUpdate, NULL, "Chunk update", 0);
    sRenderUpdateThread->SetProcessor(2);
    sRenderUpdateThread->Run();
}

GameRenderer::~GameRenderer()
{
    if (mOwnedArrays[0] != NULL)
        delete[] mOwnedArrays[0];
    if (mOwnedArrays[1] != NULL)
        delete[] mOwnedArrays[1];
    DeleteCriticalSection(&mUpdateThreadMutex);

    mPointedEntity.reset();
}

bool GameRenderer::anaglyph3d = false;

void GameRenderer::renderNameTagInWorld(Font* font, const std::wstring& text, float x, float y, float z,
                                        int nameOffset, float cameraYRot, float cameraXRot, bool thirdPerson,
                                        bool sneaking, int color, float alpha)
{
    static const float kZero = 0.0f;
    static const float kOne = 1.0f;
    static const float kNegOne = -1.0f;
    static const float k255 = 255.0f;
    static const float kNameTagScaleXY = -0.025f;
    static const float kNameTagScaleZ = 0.025f;
    static const float kBgAlpha = 0.25f;
    static const float kLineWidth = 2.0f;
    static const float kNameTagZOffset = -0.04f;

    GlStateManager::pushMatrix();
    GlStateManager::translatef(x, y, z);
    GlStateManager::rotatef(-cameraYRot, kZero, kOne, kZero);
    if (!thirdPerson)
        GlStateManager::rotatef(kOne * cameraXRot, kOne, kZero, kZero);
    else
        GlStateManager::rotatef(kNegOne * cameraXRot, kOne, kZero, kZero);
    GlStateManager::scalef(kNameTagScaleXY, kNameTagScaleXY, kNameTagScaleZ);
    GlStateManager::disableLighting();
    GlStateManager::depthMask(false);

    const int textWidth = font->width(text);
    const int halfWidth = (textWidth >> 1) + ((textWidth < 0 && (textWidth & 1) != 0) ? 1 : 0);

    GlStateManager::enableBlend();
    GlStateManager::blendFunc(4, 5);

    if (!sneaking)
    {
        Tesselator* tesselator = Tesselator::getInstance();
        BufferBuilder* builder = tesselator->getBuilder();

        if (alpha > kZero)
        {
            GlStateManager::color4f(kOne, kOne, kOne, alpha);
            GlStateManager::depthMask(false);
            GlStateManager::disableDepthTest();
            GlStateManager::disableTexture();
            builder->begin();

            int bgAlphaByte;
            if (alpha >= kOne)
            {
                builder->color(kZero, kZero, kZero, kBgAlpha);
                bgAlphaByte = static_cast<int>(alpha * k255);
            }
            else
            {
                bgAlphaByte = static_cast<int>(k255 * alpha);
                builder->color(color, bgAlphaByte);
            }

            const float left = static_cast<float>(-(halfWidth + 1));
            const float right = static_cast<float>(halfWidth + 1);
            const float top = static_cast<float>(nameOffset - 1);
            const float bottom = static_cast<float>(nameOffset + 9);

            builder->vertex(left, top, kZero);
            builder->vertex(left, bottom, kZero);
            builder->vertex(right, bottom, kZero);
            builder->vertex(right, top, kZero);
            builder->end();

            GlStateManager::enableDepthTest();
            GlStateManager::depthMask(true);
            GlStateManager::depthFunc(7);
            GlStateManager::lineWidth(kLineWidth);
            builder->begin(5);
            builder->color(color, bgAlphaByte);
            builder->vertex(left, top, kZero);
            builder->vertex(left, bottom, kZero);
            builder->vertex(right, bottom, kZero);
            builder->vertex(right, top, kZero);
            builder->vertex(left, top, kZero);
            builder->end();
            GlStateManager::depthFunc(3);
            GlStateManager::depthMask(false);
            GlStateManager::disableDepthTest();
            GlStateManager::enableTexture();
            font->draw(text, -halfWidth, nameOffset, 0x20ffffff);
            GlStateManager::enableDepthTest();
            GlStateManager::depthMask(true);
        }

        if (alpha < kOne)
        {
            GlStateManager::color4f(kOne, kOne, kOne, kOne);
            GlStateManager::disableTexture();
            GlStateManager::depthFunc(7);
            builder->begin();
            builder->color(color, 0xff);
            const float left = static_cast<float>(-(halfWidth + 1));
            const float right = static_cast<float>(halfWidth + 1);
            const float top = static_cast<float>(nameOffset - 1);
            const float bottom = static_cast<float>(nameOffset + 8);
            builder->vertex(left, top, kZero);
            builder->vertex(left, bottom, kZero);
            builder->vertex(right, bottom, kZero);
            builder->vertex(right, top, kZero);
            builder->end();
            GlStateManager::depthFunc(3);
            GlStateManager::enableTexture();
            GlStateManager::translatef(kZero, kZero, kNameTagZOffset);
        }

        if (alpha > kZero)
        {
            const int textColor = (static_cast<int>(alpha * k255) << 24) | 0xffffff;
            font->draw(text, -halfWidth, 0, textColor);
        }
    }
    else
    {
        Tesselator* tesselator = Tesselator::getInstance();
        BufferBuilder* builder = tesselator->getBuilder();
        GlStateManager::disableTexture();
        builder->begin();
        builder->color(kZero, kZero, kZero, kBgAlpha);
        const float left = static_cast<float>(-(halfWidth + 1));
        const float right = static_cast<float>(halfWidth + 1);
        const float top = static_cast<float>(nameOffset - 1);
        const float bottom = static_cast<float>(nameOffset + 9);
        builder->vertex(left, top, kZero);
        builder->vertex(left, bottom, kZero);
        builder->vertex(right, bottom, kZero);
        builder->vertex(right, top, kZero);
        builder->end();
        GlStateManager::enableTexture();
        GlStateManager::depthMask(true);
        font->draw(text, -halfWidth, 0, 0x20ffffff);
    }

    GlStateManager::enableLighting();
    GlStateManager::disableBlend();
    GlStateManager::color4f(kOne, kOne, kOne, kOne);
    GlStateManager::popMatrix();
}

unsigned int GameRenderer::getLightTexture(int index, Level*)
{
    return mLightTextures[index];
}

int GameRenderer::runUpdate(void*)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    Vec3::CreateNewThreadStorage();
    AABB::CreateNewThreadStorage();
    IntCache::CreateNewThreadStorage();
    Tesselator::CreateNewThreadStorage(0x100000);
    Compression::UseDefaultThreadStorage();
    gRenderer->InitialiseContext();
    ElementStorage::CreateNewThreadStorage(ElementStorage::eHESThreadId_Renderer);
    Block::CreateNewThreadStorage();

    unsigned int* dirtyChunkBuffer =
        static_cast<unsigned int*>(operator new[](0x20000));

    ShutdownManager::HasStarted(ShutdownManager::eThreadId_RenderUpdate, sRenderUpdateEvents);

    while (ShutdownManager::ShouldRun(ShutdownManager::eThreadId_RenderUpdate))
    {
        sRenderUpdateEvents->WaitForAll(0xffffffff);
        if (!ShutdownManager::ShouldRun(ShutdownManager::eThreadId_RenderUpdate))
            break;

        sRenderUpdateEvents->Set(0);

        int passCount = 0;
        while (passCount < 10
               && minecraft->mLevelRenderer->updateDirtyChunks(dirtyChunkBuffer) != 0)
            ++passCount;

        gRenderer->CBuffDeferredModeEnd();
        minecraft->mLevelRenderer->fullyFlagRenderableBlockEntitiesToBeRemoved();

        EnterCriticalSection(&sDeleteMutex);

        const int chunkList = (sPendingChunkDeleteList + 1) % 2;
        for (size_t i = 0; i < sPendingChunkDeletes[chunkList].size(); ++i)
        {
            LevelChunk* chunk = sPendingChunkDeletes[chunkList][i];
            if (chunk != NULL)
                delete chunk;
        }
        sPendingChunkDeletes[chunkList].clear();
        sPendingChunkDeleteList = chunkList;

        const int byteList = (sPendingByteDeleteList + 1) % 2;
        for (size_t i = 0; i < sPendingByteDeletes[byteList].size(); ++i)
            operator delete(static_cast<void*>(sPendingByteDeletes[byteList][i]));
        sPendingByteDeletes[byteList].clear();
        sPendingByteDeleteList = byteList;

        const int lightList = (sPendingLightDeleteList + 1) % 2;
        for (size_t i = 0; i < sPendingLightDeletes[lightList].size(); ++i)
            delete sPendingLightDeletes[lightList][i];
        sPendingLightDeletes[lightList].clear();
        sPendingLightDeleteList = lightList;

        const int compressedList = (sPendingCompressedDeleteList + 1) % 2;
        for (size_t i = 0; i < sPendingCompressedDeletes[compressedList].size(); ++i)
            delete sPendingCompressedDeletes[compressedList][i];
        sPendingCompressedDeletes[compressedList].clear();
        sPendingCompressedDeleteList = compressedList;

        const int dataList = (sPendingDataDeleteList + 1) % 2;
        for (size_t i = 0; i < sPendingDataDeletes[dataList].size(); ++i)
            delete sPendingDataDeletes[dataList][i];
        sPendingDataDeletes[dataList].clear();
        sPendingDataDeleteList = dataList;

        for (size_t i = 0; i < sPendingElementStorageDeletes.size();)
        {
            ElementStorageWithTick& entry = sPendingElementStorageDeletes[i];
            if (entry.tick != 0)
            {
                ++i;
                continue;
            }

            delete entry.storage;
            sPendingElementStorageDeletes.erase(sPendingElementStorageDeletes.begin() + i);
        }

        LeaveCriticalSection(&sDeleteMutex);

        AABB::resetPool();
        Vec3::resetPool();
        IntCache::Reset();

        sRenderUpdateEvents->Set(1);
    }

    ShutdownManager::HasFinished(ShutdownManager::eThreadId_RenderUpdate);
    return 0;
}

unsigned int GameRenderer::getTickCount()
{
    return mTickCount;
}

unsigned int GameRenderer::postEffectActive()
{
    return 0;
}

void GameRenderer::shutdownEffect()
{
    if (mPostEffect != NULL)
        mPostEffect->destroy();

    mPostEffect = NULL;
    mEffectIndex = EFFECT_NONE;
}

void GameRenderer::checkEntityPostEffect(boost::shared_ptr<Entity>)
{
    return;
}

void GameRenderer::cycleEffect()
{
    return;
}

void GameRenderer::onResourceManagerReload(ResourceManager*)
{
    if (mPostEffect != NULL)
        mPostEffect->destroy();

    mPostEffect = NULL;

    if (mEffectIndex == EFFECT_NONE)
    {
        boost::shared_ptr<Entity> camera = mMinecraft->getCameraEntity();
        checkEntityPostEffect(camera);
    }
}

double GameRenderer::GetFovVal()
{
    return mFovSetting;
}

void GameRenderer::UpdateElementStorageDeleteTicks()
{
    EnterCriticalSection(&sDeleteMutex);
    for (size_t i = 0; i < sPendingElementStorageDeletes.size(); ++i)
    {
        ElementStorageWithTick& entry = sPendingElementStorageDeletes[i];
        if (entry.tick != 0)
            --entry.tick;
    }
    LeaveCriticalSection(&sDeleteMutex);
}

void GameRenderer::tickFov()
{
    boost::shared_ptr<MultiplayerLocalPlayer> player = Minecraft::GetInstance()->mPlayer;
    const int playerIndex = player != NULL ? player->mUserIndex : 0;

    const double targetFov = player->getFieldOfViewModifier();

    mFovO[playerIndex] = mFov[playerIndex];
    mFov[playerIndex] = static_cast<float>((targetFov - mFov[playerIndex]) * 0.5 + mFov[playerIndex]);

    if (mFov[playerIndex] > 1.5f)
        mFov[playerIndex] = 1.5f;
    else if (mFov[playerIndex] < 0.1f)
        mFov[playerIndex] = 0.1f;
}

void GameRenderer::tickLightTexture()
{
    mBlockLightRedFlickerSpeed = static_cast<float>(
        (Math::random() - Math::random()) * Math::random() * Math::random() + mBlockLightRedFlickerSpeed);
    mBlockLightRedFlickerSpeed *= 0.9f;
    mBlockLightRedFlicker += mBlockLightRedFlickerSpeed - mBlockLightRedFlicker;
    mLightTextureDirty = true;

}

boost::shared_ptr<Entity> GameRenderer::getCamera()
{
    return Minecraft::GetInstance()->getCameraEntity();
}

void GameRenderer::tickRain()
{
    Level* level = mMinecraft->mLevel;
    float rainLevel = level->getRainLevel(1.0f);
    if (!mMinecraft->mOptions->mFancyGraphics)
        rainLevel *= 0.5f;
    if (rainLevel == 0.0f)
        return;

    const float rainIntensity = rainLevel / static_cast<float>(mMinecraft->mLevelRenderer->activePlayers() + 1);
    mRandom.setSeed(static_cast<long long>(mTickCount) * 312987231LL);
    boost::shared_ptr<Entity> camera = getCamera();
    const BlockPos cameraPos(camera);

    int particleCount = static_cast<int>(100.0f * rainIntensity * rainIntensity);
    if (mMinecraft->mOptions->mParticles == 1)
        particleCount >>= 1;
    else if (mMinecraft->mOptions->mParticles == 2)
        particleCount = 0;

    int rainHits = 0;
    double soundX = 0.0;
    double soundY = 0.0;
    double soundZ = 0.0;
    for (int i = 0; i < particleCount; ++i)
    {
        const int offsetX = mRandom.nextInt(10) - mRandom.nextInt(10);
        const int offsetZ = mRandom.nextInt(10) - mRandom.nextInt(10);
        const BlockPos rainPos = level->getTopRainBlockPos(cameraPos.offset(offsetX, 0, offsetZ));
        Biome* biome = level->getBiome(rainPos);
        const BlockPos groundPos = rainPos.below();
        const BlockState* ground = level->getBlockState(groundPos);
        if (rainPos.y > cameraPos.y + 10 || rainPos.y < cameraPos.y - 10 || !biome->hasRain()
            || biome->getTemperature(rainPos) < 0.15f)
            continue;

        const double randomX = mRandom.nextDouble();
        const double randomZ = mRandom.nextDouble();
        const AABB* shape = ground->getShape(level, rainPos);
        if (ground->getMaterial() != Material::LAVA && ground->getBlock() != Blocks::magma)
        {
            if (ground->getMaterial() != Material::AIR)
            {
                const double dropX = groundPos.x + randomX;
                const double dropY = static_cast<float>(groundPos.y) + 0.1f + shape->mMaxY;
                const double dropZ = groundPos.z + randomZ;
                if (mRandom.nextInt(++rainHits) == 0)
                {
                    soundX = dropX;
                    soundY = dropY - 1.0;
                    soundZ = dropZ;
                }
                level->addParticle(PARTICLE_DROPLET, dropX, dropY, dropZ, 0.0, 0.0, 0.0, arrayWithLength<int>());
            }
        }
        else
        {
            level->addParticle(PARTICLE_SMOKE, rainPos.x + randomX,
                               static_cast<float>(rainPos.y) + 0.1f - shape->mMinY, rainPos.z + randomZ, 0.0,
                               0.0, 0.0, arrayWithLength<int>());
        }
    }

    if (rainHits > 0 && mRandom.nextInt(3) < mRainSoundTime++)
    {
        mRainSoundTime = 0;
        MemSect(0x18);
        if (soundY > cameraPos.y + 1 && level->getTopRainBlockPos(cameraPos).y > cameraPos.y)
            level->playLocalSound(soundX, soundY, soundZ, SoundEvents::WEATHER_RAIN_ABOVE,
                                  SoundSource::eSoundSource_Weather, 0.1f, 0.5f, false, 16.0f);
        else
            level->playLocalSound(soundX, soundY, soundZ, SoundEvents::WEATHER_RAIN,
                                  SoundSource::eSoundSource_Weather, 0.2f, 1.0f, false, 16.0f);
        MemSect(0);
    }
}

void GameRenderer::tick(bool tickWorld)
{
    if (tickWorld)
        UpdateElementStorageDeleteTicks();

    tickFov();
    tickLightTexture();

    mFogBrightnessO = mFogBrightness;
    mThirdPersonDistanceO = mThirdPersonDistance;

    {
        boost::shared_ptr<Entity> cameraEntity;
        if (!mMinecraft->mOptions->mUnknownFlag4)
        {
            mSmoothCamFilterX = 0.0f;
            mSmoothCamFilterY = 0.0f;
            cameraEntity = mMinecraft->getCameraEntity();
        }
        else
        {
            const float sensitivity = mMinecraft->mOptions->mSensitivity * 0.6f + 0.2f;
            const float smoothDelta = 0.05f * (sensitivity * sensitivity * sensitivity * 8.0f);
            mSmoothCamFilterX = mSmoothTurnX.getNewDeltaValue(mSmoothCamYaw, smoothDelta);
            mSmoothCamFilterY = mSmoothTurnY.getNewDeltaValue(mSmoothCamPitch, smoothDelta);
            mSmoothCamPartialTicks = 0.0f;
            mSmoothCamPitch = 0.0f;
            mSmoothCamYaw = 0.0f;
            cameraEntity = mMinecraft->getCameraEntity();
        }

        cameraEntity->mEyeHeightO = cameraEntity->mEyeHeight;
        const double eyeHeight = mMinecraft->getCameraEntity()->getEyeHeight();
        cameraEntity->mEyeHeight += static_cast<float>(eyeHeight - cameraEntity->mEyeHeight) * 0.5f;
    }

    float blockBrightness;
    {
        boost::shared_ptr<Entity> cameraForBlockPosition = mMinecraft->getCameraEntity();
        blockBrightness = mMinecraft->mLevel->getBrightness(BlockPos(cameraForBlockPosition));
    }
    const float targetFogBrightness = static_cast<float>(blockBrightness * 0.0f + 1.0f);
    mFogBrightness += (targetFogBrightness - mFogBrightness) * 0.1f;

    mItemInHandRenderer->tick();

    tickRain();

    mDarkenWorldAmountO = mDarkenWorldAmount;

    BossHealthOverlay* bossOverlay = mMinecraft->mGui->getBossOverlay(mMinecraft->mPlayer->mUserIndex);
    if (bossOverlay->shouldDarkenScreen())
    {
        mDarkenWorldAmount += 0.05f;
        if (mDarkenWorldAmount > 1.0f)
            mDarkenWorldAmount = 1.0f;
    }
    else if (mDarkenWorldAmount > 0.0f)
    {
        mDarkenWorldAmount -= 0.0125f;
    }

    const int playerIndex = mMinecraft->mPlayer->mUserIndex;
    if (mItemActivationTicks[playerIndex] > 0)
    {
        --mItemActivationTicks[playerIndex];
        if (mItemActivationTicks[playerIndex] == 0)
            mItemActivationItems[playerIndex] = ItemInstance::EMPTY;
    }


    const int primaryPad = gProfile->GetPrimaryPad();
    if (mMinecraft->mPlayer.get() == mMinecraft->mLocalPlayers[primaryPad].get())
    {
        ++mTickCount;
    }
}

namespace
{
bool isPickableEntity_apply(boost::shared_ptr<Entity> entity)
{
    return isPickable_apply(entity);
}

Predicate<boost::shared_ptr<Entity> >* getPickEntityPredicate()
{
    static Predicate<boost::shared_ptr<Entity> >* sPickPredicate = NULL;
    if (sPickPredicate == NULL)
    {
        static Predicates<boost::shared_ptr<Entity> >::FunctionPredicate sIsPickablePredicate(
            isPickableEntity_apply);
        static const Predicate<boost::shared_ptr<Entity> >* sPredicates[2] = {
            Predicates<boost::shared_ptr<Entity> >::alwaysTrue, &sIsPickablePredicate};
        sPickPredicate = new Predicates<boost::shared_ptr<Entity> >::ConnectivePredicate(
            Predicates<boost::shared_ptr<Entity> >::ConnectivePredicate::eOperator_And, sPredicates, 2);
    }
    return sPickPredicate;
}

void deleteHitResult(HitResult* hit)
{
    if (hit != NULL)
    {
        delete hit;
    }
}
}

void GameRenderer::pick(float partialTick)
{
    static const double kPickDistanceCompare = 3.0;
    static const double kCreativePickRange = 6.0;
    static const double kNoHitDistance = 0.0;

    boost::shared_ptr<Entity> camera = getCamera();
    if (camera.get() == NULL)
    {
        return;
    }

    Level* level = mMinecraft->mLevel;
    if (level == NULL)
    {
        return;
    }

    boost::shared_ptr<Entity>& pointedEntity = mPointedEntity;
    boost::shared_ptr<Entity>& minecraftPointedEntityShared = mMinecraft->mSpectateEntity;

    pointedEntity.reset();
    minecraftPointedEntityShared.reset();

    const double pickRange = mMinecraft->mGameMode->getPickRange();

    deleteHitResult(mMinecraft->mHitResult);
    mMinecraft->mHitResult = NULL;

    MemSect(0x1f);
    mMinecraft->mHitResult = camera->pick(pickRange, partialTick);
    MemSect(0);

    Dimension* dimension = level->mDimension;
    if (mMinecraft->mHitResult != NULL && dimension->getType() != DimensionType::THE_END)
    {
        BlockPos blockPos = mMinecraft->mHitResult->getBlockPos();
        int adjustedX = blockPos.x;
        int adjustedZ = blockPos.z;
        const int stepX = mMinecraft->mHitResult->mDirection->getStepX();
        const int stepZ = mMinecraft->mHitResult->mDirection->getStepZ();
        if ((adjustedX < 0) == (stepX < 0))
        {
            adjustedX += stepX;
        }
        if ((adjustedZ < 0) == (stepZ < 0))
        {
            adjustedZ += stepZ;
        }

        const AABB* bounds = dimension->getPlayerConstraints(blockPos);
        if (adjustedX < static_cast<int>(bounds->mMinX) + 1 ||
            static_cast<int>(bounds->mMaxX) - 2 < adjustedX ||
            adjustedZ < static_cast<int>(bounds->mMinZ) + 1 ||
            static_cast<int>(bounds->mMaxZ) - 2 < adjustedZ)
        {
            deleteHitResult(mMinecraft->mHitResult);
            mMinecraft->mHitResult = NULL;
        }
    }

    boost::shared_ptr<Entity> cameraEntity = mMinecraft->getCameraEntity();
    Entity* cameraRaw = cameraEntity.get();
    Vec3* eyePosition = cameraRaw->getEyePosition(partialTick);

    double entityReach = pickRange;
    if (mMinecraft->mGameMode->hasFarPickRange())
    {
        entityReach = kCreativePickRange;
    }

    if (mMinecraft->mHitResult != NULL)
    {
        entityReach = eyePosition->distanceTo(mMinecraft->mHitResult->mLocation);
    }

    pointedEntity.reset();

    const Predicate<boost::shared_ptr<Entity> >* entityPredicate = getPickEntityPredicate();
    if (mMinecraft->mOptions->mRenderDebug != 0)
    {
        entityPredicate = NULL;
    }

    if (Minecraft::InMiniGame(eMiniGameId_Tumble, false))
    {
        return;
    }

    Vec3* viewVector = cameraRaw->getViewVector(partialTick);
    AABB* searchBox = cameraRaw->mBoundingBox->expand(viewVector->x * entityReach,
                                                      viewVector->y * entityReach,
                                                      viewVector->z * entityReach)
                          ->grow(1.0);

    std::vector<boost::shared_ptr<Entity> >* entities =
        level->getEntities(cameraEntity, searchBox, entityPredicate, NULL);

    Entity* closestEntity = NULL;
    boost::shared_ptr<Entity> closestEntityShared;
    Vec3* closestHitLocation = NULL;
    double closestDistance = entityReach;

    Vec3* rayEnd = Vec3::newTemp(eyePosition->x + viewVector->x * entityReach,
                                 eyePosition->y + viewVector->y * entityReach,
                                 eyePosition->z + viewVector->z * entityReach);

    for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin();
         it != entities->end(); ++it)
    {
        boost::shared_ptr<Entity> candidate = *it;
        Entity* candidateRaw = candidate.get();
        if (candidateRaw == NULL)
        {
            continue;
        }

        AABB* grownBox = candidateRaw->mBoundingBox->grow(candidateRaw->getPickRadius());
        HitResult* clipHit = grownBox->clip(eyePosition, rayEnd);
        const bool eyeInside = grownBox->contains(eyePosition);
        if (!eyeInside && clipHit != NULL)
        {
            const double hitDistance = eyePosition->distanceTo(clipHit->mLocation);
            if (hitDistance < closestDistance || closestDistance == kNoHitDistance)
            {
                boost::shared_ptr<Entity> candidateRoot = candidateRaw->getRootVehicle();
                boost::shared_ptr<Entity> cameraRoot = cameraRaw->getRootVehicle();
                if (candidateRoot.get() != cameraRoot.get())
                {
                    closestEntity = candidateRaw;
                    closestEntityShared = candidate;
                    closestHitLocation = clipHit->mLocation;
                    closestDistance = hitDistance;
                }
                else if (closestDistance == kNoHitDistance)
                {
                    closestEntity = candidateRaw;
                    closestEntityShared = candidate;
                    closestHitLocation = clipHit->mLocation;
                }
            }
        }
        if (clipHit != NULL)
        {
            delete clipHit;
        }
    }

    if (closestEntity != NULL)
    {
        if (mMinecraft->mHitResult == NULL && pickRange > kPickDistanceCompare)
        {
            const double blockPickDistance = eyePosition->distanceTo(closestHitLocation);
            if (blockPickDistance > kPickDistanceCompare)
            {
                deleteHitResult(mMinecraft->mHitResult);
                mMinecraft->mHitResult =
                    new HitResult(HitResult::MISS, closestHitLocation, NULL, BlockPos());
                closestHitLocation = NULL;
                closestEntity = NULL;
                closestEntityShared.reset();
            }
        }

        if (closestEntity != NULL &&
            (closestDistance < entityReach || mMinecraft->mHitResult == NULL))
        {
            pointedEntity = closestEntityShared;

            deleteHitResult(mMinecraft->mHitResult);
            mMinecraft->mHitResult = new HitResult(closestEntityShared, closestHitLocation);

            const unsigned long long entityType = closestEntity->GetType();
            if ((entityType & EntityType::LIVING_ENTITY) == EntityType::LIVING_ENTITY ||
                entityType == EntityType::ITEM_FRAME)
            {
                minecraftPointedEntityShared = closestEntityShared;
            }
        }
    }
}

double GameRenderer::getFov(float amount, bool useSmoothing)
{
    static const float kZero = 0.0f;
    static const float kOne = 1.0f;
    static const float kPanoramicFov = 90.0f;
    static const float kNauseaMax = 20.0f;
    static const float kNauseaDenom = 500.0f;
    static const float kNauseaAdd = 500.0f;
    static const float kUnderwaterScale = 60.0f;
    static const float kUnderwaterDivisor = 70.0f;

    if (mPanoramicMode)
        return static_cast<double>(kPanoramicFov);

    boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = Minecraft::GetInstance()->mPlayer;
    MultiplayerLocalPlayer* player = localPlayer.get();
    const int playerIndex = player != NULL ? player->mUserIndex : 0;

    float fov = mFovSetting;
    if (useSmoothing)
    {
        const float optionFov = mMinecraft->mOptions != NULL ? mMinecraft->mOptions->mFov : kZero;
        fov = (fov + optionFov) * ((mFov[playerIndex] - mFovO[playerIndex]) * amount + mFovO[playerIndex]);
    }


    if (player != NULL)
    {
        if (player->getHealth() <= kZero)
        {
            float deathTime = static_cast<float>(player->mDeathTime) + amount;
            if (deathTime > kNauseaMax)
                deathTime = kNauseaMax;
            const float divisor = kOne - kNauseaDenom / (deathTime + kNauseaAdd);
            fov = fov / (divisor + divisor + kOne);
        }
    }

    boost::shared_ptr<Entity> cameraEntity = mMinecraft->getCameraEntity();
    const BlockState* cameraBlock =
        Camera::getBlockAt(mMinecraft->mLevel, cameraEntity, amount);
    if (cameraBlock != NULL && cameraBlock->getMaterial()->isUnderWater())
        fov = fov * kUnderwaterScale / kUnderwaterDivisor;

    return static_cast<double>(fov);
}

void GameRenderer::bobHurt(float partialTick)
{
    static const float kZero = 0.0f;
    static const float kOne = 1.0f;
    static const float kNauseaMax = 20.0f;
    static const float kNauseaBase = 40.0f;
    static const float kNauseaDiv = 8000.0f;
    static const float kNauseaAdd = 200.0f;
    static const float kHurtBobScale = 14.0f;
    static const float kPi = 3.1415927410125732f;

    boost::shared_ptr<Entity> cameraEntity = getCamera();
    if ((cameraEntity->GetType() & EntityType::LIVING_ENTITY) != EntityType::LIVING_ENTITY)
        return;

    boost::shared_ptr<LivingEntity> living =
        boost::static_pointer_cast<LivingEntity>(cameraEntity);
    LivingEntity* entity = living.get();

    const float prevHurt = static_cast<float>(entity->mHurtTime) - partialTick;
    if (entity->getHealth() <= kZero)
    {
        float deathTime = static_cast<float>(entity->mDeathTime) + partialTick;
        if (deathTime > kNauseaMax)
            deathTime = kNauseaMax;
        GlStateManager::rotatef(kNauseaBase - kNauseaDiv / (deathTime + kNauseaAdd), kZero, kZero,
                                kOne);
    }

    if (prevHurt >= kZero)
    {
        const float divisor = static_cast<float>(entity->mHurtDuration);
        float ratio = prevHurt / divisor;
        const float shake =
            static_cast<float>(Mth::sin(ratio * ratio * ratio * ratio * kPi));
        const float hurtDir = entity->mHurtDir;
        GlStateManager::rotatef(-hurtDir, kZero, kOne, kZero);
        GlStateManager::rotatef(-shake * kHurtBobScale, kZero, kZero, kOne);
        GlStateManager::rotatef(hurtDir, kZero, kOne, kZero);
    }
}

void GameRenderer::bobView(float partialTick)
{
    static const float kZero = 0.0f;
    static const float kOne = 1.0f;
    static const float kHalf = 0.5f;
    static const float kWalkScale = 3.0f;
    static const float kRollScale = 5.0f;
    static const float kPi = 3.1415927410125732f;
    static const float kRollOffset = 0.2f;

    boost::shared_ptr<Entity> cameraEntity = getCamera();
    if ((cameraEntity->GetType() & EntityType::PLAYER) != EntityType::PLAYER)
        return;

    boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(cameraEntity);
    Player* entity = player.get();

    const float walkDistO = entity->mWalkDistO;
    const float walkDist = entity->mWalkDist;
    const float walk = (walkDist - walkDistO) * partialTick + walkDist;
    const float bobDist = (entity->mCameraYaw - entity->mPrevCameraYaw) * partialTick + entity->mPrevCameraYaw;
    const float roll = (entity->mBob - entity->mOBob) * partialTick + entity->mOBob;
    const float angle = -walk * kPi;
    const float dist = bobDist;

    const float sinAngle = Mth::sin(angle);
    const float cosAngle = Mth::cos(angle);
    GlStateManager::translatef(sinAngle * dist * kHalf, -Mth::abs(cosAngle * dist), kZero);
    GlStateManager::rotatef(sinAngle * dist * kWalkScale, kZero, kZero, kOne);
    GlStateManager::rotatef(Mth::abs(Mth::cos(angle - kRollOffset) * dist) * kRollScale, kOne, kZero,
                            kZero);
    GlStateManager::rotatef(roll, kOne, kZero, kZero);
}

unsigned int GameRenderer::getCameraThirdPerson()
{
    boost::shared_ptr<MultiplayerLocalPlayer> localPlayer =
        boost::static_pointer_cast<MultiplayerLocalPlayer>(mMinecraft->mPlayer);
    return localPlayer->GetThirdPersonView(false);
}

double GameRenderer::getCameraYRot(float amount)
{
    boost::shared_ptr<Entity> camera = mMinecraft->getCameraEntity();
    Entity* entity = camera.get();
    return static_cast<double>((entity->mYRot - entity->mYRotO) * amount + entity->mYRotO);
}

double GameRenderer::getCameraXRot(float amount)
{
    boost::shared_ptr<Entity> camera = mMinecraft->getCameraEntity();
    Entity* entity = camera.get();
    return static_cast<double>((entity->mXRot - entity->mXRotO) * amount + entity->mXRotO);
}

void GameRenderer::moveCameraToPlayer(float partialTick)
{
    static const float kZero = 0.0f;
    static const float kOne = 1.0f;
    static const float kNegOne = -1.0f;
    static const float kDegToRad = 0.01745329238474369f;
    static const float k180 = 180.0f;
    static const float kFirstPersonNudge = 0.05f;
    static const float kSleepingYOffset = 0.30000001192092896f;
    static const float kSleepingHeightAdd = 1.0f;
    static const float kBedFacingRotation = 90.0f;
    static const float kCollisionProbe = 0.10000000149011612f;
    static const float kFallFlightYawScale = 0.5f;
    static const float kFallFlightAlignmentScale = 0.25f;

    boost::shared_ptr<Entity> cameraEntity = getCamera();
    boost::shared_ptr<LivingEntity> living =
        boost::static_pointer_cast<LivingEntity>(cameraEntity);
    LivingEntity* entity = living.get();
    boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = mMinecraft->mPlayer;

    const unsigned int thirdPerson = getCameraThirdPerson();
    double eyeHeight = static_cast<double>(
        (entity->mEyeHeight - entity->mEyeHeightO) * partialTick + entity->mEyeHeightO);
    double x = (entity->mX - entity->mXo) * partialTick + entity->mXo;
    double y = (entity->mY - entity->mYo) * partialTick + entity->mYo;
    double z = (entity->mZ - entity->mZo) * partialTick + entity->mZo;
    double yEye = y + entity->getEyeHeight();

    char sleepingFlag = 0;
    if (mMinecraft->mOptions != NULL)
        sleepingFlag = mMinecraft->mOptions->mUnknownFlag5;

    if (thirdPerson == 3)
    {
        CameraController* controller = localPlayer->GetCameraController();
        if (controller != NULL && controller->getCameraMode() == 0)
        {
            localPlayer->SetThirdPersonView(1);
            sleepingFlag = mMinecraft->mOptions != NULL
                ? mMinecraft->mOptions->mUnknownFlag5
                : 0;
        }
        else if (controller != NULL)
        {
            Vec3* pos = controller->getCurrentPos();
            Vec3* rot = controller->getCurrentRot();
            GlStateManager::translatef(static_cast<float>(pos->x),
                                     static_cast<float>(pos->y),
                                     static_cast<float>(pos->z));
            GlStateManager::rotatef(static_cast<float>(rot->x), kOne, kZero, kZero);
            GlStateManager::rotatef(static_cast<float>(rot->y), kZero, kOne, kZero);
            GlStateManager::rotatef(static_cast<float>(rot->z), kZero, kZero, kOne);
            sleepingFlag = mMinecraft->mOptions != NULL
                ? mMinecraft->mOptions->mUnknownFlag5
                : 0;
        }
    }
    else if (!entity->isSleeping())
    {
        if (thirdPerson == 0)
        {
            GlStateManager::translatef(kZero, kZero, kFirstPersonNudge);
        }
        else
        {
            float distance = (mThirdPersonDistance - mThirdPersonDistanceO) * partialTick + mThirdPersonDistanceO;

            if (sleepingFlag == 0)
            {
                const float yaw = static_cast<float>(getCameraYRot(partialTick));
                float pitch = static_cast<float>(getCameraXRot(partialTick));
                if (thirdPerson == 2)
                    pitch += k180;

                const float yawRad = yaw * kDegToRad;
                const float pitchRad = pitch * kDegToRad;
                const float sinYaw = Mth::sin(yawRad);
                const float cosYaw = Mth::cos(yawRad);
                const float cosPitch = Mth::cos(pitchRad);
                const float sinPitch = Mth::sin(pitchRad);

                double offsetX = -sinYaw * cosPitch * distance;
                double offsetZ = cosYaw * cosPitch * distance;
                double offsetY = -sinPitch * distance;

                Level* level = mMinecraft->mLevel;
                for (int corner = 0; corner < 8; ++corner)
                {
                    const float probeX = static_cast<float>(((corner & 2) != 0 ? 1 : -1)) * kCollisionProbe;
                    const float probeY = static_cast<float>(((corner >> 1 & 2) != 0 ? 1 : -1)) * kCollisionProbe;
                    const float probeZ = static_cast<float>(((corner & 1) != 0 ? 1 : -1)) * kCollisionProbe;

                    Vec3* from = Vec3::newTemp(x + probeX, yEye + probeY, z + probeZ);
                    Vec3* to = Vec3::newTemp(x - offsetX + probeX,
                                             yEye - offsetY + probeY,
                                             z - offsetZ + probeZ);

                    HitResult* hitForward = level->clip(from, to, false, true, false, true, NULL, true);
                    HitResult* hitReverse = level->clip(to, from, false, true, false, true, NULL, true);

                    if (hitForward == NULL)
                    {
                        if (hitReverse != NULL)
                            delete hitReverse;
                        continue;
                    }

                    if (hitReverse == NULL)
                    {
                        BlockPos blockPos = hitForward->getBlockPos();
                        const BlockState* state = level->getBlockState(blockPos);
                        if (state != NULL && state->getBlock() != NULL
                            && state->getBlock()->isSolidBlockingCube(state))
                        {
                            if (!Minecraft::InMiniGame(eMiniGameId_Any, true)
                                || g_pApp->ShouldCollideWith(state->getBlock()))
                            {
                                Vec3* origin = Vec3::newTemp(x, yEye, z);
                                const double hitDistance = hitForward->mLocation->distanceTo(origin);
                                if (hitDistance < distance)
                                    distance = static_cast<float>(hitDistance);
                            }
                        }
                        delete hitForward;
                    }
                    else
                    {
                        BlockPos forwardPos = hitForward->getBlockPos();
                        BlockPos reversePos = hitReverse->getBlockPos();
                        const BlockState* forwardState = level->getBlockState(forwardPos);
                        const BlockState* reverseState = level->getBlockState(reversePos);
                        bool forwardSolid = forwardState != NULL && forwardState->getBlock() != NULL
                            && forwardState->getBlock()->isSolidBlockingCube(forwardState)
                            && (!Minecraft::InMiniGame(eMiniGameId_Any, true)
                                || g_pApp->ShouldCollideWith(forwardState->getBlock()));
                        bool reverseSolid = reverseState != NULL && reverseState->getBlock() != NULL
                            && reverseState->getBlock()->isSolidBlockingCube(reverseState)
                            && (!Minecraft::InMiniGame(eMiniGameId_Any, true)
                                || g_pApp->ShouldCollideWith(reverseState->getBlock()));
                        if (forwardSolid || reverseSolid)
                        {
                            Vec3* origin = Vec3::newTemp(x, yEye, z);
                            const double hitDistance = hitForward->mLocation->distanceTo(origin);
                            if (hitDistance < distance)
                                distance = static_cast<float>(hitDistance);
                        }
                        delete hitForward;
                        delete hitReverse;
                    }
                }

                if (thirdPerson == 2)
                    GlStateManager::rotatef(k180, kZero, kOne, kZero);

                GlStateManager::rotatef(pitch - static_cast<float>(getCameraXRot(partialTick)),
                                      kOne, kZero, kZero);
                GlStateManager::rotatef(yaw - static_cast<float>(getCameraYRot(partialTick)),
                                      kZero, kOne, kZero);
                GlStateManager::translatef(kZero, kZero, -distance);
                GlStateManager::rotatef(static_cast<float>(getCameraYRot(partialTick)) - yaw,
                                      kZero, kOne, kZero);
                GlStateManager::rotatef(static_cast<float>(getCameraXRot(partialTick)) - pitch,
                                      kOne, kZero, kZero);
            }
            else
            {
                GlStateManager::translatef(kZero, kZero, -distance);
            }
        }
    }
    else
    {
        eyeHeight = static_cast<double>(static_cast<float>(eyeHeight + kSleepingHeightAdd));
        GlStateManager::translatef(kZero, kSleepingYOffset, kZero);
        if (sleepingFlag == 0)
        {
            const BlockPos cameraBlockPos(cameraEntity);
            const BlockState* bedState = mMinecraft->mLevel->getBlockState(cameraBlockPos);
            if (bedState != NULL && bedState->getBlock() == Blocks::bed)
            {
                const Direction* facing =
                    bedState->getValue<const Direction*>(HorizontalDirectionalBlock::FACING_PROPERTY);
                const float bedFacingAngle =
                    static_cast<float>(facing->get2DDataValue()) * kBedFacingRotation;
                GlStateManager::rotatef(bedFacingAngle, kZero, kOne, kZero);
            }
            const float yaw = static_cast<float>(getCameraYRot(partialTick) + k180);
            GlStateManager::rotatef(yaw, kZero, kNegOne, kZero);
            GlStateManager::rotatef(static_cast<float>(getCameraXRot(partialTick)),
                                  kNegOne, kZero, kZero);
        }
    }

    if (entity->isFallFlying())
    {
        if (GameSettings::GetGameSettings(localPlayer->mUserIndex,
                                          eGameSetting_FlyingViewRolling) == 0)
        {
            const float fallFlyYaw =
                (localPlayer->mFallFlyYaw - localPlayer->mFallFlyYawOld) * partialTick
                + localPlayer->mFallFlyYawOld;
            const float fallFlyingAlignment =
                (localPlayer->mFallFlyingAlignment - localPlayer->mFallFlyingAlignmentO)
                    * partialTick
                + localPlayer->mFallFlyingAlignmentO;
            const float fallFlightRotation = fallFlyYaw * fallFlyingAlignment;
            GlStateManager::rotatef(fallFlightRotation * kFallFlightYawScale,
                                    kZero, kZero, kOne);
            GlStateManager::rotatef(fallFlightRotation * kFallFlightAlignmentScale,
                                    kZero, kOne, kZero);
        }
    }

    if (sleepingFlag == 0)
    {
        GlStateManager::rotatef(static_cast<float>(getCameraXRot(partialTick)), kOne, kZero, kZero);
        Animal* animal = dynamic_cast<Animal*>(entity);
        if (animal != NULL)
        {
            const float headYaw = animal->mYHeadRotO + (animal->mYHeadRot - animal->mYHeadRotO) * partialTick + k180;
            GlStateManager::rotatef(headYaw, kZero, kOne, kZero);
        }
        else
        {
            const float yaw = static_cast<float>(getCameraYRot(partialTick) + k180);
            GlStateManager::rotatef(yaw, kZero, kOne, kZero);
        }
    }

    GlStateManager::translatef(kZero, -static_cast<float>(eyeHeight), kZero);

    const bool inCloud = mMinecraft->mLevelRenderer->isInCloud(
        x, y + eyeHeight, z, partialTick);
    mCloudFog = inCloud;
}

void GameRenderer::getFovAndAspect(float& fov, float& aspect, float partialTick, bool useSmoothing)
{
    static const float kSplitFovScale = 0.699999988079071f;
    static const float kQuadAspectScale = 0.5f;
    static const float kQuadFovScale = 1.100000023841858f;

    aspect = static_cast<float>(mMinecraft->mWidth)
             / static_cast<float>(mMinecraft->mHeight);
    fov = static_cast<float>(getFov(partialTick, useSmoothing));

    boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = mMinecraft->mPlayer;
    const int splitMode = localPlayer->mViewport;
    if (splitMode == 1 || splitMode == 2)
    {
        aspect = aspect + aspect;
        fov = fov * kSplitFovScale;
        return;
    }
    if (splitMode == 3 || splitMode == 4)
    {
        aspect = aspect * kQuadAspectScale;
        fov = fov * kQuadFovScale;
    }
}

void GameRenderer::setupCamera(float partialTick, int eyeIndex)
{
    static const float kZero = 0.0f;
    static const float kHalf = 0.5f;
    static const float kOne = 1.0f;
    static const float kDefaultFog = 256.0f;
    static const float kProjectionEyeOffset = 0.07000000029802322f;
    static const float kModelEyeOffset = 0.10000000149011612f;
    static const float kNearPlane = 0.05000000074505806f;
    static const float kSpectatorZoomScale = 0.03999999910593033f;
    static const float kSpectatorZoomDenom = 5.0f;
    static const float kRotate90 = 90.0f;
    static const float kRotate180 = 180.0f;

    mFogDistance = kDefaultFog;

    Level* level = mMinecraft->mLevel;
    if (level != NULL && level->getFogDistance() > 0)
        mFogDistance = static_cast<float>(level->getFogDistance());

    GlStateManager::matrixMode(1);
    GlStateManager::loadIdentity();
    if (mMinecraft->mOptions != NULL
        && mMinecraft->mOptions->mAnaglyph3d)
    {
        const float eyeOffset = static_cast<float>((eyeIndex * 2 - 1) * -1);
        GlStateManager::translatef(eyeOffset * kProjectionEyeOffset, kZero, kZero);
    }

    float fov = partialTick;
    float aspect = partialTick;
    getFovAndAspect(fov, aspect, partialTick, true);

    if (mZoom != kOne)
    {
        GlStateManager::translatef(mZoomX, -mZoomY, kZero);
        GlStateManager::scaled(mZoom, mZoom, kOne);
    }

    const float farPlane = mFogDistance + mFogDistance;
    GlStateManager::perspective(fov, aspect, kNearPlane, farPlane);

    GlStateManager::matrixMode(0);
    GlStateManager::loadIdentity();
    if (mMinecraft->mOptions != NULL
        && mMinecraft->mOptions->mAnaglyph3d)
    {
        const float eyeOffset = static_cast<float>(eyeIndex * 2 - 1);
        GlStateManager::translatef(eyeOffset * kModelEyeOffset, kZero, kZero);
    }

    bobHurt(partialTick);

    boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = mMinecraft->mPlayer;
    _SkinAdjustments skinAdjustments = localPlayer->getSkinAdjustments();
    const unsigned int skinFlags = skinAdjustments.mGameFlags;
    const unsigned int skinMask = skinFlags & 0x204u;
    const unsigned int skinAlt = (skinMask - 1u) + (skinMask == 0u ? 1u : 0u);

    if (GameSettings::GetGameSettings(localPlayer->mUserIndex,
                                      eGameSetting_ViewBob) != 0)
    {
        if (!localPlayer->mAbilities.mFlying && skinMask == skinAlt)
        {
            if (!localPlayer->isUsingItem())
                bobView(partialTick);
        }
    }

    const float portalIntensity = localPlayer->mPortalEffectIntensity;
    const float previousPortalIntensity = localPlayer->mPortalEffectIntensityO;
    const float interpolatedPortalIntensity =
        (portalIntensity - previousPortalIntensity) * partialTick + previousPortalIntensity;
    if (interpolatedPortalIntensity > kZero)
    {
        float rotateAmount = 20.0f;
        if (localPlayer->hasEffect(MobEffects::CONFUSION))
            rotateAmount = 7.0f;
        const float zoom =
            -(interpolatedPortalIntensity * kSpectatorZoomScale
              - (kSpectatorZoomDenom
                 / (interpolatedPortalIntensity * interpolatedPortalIntensity
                    + kSpectatorZoomDenom)));
        const float frame = static_cast<float>(mTickCount) + partialTick;
        GlStateManager::rotatef(frame * rotateAmount, kZero, kOne, kOne);
        const float scale = kOne / (zoom * zoom);
        GlStateManager::scalef(scale, scale, scale);
        GlStateManager::rotatef(-frame * rotateAmount, kZero, kOne, kOne);
    }

    moveCameraToPlayer(partialTick);

    if (!mPanoramicMode)
        return;

    const unsigned int thirdPerson = getCameraThirdPerson();
    if (thirdPerson == 0)
        GlStateManager::rotatef(kRotate90, kZero, kZero, kOne);
    else if (thirdPerson == 1)
        GlStateManager::rotatef(kRotate180, kZero, kZero, kOne);
    else if (thirdPerson == 2)
        GlStateManager::rotatef(kRotate90, kOne, kZero, kZero);
    else if (thirdPerson == 3)
        GlStateManager::rotatef(kRotate90, kZero, kOne, kZero);
    else if (thirdPerson == 4)
        GlStateManager::rotatef(kRotate90, kOne, kZero, kZero);
}

void GameRenderer::turnOnLightLayer(double)
{
    boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = mMinecraft->mPlayer;
    MultiplayerLocalPlayer* player = localPlayer.get();
    if (player == NULL)
        return;

    const unsigned int texture = getLightTexture(
        player->mUserIndex, mMinecraft->mLevel);
    gRenderer->TextureBindVertex(static_cast<int>(texture));
    GlStateManager::texParameteri(1, 1, 1);
    GlStateManager::texParameteri(1, 2, 1);
    GlStateManager::texParameteri(1, 3, 0);
    GlStateManager::texParameteri(1, 4, 0);
}

void GameRenderer::turnOffLightLayer(double)
{
    gRenderer->TextureBindVertex(-1);
}

void GameRenderer::renderItemInHand(float partialTick, int eyeIndex)
{
    static const float kZero = 0.0f;
    static const float kHalf = 0.5f;
    static const float kOne = 1.0f;
    static const float kProjectionEyeOffset = 0.07000000029802322f;
    static const float kModelEyeOffset = 0.10000000149011612f;
    static const float kNearPlane = 0.05000000074505806f;

    if (mPanoramicMode)
    {
        return;
    }

    mRenderHand = true;

    boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = Minecraft::GetInstance()->mPlayer;
    MultiplayerLocalPlayer* player = localPlayer.get();
    boost::shared_ptr<Entity> camera = getCamera();

    bool holdingMap = false;
    if (camera->GetType() == EntityType::LOCAL_PLAYER)
    {
        if (localPlayer->mInventory != NULL)
        {
            not_null_ptr<ItemInstance> selected = localPlayer->mInventory->getSelected();
            ItemInstance* item = selected.get();
            if (item != NULL && item->getItem() == Items::filledMap)
                holdingMap = true;
        }

        if (GameSettings::GetGameSettings(localPlayer->mUserIndex,
                                          eGameSetting_DisplayHand) == 0)
            mRenderHand = false;
    }

    GlStateManager::matrixMode(1);
    GlStateManager::loadIdentity();
    if (mMinecraft->mOptions != NULL
        && mMinecraft->mOptions->mAnaglyph3d)
    {
        const float eyeOffset = static_cast<float>((eyeIndex * 2 - 1) * -1);
        GlStateManager::translatef(eyeOffset * kProjectionEyeOffset, kZero, kZero);
    }

    float fov = partialTick;
    float aspect = partialTick;
    getFovAndAspect(fov, aspect, partialTick, false);

    if (mZoom != kOne)
    {
        GlStateManager::translatef(mZoomX, -mZoomY, kZero);
        GlStateManager::scaled(mZoom, mZoom, kOne);
    }

    const float farPlane = mFogDistance + mFogDistance;
    GlStateManager::perspective(fov, aspect, kNearPlane, farPlane);

    GlStateManager::matrixMode(0);
    GlStateManager::loadIdentity();
    if (mMinecraft->mOptions != NULL
        && mMinecraft->mOptions->mAnaglyph3d)
    {
        const float eyeOffset = static_cast<float>(eyeIndex * 2 - 1);
        GlStateManager::translatef(eyeOffset * kModelEyeOffset, kZero, kZero);
    }

    GlStateManager::pushMatrix();
    bobHurt(partialTick);

    _SkinAdjustments skinAdjustments = localPlayer->getSkinAdjustments();
    const unsigned int skinFlags = skinAdjustments.mGameFlags;
    const unsigned int skinMask = skinFlags & 0x204u;
    const unsigned int skinAlt = (skinMask - 1u) + (skinMask == 0u ? 1u : 0u);

    if (GameSettings::GetGameSettings(localPlayer->mUserIndex,
                                      eGameSetting_ViewBob) != 0)
    {
        if (!localPlayer->mAbilities.mFlying && skinMask == skinAlt)
        {
            if (!localPlayer->isUsingItem())
                bobView(partialTick);
        }
    }

    bool spectatorHandEffect = false;
    if ((camera->GetType() & EntityType::LIVING_ENTITY) == EntityType::LIVING_ENTITY)
    {
        boost::shared_ptr<LivingEntity> living =
            boost::static_pointer_cast<LivingEntity>(getCamera());
        AbstractClientPlayer* clientPlayer = dynamic_cast<AbstractClientPlayer*>(living.get());
        if (clientPlayer != NULL && clientPlayer->isSpectator())
            spectatorHandEffect = true;
    }

    const unsigned int thirdPerson = getCameraThirdPerson();
    const bool skipFirstPersonHand =
        (!mRenderHand && !holdingMap) || thirdPerson != 0 || spectatorHandEffect;

    ItemInHandRenderer* itemRenderer =
        mItemInHandRenderer;

    if (skipFirstPersonHand)
    {
        GlStateManager::popMatrix();
        if (thirdPerson != 0 || spectatorHandEffect)
        {
            if (itemRenderer != NULL)
                itemRenderer->renderScreenEffect(partialTick);
            bobHurt(partialTick);
        }
    }
    else
    {
        if (player != NULL && !localPlayer->isPassenger()
            && (mMinecraft->mOptions == NULL
                || !mMinecraft->mOptions->mHideGui)
            && mMinecraft->mGameMode != NULL
            && !mMinecraft->mGameMode->isCutScene())
        {
            turnOnLightLayer(static_cast<double>(partialTick));
            if (itemRenderer != NULL)
                itemRenderer->render(partialTick);
            turnOffLightLayer(static_cast<double>(partialTick));
        }
        GlStateManager::popMatrix();
    }

    if (GameSettings::GetGameSettings(localPlayer->mUserIndex,
                                      eGameSetting_ViewBob) != 0)
    {
        if (!localPlayer->mAbilities.mFlying && skinMask == skinAlt)
        {
            if (!localPlayer->isUsingItem())
                bobView(partialTick);
        }
    }
}

double GameRenderer::getNightVisionScale(boost::shared_ptr<LivingEntity> living, float partialTick)
{
    MobEffectInstance* nightVision = living->getEffect(MobEffects::NIGHT_VISION);
    MobEffectInstance* conduitPower = living->getEffect(MobEffects::CONDUIT_POWER);
    if (nightVision == NULL && conduitPower == NULL)
    {
        return 0.0f;
    }

    int duration = 0;
    if (nightVision != NULL)
    {
        duration = nightVision->getDuration();
    }
    if (conduitPower != NULL && duration < conduitPower->getDuration())
    {
        duration = conduitPower->getDuration();
    }
    if (duration > 200)
    {
        return 1.0f;
    }

    float remaining = static_cast<float>(duration) - partialTick;
    if (remaining < 0.0f)
    {
        remaining = 0.0f;
    }
    return Mth::sin(remaining * 3.1415927f * 0.05f) * 0.3f + 0.7f;
}

double GameRenderer::getUnderWaterVisionScale(const boost::shared_ptr<Player>& player,
                                             float partialTick)
{
    static const float kMinSkyDarken = 0.13333334028720856f;
    static const float kMinBrightness = 2.0f;
    static const float kLightWeight = 0.8999999761581421f;
    static const float kBrightnessWeight = 0.10000000149011612f;
    static const float kBrightnessDivisor = 15.0f;
    static const float kPowBase = 0.4000000059604645f;
    static const float kPowExp = 0.20000000298023224f;
    static const float kOne = 1.0f;

    const float blend = powf(kPowBase, kPowExp);
    Player* entity = player.get();

    const double eyeHeight = entity->getEyeHeight();
    BlockPos eyePos(entity->mX, entity->mY + eyeHeight, entity->mZ);
    const float skyDarken = entity->mLevel->getSkyDarken(partialTick);
    float brightness = static_cast<float>(
        entity->mLevel->getBrightnessPropagate(LightLayer::SKY, eyePos));
    float underwaterLight = static_cast<float>(entity->getUnderwaterLightLevel());

    float clampedSky = skyDarken;
    if (clampedSky < kMinSkyDarken)
        clampedSky = kMinSkyDarken;
    if (brightness < kMinBrightness)
        brightness = kMinBrightness;
    if (clampedSky < kOne)
        brightness *= clampedSky;

    const float lightLevel =
        kLightWeight * underwaterLight + kBrightnessWeight * brightness;
    const float normalized = lightLevel / kBrightnessDivisor;
    const float currentScale = static_cast<float>(entity->getUnderwaterVisionScale());
    const float result = (normalized - currentScale) * (kOne - blend) + currentScale;

    entity->setUnderwaterVisionScale(result);
    entity->setUnderwaterLightLevel(lightLevel);
    return static_cast<double>(result);
}

void GameRenderer::updateLightTexture(float partialTick)
{
    static const float kZero = 0.0f;
    static const float kOne = 1.0f;
    static const float kLightmapGamma = 0.0f;
    static const float kGreyBoost = 0.699999988079071f;
    static const float kLightBase = 0.029999999329447746f;
    static const float kLightScale = 0.9599999785423279f;
    static const float k255 = 255.0f;
    static const float kSkyDarkenScale = 0.949999988079071f;
    static const float kSkyDarkenAdd = 0.05000000074505806f;
    static const float kSecondarySkyScale = 0.6499999761581421f;
    static const float kSecondarySkyAdd = 0.3499999940395355f;
    static const float kGammaSlope = 0.10000000149011612f;
    static const float kGammaBase = 1.5f;
    static const float kWaterMixA = 0.4000000059604645f;
    static const float kWaterMixC = 0.6000000238418579f;
    static const float kNetherRed = 0.75f;
    static const float kNetherGreen = 0.25f;
    static const float kNetherBlueScale = 0.2800000011920929f;
    static const float kNetherBlueAdd = 0.2199999988079071f;

    if (!mLightTextureDirty || mMinecraft->mLevel == NULL)
        return;


    for (int playerIndex = 0; playerIndex < 4; ++playerIndex)
    {
        boost::shared_ptr<MultiplayerLocalPlayer> player =
            Minecraft::GetInstance()->mLocalPlayers[playerIndex];
        MultiplayerLocalPlayer* localPlayer = player.get();
        if (localPlayer == NULL)
            continue;

        Level* level = localPlayer->mLevel;
        if (level == NULL)
            continue;

        int* pixelBuffer = mLightPixels[playerIndex].data;

        Dimension* dimension = level->mDimension;
        float* brightnessRamp = dimension->getBrightnessRamp();
        const float skyDarken = level->getSkyDarken(kOne);
        const unsigned int blockLightColor = dimension->getBlockLightColour();

        const float lightRed = static_cast<float>((blockLightColor >> 16) & 0xff) / k255;
        const float lightGreen = static_cast<float>((blockLightColor >> 8) & 0xff) / k255;
        const float lightBlue = static_cast<float>(blockLightColor & 0xff) / k255;

        const bool isNether = dimension->getType() == DimensionType::NETHER;
        boost::shared_ptr<LivingEntity> living =
            boost::static_pointer_cast<LivingEntity>(player);

        for (int lightIndex = 0; lightIndex < 0x100; ++lightIndex)
        {
            const unsigned int blockLight =
                static_cast<unsigned int>(lightIndex >> 4)
                + ((lightIndex < 0 && (lightIndex & 0xf) != 0) ? 1u : 0u);
            const unsigned int skyLight = lightIndex & 0xfu;

            float blockBrightness =
                brightnessRamp[blockLight]
                * (skyDarken * kSkyDarkenScale + kSkyDarkenAdd);
            if (level->getSkyFlashTime() > 0)
                blockBrightness = brightnessRamp[blockLight];

            const float skyBrightness =
                brightnessRamp[skyLight] * (mBlockLightRedFlicker * kGammaSlope + kGammaBase);

            float redComponent = blockBrightness;
            float greenBase =
                blockBrightness * (skyDarken * kSecondarySkyScale + kSecondarySkyAdd);
            const float redLight = skyBrightness * lightRed;
            const float blueLight =
                skyBrightness
                * (static_cast<float>(skyBrightness * skyBrightness) * kWaterMixC + kWaterMixA)
                * lightBlue;
            const float greenLight =
                skyBrightness
                * (static_cast<float>(skyBrightness * kWaterMixC + kWaterMixA) * kWaterMixC
                   + kWaterMixA)
                * lightGreen;

            double red = static_cast<double>((greenBase + redLight) * kLightScale + kLightBase);
            double green =
                static_cast<double>((greenBase + greenLight) * kLightScale + kLightBase);
            double blue =
                static_cast<double>((redComponent + blueLight) * kLightScale + kLightBase);

            if (mDarkenWorldAmount > kZero)
            {
                const float flashAmount =
                    (mDarkenWorldAmount - mDarkenWorldAmountO) * partialTick + mDarkenWorldAmountO;
                const float keep = kOne - flashAmount;
                red = red * keep + red * kGreyBoost * flashAmount;
                green = green * keep + green * kWaterMixC * flashAmount;
                blue = blue * keep + blue * kWaterMixC * flashAmount;
            }

            if (isNether)
            {
                red = static_cast<double>(redLight * kNetherRed + kNetherBlueAdd);
                green = static_cast<double>(greenLight * kNetherRed + kNetherBlueScale);
                blue = static_cast<double>(blueLight * kNetherRed + kNetherGreen);
            }

            if (living->hasEffect(MobEffects::NIGHT_VISION)
                || living->hasEffect(MobEffects::CONDUIT_POWER))
            {
                const double nightVision =
                    getNightVisionScale(living, partialTick);
                double minScale = kOne / red;
                if (kOne / green < minScale)
                    minScale = kOne / green;
                if (kOne / blue < minScale)
                    minScale = kOne / blue;
                const double keep = kOne - nightVision;
                red = red * keep + red * minScale * nightVision;
                green = green * keep + green * minScale * nightVision;
                blue = blue * keep + blue * minScale * nightVision;
            }

            if (living->isUnderLiquid(Material::WATER))
            {
                boost::shared_ptr<Player> visionPlayer =
                    boost::static_pointer_cast<Player>(player);
                const double underwaterScale =
                    getUnderWaterVisionScale(visionPlayer, partialTick);
                double minScale = kOne / red;
                if (kOne / green < minScale)
                    minScale = kOne / green;
                if (kOne / blue < minScale)
                    minScale = kOne / blue;
                const double keep = kOne - underwaterScale;
                red = red * keep + red * minScale * underwaterScale;
                green = green * keep + green * minScale * underwaterScale;
                blue = blue * keep + blue * minScale * underwaterScale;
            }

            if (red > kOne)
                red = kOne;
            if (green > kOne)
                green = kOne;
            if (blue > kOne)
                blue = kOne;

            const double redCurve = kOne - red;
            const double greenCurve = kOne - green;
            const double blueCurve = kOne - blue;
            red = (-(redCurve * redCurve * redCurve * redCurve - kOne) * kLightmapGamma + red) * kLightScale + kLightBase;
            green =
                (-(greenCurve * greenCurve * greenCurve * greenCurve - kOne) * kLightmapGamma + green) * kLightScale
                + kLightBase;
            blue =
                (-(blueCurve * blueCurve * blueCurve * blueCurve - kOne) * kLightmapGamma + blue) * kLightScale
                + kLightBase;

            if (red > kOne)
                red = kOne;
            if (green > kOne)
                green = kOne;
            if (blue > kOne)
                blue = kOne;
            if (red < kZero)
                red = kZero;
            if (green < kZero)
                green = kZero;
            if (blue < kZero)
                blue = kZero;

            pixelBuffer[lightIndex] =
                static_cast<int>(red * k255) << 24
                | static_cast<int>(green * k255) << 16
                | static_cast<int>(blue * k255) << 8
                | 0xff;
        }

        const unsigned int texture = getLightTexture(playerIndex, level);
        mMinecraft->mTextures->replaceTextureDirect(mLightPixels[playerIndex], 16, 16, texture);
    }
}

unsigned int GameRenderer::fjGetFpsCap(int value)
{
    return value < 0 ? 0 : static_cast<unsigned int>(value);
}

const BlockPos* GameRenderer::getFogOffsets()
{
    Level* level = mMinecraft->mLevel;
    const unsigned char biomeScale = level->getLevelData()->getBiomeScale();
    if (biomeScale == 1)
        return sEndFogOffsets;
    if (biomeScale == 2)
        return sNetherFogOffsets;
    return sOverworldFogOffsets;
}

double GameRenderer::getTargetFogDistance(const BlockPos& pos)
{
    static const float kUnitWeight = 1.0f;

    const BlockPos* table = getFogOffsets();
    Level* level = mMinecraft->mLevel;

    float weightSum = 0.0f;
    float distanceSum = 0.0f;

    for (int group = 0; group < 6; ++group)
    {
        for (int sample = 0; sample < 4; ++sample)
        {
            const BlockPos& delta = table[group * 4 + sample];
            BlockPos samplePos = pos.offset(delta.x, delta.y, delta.z);
            Biome* biome = level->getBiome(samplePos);
            distanceSum += biome->getFogDistance();
        }
        weightSum += kUnitWeight + kUnitWeight + kUnitWeight + kUnitWeight;
    }

    for (int i = 0; i < 3; ++i)
    {
        const BlockPos& delta = table[24 + i];
        BlockPos samplePos = pos.offset(delta.x, delta.y, delta.z);
        Biome* biome = level->getBiome(samplePos);
        distanceSum += biome->getFogDistance();
        weightSum += kUnitWeight;
    }

    return static_cast<double>(distanceSum / weightSum);
}

void GameRenderer::getTargetFogColor(const BlockPos& pos, float& red, float& green, float& blue)
{
    static const float kByteScale = 0.00390625f;
    const BlockPos* table = getFogOffsets();
    Level* level = mMinecraft->mLevel;

    float redSum = 0.0f;
    float greenSum = 0.0f;
    float blueSum = 0.0f;
    float weight = 0.0f;

    for (int pair = 0; pair < 13; ++pair)
    {
        const BlockPos& firstDelta = table[pair * 2];
        Biome* firstBiome = level->getBiome(pos.offset(firstDelta.x, firstDelta.y, firstDelta.z));
        const unsigned int firstColour = firstBiome->getUnderWaterFogColor();

        const BlockPos& secondDelta = table[pair * 2 + 1];
        Biome* secondBiome = level->getBiome(pos.offset(secondDelta.x, secondDelta.y, secondDelta.z));
        const unsigned int secondColour = secondBiome->getUnderWaterFogColor();

        redSum = static_cast<float>((secondColour >> 16) & 255u) * kByteScale
            + static_cast<float>((firstColour >> 16) & 255u) * kByteScale + redSum;
        greenSum = static_cast<float>((secondColour >> 8) & 255u) * kByteScale
            + static_cast<float>((firstColour >> 8) & 255u) * kByteScale + greenSum;
        blueSum = static_cast<float>(secondColour & 255u) * kByteScale
            + static_cast<float>(firstColour & 255u) * kByteScale + blueSum;
        weight += 1.0f + 1.0f;
    }

    const BlockPos& finalDelta = table[26];
    Biome* finalBiome = level->getBiome(pos.offset(finalDelta.x, finalDelta.y, finalDelta.z));
    const unsigned int finalColour = finalBiome->getUnderWaterFogColor();
    redSum = static_cast<float>((finalColour >> 16) & 255u) * kByteScale + redSum;
    greenSum = static_cast<float>((finalColour >> 8) & 255u) * kByteScale + greenSum;
    blueSum = static_cast<float>(finalColour & 255u) * kByteScale + blueSum;
    weight += 1.0f;

    red = redSum / weight;
    green = greenSum / weight;
    blue = blueSum / weight;
}

double GameRenderer::modifyFogDistance(const BlockPos& pos, float distance, float value)
{
    static const float kZero = 0.0f;
    static const float kFarDistance = 40.0f;
    static const float kNearFactor = 1.0f;
    static const float kFarFactor = 0.3333333432674408f;

    const float deltaY = distance - static_cast<float>(pos.y);
    if (kZero <= deltaY)
    {
        if (deltaY < kFarDistance)
            value = value * Mth::Lerp(kNearFactor, kFarFactor, deltaY / kFarDistance);
        else
            value = value * kFarFactor;
    }
    return static_cast<double>(value);
}

void GameRenderer::setupClearColor(float partialTick)
{
    Level* level = mMinecraft->mLevel;
    boost::shared_ptr<Entity> camera = getCamera();
    boost::shared_ptr<LivingEntity> living;
    if ((camera->GetType() & EntityType::LIVING_ENTITY) == EntityType::LIVING_ENTITY)
        living = boost::static_pointer_cast<LivingEntity>(camera);

    const float skyBlend = 1.0f - static_cast<float>(std::pow(0.25, 0.25));
    Vec3* skyColor = level->getSkyColor(getCamera(), partialTick);
    const float skyRed = static_cast<float>(skyColor->x);
    const float skyGreen = static_cast<float>(skyColor->y);
    const float skyBlue = static_cast<float>(skyColor->z);

    Vec3* fogColor = level->getFogColor(getCamera(), partialTick);
    mFogRed = static_cast<float>(fogColor->x);
    mFogGreen = static_cast<float>(fogColor->y);
    mFogBlue = static_cast<float>(fogColor->z);

    Vec3* sunDirection = Mth::sin(level->getSunAngle(partialTick)) > 0.0f ? Vec3::newTemp(-1.0, 0.0, 0.0)
                                                                          : Vec3::newTemp(1.0, 0.0, 0.0);
    const float sunDot = static_cast<float>(camera->getViewVector(partialTick)->dot(sunDirection));
    if (sunDot > 0.0f)
    {
        float* sunriseColor = level->mDimension->getSunriseColor(level->getTimeOfDay(partialTick), partialTick);
        if (sunriseColor != NULL)
        {
            const float sunriseBlend = sunDot * sunriseColor[3];
            mFogRed = mFogRed * (1.0f - sunriseBlend) + sunriseColor[0] * sunriseBlend;
            mFogGreen = mFogGreen * (1.0f - sunriseBlend) + sunriseColor[1] * sunriseBlend;
            mFogBlue = mFogBlue * (1.0f - sunriseBlend) + sunriseColor[2] * sunriseBlend;
        }
    }

    mFogRed += (skyRed - mFogRed) * skyBlend;
    mFogGreen += (skyGreen - mFogGreen) * skyBlend;
    mFogBlue += (skyBlue - mFogBlue) * skyBlend;

    const float rainLevel = level->getRainLevel(partialTick);
    if (rainLevel > 0.0f)
    {
        const float rainDarken = 1.0f - rainLevel * 0.5f;
        mFogRed *= rainDarken;
        mFogGreen *= rainDarken;
        mFogBlue *= 1.0f - rainLevel * 0.4f;
    }

    const float thunderLevel = level->getThunderLevel(partialTick);
    if (thunderLevel > 0.0f)
    {
        const float thunderDarken = 1.0f - thunderLevel * 0.5f;
        mFogRed *= thunderDarken;
        mFogGreen *= thunderDarken;
        mFogBlue *= thunderDarken;
    }

    if (mCloudFog)
    {
        Vec3* cloudColor = level->getCloudColor(partialTick);
        mFogRed = static_cast<float>(cloudColor->x);
        mFogGreen = static_cast<float>(cloudColor->y);
        mFogBlue = static_cast<float>(cloudColor->z);
    }

    const BlockState* cameraBlock = Camera::getBlockAt(mMinecraft->mLevel, camera, partialTick);
    if (!cameraBlock->getMaterial()->isUnderWater())
    {
        mFogHeightLocked = false;
    }
    else
    {
        boost::shared_ptr<MultiplayerLocalPlayer> player = Minecraft::GetInstance()->mPlayer;
        const int playerIndex = player != NULL ? player->mUserIndex : 0;
        const long long now = System::processTimeInMilliSecs();
        BlockPos cameraPos(getCamera());

        float targetRed = 0.0f;
        float targetGreen = 0.0f;
        float targetBlue = 0.0f;
        getTargetFogColor(cameraPos, targetRed, targetGreen, targetBlue);
        if (targetRed != mTargetFogRed[playerIndex] || targetGreen != mTargetFogGreen[playerIndex]
            || targetBlue != mTargetFogBlue[playerIndex])
        {
            mTargetFogRed[playerIndex] = targetRed;
            mTargetFogGreen[playerIndex] = targetGreen;
            mTargetFogBlue[playerIndex] = targetBlue;
            mFogColourChangeTime[playerIndex] = now;
        }

        if (!mFogHeightLocked)
        {
            const double cameraY = getCamera()->mY;
            if (cameraY <= level->getSeaLevel())
                mFogHeight = level->getSeaLevel();
            else
                mFogHeight = static_cast<float>(cameraY);
        }

        const float fogDistance = static_cast<float>(modifyFogDistance(
            cameraPos, static_cast<float>(mFogHeight), static_cast<float>(getTargetFogDistance(cameraPos))));
        if (!mFogHeightLocked)
        {
            mPlayerFogRed[playerIndex] = mTargetFogRed[playerIndex];
            mPlayerFogGreen[playerIndex] = mTargetFogGreen[playerIndex];
            mPlayerFogBlue[playerIndex] = mTargetFogBlue[playerIndex];
            mPlayerFogDistance[playerIndex] = fogDistance;
        }

        const float colourProgress =
            Mth::clamp(static_cast<float>(now - mFogColourChangeTime[playerIndex]) / 4000.0f, 0.0f, 1.0f);
        mPlayerFogRed[playerIndex] = Mth::Lerp(mPlayerFogRed[playerIndex], mTargetFogRed[playerIndex], colourProgress);
        mPlayerFogGreen[playerIndex] =
            Mth::Lerp(mPlayerFogGreen[playerIndex], mTargetFogGreen[playerIndex], colourProgress);
        mPlayerFogBlue[playerIndex] = Mth::Lerp(mPlayerFogBlue[playerIndex], mTargetFogBlue[playerIndex], colourProgress);
        mFogRed = mPlayerFogRed[playerIndex];
        mFogGreen = mPlayerFogGreen[playerIndex];
        mFogBlue = mPlayerFogBlue[playerIndex];

        const float targetDistance = static_cast<float>(modifyFogDistance(
            cameraPos, static_cast<float>(mFogHeight), static_cast<float>(getTargetFogDistance(cameraPos))));
        if (targetDistance == mTargetFogDistance[playerIndex])
        {
            const float distanceProgress =
                Mth::clamp(static_cast<float>(now - mFogDistanceChangeTime[playerIndex]) / 4000.0f, 0.0f, 1.0f);
            mPlayerFogDistance[playerIndex] =
                Mth::Lerp(mPlayerFogDistance[playerIndex], mTargetFogDistance[playerIndex], distanceProgress);
        }
        else
        {
            mTargetFogDistance[playerIndex] = targetDistance;
            mFogDistanceChangeTime[playerIndex] = now;
            mPlayerFogDistance[playerIndex] =
                Mth::Lerp(mPlayerFogDistance[playerIndex], mTargetFogDistance[playerIndex], 0.0f);
        }

        float fogStart = 0.0f;
        float fogEnd = mPlayerFogDistance[playerIndex];
        if (getCamera()->GetType() == EntityType::LOCAL_PLAYER)
        {
            const double clarity =
                boost::static_pointer_cast<LocalPlayer>(getCamera())->getUnderwaterVisionClarity();
            if (clarity < 1.0)
            {
                const float clarityScale = static_cast<float>(0.6f * clarity + 0.4f);
                fogEnd *= clarityScale;
                fogStart *= clarityScale;
            }
        }

        GlStateManager::fogStart(fogStart + 4.0f);
        GlStateManager::fogEnd(fogEnd + 4.0f + 8.0f);
        GlStateManager::fogDensity(0.05f);
        mFogHeightLocked = true;
    }

    const float fogBrightness = (mFogBrightness - mFogBrightnessO) * partialTick + mFogBrightnessO;
    if (cameraBlock->getMaterial() == Material::LAVA)
    {
        const unsigned int lavaColour = Minecraft::GetInstance()->getColourTable()->getColour(
            eMinecraftColour_Under_Lava_Clear_Colour);
        mFogRed = static_cast<float>((lavaColour >> 16) & 0xff) / 256.0f * fogBrightness;
        mFogGreen = static_cast<float>((lavaColour >> 8) & 0xff) / 256.0f * fogBrightness;
        mFogBlue = static_cast<float>(lavaColour & 0xff) / 256.0f * fogBrightness;
    }
    else
    {
        mFogRed *= fogBrightness;
        mFogGreen *= fogBrightness;
        mFogBlue *= fogBrightness;
    }

    double voidBrightness =
        ((camera->mY - camera->mYOld) * partialTick + camera->mYOld) * level->mDimension->getClearColorScale();
    if (living != NULL && living->hasEffect(MobEffects::BLINDNESS))
    {
        const int duration = living->getEffect(MobEffects::BLINDNESS)->getDuration();
        if (duration < 20)
            voidBrightness *= 1.0f - static_cast<float>(duration) / 20.0f;
        else
            voidBrightness = 0.0;
    }
    if (voidBrightness < 1.0)
    {
        if (voidBrightness < 0.0)
            voidBrightness = 0.0;
        voidBrightness *= voidBrightness;
        mFogRed = static_cast<float>(mFogRed * voidBrightness);
        mFogGreen = static_cast<float>(mFogGreen * voidBrightness);
        mFogBlue = static_cast<float>(mFogBlue * voidBrightness);
    }

    if (mDarkenWorldAmount > 0.0f)
    {
        const float darken = (mDarkenWorldAmount - mDarkenWorldAmountO) * partialTick + mDarkenWorldAmountO;
        mFogRed = mFogRed * (1.0f - darken) + mFogRed * 0.7f * darken;
        mFogGreen = mFogGreen * (1.0f - darken) + mFogGreen * 0.6f * darken;
        mFogBlue = mFogBlue * (1.0f - darken) + mFogBlue * 0.6f * darken;
    }

    if (living != NULL
        && (living->hasEffect(MobEffects::NIGHT_VISION) || living->hasEffect(MobEffects::CONDUIT_POWER)))
    {
        const float nightVision = static_cast<float>(getNightVisionScale(living, partialTick));
        float brighten = FLT_MAX;
        if (mFogRed > 0.0f && 1.0f / mFogRed < brighten)
            brighten = 1.0f / mFogRed;
        if (mFogGreen > 0.0f && 1.0f / mFogGreen < brighten)
            brighten = 1.0f / mFogGreen;
        if (mFogBlue > 0.0f && 1.0f / mFogBlue < brighten)
            brighten = 1.0f / mFogBlue;
        mFogRed = mFogRed * (1.0f - nightVision) + mFogRed * brighten * nightVision;
        mFogGreen = mFogGreen * (1.0f - nightVision) + mFogGreen * brighten * nightVision;
        mFogBlue = mFogBlue * (1.0f - nightVision) + mFogBlue * brighten * nightVision;
    }

    if (mMinecraft->mOptions->mAnaglyph3d)
    {
        const float red = (mFogRed * 30.0f + mFogGreen * 59.0f + mFogBlue * 11.0f) / 100.0f;
        const float green = (mFogRed * 30.0f + mFogGreen * 70.0f) / 100.0f;
        const float blue = (mFogRed * 30.0f + mFogBlue * 70.0f) / 100.0f;
        mFogRed = red;
        mFogGreen = green;
        mFogBlue = blue;
    }

    GlStateManager::clearColor(mFogRed, mFogGreen, mFogBlue, 1.0f);
}

FloatBuffer* GameRenderer::getBuffer(float x, float y, float z, float w)
{
    mFogBuffer->clear();
    mFogBuffer->put(x);
    mFogBuffer->put(y);
    mFogBuffer->put(z);
    mFogBuffer->put(w);
    return mFogBuffer->flip();
}

void GameRenderer::resetFogColor(bool useBlack)
{
    static const float kAlpha = 1.0f;
    static const float kZero = 0.0f;

    if (!useBlack)
    {
        const float red = mFogRed;
        const float green = mFogGreen;
        const float blue = mFogBlue;
        FloatBuffer* buffer = getBuffer(red, green, blue, kAlpha);
        GlStateManager::glFog(5, buffer);
    }
    else
    {
        FloatBuffer* buffer = getBuffer(kZero, kZero, kZero, kAlpha);
        GlStateManager::glFog(5, buffer);
    }
}

void GameRenderer::setupFog(int, float partialTick)
{
    static const float kOne = 1.0f;
    static const float kFogStartScale = 0.25f;
    static const float kFogStartTightScale = 0.05000000074505806f;
    static const float kFogEndCap = 192.0f;
    static const float kFogEndScale = 0.5f;
    static const float kExpFogDensity = 0.10000000149011612f;
    static const float kLavaFogDensity = 2.0f;
    static const float kBlindnessFogEnd = 5.0f;
    static const float kBlindnessInterpRange = 20.0f;

    resetFogColor(false);
    GlStateManager::color4f(kOne, kOne, kOne, kOne);

    Level* level = mMinecraft->mLevel;
    boost::shared_ptr<Entity> cameraEntity = getCamera();
    const BlockState* blockState = Camera::getBlockAt(level, cameraEntity, partialTick);
    Material* material = blockState->getMaterial();

    boost::shared_ptr<LivingEntity> living;
    bool hasBlindness = false;
    if ((cameraEntity->GetType() & EntityType::LIVING_ENTITY) == EntityType::LIVING_ENTITY)
    {
        living = boost::static_pointer_cast<LivingEntity>(cameraEntity);
        if (living->getEffect(MobEffects::BLINDNESS) != NULL)
            hasBlindness = true;
    }

    if (hasBlindness)
    {
        float fogEnd = kBlindnessFogEnd;
        MobEffectInstance* effect = living->getEffect(MobEffects::BLINDNESS);
        const int duration = effect->getDuration();
        const float farPlane = mFogDistance;
        if (duration < 20)
        {
            const float t = kOne - static_cast<float>(duration) / kBlindnessInterpRange;
            fogEnd = (farPlane - kBlindnessFogEnd) * t + kBlindnessFogEnd;
        }
        const float fogStart = fogEnd * kFogStartScale;
        GlStateManager::fogMode(1);
        GlStateManager::fogStart(fogStart);
        GlStateManager::fogEnd(fogEnd);
        if (gRenderer != NULL)
            gRenderer->StateSetVertexAlphaDistance(fogStart);
        GlStateManager::enableColorMaterial();
        GlStateManager::colorMaterial(0, 2);
        return;
    }

    if (mCloudFog)
    {
        GlStateManager::fogMode(2);
        GlStateManager::fogDensity(kExpFogDensity);
        GlStateManager::enableColorMaterial();
        GlStateManager::colorMaterial(0, 2);
        return;
    }

    if (material->isUnderWater())
    {
        GlStateManager::enableColorMaterial();
        GlStateManager::colorMaterial(0, 2);
        return;
    }

    if (material == Material::LAVA)
    {
        GlStateManager::fogMode(2);
        GlStateManager::fogDensity(kLavaFogDensity);
        GlStateManager::enableColorMaterial();
        GlStateManager::colorMaterial(0, 2);
        return;
    }

    float fogEnd = mFogDistance;
    GlStateManager::fogMode(1);
    float fogStart = fogEnd * kFogStartScale;
    GlStateManager::fogStart(fogStart);
    GlStateManager::fogEnd(fogEnd);

    BlockPos cameraPos(cameraEntity);
    Biome* biome = level->getBiome(cameraPos);
    bool tightenFog = biome->isFoggy();
    if (!tightenFog)
    {
        const int playerIndex = mMinecraft->mPlayer->mUserIndex;
        BossHealthOverlay* overlay = mMinecraft->mGui->getBossOverlay(playerIndex);
        if (overlay != NULL && overlay->shouldCreateWorldFog())
            tightenFog = true;
    }
    if (tightenFog)
    {
        fogStart = fogEnd * kFogStartTightScale;
        float cappedEnd = fogEnd;
        if (fogEnd > kFogEndCap)
            cappedEnd = kFogEndCap;
        fogEnd = cappedEnd * kFogEndScale;
        GlStateManager::fogStart(fogStart);
        GlStateManager::fogEnd(fogEnd);
    }

    if (gRenderer != NULL)
        gRenderer->StateSetVertexAlphaDistance(fogStart);

    GlStateManager::enableColorMaterial();
    GlStateManager::colorMaterial(0, 2);
}

void GameRenderer::prepareAndRenderClouds(LevelRenderer* levelRenderer, float partialTick, int eyeIndex,
                                         double x, double y, double z)
{
    const int playerIndex = gProfile->GetPrimaryPad();
    if (GameSettings::GetGameSettings(playerIndex, eGameSetting_Clouds) == 0)
        return;

    const MiniGameDef* miniGame = mMinecraft->GetMiniGame();
    if (miniGame == NULL || !miniGame->HasFeatureClouds())
        return;

    GlStateManager::pushMatrix();
    setupFog(-1, partialTick);
    GlStateManager::enableFog();
    levelRenderer->renderClouds(partialTick, eyeIndex, x, y, z);
    GlStateManager::disableFog();
    setupFog(0, partialTick);
    GlStateManager::popMatrix();
}

void GameRenderer::renderThermal(Culler* culler, float partialTick)
{
    LevelRuleset* ruleset = g_pApp->getGameRuleDefinitions();
    if (!Minecraft::InMiniGame(eMiniGameId_Glide, true) || ruleset == NULL || culler == NULL)
    {
        return;
    }

    FrustumCuller* frustumCuller = reinterpret_cast<FrustumCuller*>(culler);
    FrustumData* frustumData = frustumCuller->frustumData;
    if (frustumData == NULL)
    {
        return;
    }

    const double negXOffset = -frustumCuller->xOffset;
    const double negYOffset = -frustumCuller->yOffset;
    const double negZOffset = -frustumCuller->zOffset;

    float clipPlanes[24];
    float* clipPlane = clipPlanes;
    for (int planeIndex = 0; planeIndex < 6; ++planeIndex)
    {
        float* frustumPlane = frustumData->frustum[planeIndex];
        const float planeOffset =
            static_cast<float>(negXOffset * frustumPlane[0] + negYOffset * frustumPlane[1] +
                               negZOffset * frustumPlane[2] + frustumPlane[3]);
        clipPlane[0] = frustumPlane[0];
        clipPlane[1] = frustumPlane[1];
        clipPlane[2] = frustumPlane[2];
        clipPlane[3] = planeOffset;
        clipPlane += 4;
    }

    static ResourceLocation sThermalDefault(static_cast<_TEXTURE_NAME>(0xab));
    static ResourceLocation sThermalTinted(static_cast<_TEXTURE_NAME>(0xac));
    static ResourceLocation sThermalBoost(static_cast<_TEXTURE_NAME>(0xad));
    const ResourceLocation* boundThermalTexture = &sThermalDefault;

    mMinecraft->mTextures->bindTexture(&sThermalDefault);
    GlStateManager::disableFog();
    GlStateManager::disableLighting();
    GlStateManager::disableCull();
    GlStateManager::enableBlend();
    GlStateManager::blendFunc(4, 5);
    GlStateManager::depthMask(false);

    Tesselator* tesselator = Tesselator::getInstance();
    BufferBuilder* builder = tesselator->getBuilder();

    boost::shared_ptr<Entity> camera = getCamera();

    const float cameraX =
        static_cast<float>((camera->mX - camera->mXo) * partialTick + camera->mXo);
    const float cameraY =
        static_cast<float>((camera->mY - camera->mYo) * partialTick + camera->mYo);
    const float cameraZ =
        static_cast<float>((camera->mZ - camera->mZo) * partialTick + camera->mZo);
    builder->offset(-cameraX, -cameraY, -cameraZ);

    std::vector<ThermalAreaRuleDefinition*> thermalAreas;
    ruleset->getThermalAreas(&thermalAreas);

    static const float kThermalCompareZero = 0.0f;
    static const float kAreaExpandFactor = 0.7f;
    static const float kBlockScale = 0.0625f;
    static const float kTwoPi = 6.2831854820251465f;
    static const float kCornerCount = 4.0f;
    static const float kPerimeterDivisor = 2.0f;
    static const float kPhaseScale = 0.125f;
    static const float kAngleStepScale = 2.132499933242798f;
    static const double kAlphaReference = 1.0;
    static const float kThermalPhaseStepNegative = -1.0f;
    static const float kThermalPhaseStepPositive = 1.0f;
    static const float kWidthExpand = 4.0f;
    static const float kDepthPad = 5.0f;
    static const float kCornerScale = 1.5f;
    static const float kCornerScaleAlt = -2.5f;
    static const float kPerimeterScale = 0.25f;
    static const float kBlockOffset = 1.25f;
    static const float kRainPhaseMul = 0.03333333507180214f;
    static const float kRainPhaseAdd = 0.125f;
    static const float kEdgeStep = 0.125f;
    static const float kEdgeStepHalf = 0.375f;

    float thermalVertsX[16];
    float thermalVertsY[16];
    float thermalVertsZ[16];
    float thermalInnerX[16];
    float thermalInnerY[16];
    float thermalInnerZ[16];
    double edgeLengths[16];

    const int rainTime = static_cast<int>(mTickCount);
    const float rainPhaseBase = static_cast<float>(rainTime) + partialTick;

    for (std::vector<ThermalAreaRuleDefinition*>::iterator areaIt = thermalAreas.begin();
         areaIt != thermalAreas.end(); ++areaIt)
    {
        ThermalAreaRuleDefinition* rule = *areaIt;
        if (!rule->isEnabled())
        {
            continue;
        }

        boost::shared_ptr<Entity> cameraEntity = mMinecraft->getCameraEntity();
        if (!rule->getConditionsMet(cameraEntity))
        {
            continue;
        }

        AABB* area = rule->getArea();
        if (!clip(area, clipPlanes))
        {
            continue;
        }

        const double liftForce = rule->getLiftForceModifier();
        const double liftSpeed = rule->getLiftSpeed();
        const double maxHeight = rule->getMaxHeight();
        const double speedBoost = rule->getElytraSpeedBoost();

        if (!rule->isEnabled() || liftForce <= kThermalCompareZero)
        {
            if (!rule->isEnabled() || kThermalCompareZero <= liftSpeed)
            {
                const double expandX = static_cast<double>(static_cast<float>((area->mMaxX - area->mMinX) * kBlockScale));
                const double expandZ = static_cast<double>(static_cast<float>((area->mMaxZ - area->mMinZ) * kBlockScale));
                const double centerX = static_cast<double>(static_cast<float>(area->mMinX + expandX));
                const double centerZ = static_cast<double>(static_cast<float>(area->mMinZ + expandZ));

                for (int corner = 0; corner < 8; ++corner)
                {
                    const float angle = kTwoPi * (static_cast<float>(corner) - 2.0f) * kAngleStepScale;
                    const float cosAngle = cosf(angle);
                    const float sinAngle = sinf(angle);
                    thermalVertsX[corner] = static_cast<float>(expandX * cosAngle + centerX);
                    thermalVertsZ[corner] = static_cast<float>(expandZ * sinAngle + centerZ);
                    const float angle2 = kTwoPi * (static_cast<float>(corner + 1) - 2.0f) * kAngleStepScale;
                    thermalVertsX[corner + 8] = static_cast<float>(expandX * cosf(angle2) + centerX);
                    thermalVertsZ[corner + 8] = static_cast<float>(expandZ * sinf(angle2) + centerZ);
                }

                double perimeter = 0.0;
                for (int edge = 0; edge < 8; ++edge)
                {
                    const int next = (edge + 1) % 8;
                    const double edgeLength =
                        sqrt((thermalVertsX[next] - thermalVertsX[edge]) *
                                 (thermalVertsX[next] - thermalVertsX[edge]) +
                             (thermalVertsZ[next] - thermalVertsZ[edge]) *
                                 (thermalVertsZ[next] - thermalVertsZ[edge]));
                    edgeLengths[edge] = edgeLength;
                    perimeter += edgeLength;
                }

                const double perimeterAvg = perimeter / kPerimeterDivisor;
                const int blockX = static_cast<int>(area->mMinX + 0.5);
                const int blockZ = static_cast<int>(area->mMinZ + 0.5);
                const int seed = blockX * blockX * 0xC31 + blockX * 0x2B24ABB ^
                                 blockZ * blockZ * 0x66397 + blockZ * 0x35C1;
                mRandom.setSeed(seed);

                const float rainPhase = rainPhaseBase * kRainPhaseMul + static_cast<float>(mRandom.nextDouble());
                const float rainOffset = static_cast<float>(mRandom.nextDouble() + rainPhaseBase * mRandom.nextGaussian() * 0.01f);
                builder->begin();

                if (!rule->isEnabled() || speedBoost <= kAlphaReference)
                {
                    if (!rule->isEnabled() || maxHeight <= kThermalCompareZero)
                    {
                        if (rule->isEnabled() && liftSpeed != kThermalCompareZero)
                        {
                            Minecraft::GetInstance()->getColourTable()->getColour(
                                eMinecraftColour_Thermal_Constant);
                            boundThermalTexture = &sThermalTinted;
                        }
                    }
                    else
                    {
                        Minecraft::GetInstance()->getColourTable()->getColour(
                            eMinecraftColour_Thermal_ConstantToHeight);
                        boundThermalTexture = &sThermalTinted;
                    }
                }
                else
                {
                    Minecraft::GetInstance()->getColourTable()->getColour(
                        eMinecraftColour_Thermal_Lift);
                    boundThermalTexture = &sThermalDefault;
                }

                if (boundThermalTexture != &sThermalDefault)
                {
                    mMinecraft->mTextures->bindTexture(boundThermalTexture);
                }

                BlockPos samplePos(area->mMinX, area->mMinY, area->mMinZ);
                const int brightness = mMinecraft->mLevel->getLightColor(samplePos, 0, -1);
                const int lightValue = brightness * 3 + 0xF000F0;
                const int lightIndex = lightValue / 4;

                const double spanX = static_cast<double>(static_cast<float>(area->mMaxX - area->mMinX));
                const double spanOffset = rainOffset;
                double rowPhase = static_cast<double>(static_cast<float>(mRandom.nextDouble() * kRainPhaseAdd + rainPhase * kPhaseScale));
                double rowOffset = spanOffset;

                for (int edge = 0; edge < 8; ++edge)
                {
                    const int next = (edge + 1) % 8;
                    const double edgeLength = edgeLengths[edge];
                    const double xBase = static_cast<double>(static_cast<float>(area->mMinX + spanX * kEdgeStep + area->mMinX));
                    const double zBase = rowOffset;
                    const double yCoord = static_cast<double>(static_cast<float>(rowPhase));
                    const double xNext = static_cast<double>(static_cast<float>(xBase + spanX * kEdgeStepHalf));
                    builder->blockRainQuad(
                        static_cast<float>(xBase), static_cast<float>(thermalVertsX[edge]),
                        static_cast<float>(thermalVertsZ[edge]), static_cast<float>(rainPhaseBase), static_cast<float>(zBase),
                        static_cast<float>(xNext), static_cast<float>(thermalVertsX[next]),
                        static_cast<float>(thermalVertsZ[next]), static_cast<float>(rainPhaseBase), static_cast<float>(yCoord),
                        static_cast<float>(zBase), static_cast<float>(thermalVertsX[next]),
                        static_cast<float>(thermalVertsZ[next]), static_cast<float>(rainPhaseBase), static_cast<float>(yCoord + edgeLength / perimeterAvg),
                        static_cast<float>(xBase), static_cast<float>(thermalVertsX[edge]),
                        static_cast<float>(thermalVertsZ[edge]), static_cast<float>(rainPhaseBase), static_cast<float>(yCoord + edgeLength / perimeterAvg),
                        1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, lightIndex);
                    rowPhase += edgeLength / perimeterAvg;
                }

                builder->end();
            }
            else
            {
                const float expandX = static_cast<float>((area->mMaxX - area->mMinX - kBlockScale) * kAreaExpandFactor);
                const float expandZ = static_cast<float>((area->mMaxZ - area->mMinZ - kBlockScale) * kAreaExpandFactor);
                const float expandInnerX = expandX * kAreaExpandFactor;
                const float expandInnerZ = expandZ * kAreaExpandFactor;
                AABB* centerBox = area;
                Vec3* center = area->getCenter();
                const float centerX = static_cast<float>(center->x);
                const float centerY = static_cast<float>(center->y);
                const float centerZ = static_cast<float>(center->z);
                const float centerW = static_cast<float>(centerBox->mMaxZ);

                const unsigned int boostDirection = rule->getBoostDirection();
                int cornerCount = 2;
                int cornerIndex = 0;
                int edgeIndex = 8;
                int stepIndex = 1;

                if (boostDirection < 2)
                {
                    thermalVertsY[0] = centerY - expandInnerX;
                    thermalVertsY[4] = centerY + expandInnerX;
                    thermalInnerZ[0] = centerW + expandInnerZ;
                    thermalInnerZ[4] = centerW - expandInnerZ * kCornerScaleAlt;
                    thermalInnerY[0] = expandInnerX * kCornerScale + centerY;
                    thermalInnerZ[8] = centerW + expandInnerZ * kCornerScaleAlt;
                }
                else if (boostDirection < 4)
                {
                    thermalVertsY[0] = centerY - expandInnerX;
                    thermalVertsY[4] = centerY + expandInnerX;
                    thermalInnerY[0] = expandInnerX * kCornerScale + centerY;
                    thermalVertsX[0] = centerX + expandZ;
                    thermalVertsX[4] = centerX - expandZ;
                    thermalInnerX[0] = centerX + expandZ * kCornerScaleAlt;
                    thermalVertsX[8] = centerX - expandZ * kCornerScaleAlt;
                }
                else if (boostDirection < 6)
                {
                    thermalVertsY[0] = centerY - expandInnerX;
                    thermalVertsY[4] = centerY + expandInnerX;
                    thermalInnerZ[0] = centerW + expandInnerZ;
                    thermalInnerZ[4] = centerW - expandInnerZ * kCornerScaleAlt;
                    thermalInnerY[0] = expandInnerX * kCornerScale + centerY;
                    thermalInnerZ[8] = centerW + expandInnerZ * kCornerScaleAlt;
                }
                else if (boostDirection < 8)
                {
                    thermalVertsY[0] = centerY - expandInnerX;
                    thermalVertsY[4] = centerY + expandInnerX;
                    thermalInnerY[0] = expandInnerX * kCornerScale + centerY;
                    thermalVertsX[0] = centerX + expandZ;
                    thermalVertsX[4] = centerX - expandZ;
                    thermalInnerX[0] = centerX + expandZ * kCornerScaleAlt;
                    thermalVertsX[8] = centerX - expandZ * kCornerScaleAlt;
                }

                double perimeter = 0.0;
                for (int loop = 0; loop < cornerCount; ++loop)
                {
                    const int next = (cornerIndex + 1) % 4;
                    double edgeLength;
                    if (boostDirection < 2)
                    {
                        edgeLength = sqrt((thermalVertsY[next] - thermalVertsY[cornerIndex]) *
                                              (thermalVertsY[next] - thermalVertsY[cornerIndex]) +
                                          (thermalInnerZ[next] - thermalInnerZ[cornerIndex]) *
                                              (thermalInnerZ[next] - thermalInnerZ[cornerIndex]));
                    }
                    else if (boostDirection < 6)
                    {
                        edgeLength = sqrt((thermalVertsX[next] - thermalVertsX[cornerIndex]) *
                                              (thermalVertsX[next] - thermalVertsX[cornerIndex]) +
                                          (thermalVertsY[next] - thermalVertsY[cornerIndex]) *
                                              (thermalVertsY[next] - thermalVertsY[cornerIndex]));
                    }
                    else
                    {
                        edgeLength = edgeLengths[edgeIndex / 16];
                    }
                    edgeLengths[edgeIndex] = edgeLength;
                    perimeter += edgeLength;
                    edgeIndex += 16;
                    cornerIndex += 2;
                    stepIndex += 2;
                }

                const int blockX = static_cast<int>(area->mMinX + 0.5);
                const int blockZ = static_cast<int>(area->mMinZ + 0.5);
                const int seed = blockX * blockX * 0xC31 + blockX * 0x2B24ABB ^
                                 blockZ * blockZ * 0x66397 + blockZ * 0x35C1;
                mRandom.setSeed(seed);

                const float rainPhase = rainPhaseBase + static_cast<float>(mRandom.nextDouble());
                const float rainOffset = -static_cast<float>(mRandom.nextDouble() + rainPhaseBase * kPhaseScale);
                builder->begin();

                if (boostDirection < 4 || boostDirection > 7)
                {
                    Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Thermal_Speedboost);
                }
                else
                {
                    Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Thermal_Speedboost2);
                }

                if (boundThermalTexture != &sThermalBoost)
                {
                    boundThermalTexture = &sThermalBoost;
                    mMinecraft->mTextures->bindTexture(boundThermalTexture);
                }

                BlockPos samplePos(area->mMinX, area->mMinY, area->mMinZ);
                const int brightness = mMinecraft->mLevel->getLightColor(samplePos, 0, -1);
                const int lightValue = brightness * 3 + 0xF000F0;
                const int lightIndex = lightValue / 4;

                const float spanX = static_cast<float>(area->mMaxX - area->mMinX);
                const float spanZ = static_cast<float>(area->mMaxZ - area->mMinZ);
                float rowPhase = static_cast<float>(rainOffset);
                float rowOffset = static_cast<float>(area->mMinZ - kBlockScale);

                for (int edge = 0; edge < 4; ++edge)
                {
                    const int next = (edge + 1) % 4;
                    for (int slice = 0; slice < 3; ++slice)
                    {
                        const float phaseStep = (boostDirection & 1U) != 0
                                                     ? kThermalPhaseStepPositive
                                                     : kThermalPhaseStepNegative;
                        const float phaseTop = rowPhase + phaseStep;
                        if (boostDirection == 0)
                        {
                            builder->blockRainQuad(
                                spanX, thermalInnerY[edge], thermalInnerZ[edge], rainPhaseBase, rowPhase,
                                spanX, thermalInnerY[next], thermalInnerZ[next], rainPhaseBase, phaseTop,
                                spanX, thermalInnerY[next], thermalInnerZ[next], rainPhaseBase, phaseTop,
                                spanX, thermalInnerY[edge], thermalInnerZ[edge], rainPhaseBase, rowPhase,
                                1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, lightIndex);
                        }
                        else
                        {
                            builder->blockRainQuad(
                                thermalVertsX[edge], thermalVertsY[edge], rowOffset, rainPhaseBase, rowPhase,
                                thermalVertsX[next], thermalVertsY[next], rowOffset, rainPhaseBase, phaseTop,
                                thermalVertsX[next], thermalVertsY[next], rowOffset, rainPhaseBase, phaseTop,
                                thermalVertsX[edge], thermalVertsY[edge], rowOffset, rainPhaseBase, rowPhase,
                                1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, lightIndex);
                        }
                        rowPhase = phaseTop;
                    }
                    rowOffset += static_cast<float>(edgeLengths[edge] / (perimeter * kPerimeterScale));
                }

                builder->end();
            }
        }
    }

    builder->offset(cameraX, cameraY, cameraZ);
    GlStateManager::disableLighting();
    GlStateManager::enableTexture();
    GlStateManager::depthMask(true);
    GlStateManager::disableBlend();
    GlStateManager::enableFog();
}

void GameRenderer::renderGhost(float partialTick, boost::shared_ptr<Entity> entity)
{
    static const float kZero = 0.0f;
    static const float kOne = 1.0f;
    static const float kHalf = 0.5f;
    static const float kGhostScale = 0.9375f;
    static const float kModelScale = 0.0625f;
    static const float kDepthOffset = 0.125f;
    static const float kNeg180 = -180.0f;
    static const float kNeg90 = -90.0f;
    static const float kNegOne = -1.0f;
    static const float kPowBase = 1.5f;
    static const float kColourDivisor = 255.0f;
    static const float kPolygonOffset = -0.30000001192092896f;

    ColourTable* colourTable = mMinecraft->getColourTable();
    const int startColour = colourTable->getColour(eMinecraftColour_Glide_Ghost_Slow);
    const int middleColour = colourTable->getColour(eMinecraftColour_Glide_Ghost_Medium);
    const int endColour = colourTable->getColour(eMinecraftColour_Glide_Ghost_Fast);

    GhostController* ghostController = mMinecraft->GetGhostController();
    LevelGenerationOptions* generationOptions = g_pApp->getLevelGenerationOptions();
    if (ghostController == NULL || generationOptions == NULL)
    {
        return;
    }

    boost::shared_ptr<MultiplayerLocalPlayer> player = mMinecraft->mPlayer;
    const int trackIndex = player->mUserIndex;

    GhostController::GhostRecord* record = ghostController->mLoaded[trackIndex];
    if (record == NULL || !record->HasRecording() || ghostController->HasFinished(record))
    {
        return;
    }

    if (!ghostController->mPlaybackActive[trackIndex])
    {
        return;
    }

    float playbackFraction = kOne;
    unsigned int sampleIndex = ghostController->getPlaybackIndex(record, playbackFraction);

    Vec3* samplePos = record->GetPosAtIndex(sampleIndex);
    Vec3* sampleRot = record->GetRotAtIndex(sampleIndex);
    double sampleVelocity = record->GetVelocityAtIndex(sampleIndex);

    Vec3* renderPos = samplePos;
    Vec3* renderRot = sampleRot;
    float flyAmount = kOne;
    const unsigned int sampleCount = static_cast<unsigned int>(record->NumSamples());
    if (sampleIndex + 1U < sampleCount)
    {
        Vec3* nextRot = record->GetRotAtIndex(sampleIndex + 1U);
        const double nextVelocity = record->GetVelocityAtIndex(sampleIndex + 1U);
        Vec3* nextPos =
            record->GetPosAtIndex(sampleIndex + 1U);

        renderPos = samplePos->interpolateTo(nextPos, static_cast<double>(playbackFraction));
        renderRot = sampleRot->interpolateTo(nextRot, static_cast<double>(playbackFraction));
        sampleVelocity =
            Mth::Lerp(static_cast<float>(sampleVelocity), static_cast<float>(nextVelocity),
                      playbackFraction);

        if (renderPos->y < samplePos->y)
        {
            Vec3* delta = Vec3::newTemp(renderPos->x - samplePos->x, renderPos->y - samplePos->y,
                                        renderPos->z - samplePos->z);
            delta->normalize();
            const double lift = std::pow(-delta->y, (double)kPowBase);
            flyAmount = kOne - static_cast<float>(lift);
        }
    }

    const double cameraX = (player->mX - player->mXOld) * partialTick + player->mXOld;
    const double cameraY = (player->mY - player->mYOld) * partialTick + player->mYOld;
    const double cameraZ = (player->mZ - player->mZOld) * partialTick + player->mZOld;

    const float velocityAlpha =
        Mth::clamp(static_cast<float>(sampleVelocity * kDepthOffset), kZero, kOne);
    const unsigned int blendedColour =
        ColourTable::LinearBlend(startColour, middleColour, endColour, velocityAlpha);

    Vec3* playerPos = Vec3::newTemp(player->mX, player->mY, player->mZ);
    const float distanceAlpha = Mth::clamp(
        static_cast<float>(playerPos->distanceTo(renderPos) * kHalf), kZero, kOne);

    EntityRenderDispatcher* dispatcher = mMinecraft->getEntityRenderDispatcher();
    PlayerRenderer* playerRenderer =
        dispatcher->getPlayerRenderer(EPLAYER_MODEL_SLIM);
    if (playerRenderer == NULL)
    {
        return;
    }

    HumanoidModel* humanoidModel = playerRenderer->getModel();
    GlStateManager::pushMatrix();
    GlStateManager::translatef(static_cast<float>(-(cameraX)), static_cast<float>(-(cameraY)),
                               static_cast<float>(-(cameraZ)));
    GlStateManager::translatef(static_cast<float>(renderPos->x), static_cast<float>(renderPos->y),
                               static_cast<float>(renderPos->z));
    GlStateManager::rotatef(kNeg180 - static_cast<float>(renderRot->y), kZero, kOne, kZero);
    GlStateManager::rotatef(kNeg90 - static_cast<float>(renderRot->x), kOne, kZero, kZero);
    GlStateManager::rotatef(static_cast<float>(renderRot->z), kZero, kZero, kOne);
    GlStateManager::enableRescaleNormal();
    GlStateManager::scalef(kNegOne, kNegOne, kOne);
    GlStateManager::scalef(kGhostScale, kGhostScale, kOne);

    ElytraModel* elytraModel = playerRenderer->getElytraModel();
    GlStateManager::translatef(kZero, kZero, kDepthOffset);
    humanoidModel->overrideAnim();
    boost::shared_ptr<Entity> renderEntity = boost::static_pointer_cast<Entity>(player);
    humanoidModel->render(renderEntity, kZero, kZero, kZero, kZero, kZero, kModelScale, true);
    elytraModel->overrideAnim(flyAmount, static_cast<float>(renderRot->y),
                              static_cast<float>(renderRot->x), kModelScale);

    const float red =
        static_cast<float>(static_cast<double>((blendedColour >> 16) & 0xFF) /
                         kColourDivisor);
    const float green =
        static_cast<float>(static_cast<double>((blendedColour >> 8) & 0xFF) /
                         kColourDivisor);
    const float blue =
        static_cast<float>(static_cast<double>((blendedColour >> 0) & 0xFF) /
                         kColourDivisor);
    const float alpha = distanceAlpha * kHalf;
    GlStateManager::color4f(red, green, blue, alpha);

    Lighting::turnOn();
    GlStateManager::cullFace(1);
    Tesselator::getInstance()->end();

    static ResourceLocation sGhostBodyTexture(static_cast<_TEXTURE_NAME>(0xAE));
    static ResourceLocation sGhostElytraTexture(static_cast<_TEXTURE_NAME>(0xA8));
    mMinecraft->mTextures->bindTexture(&sGhostBodyTexture);
    humanoidModel->overrideRender(kModelScale, true);
    mMinecraft->mTextures->bindTexture(&sGhostElytraTexture);
    humanoidModel->render(renderEntity, kZero, kZero, kZero, static_cast<float>(renderRot->y),
                          static_cast<float>(renderRot->x), kModelScale, true);

    Tesselator::getInstance()->end();
    GlStateManager::color4f(red, green, blue, alpha);
    GlStateManager::enableBlend();
    GlStateManager::blendFuncSeparate(4, 5, 1, 0);
    GlStateManager::polygonOffset(kPolygonOffset, kPolygonOffset);

    mMinecraft->mTextures->bindTexture(&sGhostBodyTexture);
    humanoidModel->overrideRender(kModelScale, true);
    mMinecraft->mTextures->bindTexture(&sGhostElytraTexture);
    humanoidModel->render(renderEntity, kZero, kZero, kZero, static_cast<float>(renderRot->y),
                          static_cast<float>(renderRot->x), kModelScale, true);

    Lighting::turnOff();
    GlStateManager::polygonOffset(kZero, kZero);
    GlStateManager::disableBlend();
    GlStateManager::popMatrix();
}

void GameRenderer::renderGhostPath(float partialTick, boost::shared_ptr<Entity> entity)
{
    const int startColour = mMinecraft->getColourTable()->getColour(eMinecraftColour_Glide_Ghost_Slow);
    const int middleColour = mMinecraft->getColourTable()->getColour(eMinecraftColour_Glide_Ghost_Medium);
    const int endColour = mMinecraft->getColourTable()->getColour(eMinecraftColour_Glide_Ghost_Fast);

    GhostController* ghostController = mMinecraft->GetGhostController();
    if (ghostController == NULL)
        return;

    boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = mMinecraft->mPlayer;
    GhostController::GhostRecord* record = ghostController->mLoaded[localPlayer->mUserIndex];
    if (record == NULL || !record->HasRecording() || !ghostController->mPlaybackActive[localPlayer->mUserIndex])
        return;

    std::vector<PositionStruct> positions;
    std::vector<int> colours;
    std::vector<int> alphas;
    positions.reserve(record->NumSamples());
    colours.reserve(record->NumSamples());
    alphas.reserve(record->NumSamples());

    boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);
    double cameraX = (player->mX - player->mXOld) * partialTick + player->mXOld;
    double cameraY = (player->mY - player->mYOld) * partialTick + player->mYOld;
    double cameraZ = (player->mZ - player->mZOld) * partialTick + player->mZOld;
    GlStateManager::pushMatrix();
    GlStateManager::translatef(-(float)cameraX, -(float)cameraY, -(float)cameraZ);

    for (unsigned int i = 0; i < record->NumSamples(); ++i)
    {
        float velocity = Mth::clamp((float)(record->GetVelocityAtIndex(i) * 0.125f), 0.0f, 1.0f);
        int colour = ColourTable::LinearBlend(startColour, middleColour, endColour, velocity);
        Vec3* position = record->GetPosAtIndex(i);
        double distance = Mth::clamp(position->distanceTo(player->getPos(1.0f)), 0.0, 10.0);
        int alpha = (int)(255.0 * distance * 0.04);
        if (i != 0 && i + 1 < record->NumSamples())
        {
            if (record->GetPosAtIndex(i - 1)->distanceTo(position) > 20.0 ||
                position->distanceTo(record->GetPosAtIndex(i + 1)) > 20.0)
            {
                alpha = 0;
            }
        }
        alphas.push_back(alpha);
        colours.push_back(colour);
        PositionStruct linePosition;
        linePosition.x = (float)position->x;
        linePosition.y = (float)position->y;
        linePosition.z = (float)position->z;
        positions.push_back(linePosition);

        if (positions.size() == 1000)
        {
            LevelRenderer::renderLine(&positions, &colours, &alphas, 0.5f);
            colours.clear();
            alphas.clear();
            positions.clear();
        }
    }

    if (positions.size() > 1)
        LevelRenderer::renderLine(&positions, &colours, &alphas, 0.5f);
    GlStateManager::popMatrix();
}

void GameRenderer::renderSnowAndRain(float partialTick)
{
    Level* level = mMinecraft->mLevel;
    float rainLevel = level->getRainLevel(partialTick);

    bool tumbleWeather = false;
    int columnStep = 1;
    if (Minecraft::InMiniGame(eMiniGameId_Tumble, false) && gFestiveEvent)
    {
        tumbleWeather = true;
        columnStep = 2;
        rainLevel = 0.55f;
    }

    if (!tumbleWeather && rainLevel <= 0.0f)
    {
        return;
    }

    gRenderer->StateSetEnableViewportClipPlanes(true);

    turnOnLightLayer(partialTick);
    boost::shared_ptr<Entity> camera = getCamera();

    const int cameraBlockX = Mth::floor(camera->mX);
    const int cameraBlockY = Mth::floor(camera->mY);
    const int cameraBlockZ = Mth::floor(camera->mZ);

    Tesselator* tesselator = Tesselator::getInstance();
    BufferBuilder* builder = tesselator->getBuilder();

    GlStateManager::disableCull();
    GlStateManager::enableBlend();
    GlStateManager::blendFunc(4, 5);
    const float alphaRef = 0.01f;
    GlStateManager::alphaFunc(4, alphaRef);

    static ResourceLocation snowLocation(static_cast<_TEXTURE_NAME>(6));
    static ResourceLocation rainLocation(static_cast<_TEXTURE_NAME>(7));

    MemSect(0x1f);
    mMinecraft->mTextures->bindTexture(&rainLocation);
    MemSect(0);

    const double cameraOldX = camera->mXo;
    const double cameraOldY = camera->mYo;
    const double cameraOldZ = camera->mZo;
    const float cameraX = static_cast<float>((camera->mX - cameraOldX) * partialTick + cameraOldX);
    const float cameraY = static_cast<float>((camera->mY - cameraOldY) * partialTick + cameraOldY);
    const float cameraZ = static_cast<float>((camera->mZ - cameraOldZ) * partialTick + cameraOldZ);

    const int rainTime = static_cast<int>(mTickCount);

    const float negCameraX = -cameraX;
    const float negCameraY = -cameraY;
    const float negCameraZ = -cameraZ;
    builder->offset(negCameraX, negCameraY, negCameraZ);

    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);

    unsigned char activePlayerCount = mMinecraft->mLevelRenderer->activePlayers();
    int renderRadius = 9;
    if (activePlayerCount >= 2 && activePlayerCount <= 4)
    {
        static const unsigned char sRadiusByPlayerCount[5] = {0, 0, 7, 5, 5};
        renderRadius = sRadiusByPlayerCount[activePlayerCount];
    }

    int precipitationMode = -1;
    const int startBlockX = cameraBlockX - renderRadius;
    const int startBlockZ = cameraBlockZ - renderRadius;
    int xSeedAccumulator = startBlockX * startBlockX * 0x2b24abb;

    for (int blockX = startBlockX; blockX <= cameraBlockX + renderRadius; blockX += columnStep)
    {
        const int xInnerSeed = blockX * blockX * 0xc31;
        int zSeedAccumulator = startBlockZ * 0x35c1;

        for (int blockZ = startBlockZ; blockZ <= cameraBlockZ + renderRadius; blockZ += columnStep)
        {
            const int offsetIndex = (blockZ - cameraBlockZ) * 32 + (blockX - cameraBlockX) + 528;
            const float offsetX = mRainSizeX[offsetIndex] * 0.5f;
            const float offsetZ = mRainSizeZ[offsetIndex] * 0.5f;

            BlockPos samplePos(blockX, 0, blockZ);
            if (!level->hasChunkAt(samplePos))
            {
                zSeedAccumulator += columnStep * 0x35c1;
                continue;
            }

            Biome* biome = level->getBiome(samplePos);
            if (!tumbleWeather && !biome->hasRain() && !biome->hasSnow())
            {
                zSeedAccumulator += columnStep * 0x35c1;
                continue;
            }

            BlockPos topRainPos = level->getTopRainBlockPos(samplePos);
            const int topY = topRainPos.y;
            int minY = cameraBlockY - renderRadius;
            if (minY < topY)
            {
                minY = topY;
            }

            int maxY = cameraBlockY + renderRadius;
            if (maxY < topY)
            {
                maxY = topY;
            }

            const int renderY = topY < cameraBlockY ? cameraBlockY : topY;
            if (minY == maxY)
            {
                zSeedAccumulator += columnStep * 0x35c1;
                continue;
            }

            const int zInnerSeed = blockZ * blockZ * 0x66397;
            mRandom.setSeed(((xInnerSeed + xSeedAccumulator) ^ (zInnerSeed + zSeedAccumulator)) >> 31);

            const float temperature = biome->getTemperature(BlockPos(blockX, minY, blockZ));
            if (!tumbleWeather)
            {
                const float scaledTemp =
                    level->getBiomeSource()->scaleTemp(temperature, level->getSeaLevel());
                if (scaledTemp >= 0.15f)
                {
                    if (precipitationMode < 0)
                    {
                        precipitationMode = 0;
                        mMinecraft->mTextures->bindTexture(&snowLocation);
                        builder->begin();
                        builder->offset(negCameraX, negCameraY, negCameraZ);
                    }
                    else if (precipitationMode > 0)
                    {
                        builder->end();
                        precipitationMode = 0;
                        mMinecraft->mTextures->bindTexture(&snowLocation);
                        builder->begin();
                        builder->offset(negCameraX, negCameraY, negCameraZ);
                    }

                    const int dropSeed =
                        (rainTime + xInnerSeed + xSeedAccumulator + zInnerSeed + zSeedAccumulator) & 31;
                    const double dropPhase =
                        (static_cast<double>(dropSeed) + static_cast<double>(partialTick)) * 0.03125 *
                        (mRandom.nextDouble() + 3.0);

                    const float dx = static_cast<float>((blockX - 0.5f) - cameraBlockX);
                    const float dz = static_cast<float>((blockZ - 0.5f) - cameraBlockZ);
                    const float dist = Mth::sqrt(dx * dx + dz * dz) / static_cast<float>(renderRadius);
                    const float alpha =
                        (-(dist * dist - 1.0f) * 0.5f + 0.5f) * rainLevel * rainLevel;

                    BlockPos lightPos(blockX, renderY, blockZ);
                    builder->tex2(level->getLightColor(lightPos, 0, -1));
                    builder->color(1.0f, 1.0f, 1.0f, alpha);

                    const float xBase = static_cast<float>(blockX - cameraBlockX) + 0.5f;
                    const float zBase = static_cast<float>(blockZ - cameraBlockZ) + 0.5f;
                    const float yMin = static_cast<float>(minY);
                    const float yMax = static_cast<float>(maxY);
                    const float minYPhaseBase = static_cast<float>(
                        static_cast<double>(static_cast<float>(minY)) * 0.25);
                    const float maxYPhaseBase = static_cast<float>(
                        static_cast<double>(static_cast<float>(maxY)) * 0.25);
                    const float vTop = static_cast<float>(
                        static_cast<double>(minYPhaseBase) + dropPhase);
                    const float vBottom = static_cast<float>(
                        static_cast<double>(maxYPhaseBase) + dropPhase);
                    const float uLeft = xBase - offsetX;
                    const float uRight = xBase + offsetX;
                    const float vFront = zBase - offsetZ;
                    const float vBack = zBase + offsetZ;

                    builder->vertexUV(uLeft, yMin, vFront, 0.0f, vTop);
                    builder->vertexUV(uRight, yMin, vBack, 1.0f, vTop);
                    builder->color(1.0f, 1.0f, 1.0f, 1.0f);
                    builder->vertexUV(uRight, yMax, vBack, 1.0f, vBottom);
                    builder->vertexUV(uLeft, yMax, vFront, 0.0f, vBottom);
                    continue;
                }
            }

            if (precipitationMode != 1)
            {
                if (precipitationMode >= 0)
                {
                    builder->end();
                }
                precipitationMode = 1;
                mMinecraft->mTextures->bindTexture(&rainLocation);
                builder->begin();
            }

            const float rainTickPhase =
                static_cast<float>(static_cast<double>(static_cast<float>(rainTime)) +
                                   static_cast<double>(partialTick));
            const float rainScroll =
                (static_cast<float>(rainTime & 0x1ff) + partialTick) * 0.001953125f;
            const float rainGaussianScale = rainTickPhase * 0.01f;
            const double rainRandomA = mRandom.nextDouble();
            const double rainGaussianA = mRandom.nextGaussian();
            const double rainRandomB = mRandom.nextDouble();
            const double rainGaussianB = mRandom.nextGaussian();
            const double rainOffsetA =
                rainRandomA + static_cast<double>(rainGaussianScale * static_cast<float>(rainGaussianA));
            const double rainOffsetB = rainRandomB +
                static_cast<double>(rainTickPhase * static_cast<float>(rainGaussianB) * 0.001f);

            const float dx = static_cast<float>((blockX - 0.5f) - cameraBlockX);
            const float dz = static_cast<float>((blockZ - 0.5f) - cameraBlockZ);
            const float dist = static_cast<float>(std::sqrt(dx * dx + dz * dz) / renderRadius);
            builder->offset(negCameraX, negCameraY, negCameraZ);

            BlockPos lightPos(blockX, renderY, blockZ);
            builder->tex2(level->getLightColor(lightPos, 0, -1));
            const float alpha =
                (-(dist * dist - 1.0f) * 0.3f + 0.5f) * rainLevel * rainLevel;
            builder->color(1.0f, 1.0f, 1.0f, alpha);

            const float xBase = static_cast<float>(blockX - cameraBlockX) + 0.5f;
            const float zBase = static_cast<float>(blockZ - cameraBlockZ) + 0.5f;
            const float yMin = static_cast<float>(minY);
            const float yMax = static_cast<float>(maxY);
            const float vFront = zBase - offsetZ;
            const float vBack = zBase + offsetZ;
            const float minYPhaseBase = static_cast<float>(
                static_cast<double>(static_cast<float>(minY)) * 0.25);
            const float maxYPhaseBase = static_cast<float>(
                static_cast<double>(static_cast<float>(maxY)) * 0.25);
            const float vTop = static_cast<float>(
                static_cast<double>(minYPhaseBase) + static_cast<double>(rainScroll) + rainOffsetB);
            const float vBottom = static_cast<float>(
                static_cast<double>(maxYPhaseBase) + static_cast<double>(rainScroll) + rainOffsetB);

            builder->vertexUV(xBase - offsetX, yMin, vFront, static_cast<float>(rainOffsetA), vTop);
            builder->vertexUV(xBase + offsetX, yMin, vBack, static_cast<float>(1.0 + rainOffsetA), vTop);
            builder->vertexUV(xBase + offsetX, yMax, vBack, static_cast<float>(1.0 + rainOffsetA), vBottom);
            builder->vertexUV(xBase - offsetX, yMax, vFront, static_cast<float>(rainOffsetA), vBottom);
            builder->offset(1.0f, 1.0f, 1.0f);

            zSeedAccumulator += columnStep * 0x35c1;
        }

        xSeedAccumulator += columnStep * 0x2b24abb;
    }

    if (precipitationMode >= 0)
    {
        builder->end();
    }

    builder->offset(0.0f, 0.0f, 0.0f);
    GlStateManager::enableCull();
    GlStateManager::disableBlend();
    GlStateManager::alphaFunc(4, alphaRef);
    turnOffLightLayer(partialTick);

    gRenderer->StateSetEnableViewportClipPlanes(false);
}

void GameRenderer::renderEyePass(int eyeIndex, float partialTick, long long, bool sortTransparent)
{
    gHangingEntityRenderCount = 0;
    GlStateManager::enableCull();
    boost::shared_ptr<Player> cameraPlayer =
        boost::static_pointer_cast<Player>(mMinecraft->getCameraEntity());
    Camera::UpdatePlayerOffsets(cameraPlayer);

    setupClearColor(partialTick);
    GlStateManager::clear(3);
    setupCamera(partialTick, eyeIndex);

    const bool thirdPerson = getCameraThirdPerson() == 2;
    Camera::prepare(cameraPlayer, thirdPerson);

    Frustum::getFrustum();
    setupFog(-1, partialTick);
    mMinecraft->mLevelRenderer->renderSky(partialTick);

    if (mMinecraft->mTexturePackRepository->getSelectedId() == 0x402)
    {
        mMinecraft->mLevelRenderer->renderHaloRing(partialTick);
    }

    GlStateManager::enableFog();
    setupFog(0, partialTick);

    MemSect(0x1f);
    FrustumCuller frustumCuller(Frustum::getFrustum());
    MemSect(0);

    boost::shared_ptr<Entity> cameraEntity = mMinecraft->getCameraEntity();
    Entity* camera = cameraEntity.get();
    const double cameraX = (camera->mX - camera->mXo) * partialTick + camera->mXo;
    const double cameraY = (camera->mY - camera->mYo) * partialTick + camera->mYo;
    const double cameraZ = (camera->mZ - camera->mZo) * partialTick + camera->mZo;
    frustumCuller.prepare(cameraX, cameraY, cameraZ);

    Culler* culler = reinterpret_cast<Culler*>(&frustumCuller);
    mMinecraft->mLevelRenderer->cull(culler, partialTick);

    const double eyeHeight = camera->getEyeHeight();
    const double headY = camera->mY + eyeHeight;
    if (headY < static_cast<double>(mMinecraft->mLevel->getCloudHeight()))
    {
        prepareAndRenderClouds(mMinecraft->mLevelRenderer, partialTick, eyeIndex, cameraX, cameraY,
                               cameraZ);
    }

    GlStateManager::disableBlend();
    Frustum::getFrustum();
    setupFog(0, partialTick);
    GlStateManager::enableFog();

    static ResourceLocation sTerrainTexture(static_cast<_TEXTURE_NAME>(0xf1));
    MemSect(0x1f);
    mMinecraft->mTextures->bindTexture(&sTerrainTexture);
    MemSect(0);
    Lighting::turnOff();

    InfoBarManager::StartTimingCategory(InfoBarManager::eTimingCagegory_Terrain);
    mMinecraft->mLevelRenderer->render(cameraEntity, 0, partialTick, sortTransparent);
    InfoBarManager::EndTimingCategory(InfoBarManager::eTimingCagegory_Terrain);

    GlStateManager::alphaFunc(4, 0.009999999776482582f);

    if (mDebugView == 0)
    {
        Lighting::turnOn();
        frustumCuller.prepare(camera->mX, camera->mY, camera->mZ);
        mMinecraft->mLevelRenderer->renderEntities(cameraEntity, culler, partialTick);

        InfoBarManager::StartTimingCategory(InfoBarManager::eTimingCagegory_Entities);
        turnOnLightLayer(0.0);
        if (mMinecraft->mParticleEngine != NULL)
        {
            mMinecraft->mParticleEngine->renderLit(cameraEntity, partialTick, 1);
        }
        Lighting::turnOff();
        setupFog(0, partialTick);
        if (mMinecraft->mParticleEngine != NULL)
        {
            mMinecraft->mParticleEngine->render(cameraEntity, partialTick, 1);
        }
        turnOffLightLayer(0.0);

        if (mMinecraft->mHitResult != NULL && cameraPlayer->mInventory != NULL)
        {
            not_null_ptr<ItemInstance> selectedItem = cameraPlayer->mInventory->getSelected();
            mMinecraft->mLevelRenderer->renderHit(cameraPlayer, mMinecraft->mHitResult, 0,
                                                  selectedItem, partialTick);
        }

        InfoBarManager::EndTimingCategory(InfoBarManager::eTimingCagegory_Entities);
    }

    InfoBarManager::StartTimingCategory(InfoBarManager::eTimingCagegory_TerrainTransparent);
    GlStateManager::disableBlend();
    GlStateManager::enableCull();
    GlStateManager::blendFunc(4, 5);
    GlStateManager::depthMask(true);
    setupFog(0, partialTick);
    GlStateManager::enableBlend();
    GlStateManager::disableCull();
    MemSect(0x1f);
    mMinecraft->mTextures->bindTexture(&sTerrainTexture);
    MemSect(0);
    GlStateManager::blendFunc(0, 1);
    GlStateManager::enableCull();
    mMinecraft->mLevelRenderer->render(cameraEntity, 2, partialTick, sortTransparent);
    GlStateManager::blendFunc(4, 5);
    mMinecraft->mLevelRenderer->render(cameraEntity, 2, partialTick, sortTransparent);
    gRenderer->BeginOrderIndependentAlpha();
    mMinecraft->mLevelRenderer->render(cameraEntity, 1, partialTick, sortTransparent);
    InfoBarManager::EndTimingCategory(InfoBarManager::eTimingCagegory_TerrainTransparent);

    Lighting::turnOn();
    turnOnLightLayer(0.0);
    if (mMinecraft->mParticleEngine != NULL)
    {
        mMinecraft->mParticleEngine->renderLit(cameraEntity, partialTick, 0);
    }
    setupFog(0, partialTick);
    if (mMinecraft->mParticleEngine != NULL)
    {
        mMinecraft->mParticleEngine->render(cameraEntity, partialTick, 0);
    }
    turnOffLightLayer(0.0);

    gRenderer->EndOrderIndependentAlpha();
    GlStateManager::depthMask(true);
    GlStateManager::enableCull();
    GlStateManager::disableBlend();
    if (mMinecraft->mHitResult != NULL)
    {
        boost::shared_ptr<Player> outlinePlayer = boost::static_pointer_cast<Player>(cameraEntity);
        GlStateManager::disableAlphaTest();
        mMinecraft->mLevelRenderer->renderHitOutline(outlinePlayer, mMinecraft->mHitResult, 0,
                                                     partialTick);
        GlStateManager::enableAlphaTest();
    }

    renderThermal(culler, partialTick);

    if (Minecraft::InMiniGame(eMiniGameId_Glide, true))
    {
        const MiniGameDef* miniGame = mMinecraft->GetMiniGame();
        if (miniGame != NULL && miniGame->GetHostOption(eCustomGameModeHostOption_TimeTrial) != 0)
        {
            const int ghostSetting =
                GameSettings::GetGameSettings(mMinecraft->mPlayer->mUserIndex,
                                              eGameSetting_ShowGlideGhost);
            if (ghostSetting != 0 && g_pApp->IsAppPaused() == 0 && !mMinecraft->mPlayer->isSpectator())
            {
                renderGhost(partialTick, cameraEntity);
            }

            const int ghostPathSetting =
                GameSettings::GetGameSettings(mMinecraft->mPlayer->mUserIndex,
                                              eGameSetting_ShowGlideGhostPath);
            if (ghostPathSetting != 0 && !mMinecraft->mPlayer->isSpectator())
            {
                renderGhostPath(partialTick, cameraEntity);
            }
        }
    }

    boost::shared_ptr<Player> destroyPlayer = boost::static_pointer_cast<Player>(cameraEntity);
    mMinecraft->mLevelRenderer->renderDestroyAnimation(Tesselator::getInstance(), destroyPlayer,
                                                         partialTick);

    if (headY >= static_cast<double>(mMinecraft->mLevel->getCloudHeight()))
    {
        prepareAndRenderClouds(mMinecraft->mLevelRenderer, partialTick, eyeIndex, cameraX, cameraY,
                               cameraZ);
    }

    setupFog(0, partialTick);
    GlStateManager::enableFog();
    renderSnowAndRain(partialTick);
    GlStateManager::disableFog();
    GlStateManager::clear(1);
    renderItemInHand(partialTick, eyeIndex);
}

void GameRenderer::renderLevel(float partialTick, long long targetNanoTime)
{
    static const float kAlphaRef = 0.5f;

    const int playerIndex = gProfile->GetPrimaryPad();
    const bool sortTransparent =
        mMinecraft->mPlayer.get() == mMinecraft->mLocalPlayers[playerIndex].get();

    pick(partialTick);
    GlStateManager::enableDepthTest();
    GlStateManager::enableAlphaTest();
    GlStateManager::alphaFunc(4, kAlphaRef);

    if (mMinecraft->mOptions == NULL || !mMinecraft->mOptions->mAnaglyph3d)
    {
        GlStateManager::colorMask(true, true, true, false);
        renderEyePass(2, partialTick, targetNanoTime, sortTransparent);
    }
    else
    {
        sAnaglyphEyeIndex = 0;
        GlStateManager::colorMask(false, true, true, false);
        renderEyePass(0, partialTick, targetNanoTime, sortTransparent);
        sAnaglyphEyeIndex = 1;
        GlStateManager::colorMask(true, true, true, true);
        renderEyePass(1, partialTick, targetNanoTime, sortTransparent);
    }
}

void GameRenderer::setupGuiScreen(int guiScale)
{
    static const double kZero = 0.0;
    static const double kNear = 1000.0;
    static const double kFar = 3000.0;
    static const float kZeroF = 0.0f;
    static const float kTranslateZ = -2000.0f;

    ScreenSizeCalculator screenSize(mMinecraft->mOptions, guiScale);

    GlStateManager::matrixMode(1);
    GlStateManager::loadIdentity();
    GlStateManager::ortho(kZero, screenSize.mWidthScale, screenSize.mHeightScale, kZero, kNear, kFar);
    GlStateManager::matrixMode(0);
    GlStateManager::loadIdentity();
    GlStateManager::translatef(kZeroF, kZeroF, kTranslateZ);
}

void GameRenderer::renderItemActivationAnimation(int width, int height, float partialTick)
{
    static const float kPi = 3.1415927410125732f;

    const int playerIndex = mMinecraft->mPlayer->mUserIndex;
    if (mItemActivationItems[playerIndex]->isEmpty() || mItemActivationTicks[playerIndex] <= 0)
        return;

    const float progress = (static_cast<float>(40 - mItemActivationTicks[playerIndex]) + partialTick) / 40.0f;
    const float progressSquared = progress * progress;
    const float progressCubed = progress * progressSquared;
    const float curve = 10.25f * progressCubed * progressSquared - 24.95f * progressSquared * progressSquared
                        + 25.5f * progressCubed - 13.8f * progressSquared + 4.0f * progress;
    const float angle = curve * kPi;

    GlStateManager::enableAlphaTest();
    GlStateManager::pushMatrix();
    GlStateManager::enableDepthTest();
    GlStateManager::disableCull();
    Lighting::turnOn();

    const float swingX = sinf(angle * 2.0f);
    const float swingY = sinf(angle * 2.0f);
    GlStateManager::translatef(mItemActivationOffX[playerIndex] * static_cast<float>(width / 4) * Mth::abs(swingX)
                                   + static_cast<float>(width / 2),
                               mItemActivationOffY[playerIndex] * static_cast<float>(height / 4) * Mth::abs(swingY)
                                   + static_cast<float>(height / 2),
                               -50.0f);
    const float scale = 50.0f + 175.0f * Mth::sin(angle);
    GlStateManager::scalef(scale, -scale, scale);
    GlStateManager::rotatef(900.0f * Mth::abs(sinf(angle)), 0.0f, 1.0f, 0.0f);
    GlStateManager::rotatef(6.0f * cosf(progress * 8.0f), 1.0f, 0.0f, 0.0f);
    GlStateManager::rotatef(6.0f * cosf(progress * 8.0f), 0.0f, 0.0f, 1.0f);

    TextureAtlasSprite* sprite = NULL;
    mMinecraft->getItemRenderer()->renderStatic(mItemActivationItems[playerIndex], boost::shared_ptr<LivingEntity>(),
                                                ItemTransforms::FIXED, 1.0f, false, sprite);

    GlStateManager::popMatrix();
    Lighting::turnOff();
    GlStateManager::enableCull();
    GlStateManager::disableDepthTest();
}

void GameRenderer::render(float partialTick, long long frameNanoTime, bool refreshLightTexture)
{
    static const float kAlphaRef = 0.009999999776482582f;
    static const int kMinFps = 60;
    static const long long kNanoPerSecond = 1000000000LL;

    if (mLightTextureDirty && refreshLightTexture)
        this->updateLightTexture(partialTick);

    const long long nowMs = System::processTimeInMilliSecs();
    mLastActiveTime = nowMs;

    if (mMinecraft->mNoRender)
        return;

    anaglyph3d = mMinecraft->mOptions->mAnaglyph3d;

    ScreenSizeCalculator calculator(mMinecraft->mOptions, -1);
    const int screenWidth = static_cast<int>(calculator.getWidth());
    const int screenHeight = static_cast<int>(calculator.getHeight());

    int fpsCap = fjGetFpsCap(mMinecraft->mOptions != NULL
                                 ? *reinterpret_cast<int*>(reinterpret_cast<char*>(mMinecraft->mOptions) + 0x18)
                                 : 0);
    const int averageFps = Minecraft::getAverageFps();
    if (averageFps < fpsCap)
        fpsCap = averageFps;
    if (fpsCap < kMinFps)
        fpsCap = kMinFps;

    if (mMinecraft->mLevel == NULL)
    {
        GlStateManager::matrixMode(1);
        GlStateManager::loadIdentity();
        GlStateManager::matrixMode(0);
        GlStateManager::loadIdentity();
        setupGuiScreen(-1);
        const long long nanoNow = System::processTimeInNanoSecs();
        mLastNanos = nanoNow;
    }
    else
    {
        const long long startNano = System::processTimeInNanoSecs();
        const long long frameBudget = kNanoPerSecond / fpsCap;
        const long long targetNano = startNano + frameBudget - frameNanoTime;
        const long long clampedTarget = targetNano < startNano ? startNano : targetNano;
        renderLevel(partialTick, clampedTarget);

        const long long endNano = System::processTimeInNanoSecs();
        mLastNanos = endNano;

        const bool hideGui =
            mMinecraft->mOptions != NULL && mMinecraft->mOptions->mHideGui;
        if (!hideGui || mMinecraft->mScreen != NULL)
        {
            GlStateManager::alphaFunc(4, kAlphaRef);
            mMinecraft->mGui->render(partialTick, mMinecraft->mScreen != NULL, screenWidth,
                                     screenHeight - 1);
        }
    }

    if (mMinecraft->mScreen != NULL)
    {
        GlStateManager::clear(1);
        mMinecraft->mScreen->render(0, screenHeight - 1, partialTick);
    }
}

int GameRenderer::shouldRenderBlockOutline()
{
    if (!mRenderBlockOutline || mMinecraft == NULL ||
        (mMinecraft->mOptions != NULL && mMinecraft->mOptions->mHideGui))
        return 0;
    boost::shared_ptr<Entity> camera = mMinecraft->getCameraEntity();
    if (!camera)
        return 0;
    boost::shared_ptr<Player> player = boost::dynamic_pointer_cast<Player>(camera);
    if (mMinecraft->mGameMode != NULL &&
        mMinecraft->mGameMode->GetGameType() == GameType::SPECTATOR)
        return 0;
    if (player && player->mAbilities.mFlying)
        return 0;
    if (mMinecraft->mHitResult == NULL || mMinecraft->mHitResult->mType != HitResult::BLOCK ||
        mMinecraft->mLevel == NULL)
        return 0;
    BlockPos pos = mMinecraft->mHitResult->getBlockPos();
    Block* block = mMinecraft->mLevel->getBlock(pos);
    if (block == NULL)
        return 0;
    if (player && player->mInventory != NULL) {
        not_null_ptr<ItemInstance> held = player->mInventory->getSelected();
        ItemInstance* item = held.get();
        if (item != NULL && !item->isEmpty() &&
            (item->hasAdventureModeBreakTagForBlock(block) ||
             item->hasAdventureModePlaceTagForBlock(block)))
            return 1;
    }
    return 1;
}

void GameRenderer::resetData()
{
    mItemActivationItems[0] = ItemInstance::EMPTY;
    mItemActivationItems[1] = ItemInstance::EMPTY;
    mItemActivationItems[2] = ItemInstance::EMPTY;
    mItemActivationItems[3] = ItemInstance::EMPTY;
    mMapRenderer->resetData();
}

MapRenderer* GameRenderer::getMapRenderer()
{
    return mMapRenderer;
}

void GameRenderer::setItemInHandRenderer(ItemInHandRenderer* renderer)
{
    mItemInHandRenderer = renderer;
}

void GameRenderer::AddForDelete(SparseLightStorage* storage)
{
    EnterCriticalSection(&sDeleteMutex);
    sPendingLightDeletes[sPendingLightDeleteList].push_back(storage);
}

void GameRenderer::AddForDelete(LevelChunk* chunk)
{
    EnterCriticalSection(&sDeleteMutex);
    sPendingChunkDeletes[sPendingChunkDeleteList].push_back(chunk);
}

void GameRenderer::AddForDelete(ElementStorage* storage)
{
    EnterCriticalSection(&sDeleteMutex);
    ElementStorageWithTick entry;
    entry.tick = 2;
    entry.storage = storage;
    sPendingElementStorageDeletes.push_back(entry);
}

void GameRenderer::FinishedReassigning()
{
    LeaveCriticalSection(&sDeleteMutex);
}

void GameRenderer::EnableUpdateThread(int reason)
{
    EnterCriticalSection(&mUpdateThreadMutex);

    mUpdateThreadDisableMask &= ~(1u << (reason & 0x3f));

    if (mUpdateThreadDisableMask == 0 && !sRenderUpdateThreadActive)
    {
        sRenderUpdateThreadActive = true;
        sRenderUpdateEvents->Set(0);
        sRenderUpdateEvents->Set(1);
    }

    LeaveCriticalSection(&mUpdateThreadMutex);
}

void GameRenderer::DisableUpdateThread(int reason)
{
    EnterCriticalSection(&mUpdateThreadMutex);

    mUpdateThreadDisableMask |= (1u << (reason & 0x3f));

    if (mUpdateThreadDisableMask != 0 && sRenderUpdateThreadActive)
    {
        sRenderUpdateThreadActive = false;
        sRenderUpdateEvents->Clear(0);
        sRenderUpdateEvents->WaitForSingle(1, 0xffffffff);
    }

    LeaveCriticalSection(&mUpdateThreadMutex);
}

void GameRenderer::EnterDeleteThreadCS()
{
    EnterCriticalSection(&sDeleteMutex);
}

void GameRenderer::LeaveDeleteThreadCS()
{
    LeaveCriticalSection(&sDeleteMutex);
}

void GameRenderer::displayItemActivation(not_null_ptr<ItemInstance> item, int playerIndex)
{
    mItemActivationItems[playerIndex] = item;
    mItemActivationTicks[playerIndex] = 40;
    mItemActivationOffX[playerIndex] = mRandom.nextFloat() * 2.0f - 1.0f;
    mItemActivationOffY[playerIndex] = mRandom.nextFloat() * 2.0f - 1.0f;
}
