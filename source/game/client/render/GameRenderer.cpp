#include "game/client/render/GameRenderer.hpp"
#include "game/client/render/MapRenderer.hpp"
#include "game/client/render/LevelRenderer.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/client/render/Textures.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/Options.hpp"
#include "game/core/GameType.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Mth.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/core/HitResult.hpp"
#include "game/core/FloatBuffer.hpp"
#include "game/core/MemoryTracker.hpp"
#include "game/core/Minecraft.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Inventory.hpp"
#include "game/world/Level.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/biome/BiomeSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/generation/Player.hpp"
#include "game/core/Minecraft.hpp"
#include "game/world/generation/AbstractClientPlayer.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/util/ResourceLocation.hpp"

#include <cmath>
#include <cstring>

extern bool gTumbleMapFlags[3];

void MemSect(int section);

GameRenderer::GameRenderer(Minecraft* minecraft, ResourceManager* resourceManager)
    : mMinecraft(minecraft), mResourceManager(resourceManager),
      mMapRenderer(new MapRenderer(minecraft->mFont, minecraft->mOptions, minecraft->mTextures)),
      mRandom(), mFov(), mFog(), mCamera(), mZoom(), mView(), mLight(), mBuffer(NULL), mRenderBlockOutline(true)
{
    std::memset(mPrefix, 0, sizeof(mPrefix));
    std::memset(mRandomTail, 0, sizeof(mRandomTail));
    std::memset(mData, 0, sizeof(mData));
    mBuffer = MemoryTracker::createFloatBuffer(16);
}

GameRenderer::~GameRenderer()
{
    delete mMapRenderer;
}

void GameRenderer::renderNameTagInWorld(Font*, const std::wstring&, float, float, float, int, float, float, bool, bool, int, float)
{
    return;
}

unsigned int GameRenderer::getLightTexture(int, Level*)
{
    return 0;
}

void GameRenderer::runUpdate(void*)
{
    return;
}

unsigned int GameRenderer::getTickCount()
{
    unsigned int value;
    std::memcpy(&value, mData + 200, sizeof(value));
    return value;
}

unsigned int GameRenderer::postEffectActive()
{
    return 0;
}

void GameRenderer::shutdownEffect()
{
    return;
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
    return;
}

double GameRenderer::GetFovVal()
{
    double value;
    std::memcpy(&value, mData + 24, sizeof(value));
    return value;
}

void GameRenderer::UpdateElementStorageDeleteTicks()
{
    return;
}

void GameRenderer::tickFov()
{
    boost::shared_ptr<Entity> camera = Minecraft::GetInstance()->getCameraEntity();
    AbstractClientPlayer* player = dynamic_cast<AbstractClientPlayer*>(camera.get());
    const int playerIndex = player->mPlayerIndex;
    const int indexOffset = playerIndex * static_cast<int>(sizeof(float));

    float currentFov;
    std::memcpy(&currentFov, mData + 48 + indexOffset, sizeof(currentFov));
    std::memcpy(mData + 64 + indexOffset, &currentFov, sizeof(currentFov));

    const float targetFov = static_cast<float>(player->getFieldOfViewModifier());
    float interpolatedFov = (targetFov - currentFov) * 0.5f + currentFov;

    float clampedFov = interpolatedFov;
    if (interpolatedFov > 1.5f)
        clampedFov = 1.5f;
    else if (interpolatedFov < 0.1f)
        clampedFov = 0.1f;

    std::memcpy(mData + 48 + indexOffset, &clampedFov, sizeof(clampedFov));
}

void GameRenderer::tickLightTexture()
{
    return;
}

AbstractClientPlayer* GameRenderer::getCamera()
{
    boost::shared_ptr<Entity> camera = Minecraft::GetInstance()->getCameraEntity();
    return dynamic_cast<AbstractClientPlayer*>(camera.get());
}

void GameRenderer::tickRain()
{
    return;
}

void GameRenderer::tick(bool)
{
    return;
}

void GameRenderer::pick(float)
{
    return;
}

double GameRenderer::getFov(float amount, bool)
{
    return static_cast<double>(amount);
}

void GameRenderer::bobHurt(float)
{
    return;
}

void GameRenderer::bobView(float)
{
    return;
}

unsigned int GameRenderer::getCameraThirdPerson()
{
    unsigned int value;
    std::memcpy(&value, mData + 0x760, sizeof(value));
    return value;
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

void GameRenderer::moveCameraToPlayer(float)
{
    return;
}

void GameRenderer::getFovAndAspect(float& fov, float& aspect, float value, bool)
{
    fov = value;
    aspect = value;
}

void GameRenderer::setupCamera(float, int)
{
    return;
}

void GameRenderer::turnOnLightLayer(double)
{
    return;
}

void GameRenderer::turnOffLightLayer(double)
{
    return;
}

void GameRenderer::renderItemInHand(float, int)
{
    return;
}

double GameRenderer::getNightVisionScale(boost::shared_ptr<LivingEntity>, float value)
{
    return static_cast<double>(value);
}

double GameRenderer::getUnderWaterVisionScale(const boost::shared_ptr<Player>&, float value)
{
    return static_cast<double>(value);
}

void GameRenderer::updateLightTexture(float)
{
    return;
}

unsigned int GameRenderer::fjGetFpsCap(int value)
{
    return value < 0 ? 0 : static_cast<unsigned int>(value);
}

unsigned int GameRenderer::getFogOffsets()
{
    return 0;
}

double GameRenderer::getTargetFogDistance(const BlockPos&)
{
    return 0.0;
}

void GameRenderer::getTargetFogColor(const BlockPos&, float& red, float& green, float& blue)
{
    red = 0.0f;
    green = 0.0f;
    blue = 0.0f;
}

double GameRenderer::modifyFogDistance(const BlockPos&, float distance, float)
{
    return distance;
}

void GameRenderer::setupClearColor(float)
{
    return;
}

FloatBuffer* GameRenderer::getBuffer(float x, float y, float z, float w)
{
    mBuffer->clear();
    mBuffer->put(x);
    mBuffer->put(y);
    mBuffer->put(z);
    mBuffer->put(w);
    return mBuffer->flip();
}

void GameRenderer::resetFogColor(bool)
{
    return;
}

void GameRenderer::setupFog(int, float)
{
    return;
}

void GameRenderer::prepareAndRenderClouds(LevelRenderer*, float, int, double, double, double)
{
    return;
}

void GameRenderer::renderThermal(Culler*, float)
{
    return;
}

void GameRenderer::renderGhost(float, boost::shared_ptr<Entity>)
{
    return;
}

void GameRenderer::renderGhostPath(float, boost::shared_ptr<Entity>)
{
    return;
}

void GameRenderer::renderSnowAndRain(float partialTick)
{
    Level* level = mMinecraft->mLevel;
    float rainLevel = level->getRainLevel(partialTick);

    bool tumbleWeather = false;
    int columnStep = 1;
    if (Minecraft::InMiniGame(eMiniGameId_Tumble, false) && gTumbleMapFlags[1])
    {
        tumbleWeather = true;
        columnStep = 2;
        rainLevel = 0.55f;
    }

    if (!tumbleWeather && rainLevel <= 0.0f)
    {
        return;
    }

    if (gRenderer != NULL)
    {
        gRenderer->StateSetForceLOD(1);
    }

    turnOnLightLayer(0.0);
    AbstractClientPlayer* camera = getCamera();
    if (camera == NULL)
    {
        if (gRenderer != NULL)
        {
            gRenderer->StateSetForceLOD(0);
        }
        return;
    }

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

    MemSect(0x1f);
    mMinecraft->mTextures->bindTexture(L"terrain");
    MemSect(0);

    const double cameraOldX = camera->mXo;
    const double cameraOldY = camera->mYo;
    const double cameraOldZ = camera->mZo;
    const float cameraX = static_cast<float>((camera->mX - cameraOldX) * partialTick + cameraOldX);
    const float cameraY = static_cast<float>((camera->mY - cameraOldY) * partialTick + cameraOldY);
    const float cameraZ = static_cast<float>((camera->mZ - cameraOldZ) * partialTick + cameraOldZ);

    int rainTime;
    std::memcpy(&rainTime, reinterpret_cast<char*>(this) + 0x5C, sizeof(rainTime));
    const float rainAnim = static_cast<float>(rainTime) + partialTick;

    float* rainOffsetX = reinterpret_cast<float*>(reinterpret_cast<char*>(this) + 0x164);
    float* rainOffsetZ = reinterpret_cast<float*>(reinterpret_cast<char*>(this) + 0x1164);

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

    static ResourceLocation snowLocation(L"environment_snow");
    static ResourceLocation rainLocation(L"environment_rain");

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
            const float offsetX = rainOffsetX[offsetIndex] * 0.5f;
            const float offsetZ = rainOffsetZ[offsetIndex] * 0.5f;

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
                    const float dropPhase =
                        (static_cast<float>(dropSeed) + partialTick) * 1.25f *
                        static_cast<float>(mRandom.nextDouble() + 6.0);

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
                    const float vTop = static_cast<float>(minY) * 2.0f + dropPhase;
                    const float vBottom = vTop;
                    const float uLeft = xBase - offsetX + 1.75f;
                    const float uRight = xBase + offsetX + 1.75f;
                    const float vFront = zBase - offsetZ + 1.75f;
                    const float vBack = zBase + offsetZ + 1.75f;

                    builder->vertexUV(uLeft, yMin, vFront, 0.0f, vTop);
                    builder->vertexUV(uRight, yMin, vBack, 1.0f, vBottom);
                    builder->color(1.0f, 1.0f, 1.0f, 1.0f);
                    builder->vertexUV(uRight, yMax, vBack, 1.0f, vTop);
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

            const float rainPhase =
                static_cast<float>(rainAnim * 0.001953125f + mRandom.nextDouble());
            const float rainOffsetA = static_cast<float>(mRandom.nextGaussian() * -0.5f);
            const float rainOffsetB =
                static_cast<float>(rainAnim * (mRandom.nextGaussian() * 0.01f));

            const float dx = static_cast<float>((blockX - 0.5f) - cameraBlockX);
            const float dz = static_cast<float>((blockZ - 0.5f) - cameraBlockZ);
            const float dist = static_cast<float>(std::sqrt(dx * dx + dz * dz) / renderRadius);
            builder->offset(negCameraX, negCameraY, negCameraZ);

            BlockPos lightPos(blockX, renderY, blockZ);
            builder->tex2(level->getLightColor(lightPos, 0, -1));
            const float alpha =
                (-(dist * dist - 1.875f) * 0.3f + 0.5f) * rainLevel * rainLevel;
            builder->color(1.0f, 1.0f, 1.0f, alpha);

            const float xBase = static_cast<float>(blockX - cameraBlockX) + 0.5f;
            const float zBase = static_cast<float>(blockZ - cameraBlockZ) + 0.5f;
            const float yMin = static_cast<float>(minY);
            const float yMax = static_cast<float>(maxY);
            const float vFront = zBase - offsetZ + 1.75f;
            const float vBack = zBase + offsetZ + 1.75f;
            const float uCoord = rainPhase + rainOffsetB;
            const float vTop = static_cast<float>(maxY) * 2.0f + rainOffsetA + uCoord;
            const float vBottom = vTop;

            builder->vertexUV(xBase - offsetX + 1.75f, yMin, vFront, 1.875f, vTop);
            builder->vertexUV(xBase + offsetX + 1.75f, yMin, vBack, 1.875f + 1.0f, vBottom);
            const float vLower = static_cast<float>(minY) * 2.0f + rainOffsetA + uCoord;
            builder->vertexUV(xBase + offsetX + 1.75f, yMax, vBack, 1.875f + 1.0f, vLower);
            builder->vertexUV(xBase - offsetX + 1.75f, yMax, vFront, 1.875f, vLower);
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
    turnOffLightLayer(0.0);

    if (gRenderer != NULL)
    {
        gRenderer->StateSetForceLOD(0);
    }
}

void GameRenderer::renderEyePass(int, float, long long, bool)
{
    return;
}

void GameRenderer::renderLevel(float, long long)
{
    return;
}

void GameRenderer::setupGuiScreen(int)
{
    return;
}

void GameRenderer::renderItemActivationAnimation(int, int, float)
{
    return;
}

void GameRenderer::render(float, long long, bool)
{
    return;
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
    not_null_ptr<ItemInstance>* activationItems =
        reinterpret_cast<not_null_ptr<ItemInstance>*>(reinterpret_cast<char*>(this) + 0x225c);
    for (int i = 0; i < 4; ++i)
        activationItems[i] = ItemInstance::EMPTY;
    mMapRenderer->resetData();
}

MapRenderer* GameRenderer::getMapRenderer()
{
    return mMapRenderer;
}

void GameRenderer::AddForDelete(SparseLightStorage*)
{
    return;
}

void GameRenderer::AddForDelete(LevelChunk*)
{
    return;
}

void GameRenderer::AddForDelete(ElementStorage*)
{
    return;
}

void GameRenderer::FinishedReassigning()
{
    return;
}

void GameRenderer::EnableUpdateThread(int)
{
    return;
}

void GameRenderer::DisableUpdateThread(int)
{
    return;
}

void GameRenderer::EnterDeleteThreadCS()
{
    return;
}

void GameRenderer::LeaveDeleteThreadCS()
{
    return;
}

unsigned long long GameRenderer::displayItemActivation(not_null_ptr<ItemInstance>, int)
{
    return 0;
}
