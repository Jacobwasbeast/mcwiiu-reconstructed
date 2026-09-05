#include "game/client/render/GameRenderer.hpp"

#include <cstring>

GameRenderer::GameRenderer(Minecraft* minecraft, ResourceManager* resourceManager)
    : mMinecraft(minecraft), mResourceManager(resourceManager), mRandom(), mFov(), mFog(), mCamera(), mZoom(), mView(), mLight()
{
    std::memset(mPrefix, 0, sizeof(mPrefix));
    std::memset(mRandomTail, 0, sizeof(mRandomTail));
    std::memset(mData, 0, sizeof(mData));
}

GameRenderer::~GameRenderer()
{
    return;
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
    return 0;
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

unsigned int GameRenderer::getBuffer(float, float, float, float)
{
    return 0;
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
    return 0;
}

void GameRenderer::resetData()
{
    return;
}

MapRenderer* GameRenderer::getMapRenderer()
{
    return 0;
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
