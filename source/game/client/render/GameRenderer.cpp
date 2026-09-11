#include "game/client/render/GameRenderer.hpp"
#include "game/client/render/Font.hpp"
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
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ColourTable.hpp"
#include "game/core/Direction.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/System.hpp"
#include "game/entity/mob/Animal.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/core/Options.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/generation/LocalPlayer.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/Dimension.hpp"
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

#include <cmath>
#include <cstring>
#include <vector>

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
    Level* level = mMinecraft->mLevel;
    if (level == NULL)
    {
        return;
    }

    float rainLevel = level->getRainLevel(1.0f);
    Options* options = mMinecraft->mOptions;
    if (options != NULL && !options->mFancyGraphics)
    {
        rainLevel *= 0.5f;
    }

    if (rainLevel <= 0.0f)
    {
        return;
    }

    const unsigned char activePlayerCount = mMinecraft->mLevelRenderer->activePlayers();
    const float rainIntensity = rainLevel / static_cast<float>(activePlayerCount + 1);

    int rainTime;
    std::memcpy(&rainTime, reinterpret_cast<char*>(this) + 0x5C, sizeof(rainTime));
    mRandom.setSeed(static_cast<long long>(0x12a7ce5f) * static_cast<long long>(rainTime));

    AbstractClientPlayer* camera = getCamera();
    if (camera == NULL)
    {
        return;
    }

    const BlockPos cameraPos(camera);
    const int cameraBlockY = cameraPos.y;

    int particleCount = static_cast<int>(100.0f * rainIntensity * rainIntensity);
    if (options != NULL)
    {
        if (options->mParticles == 1)
        {
            particleCount >>= 1;
        }
        else if (options->mParticles == 2)
        {
            particleCount = 0;
        }
    }

    if (particleCount <= 0)
    {
        return;
    }

    static const float kRainYOffset = 0.1f;
    static const float kDropletYOffset = 1.875f;
    static const float kMinBrightness = 0.15f;
    static const float kBelowRainSoundVolume = 0.2f;
    static const float kAboveRainSoundVolume = 0.125f;
    static const float kBelowRainSoundPitch = 1.0f;
    static const float kAboveRainSoundPitch = 0.5f;
    static const float kRainSoundRange = 16.0f;

    int rainHitCount = 0;
    double soundX = 0.0;
    double soundY = 0.0;
    double soundZ = 0.0;

    for (int attempt = 0; attempt < particleCount; ++attempt)
    {
        const int offsetX = mRandom.nextInt(10) - mRandom.nextInt(10);
        const int offsetZ = mRandom.nextInt(10) - mRandom.nextInt(10);
        const BlockPos samplePos = cameraPos.offset(offsetX, 0, offsetZ);
        const BlockPos topRainPos = level->getTopRainBlockPos(samplePos);

        if (topRainPos.y > cameraBlockY + 10 || topRainPos.y < cameraBlockY - 10)
        {
            continue;
        }

        Biome* biome = level->getBiome(topRainPos);
        if (biome == NULL || !biome->hasRain())
        {
            continue;
        }

        if (level->getBrightness(topRainPos) < kMinBrightness)
        {
            continue;
        }

        const double randX = mRandom.nextDouble();
        const double randZ = mRandom.nextDouble();
        const BlockPos belowPos = topRainPos.below();
        Block* belowBlock = level->getBlock(belowPos);
        const BlockState* belowState = level->getBlockState(belowPos);
        if (belowBlock == NULL || belowState == NULL)
        {
            continue;
        }

        Vec3* blockOffset = belowBlock->getOffset(belowState, level, belowPos);

        if (belowBlock != Blocks::WATER && belowBlock != Blocks::FLOWING_WATER
            && !belowBlock->isSolidRender(belowState) && belowBlock != Blocks::FIRE)
        {
            ++rainHitCount;
            const int splashRoll = mRandom.nextInt(rainHitCount);
            const double worldX = static_cast<double>(topRainPos.x) + randX;
            const double worldZ = static_cast<double>(topRainPos.z) + randZ;

            if (splashRoll == 0)
            {
                const double worldY = static_cast<double>(topRainPos.y) + kRainYOffset + kRainYOffset
                                      + blockOffset->y - kDropletYOffset;
                level->addParticle(PARTICLE_DROPLET, worldX, worldY, worldZ, 0.0, 0.0, 0.0,
                                   arrayWithLength<int>());
            }
            else
            {
                const double worldY =
                    static_cast<double>(topRainPos.y) + kRainYOffset + kRainYOffset + blockOffset->y;
                level->addParticle(PARTICLE_DROPLET, worldX, worldY, worldZ, rainLevel, rainLevel,
                                   rainLevel, arrayWithLength<int>());
            }

            soundX = worldX;
            soundY = static_cast<double>(topRainPos.y) + kRainYOffset + kRainYOffset + blockOffset->y;
            soundZ = worldZ;
            continue;
        }

        const double splashX = static_cast<double>(topRainPos.x) + randX;
        const double splashY =
            static_cast<double>(topRainPos.y) + kRainYOffset + kRainYOffset - blockOffset->y;
        const double splashZ = static_cast<double>(topRainPos.z) + randZ;
        level->addParticle(PARTICLE_SPLASH, splashX, splashY, splashZ, rainLevel, rainLevel,
                           rainLevel, arrayWithLength<int>());
    }

    if (rainHitCount > 0)
    {
        int& rainSoundTicks = *reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 0x160);
        const int previousRainSoundTicks = rainSoundTicks;
        rainSoundTicks = previousRainSoundTicks + 1;
        if (mRandom.nextInt(3) < previousRainSoundTicks)
        {
            rainSoundTicks = 0;
            MemSect(0x18);

            if (soundY <= static_cast<double>(cameraBlockY + 1))
            {
                level->playLocalSound(soundX, soundY, soundZ, SoundEvents::WEATHER_RAIN,
                                      SoundSource::eSoundSource_Weather, kBelowRainSoundVolume,
                                      kBelowRainSoundPitch, false, kRainSoundRange);
            }
            else
            {
                BlockPos topPos = level->getTopRainBlockPos(cameraPos);
                if (topPos.y <= cameraBlockY)
                {
                    level->playLocalSound(soundX, soundY, soundZ, SoundEvents::WEATHER_RAIN,
                                          SoundSource::eSoundSource_Weather, kBelowRainSoundVolume,
                                          kBelowRainSoundPitch, false, kRainSoundRange);
                }
                else
                {
                    level->playLocalSound(soundX, soundY, soundZ, SoundEvents::WEATHER_RAIN_ABOVE,
                                          SoundSource::eSoundSource_Weather, kAboveRainSoundVolume,
                                          kAboveRainSoundPitch, false, kRainSoundRange);
                }
            }

            MemSect(0);
        }
    }
}

void GameRenderer::tick(bool)
{
    return;
}

namespace
{
Predicate<boost::shared_ptr<Entity> >* getPickEntityPredicate()
{
    static Predicate<boost::shared_ptr<Entity> >* sPickPredicate = NULL;
    if (sPickPredicate == NULL)
    {
        static Predicates<boost::shared_ptr<Entity> >::FunctionPredicate sIsPickablePredicate(
            isPickable_apply);
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

    AbstractClientPlayer* camera = getCamera();
    if (camera == NULL)
    {
        return;
    }

    Level* level = mMinecraft->mLevel;
    if (level == NULL)
    {
        return;
    }

    int& pointedEntityId = *reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 96);
    boost::shared_ptr<Entity>& pointedEntity =
        *reinterpret_cast<boost::shared_ptr<Entity>*>(reinterpret_cast<char*>(this) + 100);
    Entity** minecraftPointedEntity =
        reinterpret_cast<Entity**>(reinterpret_cast<char*>(mMinecraft) + 200);
    boost::shared_ptr<Entity>& minecraftPointedEntityShared =
        *reinterpret_cast<boost::shared_ptr<Entity>*>(reinterpret_cast<char*>(mMinecraft) + 204);

    pointedEntityId = 0;
    pointedEntity.reset();
    *minecraftPointedEntity = NULL;
    minecraftPointedEntityShared.reset();

    const double pickRange = mMinecraft->mGameMode->getPickRange();

    deleteHitResult(mMinecraft->mHitResult);
    mMinecraft->mHitResult = NULL;

    MemSect(0x1f);
    mMinecraft->mHitResult = camera->pick(pickRange, partialTick);
    MemSect(0);

    if (mMinecraft->mHitResult != NULL && mMinecraft->mHitResult->mType == HitResult::BLOCK)
    {
        BlockPos blockPos = mMinecraft->mHitResult->getBlockPos();
        LevelChunk* chunk = level->getChunkAt(blockPos);
        if (chunk != NULL)
        {
            ChunkPos chunkPos = chunk->getPos();
            const int localX = blockPos.x - chunkPos.getMinBlockX();
            const int localZ = blockPos.z - chunkPos.getMinBlockZ();
            if (localX < 1 || localX > 14 || localZ < 1 || localZ > 14)
            {
                deleteHitResult(mMinecraft->mHitResult);
                mMinecraft->mHitResult = NULL;
            }
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

    pointedEntityId = 0;
    pointedEntity.reset();

    const Predicate<boost::shared_ptr<Entity> >* entityPredicate = getPickEntityPredicate();
    if (mMinecraft->mPlayer.get() != NULL &&
        *reinterpret_cast<int*>(reinterpret_cast<char*>(mMinecraft->mPlayer.get()) + 0xE0) != 0)
    {
        entityPredicate = NULL;
    }

    if (Minecraft::InMiniGame(eMiniGameId_Glide, false))
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
    double closestDistance = kNoHitDistance;

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
        if (clipHit == NULL)
        {
            continue;
        }

        const double hitDistance = eyePosition->distanceTo(clipHit->mLocation);
        const bool closerHit = hitDistance < closestDistance || closestDistance == kNoHitDistance;
        const bool sameDistancePreferLowerId =
            hitDistance == closestDistance && closestEntity != NULL &&
            candidateRaw->getId() < closestEntity->getId();

        if (closerHit || sameDistancePreferLowerId)
        {
            if (closestHitLocation != NULL)
            {
                delete closestHitLocation;
            }
            closestEntity = candidateRaw;
            closestEntityShared = candidate;
            closestHitLocation = clipHit;
            closestDistance = hitDistance;
        }
        else
        {
            delete clipHit;
        }
    }

    delete entities;

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
            pointedEntityId = reinterpret_cast<int>(closestEntity);
            pointedEntity = closestEntityShared;

            deleteHitResult(mMinecraft->mHitResult);
            mMinecraft->mHitResult = new HitResult(closestEntityShared, closestHitLocation);

            const unsigned long long entityType = closestEntity->GetType();
            if ((entityType & ENTITY_TYPE_LIVING_ENTITY) == ENTITY_TYPE_LIVING_ENTITY ||
                entityType == 0x000000000A080002ULL)
            {
                *minecraftPointedEntity = closestEntity;
                minecraftPointedEntityShared = closestEntityShared;
            }
        }
        else if (closestHitLocation != NULL)
        {
            delete closestHitLocation;
        }
    }
    else if (closestHitLocation != NULL)
    {
        delete closestHitLocation;
    }
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

void GameRenderer::moveCameraToPlayer(float partialTick)
{
    static const float kZero = 0.0f;
    static const float kOne = 1.0f;
    static const float kNegOne = -1.0f;
    static const float kDegToRad = 0.01745329238474369f;
    static const float k180 = 180.0f;
    static const float kFirstPersonNudge = 0.05f;
    static const float kRidingYOffset = 0.30000001192092896f;
    static const float kRidingHeightAdd = 1.0f;
    static const float kCollisionProbe = 0.10000000149011612f;
    static const float kViewBobScale1 = 0.5f;
    static const float kViewBobScale2 = 2.0f;

    boost::shared_ptr<Entity> cameraEntity = mMinecraft->getCameraEntity();
    boost::shared_ptr<LivingEntity> living =
        boost::static_pointer_cast<LivingEntity>(cameraEntity);
    LivingEntity* entity = living.get();

    const unsigned int thirdPerson = getCameraThirdPerson();
    const double eyeHeight = entity->getEyeHeight();
    double x = (entity->mX - entity->mXOld) * partialTick + entity->mXOld;
    double y = (entity->mY - entity->mYOld) * partialTick + entity->mYOld;
    double z = (entity->mZ - entity->mZOld) * partialTick + entity->mZOld;
    double yEye = y + eyeHeight;

    char sleepingFlag = 0;
    if (mMinecraft->mOptions != NULL)
        sleepingFlag = *reinterpret_cast<char*>(reinterpret_cast<char*>(mMinecraft->mOptions) + 0x10e);

    if (thirdPerson == 3)
    {
        boost::shared_ptr<Player> localPlayer =
            boost::static_pointer_cast<Player>(mMinecraft->mLocalPlayers[0]);
        CameraController* controller = localPlayer->GetCameraController();
        if (controller != NULL && controller->getCameraMode() == 0)
        {
            localPlayer->SetThirdPersonView(1);
            sleepingFlag = mMinecraft->mOptions != NULL
                ? *reinterpret_cast<char*>(reinterpret_cast<char*>(mMinecraft->mOptions) + 0x10e)
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
                ? *reinterpret_cast<char*>(reinterpret_cast<char*>(mMinecraft->mOptions) + 0x10e)
                : 0;
        }
    }
    else if (!entity->isPassenger())
    {
        if (thirdPerson == 0)
        {
            GlStateManager::translatef(kZero, kZero, -kFirstPersonNudge);
        }
        else
        {
            float* zoomCurrent = reinterpret_cast<float*>(reinterpret_cast<char*>(this) + 0xb4);
            float* zoomPrevious = reinterpret_cast<float*>(reinterpret_cast<char*>(this) + 0xb0);
            float distance = (*zoomCurrent - *zoomPrevious) * partialTick + *zoomPrevious;

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
        yEye += kRidingHeightAdd;
        GlStateManager::translatef(kZero, kRidingYOffset, kZero);
        if (sleepingFlag == 0)
        {
            Player* ridingPlayer = dynamic_cast<Player*>(entity);
            if (ridingPlayer != NULL)
            {
                const BlockState* bedState = mMinecraft->mLevel->getBlockState(BlockPos(cameraEntity));
                if (bedState != NULL && bedState->getBlock() == Blocks::bed)
                    GlStateManager::rotatef(static_cast<float>(ridingPlayer->getSleepRotation()),
                                          kZero, kOne, kZero);
            }
            const float yaw = static_cast<float>(getCameraYRot(partialTick) + k180);
            GlStateManager::rotatef(yaw, kZero, kNegOne, kZero);
            GlStateManager::rotatef(static_cast<float>(getCameraXRot(partialTick)),
                                  kNegOne, kZero, kZero);
        }
    }

    LocalPlayer* localBobPlayer = dynamic_cast<LocalPlayer*>(entity);
    if (localBobPlayer != NULL && entity->isPassenger())
    {
        const int playerIndex = localBobPlayer->mUserIndex;
        if (GameSettings::GetGameSettings(playerIndex, static_cast<eGameSetting>(0x28)) == 0)
        {
            const int indexOffset = playerIndex * static_cast<int>(sizeof(float));
            const float bobO = *reinterpret_cast<float*>(reinterpret_cast<char*>(player) + 0x8ec + indexOffset);
            const float bob = *reinterpret_cast<float*>(reinterpret_cast<char*>(player) + 0x8e8 + indexOffset);
            const float bob2O = *reinterpret_cast<float*>(reinterpret_cast<char*>(player) + 0x8f4 + indexOffset);
            const float bob2 = *reinterpret_cast<float*>(reinterpret_cast<char*>(player) + 0x8f0 + indexOffset);
            const float hurtBob = ((bob - bobO) * partialTick + bobO)
                * ((bob2 - bob2O) * partialTick + bob2O);
            GlStateManager::rotatef(hurtBob * kViewBobScale1, kZero, kZero, kOne);
            GlStateManager::rotatef(hurtBob * kViewBobScale2, kOne, kZero, kZero);
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

    GlStateManager::translatef(kZero, -static_cast<float>(yEye), kZero);

    const bool inCloud = mMinecraft->mLevelRenderer->isInCloud(x, yEye, z, partialTick);
    *reinterpret_cast<unsigned char*>(reinterpret_cast<char*>(this) + 0x11c) = inCloud ? 1 : 0;
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

void GameRenderer::renderItemInHand(float partialTick, int eyeIndex)
{
    static const float kZero = 0.0f;
    static const float kHalf = 0.5f;
    static const float kOne = 1.0f;
    static const float kStereoBase = 176.0f;
    static const float kProjectionEyeOffset = 0.07000000029802322f;
    static const float kModelEyeOffset = 0.10000000149011612f;
    static const float kNearPlane = 0.05000000074505806f;
    static const float kHandScaleFactor = 1.875f;

    char* self = reinterpret_cast<char*>(this);
    if (*reinterpret_cast<unsigned char*>(self + 0x224c) != 0)
        return;

    unsigned char* renderHandFlag = reinterpret_cast<unsigned char*>(self + 0x11d);
    *renderHandFlag = 1;

    boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = Minecraft::GetInstance()->mPlayer;
    MultiplayerLocalPlayer* player = localPlayer.get();
    AbstractClientPlayer* camera = getCamera();
    if (camera == NULL)
        return;

    bool holdingMap = false;
    if (camera->GetType() == ENTITY_TYPE_LOCAL_PLAYER && player != NULL)
    {
        if (player->mInventory != NULL)
        {
            not_null_ptr<ItemInstance> selected = player->mInventory->getSelected();
            ItemInstance* item = selected.get();
            if (item != NULL && item->getItem() == Items::map)
                holdingMap = true;
        }

        if (GameSettings::GetGameSettings(player->mUserIndex, static_cast<eGameSetting>(0x18)) != 0)
            *renderHandFlag = 0;
    }

    if (mMinecraft->mOptions != NULL
        && *reinterpret_cast<unsigned char*>(reinterpret_cast<char*>(mMinecraft->mOptions) + 0x15) != 0)
    {
        GlStateManager::matrixMode(1);
        GlStateManager::loadIdentity();
        const float eyeOffset = static_cast<float>((eyeIndex * 2 - 1) * -1);
        GlStateManager::translatef((eyeOffset - kStereoBase) * kProjectionEyeOffset, kHalf, kHalf);
    }

    float fov = partialTick;
    float aspect = partialTick;
    getFovAndAspect(fov, aspect, partialTick, false);

    float* handScale = reinterpret_cast<float*>(self + 0x2250);
    if (*handScale != kOne)
    {
        float* handTransX = reinterpret_cast<float*>(self + 0x2254);
        float* handTransY = reinterpret_cast<float*>(self + 0x2258);
        GlStateManager::translatef(*handTransX, -*handTransY, kHalf);
        GlStateManager::scaled(*handScale, *handScale, kHandScaleFactor);
    }

    const float farPlane = *reinterpret_cast<float*>(self + 0x50) + *reinterpret_cast<float*>(self + 0x50);
    GlStateManager::perspective(fov, aspect, kNearPlane, farPlane);

    GlStateManager::matrixMode(0);
    GlStateManager::loadIdentity();
    if (mMinecraft->mOptions != NULL
        && *reinterpret_cast<unsigned char*>(reinterpret_cast<char*>(mMinecraft->mOptions) + 0x15) != 0)
    {
        const float eyeOffset = static_cast<float>(eyeIndex * 2 - 1);
        GlStateManager::translatef((eyeOffset - kStereoBase) * kModelEyeOffset, kHalf, kHalf);
    }

    GlStateManager::pushMatrix();
    bobHurt(partialTick);

    _SkinAdjustments skinAdjustments = camera->getSkinAdjustments();
    const unsigned int skinFlags = skinAdjustments.mGameFlags;
    const unsigned int skinMask = skinFlags & 0x204u;
    const unsigned int skinAlt = (skinMask - 1u) + (skinMask == 0u ? 1u : 0u);

    if (player != NULL
        && GameSettings::GetGameSettings(player->mUserIndex, static_cast<eGameSetting>(6)) != 0)
    {
        if (*reinterpret_cast<unsigned char*>(reinterpret_cast<char*>(camera) + 0x70d) == 0
            && skinMask == skinAlt)
        {
            if (!camera->isUsingItem())
                bobView(partialTick);
        }
    }

    bool spectatorHandEffect = false;
    if ((camera->GetType() & 0x0C000000ULL) == 0x0C000000ULL)
    {
        boost::shared_ptr<LivingEntity> living =
            boost::static_pointer_cast<LivingEntity>(mMinecraft->getCameraEntity());
        AbstractClientPlayer* clientPlayer = dynamic_cast<AbstractClientPlayer*>(living.get());
        if (clientPlayer != NULL && clientPlayer->isSpectator())
            spectatorHandEffect = true;
    }

    const unsigned int thirdPerson = getCameraThirdPerson();
    const bool skipFirstPersonHand =
        (*renderHandFlag == 0 && !holdingMap) || thirdPerson != 0 || spectatorHandEffect;

    ItemInHandRenderer* itemRenderer =
        *reinterpret_cast<ItemInHandRenderer**>(self + 0x54);

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
        if (player != NULL && !camera->isPassenger()
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

    if (player != NULL
        && GameSettings::GetGameSettings(player->mUserIndex, static_cast<eGameSetting>(6)) != 0)
    {
        if (*reinterpret_cast<unsigned char*>(reinterpret_cast<char*>(camera) + 0x70d) == 0
            && skinMask == skinAlt)
        {
            if (!camera->isUsingItem())
                bobView(partialTick);
        }
    }
}

double GameRenderer::getNightVisionScale(boost::shared_ptr<LivingEntity>, float value)
{
    return static_cast<double>(value);
}

double GameRenderer::getUnderWaterVisionScale(const boost::shared_ptr<Player>&, float value)
{
    return static_cast<double>(value);
}

void GameRenderer::updateLightTexture(float partialTick)
{
    static const float kZero = 0.0f;
    static const float kOne = 1.0f;
    static const float kHalf = 0.5f;
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

    char* self = reinterpret_cast<char*>(this);
    if (*reinterpret_cast<unsigned char*>(self + 0x22c8) == 0 || mMinecraft->mLevel == NULL)
        return;

    float* gamma = reinterpret_cast<float*>(self + 0x158);
    float* damageFlash = reinterpret_cast<float*>(self + 0x114);
    float* damageFlashO = reinterpret_cast<float*>(self + 0x118);

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

        int* pixelBuffer =
            reinterpret_cast<int*>(*reinterpret_cast<uint32_t*>(self + 0x138 + playerIndex * 8));
        if (pixelBuffer == NULL)
            continue;

        Dimension* dimension = level->mDimension;
        float* brightnessRamp = dimension->getBrightnessRamp();
        const float skyDarken = level->getSkyDarken(kOne);
        Biome* biome = level->getBiome(BlockPos(0, 64, 0));
        const unsigned int waterColor =
            biome != NULL ? biome->getWaterColor() : 0u;

        const float waterRed =
            static_cast<float>(((waterColor >> 16) & 0xff) - 176.0f) / k255;
        const float waterBlue =
            static_cast<float>((waterColor & 0xff) - 176.0f) / k255;
        const float waterGreen =
            static_cast<float>(((waterColor >> 8) & 0xff) - 176.0f) / k255;

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
                brightnessRamp[blockLight * 4]
                * (skyDarken * kSkyDarkenScale + kSkyDarkenAdd);
            if (level->getSkyFlashTime() > 0)
                blockBrightness = brightnessRamp[blockLight * 4];

            const float skyBrightness =
                brightnessRamp[skyLight * 4] * (*gamma * kGammaSlope + kGammaBase);

            float redComponent = blockBrightness;
            float greenBase =
                blockBrightness * (skyDarken * kSecondarySkyScale + kSecondarySkyAdd);
            const float redWater = skyBrightness * waterRed;
            const float greenWater =
                skyBrightness
                * (static_cast<float>(skyBrightness * skyBrightness) * kWaterMixC + kWaterMixA);
            const float blueWater =
                skyBrightness
                * (static_cast<float>(skyBrightness * kWaterMixC + kWaterMixA) * kWaterMixC
                   + kWaterMixA);

            double red = static_cast<double>((redComponent + greenWater) * kLightScale + kLightBase);
            double green =
                static_cast<double>((greenBase + redWater) * kLightScale + kLightBase);
            double blue =
                static_cast<double>((greenBase + blueWater) * kLightScale + kLightBase);

            if (*damageFlash > kHalf)
            {
                const float flashAmount =
                    (*damageFlash - *damageFlashO) * partialTick + *damageFlashO;
                const float keep = kOne - flashAmount;
                red = red * keep + red * kGreyBoost * flashAmount;
                green = green * keep + green * kWaterMixC * flashAmount;
                blue = blue * keep + blue * kWaterMixC * flashAmount;
            }

            if (isNether)
            {
                red = static_cast<double>(greenWater * kNetherRed + kNetherGreen);
                green = static_cast<double>(redWater * kNetherRed + kNetherBlueScale);
                blue = static_cast<double>(blueWater * kNetherRed + kNetherBlueAdd);
            }

            if (living->hasEffect(MobEffects::NIGHT_VISION)
                || living->hasEffect(MobEffects::CONFUSION))
            {
                const double nightVision =
                    getNightVisionScale(living, partialTick);
                double minScale = kOne / red;
                if (green > kHalf && kOne / green < minScale)
                    minScale = kOne / green;
                if (blue > kHalf && kOne / blue < minScale)
                    minScale = kOne / blue;
                const double keep = kOne - nightVision;
                red = red * keep + red * minScale * nightVision;
                green = green * keep + green * minScale * nightVision;
                blue = blue * keep + blue * minScale * nightVision;
            }

            if (living->hasEffect(MobEffects::WATER_BREATHING))
            {
                boost::shared_ptr<Player> visionPlayer =
                    boost::static_pointer_cast<Player>(player);
                const double underwaterScale =
                    getUnderWaterVisionScale(visionPlayer, partialTick);
                double minScale = kOne / red;
                if (green > kHalf && kOne / green < minScale)
                    minScale = kOne / green;
                if (blue > kHalf && kOne / blue < minScale)
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
            red = (-(redCurve * redCurve * redCurve - kOne) * kHalf + red) * kLightScale + kLightBase;
            green =
                (-(greenCurve * greenCurve * greenCurve - kOne) * kHalf + green) * kLightScale
                + kLightBase;
            blue =
                (-(blueCurve * blueCurve * blueCurve - kOne) * kHalf + blue) * kLightScale
                + kLightBase;

            if (red > kOne)
                red = kOne;
            if (green > kOne)
                green = kOne;
            if (blue > kOne)
                blue = kOne;
            if (red < kZero)
                red = kHalf;
            if (green < kZero)
                green = kHalf;
            if (blue < kZero)
                blue = kHalf;

            pixelBuffer[lightIndex] =
                static_cast<int>(red * k255) << 24
                | static_cast<int>(green * k255) << 16
                | static_cast<int>(blue * k255) << 8
                | 0xff;
        }

        arrayWithLength<int> pixels;
        pixels.data = pixelBuffer;
        pixels.length = *reinterpret_cast<unsigned int*>(self + 0x13c + playerIndex * 8);
        const unsigned int texture = getLightTexture(playerIndex, level);
        mMinecraft->mTextures->replaceTextureDirect(pixels, 16, 16, texture);
    }
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

void GameRenderer::setupClearColor(float partialTick)
{
    static const float kOne = 1.0f;
    static const float kHalf = 0.5f;
    static const float kBlindnessBase = 0.25f;
    static const float kMinBrightness = 1.0f;
    static const float kFogLerpMs = 4000.0f;
    static const float kFogStartOffset = 4.0f;
    static const float kFogEndOffset = 8.0f;
    static const float kFogDensity = 0.05000000074505806f;
    static const float kRainGreyScale = 0.4000000059604645f;
    static const float kDamageBlend = 0.6000000238418579f;
    static const float kColorNormalize = 176.0f;
    static const float kColorScale = 0.00390625f;
    static const float kColorBlindDivisor = 100.0f;
    static const float kNightVisionCap = 30.0f;
    static const float kUnderwaterBrighten = 0.4000000059604645f;
    static const float kUnderwaterMix = 0.6000000238418579f;
    static const float kGreyBoost = 0.699999988079071f;

    Level* level = mMinecraft->mLevel;
    boost::shared_ptr<Entity> cameraEntity = mMinecraft->getCameraEntity();
    boost::shared_ptr<LivingEntity> living =
        boost::static_pointer_cast<LivingEntity>(cameraEntity);

    float* fogRed = reinterpret_cast<float*>(reinterpret_cast<char*>(this) + 0x2168);
    float* fogGreen = reinterpret_cast<float*>(reinterpret_cast<char*>(this) + 0x216c);
    float* fogBlue = reinterpret_cast<float*>(reinterpret_cast<char*>(this) + 0x2170);

    float darken = kOne;
    if (living->getEffect(MobEffects::BLINDNESS) != NULL)
        darken = kOne - static_cast<float>(std::pow(kBlindnessBase, kBlindnessBase));

    Vec3* skyColor = level->getSkyColor(cameraEntity, partialTick);
    *fogRed = static_cast<float>(skyColor->x);
    *fogGreen = static_cast<float>(skyColor->y);
    *fogBlue = static_cast<float>(skyColor->z);

    Vec3* biomeFog = level->getFogColor(cameraEntity, partialTick);
    const float skyRed = *fogRed;
    const float skyGreen = *fogGreen;
    const float skyBlue = *fogBlue;

    level->getSunAngle(partialTick);
    const float sunSin = Mth::sin(level->getTimeOfDay(partialTick) * 6.2831855f);
    Vec3* sunVector = sunSin > kHalf
        ? Vec3::newTemp(-1.0, 0.0, 0.0)
        : Vec3::newTemp(1.0, 0.0, 0.0);
    const float sunDot = static_cast<float>(
        living->getViewVector(partialTick)->dot(sunVector));

    if (sunDot > kHalf)
    {
        const float timeOfDay = level->getTimeOfDay(partialTick);
        float* sunrise = level->mDimension->getSunriseColor(timeOfDay, partialTick);
        const float blend = sunDot * sunrise[3];
        const float keep = kOne - blend;
        const float red = *fogRed * keep + sunrise[0] * blend;
        const float green = *fogGreen * keep + sunrise[1] * blend;
        const float blue = *fogBlue * keep + sunrise[2] * blend;
        *fogRed = (skyRed - red) * darken + red;
        *fogGreen = (skyGreen - green) * darken + green;
        *fogBlue = (skyBlue - blue) * darken + blue;
    }
    else
    {
        *fogRed = (biomeFog->x - *fogRed) * darken + *fogRed;
        *fogGreen = (biomeFog->y - *fogGreen) * darken + *fogGreen;
        *fogBlue = (biomeFog->z - *fogBlue) * darken + *fogBlue;
    }

    const float rainLevel = level->getRainLevel(partialTick);
    if (rainLevel > kHalf)
    {
        const float rainScale = -(rainLevel * kHalf - kOne);
        *fogRed *= rainScale;
        *fogGreen *= rainScale;
        *fogBlue *= -(rainLevel * kRainGreyScale - kOne);
    }

    const float thunderLevel = level->getThunderLevel(partialTick);
    if (thunderLevel > kHalf)
    {
        const float thunderScale = -(thunderLevel * kHalf - kOne);
        *fogRed *= thunderScale;
        *fogGreen *= thunderScale;
        *fogBlue *= thunderScale;
    }

    if (*reinterpret_cast<unsigned char*>(reinterpret_cast<char*>(this) + 0x11c) != 0)
    {
        Vec3* cloudColor = level->getCloudColor(partialTick);
        *fogRed = static_cast<float>(cloudColor->x);
        *fogGreen = static_cast<float>(cloudColor->y);
        *fogBlue = static_cast<float>(cloudColor->z);
    }

    const BlockState* cameraBlock =
        Camera::getBlockAt(level, cameraEntity, partialTick);
    Material* blockMaterial = cameraBlock->getMaterial();

    unsigned char* underwaterActive =
        reinterpret_cast<unsigned char*>(reinterpret_cast<char*>(this) + 0x21fc);
    if (!blockMaterial->isUnderWater())
    {
        *underwaterActive = 0;
        if (blockMaterial != Material::LAVA)
        {
            float* blurO = reinterpret_cast<float*>(reinterpret_cast<char*>(this) + 0x21f4);
            float* blur = reinterpret_cast<float*>(reinterpret_cast<char*>(this) + 0x21f8);
            const float blurScale = (*blur - *blurO) * partialTick + *blurO;
            *fogRed *= blurScale;
            *fogGreen *= blurScale;
            *fogBlue *= blurScale;
        }
        else
        {
            const unsigned int damageColor =
                Minecraft::GetInstance()->getColourTable()->getColour(
                    static_cast<eMinecraftColour>(0x184));
            float* blurO = reinterpret_cast<float*>(reinterpret_cast<char*>(this) + 0x21f4);
            float* blur = reinterpret_cast<float*>(reinterpret_cast<char*>(this) + 0x21f8);
            const float blurScale = (*blur - *blurO) * partialTick + *blurO;
            *fogRed = static_cast<float>(((damageColor >> 16) & 255) - kColorNormalize) * kColorScale * blurScale;
            *fogGreen = static_cast<float>(((damageColor >> 8) & 255) - kColorNormalize) * kColorScale * blurScale;
            *fogBlue = static_cast<float>((damageColor & 255) - kColorNormalize) * kColorScale * blurScale;
        }
    }
    else
    {
        MultiplayerLocalPlayer* localPlayer =
            dynamic_cast<MultiplayerLocalPlayer*>(getCamera());
        const int playerIndex = localPlayer != NULL ? localPlayer->mUserIndex : 0;
        const int playerOffset = playerIndex * 4;

        float targetRed = 0.0f;
        float targetGreen = 0.0f;
        float targetBlue = 0.0f;
        BlockPos cameraBlockPos(cameraEntity);
        getTargetFogColor(cameraBlockPos, targetRed, targetGreen, targetBlue);

        float* storedTargetRed =
            reinterpret_cast<float*>(reinterpret_cast<char*>(this) + 0x21b4 + playerOffset);
        float* storedTargetGreen =
            reinterpret_cast<float*>(reinterpret_cast<char*>(this) + 0x21c4 + playerOffset);
        float* storedTargetBlue =
            reinterpret_cast<float*>(reinterpret_cast<char*>(this) + 0x21d4 + playerOffset);
        if (targetRed != *storedTargetRed || targetGreen != *storedTargetGreen
            || targetBlue != *storedTargetBlue)
        {
            *storedTargetRed = targetRed;
            *storedTargetGreen = targetGreen;
            *storedTargetBlue = targetBlue;
            *reinterpret_cast<long long*>(reinterpret_cast<char*>(this) + 0x2200 + playerIndex * 8) =
                System::processTimeInMilliSecs();
        }

        double* targetFogDistance =
            reinterpret_cast<double*>(reinterpret_cast<char*>(this) + 0x2240);
        if (*underwaterActive == 0)
        {
            const int seaLevel = level->getSeaLevel();
            const double cameraY = cameraEntity->mY;
            if (cameraY <= static_cast<double>(-seaLevel))
                *targetFogDistance = static_cast<double>(-seaLevel);
            else
                *targetFogDistance = cameraY;
        }

        float* currentRed =
            reinterpret_cast<float*>(reinterpret_cast<char*>(this) + 0x2174 + playerOffset);
        float* currentGreen =
            reinterpret_cast<float*>(reinterpret_cast<char*>(this) + 0x2184 + playerOffset);
        float* currentBlue =
            reinterpret_cast<float*>(reinterpret_cast<char*>(this) + 0x2194 + playerOffset);
        float* currentDistance =
            reinterpret_cast<float*>(reinterpret_cast<char*>(this) + 0x21a4 + playerOffset);

        if (*underwaterActive == 0)
        {
            *currentRed = targetRed;
            *currentGreen = targetGreen;
            *currentBlue = targetBlue;
            *currentDistance = static_cast<float>(
                modifyFogDistance(cameraBlockPos,
                                  static_cast<float>(*targetFogDistance),
                                  partialTick));
        }

        const long long now = System::processTimeInMilliSecs();
        long long* colorLerpStart =
            reinterpret_cast<long long*>(reinterpret_cast<char*>(this) + 0x2200 + playerIndex * 8);
        const float colorLerp = Mth::clamp(
            static_cast<float>((now - *colorLerpStart) / kFogLerpMs), 0.0f, kOne);
        *currentRed = Mth::Lerp(*currentRed, targetRed, colorLerp);
        *currentGreen = Mth::Lerp(*currentGreen, targetGreen, colorLerp);
        *currentBlue = Mth::Lerp(*currentBlue, targetBlue, colorLerp);
        *fogRed = *currentRed;
        *fogGreen = *currentGreen;
        *fogBlue = *currentBlue;

        const float modifiedDistance = static_cast<float>(
            modifyFogDistance(cameraBlockPos,
                              static_cast<float>(*targetFogDistance),
                              partialTick));
        float* targetDistance =
            reinterpret_cast<float*>(reinterpret_cast<char*>(this) + 0x21e4 + playerOffset);
        if (modifiedDistance == *targetDistance)
        {
            long long* distanceLerpStart =
                reinterpret_cast<long long*>(reinterpret_cast<char*>(this) + 0x2220 + playerIndex * 8);
            const float distanceLerp = Mth::clamp(
                static_cast<float>((now - *distanceLerpStart) / kFogLerpMs), 0.0f, kOne);
            *currentDistance = Mth::Lerp(*currentDistance, *targetDistance, distanceLerp);
        }
        else
        {
            *targetDistance = modifiedDistance;
            *reinterpret_cast<long long*>(reinterpret_cast<char*>(this) + 0x2220 + playerIndex * 8) =
                now;
            *currentDistance = Mth::Lerp(*currentDistance, *targetDistance, 0.0f);
        }

        float fogStart = kOne;
        float fogEnd = static_cast<float>(modifiedDistance);
        if (localPlayer != NULL
            && localPlayer->getEffect(MobEffects::WATER_BREATHING) != NULL
            && blockMaterial->isUnderWater())
        {
            const double clarity = localPlayer->getUnderwaterVisionClarity();
            if (clarity < kOne)
            {
                const float mix = static_cast<float>(kUnderwaterBrighten * clarity + kUnderwaterMix);
                fogEnd = static_cast<float>(fogEnd * mix);
                fogStart = kOne * mix;
            }
        }

        GlStateManager::fogStart(fogStart + kFogStartOffset);
        GlStateManager::fogEnd(fogEnd + kFogEndOffset + 6.0f);
        GlStateManager::fogDensity(kFogDensity);
        *underwaterActive = 1;
    }

    const double clearScale = level->mDimension->getClearColorScale();
    double brightness = ((cameraEntity->mY - cameraEntity->mYOld) * partialTick + cameraEntity->mYOld)
        * clearScale;
    if (living->hasEffect(MobEffects::BLINDNESS))
    {
        MobEffectInstance* effect = living->getEffect(MobEffects::BLINDNESS);
        if (effect->getDuration() < 20)
            brightness *= kOne - static_cast<float>((20 - effect->getDuration()) / 20.0f);
    }
    if (brightness < kMinBrightness)
        brightness = kMinBrightness;
    brightness *= brightness;
    *fogRed = static_cast<float>(*fogRed * brightness);
    *fogGreen = static_cast<float>(*fogGreen * brightness);
    *fogBlue = static_cast<float>(*fogBlue * brightness);

    float* damageFlash = reinterpret_cast<float*>(reinterpret_cast<char*>(this) + 0x114);
    float* damageFlashO = reinterpret_cast<float*>(reinterpret_cast<char*>(this) + 0x118);
    if (*damageFlash > kHalf)
    {
        const float damageAmount =
            (*damageFlash - *damageFlashO) * partialTick + *damageFlashO;
        const float keep = kOne - damageAmount;
        *fogRed = *fogRed * keep + *fogRed * kGreyBoost * damageAmount;
        *fogGreen = *fogGreen * keep + *fogGreen * kDamageBlend * damageAmount;
        *fogBlue = *fogBlue * keep + *fogBlue * kDamageBlend * damageAmount;
    }

    float red = *fogRed;
    float green = *fogGreen;
    float blue = *fogBlue;
    if (living->hasEffect(MobEffects::NIGHT_VISION)
        || living->hasEffect(MobEffects::CONDUIT_POWER)
        || living->hasEffect(MobEffects::GLOWING))
    {
        const double nightVision =
            getNightVisionScale(living, partialTick);
        float minScale = kNightVisionCap;
        if (red > kHalf && kOne / red < minScale)
            minScale = kOne / red;
        if (green > kHalf && kOne / green < minScale)
            minScale = kOne / green;
        if (blue > kHalf && kOne / blue < minScale)
            minScale = kOne / blue;
        const float keep = kOne - static_cast<float>(nightVision);
        red = static_cast<float>(red * keep + red * minScale * nightVision);
        green = static_cast<float>(green * keep + green * minScale * nightVision);
        blue = static_cast<float>(blue * keep + blue * minScale * nightVision);
        *fogRed = red;
        *fogGreen = green;
        *fogBlue = blue;
    }

    if (mMinecraft->mOptions != NULL
        && *reinterpret_cast<unsigned char*>(reinterpret_cast<char*>(mMinecraft->mOptions) + 0x15) != 0)
    {
        const float sum = red * 0.30f + green * 0.59f + blue * 0.11f;
        *fogRed = (blue * 0.30f + green * 0.59f + red * 0.11f) / kColorBlindDivisor;
        *fogGreen = (sum + red) / kColorBlindDivisor;
        *fogBlue = (sum + blue * 0.70f) / kColorBlindDivisor;
    }

    GlStateManager::clearColor(*fogRed, *fogGreen, *fogBlue, kOne);
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

    static ResourceLocation sThermalDefault(L"terrain");
    static ResourceLocation sThermalTinted(L"terrain");
    static ResourceLocation sThermalBoost(L"terrain");
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

    AbstractClientPlayer* camera = getCamera();
    if (camera == NULL)
    {
        return;
    }

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
    static const float kAlphaReference = 1.875f;
    static const float kDepthStepNeg = -0.0078125f;
    static const float kDepthStepPos = 0.00390625f;
    static const float kWidthExpand = 4.0f;
    static const float kHeightPad = 8.0f;
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

    int rainTime;
    std::memcpy(&rainTime, reinterpret_cast<char*>(this) + 0x5C, sizeof(rainTime));
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
                                static_cast<eMinecraftColour>(0x1CF));
                            boundThermalTexture = &sThermalTinted;
                        }
                    }
                    else
                    {
                        Minecraft::GetInstance()->getColourTable()->getColour(
                            static_cast<eMinecraftColour>(0x1D0));
                        boundThermalTexture = &sThermalTinted;
                    }
                }
                else
                {
                    Minecraft::GetInstance()->getColourTable()->getColour(
                        static_cast<eMinecraftColour>(0x1CE));
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
                        static_cast<float>(thermalVertsZ[next]), static_cast<float>(yCoord),
                        static_cast<float>(zBase), static_cast<float>(thermalVertsX[next]),
                        static_cast<float>(thermalVertsZ[next]), static_cast<float>(yCoord + edgeLength / perimeterAvg),
                        static_cast<float>(xBase), static_cast<float>(thermalVertsX[edge]),
                        static_cast<float>(thermalVertsZ[edge]), static_cast<float>(yCoord + edgeLength / perimeterAvg),
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
                    Minecraft::GetInstance()->getColourTable()->getColour(static_cast<eMinecraftColour>(0x1D1));
                }
                else
                {
                    Minecraft::GetInstance()->getColourTable()->getColour(static_cast<eMinecraftColour>(0x1D2));
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
                        float depthStep = kDepthStepPos;
                        if (slice == 0 || slice == 2)
                        {
                            depthStep = kDepthStepNeg;
                            if (boostDirection == 1 || boostDirection == 5)
                            {
                                depthStep = kHeightPad;
                            }
                        }
                        else if (boostDirection == 5)
                        {
                            depthStep = kHeightPad;
                        }

                        const float phaseTop = rowPhase + depthStep;
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
    static const float kColourBase = 176.0f;
    static const float kColourDivisor = 255.0f;
    static const float kPolygonOffset = -0.30000001192092896f;

    ColourTable* colourTable = mMinecraft->getColourTable();
    const int startColour = colourTable->getColour(static_cast<eMinecraftColour>(0x1F6));
    const int middleColour = colourTable->getColour(static_cast<eMinecraftColour>(0x1F7));
    const int endColour = colourTable->getColour(static_cast<eMinecraftColour>(0x1F8));

    GhostController* ghostController = mMinecraft->GetGhostController();
    LevelGenerationOptions* generationOptions = g_pApp->getLevelGenerationOptions();
    if (ghostController == NULL || generationOptions == NULL)
    {
        return;
    }

    boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);
    const int trackIndex =
        *reinterpret_cast<int*>(reinterpret_cast<char*>(player.get()) + 0x960);

    char* ghostBytes = reinterpret_cast<char*>(ghostController);
    GhostController::GhostRecord* record = *reinterpret_cast<GhostController::GhostRecord**>(
        ghostBytes + 0xD4 + trackIndex * 4);
    if (record == NULL || !record->HasRecording() || !ghostController->HasFinished(record))
    {
        return;
    }

    if (ghostBytes[0x81 + trackIndex] == 0)
    {
        return;
    }

    float playbackFraction = kOne;
    unsigned int sampleIndex = ghostController->getPlaybackIndex(record, playbackFraction);

    Vec3* samplePos = reinterpret_cast<Vec3*>(uintptr_t)record->GetPosAtIndex(sampleIndex);
    Vec3* sampleRot = reinterpret_cast<Vec3*>(uintptr_t)record->GetRotAtIndex(sampleIndex);
    double sampleVelocity = record->GetVelocityAtIndex(sampleIndex);

    Vec3* renderPos = samplePos;
    Vec3* renderRot = sampleRot;
    float flyAmount = kOne;
    const unsigned int sampleCount = static_cast<unsigned int>(record->NumSamples());
    if (sampleIndex + 1U < sampleCount)
    {
        Vec3* nextRot = reinterpret_cast<Vec3*>(uintptr_t)record->GetRotAtIndex(sampleIndex + 1U);
        const double nextVelocity = record->GetVelocityAtIndex(sampleIndex + 1U);
        Vec3* nextPos =
            reinterpret_cast<Vec3*>(uintptr_t)record->GetPosAtIndex(sampleIndex + 1U);

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
            const double lift = std::pow(-delta->y, kPowBase);
            flyAmount = kOne - static_cast<float>(lift);
        }
    }

    const double cameraX =
        (*reinterpret_cast<double*>(reinterpret_cast<char*>(player.get()) + 0x118) -
         *reinterpret_cast<double*>(reinterpret_cast<char*>(player.get()) + 0x1C0)) *
            partialTick +
        *reinterpret_cast<double*>(reinterpret_cast<char*>(player.get()) + 0x1C0);
    const double cameraY =
        (*reinterpret_cast<double*>(reinterpret_cast<char*>(player.get()) + 0x120) -
         *reinterpret_cast<double*>(reinterpret_cast<char*>(player.get()) + 0x1C8)) *
            partialTick +
        *reinterpret_cast<double*>(reinterpret_cast<char*>(player.get()) + 0x1C8);
    const double cameraZ =
        (*reinterpret_cast<double*>(reinterpret_cast<char*>(player.get()) + 0x128) -
         *reinterpret_cast<double*>(reinterpret_cast<char*>(player.get()) + 0x1D0)) *
            partialTick +
        *reinterpret_cast<double*>(reinterpret_cast<char*>(player.get()) + 0x1D0);

    const float velocityAlpha =
        Mth::clamp(static_cast<float>(sampleVelocity * kDepthOffset), kZero, kOne);
    const unsigned int blendedColour =
        ColourTable::LinearBlend(startColour, middleColour, endColour, velocityAlpha);

    Vec3* playerPos = Vec3::newTemp(
        *reinterpret_cast<double*>(reinterpret_cast<char*>(player.get()) + 0x118),
        *reinterpret_cast<double*>(reinterpret_cast<char*>(player.get()) + 0x120),
        *reinterpret_cast<double*>(reinterpret_cast<char*>(player.get()) + 0x128));
    const float distanceAlpha = Mth::clamp(
        static_cast<float>(playerPos->distanceTo(renderPos) * kHalf), kZero, kOne);

    EntityRenderDispatcher* dispatcher = mMinecraft->getEntityRenderDispatcher();
    PlayerRenderer* playerRenderer =
        dispatcher->getPlayerRenderer(static_cast<EPlayerModelType>(1));
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
        static_cast<float>((static_cast<double>((blendedColour >> 16) & 0xFF) - kColourBase) /
                         kColourDivisor);
    const float green =
        static_cast<float>((static_cast<double>((blendedColour >> 8) & 0xFF) - kColourBase) /
                         kColourDivisor);
    const float blue =
        static_cast<float>((static_cast<double>((blendedColour >> 0) & 0xFF) - kColourBase) /
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
    static const float kZero = 0.0f;
    static const float kOne = 1.0f;
    static const float kHalf = 0.5f;
    static const float kLineWidth = 0.5f;
    static const float kVelocityScale = 0.125f;
    static const float kMaxDistance = 10.0f;
    static const float kAlphaScale = 255.0f;
    static const float kAlphaFactor = 0.04f;
    static const float kGapDistance = 20.0f;
    static const unsigned int kBatchSize = 1000U;

    ColourTable* colourTable = mMinecraft->getColourTable();
    const int startColour = colourTable->getColour(static_cast<eMinecraftColour>(0x1F6));
    const int middleColour = colourTable->getColour(static_cast<eMinecraftColour>(0x1F7));
    const int endColour = colourTable->getColour(static_cast<eMinecraftColour>(0x1F8));

    GhostController* ghostController = mMinecraft->GetGhostController();
    if (ghostController == NULL)
    {
        return;
    }

    boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);
    if (player.get() == NULL)
    {
        return;
    }

    const int trackIndex =
        *reinterpret_cast<int*>(reinterpret_cast<char*>(player.get()) + 0x960);
    char* ghostBytes = reinterpret_cast<char*>(ghostController);
    GhostController::GhostRecord* record = *reinterpret_cast<GhostController::GhostRecord**>(
        ghostBytes + 0xD4 + trackIndex * 4);
    if (record == NULL || !record->HasRecording())
    {
        return;
    }

    if (ghostBytes[0x81 + trackIndex] == 0)
    {
        return;
    }

    const unsigned int sampleCount = static_cast<unsigned int>(record->NumSamples());
    if (sampleCount == 0U)
    {
        return;
    }

    std::vector<PositionStruct> positions;
    std::vector<int> colors;
    std::vector<int> alphas;
    positions.reserve(sampleCount);
    colors.reserve(sampleCount);
    alphas.reserve(sampleCount);

    const double cameraX =
        (*reinterpret_cast<double*>(reinterpret_cast<char*>(player.get()) + 0x118) -
         *reinterpret_cast<double*>(reinterpret_cast<char*>(player.get()) + 0x1C0)) *
            partialTick +
        *reinterpret_cast<double*>(reinterpret_cast<char*>(player.get()) + 0x1C0);
    const double cameraY =
        (*reinterpret_cast<double*>(reinterpret_cast<char*>(player.get()) + 0x120) -
         *reinterpret_cast<double*>(reinterpret_cast<char*>(player.get()) + 0x1C8)) *
            partialTick +
        *reinterpret_cast<double*>(reinterpret_cast<char*>(player.get()) + 0x1C8);
    const double cameraZ =
        (*reinterpret_cast<double*>(reinterpret_cast<char*>(player.get()) + 0x128) -
         *reinterpret_cast<double*>(reinterpret_cast<char*>(player.get()) + 0x1D0)) *
            partialTick +
        *reinterpret_cast<double*>(reinterpret_cast<char*>(player.get()) + 0x1D0);

    GlStateManager::pushMatrix();
    GlStateManager::translatef(static_cast<float>(-cameraX), static_cast<float>(-cameraY),
                               static_cast<float>(-cameraZ));

    Vec3* playerPos = Vec3::newTemp(
        *reinterpret_cast<double*>(reinterpret_cast<char*>(player.get()) + 0x118),
        *reinterpret_cast<double*>(reinterpret_cast<char*>(player.get()) + 0x120),
        *reinterpret_cast<double*>(reinterpret_cast<char*>(player.get()) + 0x128));

    for (unsigned int sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex)
    {
        const float velocityAlpha = Mth::clamp(
            static_cast<float>(record->GetVelocityAtIndex(sampleIndex) * kVelocityScale), kZero, kOne);
        const int blendedColour =
            ColourTable::LinearBlend(startColour, middleColour, endColour, velocityAlpha);

        Vec3* samplePos = reinterpret_cast<Vec3*>(uintptr_t)record->GetPosAtIndex(sampleIndex);
        const float distance =
            Mth::clamp(static_cast<float>(playerPos->distanceTo(samplePos)), kZero, kMaxDistance);
        int alpha = static_cast<int>(kAlphaScale * distance * kAlphaFactor);

        if (sampleIndex != 0U && sampleIndex + 1U < sampleCount)
        {
            Vec3* previousPos =
                reinterpret_cast<Vec3*>(uintptr_t)record->GetPosAtIndex(sampleIndex - 1U);
            Vec3* nextPos = reinterpret_cast<Vec3*>(uintptr_t)record->GetPosAtIndex(sampleIndex + 1U);
            if (previousPos->distanceTo(samplePos) > kGapDistance
                || samplePos->distanceTo(nextPos) > kGapDistance)
            {
                alpha = 0;
            }
        }

        PositionStruct position;
        position.x = static_cast<float>(samplePos->x);
        position.y = static_cast<float>(samplePos->y);
        position.z = static_cast<float>(samplePos->z);
        positions.push_back(position);
        colors.push_back(blendedColour);
        alphas.push_back(alpha);

        if (positions.size() == kBatchSize)
        {
            LevelRenderer::renderLine(&positions, &colors, &alphas, kLineWidth);
            positions.clear();
            colors.clear();
            alphas.clear();
        }
    }

    if (positions.size() > 1U)
    {
        LevelRenderer::renderLine(&positions, &colors, &alphas, kLineWidth);
    }

    GlStateManager::popMatrix();
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

void GameRenderer::renderEyePass(int eyeIndex, float partialTick, long long, bool sortTransparent)
{
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

    static ResourceLocation sTerrainTexture(L"terrain");
    MemSect(0x1f);
    mMinecraft->mTextures->bindTexture(&sTerrainTexture);
    MemSect(0);
    Lighting::turnOff();

    InfoBarManager::StartTimingCategory(InfoBarManager::eTimingCagegory_Terrain);
    mMinecraft->mLevelRenderer->render(cameraEntity, 0, partialTick, sortTransparent);
    InfoBarManager::EndTimingCategory(InfoBarManager::eTimingCagegory_Terrain);

    GlStateManager::alphaFunc(4, 0.009999999776482582f);

    int renderEntitiesFlag;
    std::memcpy(&renderEntitiesFlag, reinterpret_cast<char*>(this) + 0x22D8, sizeof(renderEntitiesFlag));
    if (renderEntitiesFlag == 0)
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

    if (mMinecraft->mHitResult != NULL)
    {
        boost::shared_ptr<Player> outlinePlayer = boost::static_pointer_cast<Player>(cameraEntity);
        mMinecraft->mLevelRenderer->renderHitOutline(outlinePlayer, mMinecraft->mHitResult, 0,
                                                     partialTick);
    }

    renderThermal(culler, partialTick);

    if (Minecraft::InMiniGame(eMiniGameId_Glide, true))
    {
        MiniGameDef* miniGame = mMinecraft->GetMiniGame();
        if (miniGame != NULL && miniGame->GetHostOption(static_cast<eCustomGameModeHostOption>(0xE)) != 0)
        {
            const int ghostSetting =
                GameSettings::GetGameSettings(mMinecraft->mPlayer->mUserIndex,
                                              static_cast<eGameSetting>(0x2A));
            if (ghostSetting != 0 && g_pApp->IsAppPaused() == 0 && !mMinecraft->mPlayer->isSpectator())
            {
                renderGhost(partialTick, cameraEntity);
            }

            const int ghostPathSetting =
                GameSettings::GetGameSettings(mMinecraft->mPlayer->mUserIndex,
                                              static_cast<eGameSetting>(0x2B));
            if (ghostPathSetting != 0 && !mMinecraft->mPlayer->isSpectator())
            {
                renderGhostPath(partialTick, cameraEntity);
            }
        }
    }

    boost::shared_ptr<Player> destroyPlayer = boost::static_pointer_cast<Player>(cameraEntity);
    mMinecraft->mLevelRenderer->renderDestroyAnimation(Tesselator::getInstance(), destroyPlayer,
                                                         partialTick);

    if (headY < static_cast<double>(mMinecraft->mLevel->getCloudHeight()))
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
