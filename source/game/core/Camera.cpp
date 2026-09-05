#include "game/core/Camera.hpp"

#include <cmath>

#include "game/core/CameraController.hpp"
#include "game/core/FloatBuffer.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/LiquidBlock.hpp"
#include "game/world/block/Material.hpp"

extern FloatBuffer* gCameraModelViewMatrix;
extern FloatBuffer* gCameraProjectionMatrix;
extern float gCameraOffsetsX[8];
extern float gCameraOffsetsY[8];
extern float gCameraOffsetsZ[8];
extern float gCameraPositionX;
extern float gCameraPositionY;
extern float gCameraPositionZ;
extern float gCameraForwardX;
extern float gCameraForwardY;
extern float gCameraForwardZ;
extern float gCameraRightX;
extern float gCameraRightY;
extern float gCameraRightZ;
extern float gCameraUpX;
extern float gCameraUpY;
extern float gCameraUpZ;

extern void ASM_MTX44Concat(float* first, float* second, float* result);
extern void C_MTX44Inverse(float* matrix, float* inverse);

extern CameraController* cameraGetController(Player* player) __asm("GetCameraController__6PlayerFv");
extern int cameraGetThirdPersonView(Player* player, bool includeCamera) __asm("GetThirdPersonView__6PlayerFb");

static Vec3* cameraPosition(Entity* entity, double partialTick)
{
    return Vec3::newTemp(
        (entity->mX - entity->mXOld) * partialTick + entity->mXOld + gCameraPositionX,
        (entity->mY - entity->mYOld) * partialTick + entity->mYOld + gCameraPositionY,
        (entity->mZ - entity->mZOld) * partialTick + entity->mZOld + gCameraPositionZ);
}

void Camera::prepare(boost::shared_ptr<Player> player, bool secondPass)
{
    GlStateManager::getMatrix(0, gCameraModelViewMatrix);
    GlStateManager::getMatrix(1, gCameraProjectionMatrix);

    float modelView[16];
    float projection[16];
    float combined[16];
    float inverse[16];
    int index;
    for (index = 0; index < 16; ++index)
    {
        modelView[index] = gCameraModelViewMatrix->get(index);
        projection[index] = gCameraProjectionMatrix->get(index);
    }
    ASM_MTX44Concat(modelView, projection, combined);
    C_MTX44Inverse(combined, inverse);
    gCameraPositionX = inverse[12] / inverse[15];
    gCameraPositionY = inverse[13] / inverse[15];
    gCameraPositionZ = inverse[14] / inverse[15];

    Entity* entity = (Entity*)player.get();
    float xRotation = entity->mXRot;
    float yRotation = entity->mYRot;
    CameraController* controller = cameraGetController(player.get());
    if (controller != NULL && cameraGetThirdPersonView(player.get(), false) == 3)
    {
        Vec3* rotation = controller->getCurrentRot();
        xRotation += (float)rotation->x;
        yRotation += (float)rotation->y;
    }

    float degreesToRadians = 0.01745329238474369f;
    float yaw = yRotation * degreesToRadians;
    float pitch = xRotation * degreesToRadians;
    float side = (float)(secondPass ? -1 : 1);
    float cosYaw = cosf(yaw);
    float sinYaw = sinf(yaw);
    float cosPitch = cosf(pitch);
    float sinPitch = sinf(pitch);

    gCameraForwardX = cosYaw * cosPitch * side;
    gCameraForwardY = -sinPitch * side;
    gCameraForwardZ = sinYaw * cosPitch * side;
    gCameraRightX = sinYaw * side;
    gCameraRightY = 0.0f;
    gCameraRightZ = cosYaw * side;
    gCameraUpX = -sinPitch * cosYaw * side;
    gCameraUpY = -cosPitch * side;
    gCameraUpZ = -sinPitch * sinYaw * side;
    UpdatePlayerOffsets(player);
}

Vec3* Camera::getCameraPos(boost::shared_ptr<Entity> entity, double partialTick)
{
    return cameraPosition(entity.get(), partialTick);
}

BlockPos Camera::getCameraBlockPos(boost::shared_ptr<LivingEntity> entity, double partialTick)
{
    const boost::shared_ptr<Entity>& base = (const boost::shared_ptr<Entity>&)entity;
    return BlockPos(getCameraPos(base, partialTick));
}

Block* Camera::getBlockAt(Level* level, boost::shared_ptr<Entity> entity, float partialTick)
{
    LevelSource* levelSource = (LevelSource*)level;
    Vec3* position = getCameraPos(entity, partialTick);
    BlockPos blockPos(position);
    const BlockState* state = levelSource->getBlockState(blockPos);
    unsigned short extraElement = levelSource->getExtraElement(blockPos.x, blockPos.y, blockPos.z);
    if (extraElement != 0)
    {
        state = Block::getStateByElementId(extraElement);
    }

    Block* block = state->getBlock();
    float height = 0.0f;
    if (state->getMaterial()->isLiquid() && block->isLiquidBlock())
    {
        const BlockState* liquidState = levelSource->getLiquidBlockState(blockPos);
        if (liquidState != NULL && liquidState->getBlock() == block)
        {
            height = (float)(LiquidBlock::getHeightFromData(
                levelSource->getBlockData(blockPos.x, blockPos.y, blockPos.z)) - 0.1111111119389534);
        }
    }

    if ((float)(blockPos.y + 1) - height <= position->y)
    {
        block = levelSource->getBlock(blockPos.above());
    }
    return block;
}

void Camera::UpdatePlayerOffsets(boost::shared_ptr<Player> player)
{
    Entity* entity = (Entity*)player.get();
    int dimension = entity->mDimension;
    gCameraPositionX = gCameraOffsetsX[dimension];
    gCameraPositionY = gCameraOffsetsY[dimension];
    gCameraPositionZ = gCameraOffsetsZ[dimension];
}
