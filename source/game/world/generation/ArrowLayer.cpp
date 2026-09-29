#include "game/world/generation/ArrowLayer.hpp"

#include <cmath>

#include <boost/shared_ptr.hpp>

#include "game/client/render/EntityRenderer.hpp"
#include "game/client/render/EntityRenderDispatcher.hpp"
#include "game/client/render/LivingEntityRenderer.hpp"
#include "game/client/render/Model.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/core/Cube.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Lighting.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/projectile/TippableArrow.hpp"
#include "game/util/Random.hpp"

ArrowLayer::ArrowLayer(LivingEntityRenderer* renderer)
    : mRenderer(renderer)
{
}

ArrowLayer::~ArrowLayer()
{
}

void ArrowLayer::render(boost::shared_ptr<LivingEntity>& entity, float, float, float partialTicks, float, float, float, float scale, bool)
{
    int arrowCount = entity->getArrowCount();
    if (arrowCount < 1)
        return;

    boost::shared_ptr<Entity> arrow(new TippableArrow(entity->mLevel, entity->mX, entity->mY, entity->mZ));
    Random random(entity->getId());
    Lighting::turnOff();

    Model* model = mRenderer->getModel();
    for (int i = 0; i < arrowCount; ++i)
    {
        ModelPart* modelPart = model->getRandomModelPart(random);
        if (modelPart != NULL && !modelPart->mCubes.empty())
        {
            GlStateManager::pushMatrix();
            Cube* cube = modelPart->mCubes.at(random.nextInt(static_cast<int>(modelPart->mCubes.size())));
            modelPart->translateTo(0.0625f);

            float x = random.nextFloat();
            float y = random.nextFloat();
            float z = random.nextFloat();
            GlStateManager::translatef(
                (cube->mX1 - cube->mX0) * x * 0.0625f + cube->mX0 * 0.0625f,
                (cube->mY1 - cube->mY0) * y * 0.0625f + cube->mY0 * 0.0625f,
                (cube->mZ1 - cube->mZ0) * z * 0.0625f + cube->mZ0 * 0.0625f);

            float horizontalX = 1.0f - 2.0f * x;
            float horizontalZ = 1.0f - 2.0f * z;
            float horizontal = Mth::sqrt(horizontalX * horizontalX + horizontalZ * horizontalZ);
            arrow->mYRot = static_cast<float>(std::atan2(horizontalX, horizontalZ) * 57.2957763671875);
            arrow->mXRot = static_cast<float>(std::atan2(1.0f - 2.0f * y, horizontal) * 57.2957763671875);
            arrow->mYRotO = arrow->mYRot;
            arrow->mXRotO = arrow->mXRot;

            mRenderer->getDispatcher()->render(arrow, 0.0, 0.0, 0.0, 0.0f, partialTicks, false, false, true, false);
            GlStateManager::popMatrix();
        }
    }

    Lighting::turnOn();
}

bool ArrowLayer::colorsOnDamage()
{
    return 0;
}
