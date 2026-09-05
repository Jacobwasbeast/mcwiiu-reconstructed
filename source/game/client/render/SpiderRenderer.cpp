#include "game/client/render/SpiderRenderer.hpp"

#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/SpiderModel.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/generation/SpiderEyesLayer.hpp"

static ResourceLocation SPIDER_LOCATION;

SpiderRenderer::SpiderRenderer(EntityRenderDispatcher* dispatcher)
    : MobRenderer(dispatcher, new SpiderModel(ModelDefinition()), 1.0F)
{
    mSpiderEyesLayer = new SpiderEyesLayer(this);
    addLayer(mSpiderEyesLayer);
}

SpiderRenderer::~SpiderRenderer()
{
}

double SpiderRenderer::getFlipDegrees(boost::shared_ptr<LivingEntity>&)
{
    return 180.0;
}

const ResourceLocation* SpiderRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return &SPIDER_LOCATION;
}

void SpiderRenderer::setupForTexturePack(TexturePack* pack)
{
    LivingEntityRenderer::setupForTexturePack(pack);
    if (mModel->hasOverriddenParts())
    {
        removeLayer(mSpiderEyesLayer);
        return;
    }

    std::vector<RenderLayer*>::iterator layer = mLayers.begin();
    while (layer != mLayers.end())
    {
        if (*layer == mSpiderEyesLayer)
            break;
        ++layer;
    }
    if (layer == mLayers.end())
        addLayer(mSpiderEyesLayer);
}

void SpiderRenderer::renderModel(boost::shared_ptr<LivingEntity>& entity, float limbSwing,
                                 float limbSwingAmount, float ageInTicks, float netHeadYaw,
                                 float headPitch, float scale)
{
    bool translucent = false;
    if (!isVisible(entity))
    {
        boost::shared_ptr<Player> player = Minecraft::GetInstance()->mPlayer;
        if (entity->isInvisibleTo(player))
            return;
        translucent = true;
        GlStateManager::setProfile();
    }

    Model* model = getModel(entity);
    boost::shared_ptr<Entity> renderEntity = boost::static_pointer_cast<Entity>(entity);
    model->render(renderEntity, limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale,
                  translucent);

    if (translucent)
        GlStateManager::unsetProfile();
}

Model* SpiderRenderer::createModel(const ModelDefinition& definition)
{
    return new SpiderModel(definition);
}
