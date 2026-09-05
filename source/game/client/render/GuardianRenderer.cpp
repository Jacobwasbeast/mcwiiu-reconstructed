#include "game/client/render/GuardianRenderer.hpp"

#include <cmath>

#include <boost/shared_ptr.hpp>

#include "game/client/render/GuardianModel.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/Culler.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Guardian.hpp"
#include "game/world/Level.hpp"
#include "game/util/AABB.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/util/Vec3.hpp"

static ResourceLocation GUARDIAN_LOCATION;

GuardianRenderer::GuardianRenderer(EntityRenderDispatcher* dispatcher)
    : MobRenderer(dispatcher, new GuardianModel(ModelDefinition()), 0.5f)
{
}

GuardianRenderer::~GuardianRenderer()
{
}

Vec3* GuardianRenderer::getPosition(boost::shared_ptr<Entity>& entity, double yOffset, float partialTicks)
{
    return Vec3::newTemp((entity->mXo - entity->mX) * partialTicks + entity->mX,
                         (entity->mYo - entity->mY) * partialTicks + entity->mY + yOffset,
                         (entity->mZo - entity->mZ) * partialTicks + entity->mZ);
}

bool GuardianRenderer::shouldRender(boost::shared_ptr<Entity>& entity, Culler* culler,
                                    double x, double y, double z)
{
    if (MobRenderer::shouldRender(entity, culler, x, y, z))
        return true;

    boost::shared_ptr<Guardian> guardian = boost::static_pointer_cast<Guardian>(entity);
    if (!guardian->hasActiveAttackTarget())
        return false;

    boost::shared_ptr<Entity> target = guardian->getActiveAttackTarget();
    if (!target)
        return false;

    Vec3* source = getPosition(entity, 1.0, 1.75f);
    Vec3* destination = getPosition(target, 1.0, 1.75f);
    return culler->isVisible(AABB::minmax(destination->x, destination->y, destination->z,
                                           source->x, source->y, source->z));
}

void GuardianRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z,
                              float yaw, float partialTick)
{
    MobRenderer::render(entity, x, y, z, yaw, partialTick);

    boost::shared_ptr<Guardian> guardian = boost::static_pointer_cast<Guardian>(entity);
    boost::shared_ptr<Entity> target = guardian->getActiveAttackTarget();
    if (!target)
        return;

    float attackScale = guardian->getAttackAnimationScale(partialTick);
    Tesselator* tesselator = Tesselator::getInstance();
    BufferBuilder* builder = tesselator->getBuilder();
    bindTexture(&GUARDIAN_LOCATION);
    GlStateManager::texParameteri(3, 1);
    GlStateManager::texParameteri(4, 1);
    GlStateManager::disableLighting();
    GlStateManager::disableCull();
    GlStateManager::disableBlend();
    GlStateManager::depthMask(true);
    GlStateManager::multiTexCoord2f(240.0f, 240.0f);
    GlStateManager::blendFuncSeparate(4, 1, 1, 0);

    double gameTime = (double)guardian->mLevel->getGameTime();
    double targetHeight = (double)target->getEyeHeight();
    GlStateManager::pushMatrix();
    GlStateManager::translatef((float)x, (float)(y + targetHeight), (float)z);

    Vec3* source = getPosition(target, (double)target->mBbHeight * 1.75, partialTick);
    Vec3* destination = getPosition(entity, targetHeight, partialTick);
    Vec3* direction = source->subtract(destination);
    double length = direction->length() + 1.875;
    direction->normalize();
    float degrees = 57.2957764f;
    GlStateManager::rotatef((float)((1.5707964 - std::atan2(direction->z, direction->x)) * degrees),
                            0.0f, 1.0f, 0.0f);
    GlStateManager::rotatef((float)(std::acos(direction->y) * degrees), 1.0f, 0.0f, 0.0f);

    float time = (float)(gameTime + partialTick) * -0.075f;
    builder->begin();
    int intensity = (int)(attackScale * attackScale * 191.0f);
    builder->color(intensity + 64, intensity + 32,
                   128 - (int)(attackScale * attackScale * 64.0f), 255);

    const float width = 0.4999f;
    const float v = (float)length * 2.5f - 1.0f;
    const float radius = 0.2f;
    float x0 = std::cos(time + 3.1415927f) * radius;
    float z0 = std::sin(time + 3.1415927f) * radius;
    float x1 = std::cos(time + 4.712389f) * radius;
    float z1 = std::sin(time + 4.712389f) * radius;
    float x2 = std::cos(time + 0.0f) * radius;
    float z2 = std::sin(time + 0.0f) * radius;
    float x3 = std::cos(time + 1.5707964f) * radius;
    float z3 = std::sin(time + 1.5707964f) * radius;
    builder->vertexUV(x0, (float)length, z0, 0.4999f, v);
    builder->vertexUV(x0, 0.0f, z0, 0.4999f, -1.0f);
    builder->vertexUV(x2, 0.0f, z2, -1.0f, -1.0f);
    builder->vertexUV(x2, (float)length, z2, -1.0f, v);
    builder->vertexUV(x1, (float)length, z1, 0.4999f, v);
    builder->vertexUV(x1, 0.0f, z1, 0.4999f, -1.0f);
    builder->vertexUV(x3, 0.0f, z3, -1.0f, -1.0f);
    builder->vertexUV(x3, (float)length, z3, -1.0f, v);
    if (!guardian->getSharedFlag(1))
    {
        const float guardianRadius = 0.282f;
        builder->vertexUV(std::cos(time) * guardianRadius, (float)length,
                          std::sin(time) * guardianRadius, 0.0f, v);
        builder->vertexUV(std::cos(time + 0.7853982f) * guardianRadius, (float)length,
                          std::sin(time + 0.7853982f) * guardianRadius, 0.0f, v);
        builder->vertexUV(std::cos(time + 5.4977875f) * guardianRadius, (float)length,
                          std::sin(time + 5.4977875f) * guardianRadius, 0.0f, -1.0f);
        builder->vertexUV(std::cos(time + 3.926991f) * guardianRadius, (float)length,
                          std::sin(time + 3.926991f) * guardianRadius, 0.0f, -1.0f);
    }
    builder->end();
    GlStateManager::popMatrix();
}

const ResourceLocation* GuardianRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return &GUARDIAN_LOCATION;
}

Model* GuardianRenderer::createModel(const ModelDefinition& definition)
{
    return new GuardianModel(definition);
}
