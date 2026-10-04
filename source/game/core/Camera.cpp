#include "game/core/Camera.hpp"

#include <cmath>
#include <cstring>

#include "game/core/CameraController.hpp"
#include "game/core/FloatBuffer.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/MemoryTracker.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/LiquidBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"

FloatBuffer* gCameraModelViewMatrix = MemoryTracker::createFloatBuffer(16);
FloatBuffer* gCameraProjectionMatrix = MemoryTracker::createFloatBuffer(16);
float gCameraPositionX = 0.0f;
float gCameraPositionY = 0.0f;
float gCameraPositionZ = 0.0f;
float gCameraXOff = 0.0f;
float gCameraYOff = 0.0f;
float gCameraZOff = 0.0f;
float gCameraYOff2 = 0.0f;
float gCameraXOff2 = 0.0f;
float gCameraOffsetsX[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
float gCameraOffsetsY[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
float gCameraOffsetsZ[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

extern "C" void ASM_MTX44Concat(float* first, float* second, float* result);
extern "C" void C_MTX44Inverse(float* matrix, float* inverse);

void Camera::prepare(boost::shared_ptr<Player> player, bool secondPass)
{
    GlStateManager::getMatrix(0, gCameraModelViewMatrix);
    GlStateManager::getMatrix(1, gCameraProjectionMatrix);

    float modelView[16];
    float projection[16];
    float combined[16];
    float inverse[16];
    std::memcpy(modelView, gCameraModelViewMatrix->mBuffer, sizeof(modelView));
    std::memcpy(projection, gCameraProjectionMatrix->mBuffer, sizeof(projection));
    ASM_MTX44Concat(modelView, projection, combined);
    C_MTX44Inverse(combined, inverse);
    gCameraPositionX = inverse[12] / inverse[15];
    gCameraPositionY = inverse[13] / inverse[15];
    gCameraPositionZ = inverse[14] / inverse[15];

    CameraController* controller = player->GetCameraController();
    float xRotation = player->mXRot;
    float yRotation = player->mYRot;
    if (controller != NULL && player->GetThirdPersonView(false) == 3)
    {
        xRotation += (float)controller->getCurrentRot()->x;
        yRotation += (float)controller->getCurrentRot()->y;
    }

    float side = (float)(1 - 2 * (int)secondPass);
    float yaw = yRotation * 0.017453292f;
    float pitch = xRotation * 0.017453292f;

    gCameraXOff = cosf(yaw) * side;
    gCameraZOff = sinf(yaw) * side;
    gCameraYOff2 = -gCameraZOff * sinf(pitch) * side;
    gCameraXOff2 = gCameraXOff * sinf(pitch) * side;
    gCameraYOff = cosf(pitch);

    MultiplayerLocalPlayer* localPlayer = dynamic_cast<MultiplayerLocalPlayer*>(player.get());
    gCameraOffsetsX[localPlayer->mUserIndex] = gCameraPositionX;
    gCameraOffsetsY[localPlayer->mUserIndex] = gCameraPositionY;
    gCameraOffsetsZ[localPlayer->mUserIndex] = gCameraPositionZ;
}

Vec3* Camera::getCameraPos(boost::shared_ptr<Entity> entity, double partialTick)
{
    return Vec3::newTemp(
        (entity->mX - entity->mXo) * partialTick + entity->mXo + gCameraPositionX,
        (entity->mY - entity->mYo) * partialTick + entity->mYo + gCameraPositionY,
        (entity->mZ - entity->mZo) * partialTick + entity->mZo + gCameraPositionZ);
}

BlockPos Camera::getCameraBlockPos(boost::shared_ptr<LivingEntity> entity, double partialTick)
{
    return BlockPos(getCameraPos(entity, partialTick));
}

const BlockState* Camera::getBlockAt(Level* level, boost::shared_ptr<Entity> entity, float partialTick)
{
    Vec3* position = getCameraPos(entity, partialTick);
    BlockPos blockPos(position);
    const BlockState* state = level->getBlockState(blockPos);
    unsigned short extraElement = level->getExtraElement(blockPos.x, blockPos.y, blockPos.z);
    if (extraElement != 0)
    {
        state = Block::getStateByElementId(extraElement);
    }

    if (state->getMaterial()->isLiquid())
    {
        float height = 0.0f;
        if (state->getBlock()->isLiquidBlock())
        {
            height = LiquidBlock::getHeightFromData(state->getValue(LiquidBlock::LEVEL)) - 0.11111111f;
        }

        if ((float)(blockPos.y + 1) - height <= position->y)
        {
            state = level->getBlockState(blockPos.above());
        }
    }
    return state;
}

void Camera::UpdatePlayerOffsets(boost::shared_ptr<Player> player)
{
    MultiplayerLocalPlayer* localPlayer = dynamic_cast<MultiplayerLocalPlayer*>(player.get());
    gCameraPositionX = gCameraOffsetsX[localPlayer->mUserIndex];
    gCameraPositionY = gCameraOffsetsY[localPlayer->mUserIndex];
    gCameraPositionZ = gCameraOffsetsZ[localPlayer->mUserIndex];
}
