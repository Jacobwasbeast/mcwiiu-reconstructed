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
static ResourceLocation GUARDIAN_BEAM_LOCATION;

GuardianRenderer::GuardianRenderer(EntityRenderDispatcher* dispatcher)
    : MobRenderer(dispatcher, new GuardianModel(ModelDefinition()), 0.5f)
{
}

GuardianRenderer::~GuardianRenderer()
{
}

Vec3* GuardianRenderer::getPosition(boost::shared_ptr<Entity>& entity, double yOffset, float partialTicks)
{
    return Vec3::newTemp((entity->mX - entity->mXOld) * partialTicks + entity->mXOld,
                         (entity->mY - entity->mYOld) * partialTicks + entity->mYOld + yOffset,
                         (entity->mZ - entity->mZOld) * partialTicks + entity->mZOld);
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

    Vec3* targetPosition = getPosition(target, (double)target->mBbHeight * 0.5, 1.0f);
    Vec3* guardianPosition = getPosition(entity, guardian->getEyeHeight(), 1.0f);
    return culler->isVisible(AABB::minmax(guardianPosition->x, guardianPosition->y, guardianPosition->z,
                                           targetPosition->x, targetPosition->y, targetPosition->z));
}

void GuardianRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z,
                              float yaw, float partialTick)
{
    boost::shared_ptr<Guardian> guardian = boost::static_pointer_cast<Guardian>(entity);
    MobRenderer::render(entity, x, y, z, yaw, partialTick);

    boost::shared_ptr<Entity> target = guardian->getActiveAttackTarget();
    if (!target)
        return;

    double attackScale = guardian->getAttackAnimationScale(partialTick);
    Tesselator* tesselator = Tesselator::getInstance();
    BufferBuilder* builder = tesselator->getBuilder();
    bindTexture(&GUARDIAN_BEAM_LOCATION);
    GlStateManager::texParameteri(3, 1);
    GlStateManager::texParameteri(4, 1);
    GlStateManager::disableLighting();
    GlStateManager::disableCull();
    GlStateManager::disableBlend();
    GlStateManager::depthMask(true);
    GlStateManager::multiTexCoord2f(240.0f, 240.0f);
    GlStateManager::blendFuncSeparate(4, 1, 1, 0);

    float gameTime = (float)guardian->mLevel->getGameTime() + partialTick;
    double guardianEyeHeight = guardian->getEyeHeight();
    GlStateManager::pushMatrix();
    GlStateManager::translatef((float)x, (float)(y + guardianEyeHeight), (float)z);

    Vec3* source = getPosition(target, (double)target->mBbHeight * 0.5, partialTick);
    Vec3* destination = getPosition(entity, guardianEyeHeight, partialTick);
    Vec3* direction = source->subtract(destination);
    double length = direction->length() + 1.875;
    direction->normalize();
    float degrees = 57.2957764f;
    float xAngle = (float)std::acos(direction->y);
    float yAngle = (float)std::atan2(direction->z, direction->x);
    GlStateManager::rotatef((1.5707964f - yAngle) * degrees,
                            0.0f, 1.0f, 0.0f);
    GlStateManager::rotatef(xAngle * degrees, 1.0f, 0.0f, 0.0f);

    double time = (double)gameTime * 0.05 * -1.5;
    builder->begin();
    float attackScaleSquared = (float)(attackScale * attackScale);
    int intensity = (int)(attackScaleSquared * 191.0f);
    builder->color(intensity + 64, intensity + 32,
                   128 - (int)(attackScaleSquared * 64.0f), 255);

    const double guardianRadius = 0.282;
    double guardianX0 = std::cos(time + 2.35619455575943) * guardianRadius;
    double guardianZ0 = std::sin(time + 2.35619455575943) * guardianRadius;
    double guardianX1 = std::cos(time + 0.7853981852531433) * guardianRadius;
    double guardianZ1 = std::sin(time + 0.7853981852531433) * guardianRadius;
    double guardianX2 = std::cos(time + 3.9269909262657166) * guardianRadius;
    double guardianZ2 = std::sin(time + 3.9269909262657166) * guardianRadius;
    double guardianX3 = std::cos(time + 5.497787296772003) * guardianRadius;
    double guardianZ3 = std::sin(time + 5.497787296772003) * guardianRadius;

    const double radius = 0.2;
    double x0 = std::cos(time + 3.1415927410125732) * radius;
    double z0 = std::sin(time + 3.1415927410125732) * radius;
    double x2 = std::cos(time) * radius;
    double z2 = std::sin(time) * radius;
    double x1 = std::cos(time + 4.71238911151886) * radius;
    double z1 = std::sin(time + 4.71238911151886) * radius;
    double x3 = std::cos(time + 1.5707963705062866) * radius;
    double z3 = std::sin(time + 1.5707963705062866) * radius;
    const double v = length * 2.5 - 1.0;
    const float width = 0.4999f;
    builder->vertexUV(x0, (float)length, z0, width, v);
    builder->vertexUV(x0, 0.0f, z0, width, -1.0f);
    builder->vertexUV(x2, 0.0f, z2, -1.0f, -1.0f);
    builder->vertexUV(x2, (float)length, z2, -1.0f, v);
    builder->vertexUV(x1, (float)length, z1, width, v);
    builder->vertexUV(x1, 0.0f, z1, width, -1.0f);
    builder->vertexUV(x3, 0.0f, z3, -1.0f, -1.0f);
    builder->vertexUV(x3, (float)length, z3, -1.0f, v);
    float frameV = (guardian->mTickCount & 1) == 0 ? 0.5f : 0.0f;
    builder->vertexUV(guardianX0, (float)length, guardianZ0, 0.5f, frameV + 0.5f);
    builder->vertexUV(guardianX1, (float)length, guardianZ1, 1.0f, frameV + 0.5f);
    builder->vertexUV(guardianX3, (float)length, guardianZ3, 1.0f, frameV);
    builder->vertexUV(guardianX2, (float)length, guardianZ2, 0.5f, frameV);
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
