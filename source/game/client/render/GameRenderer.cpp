#include "game/client/render/GameRenderer.hpp"
#include "game/client/render/MapRenderer.hpp"
#include "game/core/Options.hpp"
#include "game/core/GameType.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/core/HitResult.hpp"
#include "game/core/FloatBuffer.hpp"
#include "game/core/MemoryTracker.hpp"
#include "game/core/Minecraft.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Inventory.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/generation/Player.hpp"
#include "game/core/Minecraft.hpp"
#include "game/world/generation/AbstractClientPlayer.hpp"

#include <cstring>

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
    return;
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
    return static_cast<double>(amount);
}

double GameRenderer::getCameraXRot(float amount)
{
    return static_cast<double>(amount);
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

void GameRenderer::renderSnowAndRain(float)
{
    return;
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
    return;
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
