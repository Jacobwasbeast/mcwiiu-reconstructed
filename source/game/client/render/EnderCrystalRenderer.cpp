#include "game/client/render/EnderCrystalRenderer.hpp"

#include <cmath>

#include "game/client/render/EnderCrystalModel.hpp"
#include "game/client/render/EnderDragonRenderer.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/core/EnderCrystal.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation ENDER_CRYSTAL_LOCATION;

EnderCrystalRenderer::EnderCrystalRenderer(EntityRenderDispatcher* dispatcher)
    : EntityRenderer(dispatcher)
{
    mModelBase = new EnderCrystalModel(0.0F, true, ModelDefinition());
    mModelNoBase = new EnderCrystalModel(0.0F, false, ModelDefinition());
    mShadowRadius = 0.5F;
}

EnderCrystalRenderer::~EnderCrystalRenderer()
{
}

void EnderCrystalRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z,
                                  float yaw, float partialTick)
{
    boost::shared_ptr<EnderCrystal> crystal = boost::static_pointer_cast<EnderCrystal>(entity);
    float animation = (float)crystal->mTickCount;
    animation = (float)((double)animation + (double)partialTick);
    animation = std::sin(animation * 0.2F) * 0.5F + 0.5F;
    animation = animation * animation + animation;

    GlStateManager::pushMatrix();
    GlStateManager::translatef((float)x, (float)y, (float)z);
    bindTexture(&ENDER_CRYSTAL_LOCATION);
    if (mSolidRender)
    {
        GlStateManager::enableColorMaterial();
        getTeamColor(entity);
        GlStateManager::setupSolidRenderingTextureCombine(1);
    }

    if (crystal->showsBottom())
        mModelBase->render(entity, animation * 3.0F, animation * 0.2F, 0.0F, 0.0F, 0.0625F, mSolidRender, true);
    else
        mModelNoBase->render(entity, animation * 3.0F, animation * 0.2F, 0.0F, 0.0F, 0.0625F, mSolidRender, true);

    if (mSolidRender)
    {
        GlStateManager::tearDownSolidRenderingTextureCombine();
        GlStateManager::disableColorMaterial();
    }
    GlStateManager::popMatrix();

    BlockPos* beamTarget = crystal->getBeamTarget();
    if (beamTarget != NULL)
    {
        bindTexture(&ENDER_CRYSTAL_LOCATION);
        double targetX = (double)beamTarget->x + 0.5;
        double targetY = (double)beamTarget->y + 0.5;
        double targetZ = (double)beamTarget->z + 0.5;
        GlStateManager::multiTexCoord2f(0.0F, 0.0F);
        EnderDragonRenderer::renderCrystalBeams(
            x + targetX - crystal->mX, y - 0.3 + animation * 0.4f + targetY - crystal->mY,
            z + targetZ - crystal->mZ, partialTick, targetX, targetY, targetZ,
            crystal->mTickCount, crystal->mX, crystal->mY, crystal->mZ);
    }

    EntityRenderer::render(entity, x, y, z, yaw, partialTick);
}

const ResourceLocation* EnderCrystalRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return &ENDER_CRYSTAL_LOCATION;
}

bool EnderCrystalRenderer::shouldRender(boost::shared_ptr<Entity>& entity, Culler* culler,
                                        double x, double y, double z)
{
    if (EntityRenderer::shouldRender(entity, culler, x, y, z))
        return true;
    boost::shared_ptr<EnderCrystal> crystal = boost::static_pointer_cast<EnderCrystal>(entity);
    return crystal->getBeamTarget() != NULL;
}

bool EnderCrystalRenderer::shouldRender(boost::shared_ptr<Entity>& entity, Culler* culler, double distance)
{
    if (EntityRenderer::shouldRender(entity, culler, distance))
        return true;
    boost::shared_ptr<EnderCrystal> crystal = boost::static_pointer_cast<EnderCrystal>(entity);
    return crystal->getBeamTarget() != NULL;
}

Model* EnderCrystalRenderer::createModel(const ModelDefinition& definition)
{
    return new EnderCrystalModel(0.0F, true, definition);
}
